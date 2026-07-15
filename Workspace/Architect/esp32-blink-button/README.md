# ESP32-S3 Blink + Button 演示

ESP-IDF LED 彩灯闪烁示例，支持按钮切换颜色，适合给小朋友演示。

## 功能

- LED 自动闪烁，循环显示 10 种颜色
- 按下按钮可手动切换到下一种颜色
- 颜色亮度约 25%，不刺眼

## 颜色列表

| 索引 | 颜色 | RGB |
|------|------|-----|
| 0 | 红 | (64, 0, 0) |
| 1 | 绿 | (0, 64, 0) |
| 2 | 蓝 | (0, 0, 64) |
| 3 | 黄 | (64, 64, 0) |
| 4 | 青 | (0, 64, 64) |
| 5 | 紫 | (64, 0, 64) |
| 6 | 橙 | (64, 32, 0) |
| 7 | 粉 | (32, 64, 32) |
| 8 | 蓝绿 | (0, 32, 64) |
| 9 | 白 | (64, 64, 64) |

## 硬件接线

### LED（RGB 彩灯）

| ESP32-S3 | RGB LED |
|----------|---------|
| GPIO8 (或 menuconfig 配置) | DIN (数据输入) |
| 5V | VCC |
| GND | GND |

### 按钮

| ESP32-S3 | 按钮 |
|----------|------|
| GPIO19 | ---[按钮]--- GND |

**注意**：
- GPIO19 已内部上拉，无需外接上拉电阻
- 按钮另一端必须接 GND

## 编译命令

```bash
cd /Users/zhangyuchi/.espressif/v6.0.2/esp-idf/examples/get-started/blink

# 设置目标芯片
idf.py set-target esp32s3

# 配置（可选：修改 LED GPIO 和闪烁周期）
idf.py menuconfig
#  → Example Configuration → Blink GPIO number
#  → Example Configuration → Blink period in ms

# 编译
idf.py build

# 烧录
idf.py flash

# 查看串口输出
idf.py monitor
```

## 一键编译烧录

```bash
idf.py build flash monitor
```

## 退出串口监视器

```
Ctrl + ]
```

## 修改默认 GPIO

代码中默认配置：

- LED GPIO: `CONFIG_BLINK_GPIO`（menuconfig 配置，默认 GPIO8）
- 按钮 GPIO: `BUTTON_GPIO`（代码中硬编码为 19）

如需修改按钮 GPIO，编辑 `main/blink_example_main.c` 第 24 行：

```c
#define BUTTON_GPIO 19  // 改成你想要的 GPIO
```

## 文件结构

```
blink/
├── main/
│   ├── blink_example_main.c  # 主程序代码
│   └── Kconfig.projbuild    # menuconfig 配置项
├── CMakeLists.txt
└── README.md
```

## 原版说明

本项目基于 ESP-IDF 官方 blink 示例修改，添加了按钮控制功能。

原版示例请参考：[ESP-IDF Blink Example](https://github.com/espressif/esp-idf/tree/master/examples/get-started/blink)

## 许可证

本示例代码采用 CC0 公共领域许可证。
