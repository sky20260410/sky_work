# ARMv8-M 架构 BASEPRI 寄存器中文翻译

## 来源文档
Introduction to the Armv8-M Architecture and its Programmers Model
版本 1.1，用户指南，非保密文档

## BASEPRI 寄存器详解

### 寄存器概述
BASEPRI 是一个 8 位的基础优先级屏蔽寄存器，用于根据优先级级别来屏蔽异常和中断。

### 主要特性
- **宽度**: 8 位寄存器
- **功能**: 基于优先级的中断屏蔽
- **复位值**: 0（禁用状态）

### 工作原理
当 BASEPRI 设置为非零值时：
- 屏蔽相同或更低优先级（数值更大）的异常和中断
- 允许更高优先级（数值更小）的异常和中断

当 BASEPRI 设置为 0 时：
- 禁用屏蔽功能，所有中断都可正常响应

### 汇编指令访问
由于 BASEPRI 是特殊功能寄存器，必须使用 MRS 和 MSR 指令访问：

```assembly
MRS R0, BASEPRI        ; 读取 BASEPRI 寄存器值
MSR BASEPRI, R2        ; 将 R2 值写入 BASEPRI 寄存器
MSR BASEPRI_MAX, R3    ; 仅当 R3 值比当前 BASEPRI 更高优先级时才写入
```

### BASEPRI_MAX 限定版本
MSR 指令还可以用于 BASEPRI 的限定版本 BASEPRI_MAX：
- 仅当新值比当前 BASEPRI 设置更高优先级时才更新寄存器
- 如果当前 BASEPRI 已经是更高优先级，则寄存器保持不变

**示例**: 
- 如果 BASEPRI 当前配置为优先级值 16
- 向 BASEPRI_MAX 写入优先级值 12，则执行优先级将提升到新值 12
- 向 BASEPRI_MAX 写入优先级值 20，则 BASEPRI 保持为 16 不变

### CMSIS 函数接口
```c
__get_BASEPRI(void)              // 读取 BASEPRI 寄存器
__set_BASEPRI(uint32_t basePri)  // 设置 BASEPRI 新值
__set_BASEPRI_MAX(uint32_t basePri)  // 仅当新值提升优先级时设置
```

### 架构可用性
- 在 ARMv8-M 主线架构（Mainline）中可用
- 在 ARMv8-M 基础架构（Baseline，如 Cortex-M23）中不包含
- 仅能在特权模式下修改

### TrustZone 支持
在 Cortex-M33 等支持 TrustZone 的处理器中：
- BASEPRI_S：安全状态使用
- BASEPRI_NS：非安全状态使用

### 优先级编号规则
- 数值越小，优先级越高（0 为最高优先级）
- 实际实现的位数取决于 NVIC_PRIO_BITS 配置
- 未实现的低位读取为 0，写入被忽略