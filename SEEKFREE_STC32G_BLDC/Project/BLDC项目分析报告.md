# STC32G BLDC 电机控制项目分析报告

## 概述

这是一个功能完整的**无传感器BLDC（无刷直流）电机控制器**，基于STC32G12K128单片机，使用BEMF（反电动势）过零检测技术实现闭环控制。项目由逐飞科技（SeekFree）开发，适用于智能车竞赛。

---

## 1. 项目结构

```
SEEKFREE_STC32G_BLDC/
├── bldc/                    # 核心控制模块
│   ├── motor.c/h           # 电机控制逻辑、换相、启动序列
│   ├── pwm.c/h             # 6路PWM生成（3相半桥）
│   ├── comparator.c/h     # 内部比较器（BEMF检测）
│   ├── pwm_input.c/h       # PWM输入捕获（油门信号）
│   ├── battery.c/h         # 电池电压监测
│   ├── pit_timer.c/h       # 周期定时器和LED状态
│   └── bldc_config.h      # 配置文件
├── Libraries/
│   ├── libraries/         # STC SDK基础库
│   ├── seekfree_libraries/ # SeekFree HAL库
│   └── seekfree_peripheral/# 外设驱动
└── Project/
    ├── MDK/                # Keil工程文件
    └── USER/               # 用户代码
```

---

## 2. 核心模块

### 2.1 Motor模块 (bldc/motor.c)

**主要功能：**
- 三段式启动控制（预定位 → 开环 → 闭环）
- 电机换相逻辑
- 换相时间统计和堵转检测

**关键数据结构 `motor_struct`：**
```c
typedef struct {
    uint8   step;              // 当前步数(0-5)
    uint16  duty;              // 目标PWM占空比
    uint16  duty_register;    // 实际寄存器占空比
    uint8   run_flag;          // 运行状态标志
    uint8   start_phase;       // 启动阶段(1-4)
    uint16  commutation_time[6]; // 最近6次换相时间
    // ...其他参数
} motor_struct;
```

**核心函数：**
- `motor_start()` - 启动电机（三段式启动）
- `motor_stop()` - 停止电机
- `motor_commutation()` - 换相控制
- `calculate_acceleration_curve()` - 计算加速曲线（线性/指数/S曲线）

### 2.2 PWM模块 (bldc/pwm.c)

**硬件配置：**
- 使用STC32G的PWMA模块
- 6路PWM输出（3相半桥）
- 中心对齐模式

**PWM输出引脚定义：**
| 引脚 | 功能 |
|------|------|
| P20 | A相上桥 (H) |
| P21 | A相下桥 (L) |
| P22 | B相上桥 (H) |
| P23 | B相下桥 (L) |
| P24 | C相上桥 (H) |
| P25 | C相下桥 (L) |

**死区时间：** ~500ns（通过18个NOP实现）

**换相输出函数：**
- `pwm_a_bn_output()` - A上B下
- `pwm_a_cn_output()` - A上C下
- `pwm_b_cn_output()` - B上C下
- `pwm_b_an_output()` - B上A下
- `pwm_c_an_output()` - C上A下
- `pwm_c_bn_output()` - C上B下

### 2.3 Comparator模块 (bldc/comparator.c)

**BEMF检测配置：**
| 引脚 | 功能 |
|------|------|
| P3_6 | 虚拟中性点 (参考电压) |
| P3_7 | A相 BEMF检测 |
| P5_1 | B相 BEMF检测 |
| P5_0 | C相 BEMF检测 |

**功能：**
- 内部比较器用于检测BEMF过零
- 可配置上升沿/下降沿触发
- 滤波时间：63个时钟周期

### 2.4 PWM输入模块 (bldc/pwm_input.c)

**输入捕获配置：**
- 输入引脚：P00（使用PWMB）

**输入信号规格：**
- 频率：30-400 Hz（有效范围50-300Hz）
- 脉宽：1ms（停止）到2ms（满油门）
- 转换公式：`throttle = (high_time - 1000)`

---

## 3. 硬件资源配置

| 功能 | 引脚 | 说明 |
|------|------|------|
| PWM输出 | P20-P25 | 3相6路PWM |
| BEMF检测 | P3_6, P3_7, P5_0, P5_1 | 比较器输入 |
| PWM输入 | P00 | PWMB捕获 |
| 电池电压 | P03 | ADC检测 |
| LED状态 | P43 | 状态指示 |
| 系统时钟 | 33.1776MHz | - |

---

## 4. 中断系统和优先级

| 中断源 | 中断号 | 优先级 | 功能 |
|--------|--------|--------|------|
| PWMA | 26 | **最高** | PWM更新中断（BEMF采样） |
| Comparator | 21 | 高 | BEMF过零检测 |
| TM4 | 20 | 高 | 换相超时检测（堵转） |
| TM3 | 19 | 正常 | 开环换相定时 |
| TM1 | 3 | 正常 | 10ms周期定时（状态机） |
| TM0 | 1 | 正常 | 消磁延时定时 |
| PWMB | 27 | 正常 | PWM输入捕获 |

**优先级配置（motor_init中）：**
```c
// 比较器和PWM设为最高优先级
IP |= 1<<7;    // PWMA最高
IPH |= 1<<7;
IP2 |= 1<<5;   // Comparator高
IP2H |= 1<<5;
```

---

## 5. 电机控制状态机

**run_flag状态定义：**
| 状态值 | 名称 | 说明 |
|--------|------|------|
| 0 | 停止 | 电机停止 |
| 1 | 预定位阶段 | Alignment - 300ms |
| 2 | 开环加速阶段 | Open-loop - 强制换相 |
| 3 | 过渡阶段 | Transition - 准备切入闭环 |
| 4 | 闭环运行阶段 | Closed-loop - BEMF换相 |

---

## 6. 三段式启动流程

### 6.1 预定位阶段 (run_flag=1)
- **持续时间：** 300ms (`START_ALIGN_TIME_MS`)
- **占空比：** 50%基础占空比 (`START_ALIGN_DUTY_RATIO=0.5`)
- **作用：** 强制转子对齐到已知位置

### 6.2 开环加速阶段 (run_flag=2)
- **步数：** 100步 (`START_OPEN_LOOP_STEP_NUM`)
- **换相延迟：** 从3500us递减到200us
- **支持三种加速曲线：**
  - 0 = 线性加速
  - 1 = 指数加速（先慢后快）
  - 2 = S曲线加速（Sigmoid函数）
- **占空比斜坡上升**

### 6.3 过渡阶段 (run_flag=3)
- **检测过零信号质量**
- 需要连续6次有效过零 (`START_SWITCH_ZC_NUM=6`)
- 过零误差<30%认为有效

### 6.4 闭环运行 (run_flag=4)
- **关闭PWM中断，开启比较器中断**
- 使用BEMF过零检测换相

---

## 7. 安全特性

### 7.1 低电压保护
- 低于10V停止 (`BLDC_MIN_BATTERY=10.0V`)
- 有消抖处理（100次检测确认）

### 7.2 堵转检测
- **换相超时：** Timer4定时器检测
- **统计检测：** 连续换相失败次数过多（150次）判定堵转

### 7.3 自动重启
- 堵转后延迟500ms重启 (`BLDC_START_DELAY=50*10ms`)

### 7.4 消磁延时
- 换相后延时等待反电动势消失
- 使用Timer0实现，防止误检测

---

## 8. 关键配置文件 (bldc_config.h)

```c
// ==================== PWM参数 ====================
#define BLDC_MAX_DUTY                   750     // 最大占空比
#define BLDC_MIN_DUTY                   50      // 最小占空比

// ==================== 启动参数 ====================
#define BLDC_START_VOLTAGE              1.0f    // 启动电压因子(0-1.5V)
#define BLDC_MIN_BATTERY                10.0f  // 最低电池电压(V)

// ==================== 安全参数 ====================
#define BLDC_COMMUTATION_FAILED_MAX     150     // 最大换相失败次数
#define BLDC_START_DELAY                50      // 重启延迟(10ms)
#define BLDC_START_WAIT_TIME            15      // 启动等待时间(10ms)

// ==================== 三段式启动参数 ====================
// 预定位阶段
#define START_ALIGN_DUTY_RATIO          0.5f    // 预定位占空比
#define START_ALIGN_TIME_MS             300     // 预定位时间(ms)

// 开环加速阶段
#define START_OPEN_LOOP_MIN_DELAY       200     // 最小换相延迟(us)
#define START_OPEN_LOOP_MAX_DELAY       3500    // 最大换相延迟(us)
#define START_OPEN_LOOP_STEP_NUM        100     // 开环步数
#define START_OPEN_LOOP_ACCEL_CURVE     2       // 加速曲线(0=线性,1=指数,2=S曲线)

// 过渡阶段
#define START_SWITCH_ZC_NUM             6       // 切入闭环所需过零次数
#define START_SWITCH_ZC_MAX_ERR         30      // 过零误差容忍度(%)
```

---

## 9. LED状态指示

| 运行状态 | LED模式 |
|----------|---------|
| 低电压 (<10V) | 亮0.1s / 灭0.9s |
| 堵转 | 亮0.05s / 灭0.05s |
| 停止 (run_flag=0) | 亮1s / 灭1s |
| 预定位 (run_flag=1) | 亮0.5s / 灭0.5s |
| 开环加速 (run_flag=2) | 快闪 0.05s间隔 |
| 过渡阶段 (run_flag=3) | 常亮 |
| 闭环运行 (run_flag=4) | 常亮 |

---

## 10. 代码质量评估

### ✅ 正确实现的部分

1. **状态机逻辑** - 正确实现三段式启动流程
2. **比较器通道选择** - 在pwm.c的pwm_xxx_output()函数中正确切换
3. **PWM/比较器中断切换** - 正确从PWMA中断切换到比较器中断
4. **堵转检测** - 有双重保护（Timer4超时 + 换相失败计数）
5. **低电压保护** - 有消抖处理（100次检测确认）
6. **PWM输入捕获** - 频率和脉宽检查完整
7. **死区时间** - 约500ns防止上下桥直通

### ⚠️ 可优化点（非问题）

1. **tim3_reconfig函数**：当延时值较大时(>26ms)，计算可能溢出，但实际使用中延时值较小，不会触发
2. **启动参数**：可根据实际电机特性调整`bldc_config.h`中的启动参数

---

## 11. 总结

这是一个**工业级无传感器BLDC电机控制器**，具有：

- ✅ 完整的启动序列（预定位→开环→闭环）
- ✅ 完善的保护机制（低压、堵转检测）
- ✅ 高效的BEMF检测方案
- ✅ 清晰的中断优先级设计
- ✅ 模块化的代码结构

项目代码质量良好，没有发现明显bug，可直接使用或根据具体电机特性调整启动参数。

---

## 附录：Build信息

- **IDE：** Keil MDK for C251
- **目标MCU：** STC32G12K128-LQFP48
- **系统时钟：** 33.1776 MHz
- **输出文件：** `Project/MDK/Out_File/`
