 #include "zf_nvic.h"

//-------------------------------------------------------------------------------------------------------------------
//  @brief      中断优先级管理
//  @param      irqn         选择IRQn模块
//  @param      priority     从0到3，优先级依次增加
//  @return     void
//  Sample usage:         	 NVIC_SetPriority(UART1_IRQn, 3);	//设置串口 1 中断优先级为 3 级（最高级）
//  @note					 如果在周期中断中，通过串口发送数据。
//							 那么串口中断的优先级一定要高于周期中断的优先级
//						 	 这样才能正常发送数据。	
//-------------------------------------------------------------------------------------------------------------------
void NVIC_SetPriority(NVIC_IRQn_enum irqn,uint8 priority)
{
	if((irqn >> 4) == 0)
	{
		IP  |= (priority & 0x01) << (irqn & 0x0F);
		IPH |= ((priority >> 1) & 0x01) << (irqn & 0x0F);
	}
	else if((irqn >> 4) == 1)
	{
		IP2  |= (priority & 0x01) << (irqn & 0x0F);
		IP2H |= ((priority >> 1) & 0x01) << (irqn & 0x0F);
	}
	else if((irqn >> 4) == 2)
	{
//		IP3  |= (priority & 0x01) << (irqn & 0x0F);
//		IP3H |= ((priority >> 1) & 0x01) << (irqn & 0x0F);
	}
}

