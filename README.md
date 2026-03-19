# STC32G BLDC 电机控制器

基于 STC32G12K128 单片机的无传感器无刷直流（BLDC）电机控制器，采用 BEMF（反电动势）过零检测技术实现闭环控制。

## 硬件配置

| 项目 | 参数 |
|------|------|
| MCU | STC32G12K128-LQFP48 |
| 系统时钟 | 33.1776 MHz |
| PWM频率 | ~44 kHz（最大占空比750） |
| 供电电压 | 3S锂电（12.6V满电） |
| 最低工作电压 | 10V |

## 项目结构

```
SEEKFREE_STC32G_BLDC/
├── bldc/                          # 核心控制模块
│   ├── motor.c / motor.h          # 电机控制逻辑、换相、启动序列
│   ├── pwm.c / pwm.h             # 6路PWM生成（3相半桥）
│   ├── comparator.c / comparator.h # 内部比较器（BEMF检测）
│   ├── pwm_input.c / pwm_input.h  # PWM输入捕获（油门信号）
│   ├── battery.c / battery.h      # 电池电压监测
│   ├── pit_timer.c / pit_timer.h  # 周期定时器和LED状态
│   └── bldc_config.h              # 配置文件
├── Libraries/
│   ├── libraries/                 # STC SDK基础库
│   ├── seekfree_libraries/        # SeekFree HAL库
│   └── seekfree_peripheral/       # 外设驱动
└── Project/
    ├── MDK/                       # Keil工程文件
    └── USER/                       # 用户代码
```

## 模块说明

### 1. 电机控制模块 (motor.c)

**三段式启动控制：**
- **预定位阶段** (run_flag=1): 强制转子对齐到已知位置，300ms
- **开环加速阶段** (run_flag=2): 100步强制换相，支持线性/指数/S曲线加速
- **过渡阶段** (run_flag=3): 检测过零信号质量，准备切入闭环
- **闭环运行阶段** (run_flag=4): BEMF过零检测换相

**关键特性：**
- 堵转检测（换相超时 + 换相失败计数）
- 自动重启（500ms延迟）
- 消磁延时保护

### 2. PWM模块 (pwm.c)

**硬件配置：**
- 使用STC32G的PWMA模块
- 6路PWM输出（3相半桥）
- 中心对齐模式

**输出引脚：**
| 引脚 | 功能 |
|------|------|
| P20 | A相上桥 (H) |
| P21 | A相下桥 (L) |
| P22 | B相上桥 (H) |
| P23 | B相下桥 (L) |
| P24 | C相上桥 (H) |
| P25 | C相下桥 (L) |

**死区时间：** ~500ns（18个NOP）

### 3. 比较器模块 (comparator.c)

**BEMF检测配置：**
| 引脚 | 功能 |
|------|------|
| P3_6 | 虚拟中性点（参考电压） |
| P3_7 | A相 BEMF检测 |
| P5_1 | B相 BEMF检测 |
| P5_0 | C相 BEMF检测 |

**功能：**
- 内部比较器检测BEMF过零
- 可配置上升沿/下降沿触发
- 滤波时间：63个时钟周期

### 4. PWM输入模块 (pwm_input.c)

**输入捕获配置：**
- 输入引脚：P00（使用PWMB）

**输入信号规格：**
- 频率：30-400 Hz（有效范围50-300Hz）
- 脉宽：1ms（停止）到2ms（满油门）
- 转换公式：`throttle = (high_time - 1000)`

### 5. 电池监测模块 (battery.c)

- ADC检测电池电压（P03引脚）
- 低电压保护：<10V停止
- 消抖处理（100次检测确认）

### 6. 周期定时器模块 (pit_timer.c)

- 10ms周期定时器（TM1）
- LED状态指示
- 启动/停止控制

## 中断优先级

| 中断源 | 中断号 | 优先级 | 功能 |
|--------|--------|--------|------|
| PWMA | 26 | 最高 | PWM更新中断（BEMF采样） |
| Comparator | 21 | 高 | BEMF过零检测 |
| TM4 | 20 | 高 | 换相超时检测（堵转） |
| TM3 | 19 | 正常 | 开环换相定时 |
| TM1 | 3 | 正常 | 10ms周期定时（状态机） |
| TM0 | 1 | 正常 | 消磁延时定时 |
| PWMB | 27 | 正常 | PWM输入捕获 |

## 配置文件 (bldc_config.h)

```c
// PWM参数
#define BLDC_MAX_DUTY              750     // 最大占空比
#define BLDC_MIN_DUTY              50      // 最小占空比

// 启动参数
#define BLDC_START_VOLTAGE         1.0f    // 启动电压因子
#define BLDC_MIN_BATTERY          10.0f   // 最低电池电压(V)

// 三段式启动参数
#define START_ALIGN_DUTY_RATIO     0.5f    // 预定位占空比
#define START_ALIGN_TIME_MS        300     // 预定位时间(ms)
#define START_OPEN_LOOP_MIN_DELAY  200     // 最小换相延迟(us)
#define START_OPEN_LOOP_MAX_DELAY  3500    // 最大换相延迟(us)
#define START_OPEN_LOOP_STEP_NUM   100     // 开环步数
#define START_OPEN_LOOP_ACCEL_CURVE 2      // 加速曲线(0=线性,1=指数,2=S曲线)
```

## LED状态指示

| 运行状态 | LED模式 |
|----------|---------|
| 低电压 (<10V) | 亮0.1s / 灭0.9s |
| 堵转 | 亮0.05s / 灭0.05s |
| 停止 (run_flag=0) | 亮1s / 灭1s |
| 预定位 (run_flag=1) | 亮0.5s / 灭0.5s |
| 开环加速 (run_flag=2) | 快闪 0.05s间隔 |
| 过渡阶段 (run_flag=3) | 常亮 |
| 闭环运行 (run_flag=4) | 常亮 |

## 使用方法

1. **初始化**：按顺序初始化电池检测、LED、PWM输入、比较器、电机
2. **启动电机**：当油门 > 最小占空比且电池电压正常时，自动启动
3. **停止电机**：油门设为0或电池低压时自动停止
4. **调速**：通过PWM输入信号（1ms-2ms）控制电机速度

## 注意事项

- PWM初始化**务必**放在电机初始化函数**之后**，否则会烧毁电机
- 开环阶段占空比逐步增加，避免电流冲击
- 消磁延时等待反电动势消失后再检测过零
