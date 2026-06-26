# FreeRTOS 定时器接口说明

> 基于 FreeRTOS Kernel V11.1.0 / FreeRTOS-LTS 源码整理

---

## 一、概述

FreeRTOS 软件定时器由 **定时器服务任务（Daemon Task）** 统一管理，所有定时器操作通过**定时器命令队列**异步下发。因此定时器 API 均为非阻塞式，调用成功只表示命令已入队，实际执行由 Daemon Task 完成。

---

## 二、核心数据类型

| 类型 | 说明 |
|------|------|
| `TimerHandle_t` | 定时器句柄，通过 `xTimerCreate()` 返回 |
| `TimerCallbackFunction_t` | 回调函数类型，`void (*)(TimerHandle_t)` |
| `StaticTimer_t` | 静态定时器结构体（由用户提供内存） |
| `TickType_t` | 周期单位，通常为 uint32_t，以 tick 计数 |

---

## 三、创建与删除

### 3.1 动态创建（推荐）

```c
TimerHandle_t xTimerCreate(
    const char * const pcTimerName,       // 定时器名称（仅用于调试）
    const TickType_t xTimerPeriodInTicks, // 周期（tick），必须 > 0
    const BaseType_t xAutoReload,         // pdTRUE=自动重载  pdFALSE=单次
    void * const pvTimerID,               // 用户自定义参数指针（回调中可获取）
    TimerCallbackFunction_t pxCallbackFunction // 超时时调用的回调
);
```

- `xAutoReload = pdTRUE`：每次超时后**自动重新加载**周期，无需在回调中手动重启
- `xAutoReload = pdFALSE`：单次定时器，超时后进入休眠，需手动调用 `xTimerStart()` 重启

### 3.2 静态创建（无需堆内存）

```c
TimerHandle_t xTimerCreateStatic(
    const char * const pcTimerName,
    const TickType_t xTimerPeriodInTicks,
    const BaseType_t xAutoReload,
    void * const pvTimerID,
    TimerCallbackFunction_t pxCallbackFunction,
    StaticTimer_t * pxTimerBuffer         // 用户提供的 StaticTimer_t 结构
);
```

### 3.3 删除

```c
BaseType_t xTimerDelete(TimerHandle_t xTimer, TickType_t xTicksToWait);
```

---

## 四、运行控制

| 函数 | 说明 |
|------|------|
| `xTimerStart(xTimer, ticks)` | 启动 / 重启定时器 |
| `xTimerStop(xTimer, ticks)` | 停止定时器 |
| `xTimerReset(xTimer, ticks)` | 重置（重新开始计时） |
| `xTimerChangePeriod(xTimer, period, ticks)` | **修改周期并自动重启** |

> 注意：`xTimerChangePeriod()` 是满足"修改周期 + 继续运行"需求的核心接口。

### 中断安全版本（FromISR）

| 函数 |
|------|
| `xTimerStartFromISR()` |
| `xTimerStopFromISR()` |
| `xTimerResetFromISR()` |
| `xTimerChangePeriodFromISR()` |

---

## 五、查询与配置

| 函数 | 说明 |
|------|------|
| `pvTimerGetTimerID(xTimer)` | 获取定时器 ID（void *） |
| `vTimerSetTimerID(xTimer, newID)` | 设置定时器 ID |
| `xTimerIsTimerActive(xTimer)` | 查询是否在运行（pdTRUE=运行中） |
| `xTimerGetTimerDaemonTaskHandle()` | 获取 Daemon Task 句柄 |

---

## 六、典型应用模式

### 模式 A：自动重载定时器（固定周期）

```c
xTimer = xTimerCreate("AutoReload", 1000, pdTRUE, NULL, vCallback);
xTimerStart(xTimer, 0);
// 无需在回调中做任何操作，超时自动重载
```

### 模式 B：单次定时器，手动控制是否继续（推荐）

适用于**每次超时后需要动态决定周期和是否继续**的场景：

```c
typedef struct {
    TickType_t current_period;   // 当前周期
    uint8_t    run_count;        // 已运行次数
    uint8_t    max_runs;         // 最大次数，0=无限
} MyCtx_t;

static MyCtx_t ctx = { .current_period = 1000, .run_count = 0, .max_runs = 0 };

static void vCallback(TimerHandle_t xTimer)
{
    MyCtx_t *p = (MyCtx_t *)pvTimerGetTimerID(xTimer);
    p->run_count++;

    // 超过最大次数则停止
    if (p->max_runs > 0 && p->run_count >= p->max_runs) {
        xTimerStop(xTimer, 0);
        return;
    }

    // 动态修改周期（示例：偶数次翻倍）
    if (p->run_count % 2 == 0) {
        p->current_period *= 2;
    } else {
        p->current_period = 1000;
    }

    // 修改周期并自动重启（核心接口）
    xTimerChangePeriod(xTimer, p->current_period, 0);
}

void vInit(void)
{
    xTimer = xTimerCreate("MyTimer", 1000, pdFALSE, &ctx, vCallback);
    xTimerStart(xTimer, 0);
}
```

---

## 七、注意事项

1. **回调中不能阻塞**：定时器回调运行在 Daemon Task 上下文，不能调用阻塞 API，`xTicksToWait` 必须传 `0`。
2. **命令队列可能满**：所有 Timer API 都有 `xTicksToWait` 参数，队列满时任务会阻塞，建议用 `0` 避免回调中阻塞。
3. **`xTimerChangePeriod()` 会自动重启**：如果定时器正在运行，调用此函数会**重置计时器并用新周期开始**，无需额外调用 `xTimerStart()`。
4. **定时器精度**：软件定时器依赖系统 tick，精度受 `configTICK_RATE_HZ` 影响，最小周期为 1 tick。

---

## 八、相关配置常量

| 常量 | 默认值 | 说明 |
|------|--------|------|
| `configUSE_TIMERS` | 1 | 开启软件定时器功能 |
| `configTIMER_TASK_PRIORITY` | 2 | Daemon Task 优先级 |
| `configTIMER_QUEUE_LENGTH` | 10 | 命令队列长度 |
| `configTIMER_TASK_STACK_DEPTH` | configMINIMAL_STACK_SIZE | Daemon Task 栈大小 |