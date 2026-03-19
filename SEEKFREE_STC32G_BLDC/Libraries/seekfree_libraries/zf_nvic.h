#ifndef __ZF_NVIC_H
#define __ZF_NVIC_H

#include "common.h"

//该枚举体禁止用户修改
//中断优先级控制枚举体
typedef enum
{
	INT0_IRQn = 0x00,
	TIMER0_IRQn,
	INT1_IRQn,
	TIMER1_IRQn,
	UART1_IRQn,
	ADC_IRQn,
	LVD_IRQn,			//低压检测中断
	//CCP_PCA_PWM_IRQn,	//CCP/PCA/PWM 中断 STC8H没有此中断
	
	UART2_IRQn = 0x10,
	SPI_IRQn,
	PWM1_IRQn,
	PWM2_IRQn,
	INT4_IRQn,
	CMP_IRQn,
	IIC_IRQn,
	USB_IRQn,	//增强型 PWM2 异常检测中断 和 触摸按键中断

	UART3_IRQn = 0x20,
	UART4_IRQn,
//  STC8H没有这些中断
//	PWM1_IRQn,
//	PWM2_IRQn,
//	PWM3_IRQn,
//	PWM4_IRQn,
//	PWM5_IRQn,
//	PWM4FD_IRQn,

}NVIC_IRQn_enum;


void NVIC_SetPriority(NVIC_IRQn_enum irqn,uint8 priority);



#endif