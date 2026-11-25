# 2024 年浙江省 TI 杯电子设计竞赛资料

![Platform](https://img.shields.io/badge/Platform-TI%20MSP432E%20%7C%20C2000-c80000?style=flat-square)
![Language](https://img.shields.io/badge/Language-C-1769aa?style=flat-square)
![Status](https://img.shields.io/badge/Status-Archived%20work-informational?style=flat-square)

本仓库整理 2024 年浙江省 TI 杯电子设计竞赛相关的程序和参考资料，包括 MSP432E401Y 采样与信号处理工程、MSP432E 外设学习示例、F28027 电源控制源码，以及器件手册、板级库和历史工程压缩包。

> 这是保留原始结构的资料快照，包含多个独立示例和参考工程。各目录之间的硬件连接、功能和版本关系尚未完整确认，请结合实物及原始工程核对。

## 内容概览

| 内容 | 说明 |
| --- | --- |
| MSP432E 采样工程 | ADC、DMA、采样通道切换、频率和 THD 计算代码，以及 LCD、串口相关代码 |
| MSP432E 学习示例 | GPIO、延时、UART、定时器、ADC，以及 FFT、I²C、SPI 等示例资料 |
| 往年题目参考 | 电感电容测量装置及放大器非线性失真研究装置相关资料和工程 |
| F28027 控制源码 | 三相 AC-DC 与 Buck DC-DC 两份独立 C 文件，包含 ADC、ePWM 和 PI 控制相关代码 |
| 硬件与参考资料 | Altium 原理图库、PCB 库，TI 文档、教学图片、器件支持包和历史工程压缩包 |

## 资料结构

```text
.
├── README.md
└── 2024TI杯/
    ├── MSP432/                         # MSP432E401Y 应用工程及驱动
    │   ├── ARM/BlinkLED.uvprojx         # Keil 工程入口
    │   ├── main.c                      # 采样与信号处理主程序
    │   ├── User/                       # ADC、LCD、按键代码
    │   └── ti/                         # 随工程保存的 TI 驱动及库
    ├── MSP432E/MSP432E/1程序源码/       # 独立外设示例及往年题目参考
    ├── f28027_ac_dc.c                  # 三相 AC-DC 控制源码
    ├── f28027_dc_dc.c                  # Buck DC-DC 控制源码
    ├── MSP432DIBAN.SchLib              # Altium 原理图库
    ├── MSP432diban.PcbLib              # Altium PCB 库
    ├── Cloudczk_文档/                  # TI 手册和参考文档
    ├── Cloudczk_教学图片/              # 教学图片
    ├── Cloudczk_PACK/                  # MSP432E4 器件支持包
    └── *.zip                           # 历史工程压缩包
```

## 主要工程

### MSP432E401Y 采样与信号处理

- [Keil 工程入口](./2024TI杯/MSP432/ARM/BlinkLED.uvprojx)
- [主程序](./2024TI杯/MSP432/main.c)
- [ADC 配置](./2024TI杯/MSP432/User/ADC/bsp_ADC.c)
- [LCD 驱动](./2024TI杯/MSP432/User/LCD/lcd.c)
- [按键代码](./2024TI杯/MSP432/User/Key/key.c)

工程目标器件为 **MSP432E401Y**。主程序通过 ADC 和 DMA 采集 1024 个样本，包含输入通道切换、采样频率调整及谐波、THD 计算逻辑，也包含 LCD 初始化和串口发送代码。

**当前限制：** 主程序中的 CMSIS FFT 初始化、FFT 运算及复数幅值计算调用处于注释状态，因此不能将当前快照视为已经完成并验证的频谱或 THD 测量实现。采样频率、量程、校准和测量精度均需重新检查。

### 外设与往年题目示例

示例位于 [MSP432E/MSP432E/1程序源码](./2024TI杯/MSP432E/MSP432E/1程序源码/)，每个目录保留自己的源码与依赖。其中可直接定位的 Keil 工程包括：

- [GPIO 示例](./2024TI杯/MSP432E/MSP432E/1程序源码/2GPIO/msp432_proj.uvprojx)
- [UART 发送示例](<./2024TI杯/MSP432E/MSP432E/1程序源码/4UART/Send/msp432_proj.uvprojx>)和 [接收中断示例](<./2024TI杯/MSP432E/MSP432E/1程序源码/4UART/Receive%20interrupt/msp432_proj.uvprojx>)
- [定时器示例](./2024TI杯/MSP432E/MSP432E/1程序源码/5TIM/msp432_proj.uvprojx)
- [ADC 2M 示例](./2024TI杯/MSP432E/MSP432E/1程序源码/6AD/2M/msp432_proj.uvprojx)和 [交流电压示例](<./2024TI杯/MSP432E/MSP432E/1程序源码/6AD/AC%20voltage/msp432_proj.uvprojx>)
- [电感电容测量参考工程](<./2024TI杯/MSP432E/MSP432E/1程序源码/10电赛真题/CDLQ(23C题)/msp432_proj.uvprojx>)
- [非线性失真研究参考工程](<./2024TI杯/MSP432E/MSP432E/1程序源码/10电赛真题/THD(20年E题)/msp432_proj.uvprojx>)

目录还包含 FFT、I²C、SPI 和其他 ADC 示例资料；各示例的工程文件与完整性应逐项核查。

### F28027 电源控制代码

- [f28027_ac_dc.c](./2024TI杯/f28027_ac_dc.c)：文件注释描述为带关断控制的三相 AC-DC 单周期控制，代码包含 ADC、ePWM、中断和 PI 运算。
- [f28027_dc_dc.c](./2024TI杯/f28027_dc_dc.c)：文件注释描述为 Buck 电压调节，代码包含 ADC 采样、ePWM 和 PI 运算。

这两份文件依赖 `DSP28x_Project.h` 等 TI C2000 工程支持文件。当前资料中没有与其对应的完整 CCS 工程和硬件原理图；使用前需确认目标芯片、库、链接脚本、外设配置和电路拓扑。

## 环境与使用

| 用途 | 需要的环境 |
| --- | --- |
| MSP432E 工程 | Keil MDK / µVision、MSP432E4 器件支持包、匹配的 TI DriverLib 和调试器 |
| F28027 控制代码 | TI Code Composer Studio、匹配的 C2000 器件支持文件、编译器和调试器 |
| 电路图库 | 可读取 `.SchLib` 和 `.PcbLib` 的 Altium Designer |

### 打开 MSP432E 工程

1. 安装 Keil MDK 和匹配的 MSP432E4 器件支持包。仓库包含 [TexasInstruments.MSP432E4_DFP.3.2.6.pack](./2024TI杯/Cloudczk_PACK/TexasInstruments.MSP432E4_DFP.3.2.6.pack)。
2. 打开 [BlinkLED.uvprojx](./2024TI杯/MSP432/ARM/BlinkLED.uvprojx)，确认目标器件为 `MSP432E401Y`。
3. 检查工程中的驱动库、包含路径及编译器设置，再执行 Build。烧录前核对板卡、供电、ADC 输入电平和外设接线。

参考示例也可分别打开各自的 `msp432_proj.uvprojx`。项目目录中原有的 `Objects`、`Listings` 等编译中间文件未纳入仓库，应在本机重新构建。

### 安装包说明

原始目录中的 `simplelink_msp432e4_sdk_4_20_00_12.exe` 和 `ti_emupack_setup_9.2.0.00002_win_64.exe` 是体积较大的第三方安装包，未上传。需要时请从 [TI MSP432E4 SDK 下载页面](https://www.ti.com/tool/SIMPLELINK-MSP432E4-SDK)和 [TI 调试探针软件下载页面](https://www.ti.com/tool/EMUPACK)获取相应版本，并以 TI 官方版本说明为准。

## 状态与待验证事项

- 本仓库为竞赛相关资料及开发快照，未提供可核验的比赛成绩、最终题目说明或完整实测数据。
- MSP432E 工程的 FFT 相关调用当前被注释，采样与 THD 流程需要补全并通过已知信号测试。
- F28027 文件的完整工程、硬件连接、控制参数和保护逻辑需要确认；文件名和代码注释本身不足以证明电源系统已联调完成。
- 物理电路、Altium 库、历史压缩包与当前源码之间的版本对应关系尚待核对。
- 此次整理未重新编译固件，也未进行烧录或硬件测试。

## 第三方资料与许可

目录包含 TI 器件支持文件、第三方软件组件及参考文档。使用和再分发时请遵循各文件或组件随附的许可及版权说明。项目自身尚未指定统一的开源许可证；历史压缩包和第三方文档的授权范围也需要分别确认。
