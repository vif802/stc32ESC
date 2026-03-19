#ifndef __ZF_EXTI_H
#define __ZF_EXTI_H
#include "common.h"






//此枚举定义不允许用户修改
typedef enum    // 枚举ADC通道
{
	INT0_P32 = 0,	//支持边沿，下降沿中断
	INT1_P33,		//支持边沿，下降沿中断
	INT2_P36,		//支持下降沿中断		
 	INT3_P37,		//支持下降沿中断
	INT4_P30,		//支持下降沿中断
}INTN_enum;

#define INT0_CLEAR_FLAG (TCON &= (~(0x01 << 1)))		//外部中断 0 中断请求标志。中断服务程序中，硬件自动清零。
#define INT1_CLEAR_FLAG (TCON &= (~(0x01 << 3)))		//外部中断 1 中断请求标志。中断服务程序中，硬件自动清零。

#define INT2_CLEAR_FLAG (AUXINTIF &= (~(0x01 << 0)))	//外部中断 2 中断请求标志。需要软件清零。
#define INT3_CLEAR_FLAG (AUXINTIF &= (~(0x01 << 1)))	//外部中断 3 中断请求标志。需要软件清零。
#define INT4_CLEAR_FLAG (AUXINTIF &= (~(0x01 << 2)))	//外部中断 4 中断请求标志。需要软件清零。


typedef enum    // 枚举ADC通道
{
	BOTH,			//边沿
	FALLING_EDGE,	//下降沿
//	RISING_EDGE,	//不支持上升沿

}INT_MODE_enum;


void exit_init(INTN_enum int_n,INT_MODE_enum mode);


#endif