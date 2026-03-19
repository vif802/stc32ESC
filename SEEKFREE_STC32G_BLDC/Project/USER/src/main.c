#include "motor.h"
#include "comparator.h"
#include "bldc_config.h"
#include "pwm.h"
#include "pwm_input.h"
#include "pit_timer.h"
#include "battery.h"
#include "headfile.h"



// 关于内核频率的设定，可以查看board.h文件
// 在board_init中,已经将P54引脚设置为复位
// 如果需要使用P54引脚,可以在board.c文件中的board_init()函数中删除SET_P54_RESRT即可

// 无刷电机LED状态灯说明 (三段式启动)
// 电池电压过低时，             LED亮0.1s    灭0.9s
// 电机遇到堵转，               LED亮0.05s   灭0.05s
// 电机未运行时(run_flag=0)，   LED亮1s      灭1s
// 预定位阶段(run_flag=1)，     LED亮0.5s    灭0.5s
// 开环加速阶段(run_flag=2)，   LED快速闪烁  0.05s间隔
// 过渡阶段(run_flag=3)，       LED常亮
// 闭环运行阶段(run_flag=4)，   LED常亮


void main()
{
    CKCON = 0;
    WTST = 0;               // 设置程序代码等待参数，赋值为0可将CPU执行程序的速度设置为最快
	DisableGlobalIRQ();		// 关闭总中断
    sys_clk = 33177600;     // 设置系统频率为33177600Hz
	board_init();			// 初始化寄存器
    EnableGlobalIRQ();		// 开启总中断
    download_flag = 0;
    
    // 此处编写用户代码(例如：外设初始化代码等)
    battery_init();         // 电池电压检测初始化
    led_init();             // LED初始化
    pwm_input_init();       // PWM输入捕获初始化
    comparator_init();      // 比较器初始化 
    motor_init();           // 电机相关初始化 

    // PWM初始化务必放在电机电机初始化函数之后，否则会烧毁电机
    // PWM初始化务必放在电机电机初始化函数之后，否则会烧毁电机
    // PWM初始化务必放在电机电机初始化函数之后，否则会烧毁电机
    pwm_center_init();      // PWM初始化 采用中心对齐
    
    pit_timer_init();       // 周期定时器初始化
    
    while(1)
	{
        if(0 == download_flag)
        {
            data_conversion(battery_voltage, motor.run_flag, pwmin.frequency, pwmin.throttle, virtual_scope_data);
            uart_putbuff(UART_1, virtual_scope_data, 10);
        }

    }
}

