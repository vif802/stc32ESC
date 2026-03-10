
#ifndef _motor_h_
#define _motor_h_

#include "common.h"


typedef struct
{
    uint8   step;                   // 电机运行的步数
    uint16  duty;                   // PWM占空比 用户改变电机速度时修改此变量
    uint16  duty_register;          // PWM占空比寄存器 用户不可操作
    vuint8  run_flag;               // 电机正在运行标志位
                                    // 0:已停止
                                    // 1：预定位阶段(Alignment)
                                    // 2：开环加速阶段(Open-loop)
                                    // 3：切入闭环过渡阶段
                                    // 4：启动完成正在运行(Closed-loop)
    uint8   degauss_flag;           // 0：消磁完成 2：换相完成，正在延时消磁
    float   motor_start_delay;      // 开环启动的换相的延时时间
    uint16  motor_start_wait;       // 开环启动时，换相时间已经降低到最小值后，统计换相的次数
    uint16  restart_delay;          // 电机延时重启
    uint16  commutation_failed_num; // 换相错误次数
    uint16  commutation_time[6];    // 最近6次换相时间
    uint32  commutation_time_sum;   // 最近6次换相时间总和
    uint32  commutation_num;        // 统计换相次数

    // 三段式启动新增参数
    uint16  align_duty;             // 预定位占空比
    uint16  target_duty;            // 目标占空比 开环阶段逐步增加到此值

    uint8   zc_detect_count;        // 过零信号检测计数
    uint8   start_phase;            // 启动阶段 0:未启动 1:预定位 2:开环 3:过渡 4:闭环
    uint16  open_loop_step;         // 开环加速步数计数
    uint16  duty_ramp_timer;        // 占空比斜坡定时器
}motor_struct;


extern motor_struct motor;


void motor_power_on_beep(uint16 volume);
void motor_stop(void);
void motor_start(void);
void motor_init(void);





#endif
