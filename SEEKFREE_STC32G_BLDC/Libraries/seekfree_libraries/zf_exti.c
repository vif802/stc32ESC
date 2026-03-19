#include "zf_exti.h"


//-------------------------------------------------------------------------------------------------------------------
//  @brief      外部中断初始化
//  @param      NULL            	
//  @return     void
//  Sample usage:               exit_init(INT0_P32,BOTH) //初始化P32 作为外部中断引脚，双边沿触发。
//-------------------------------------------------------------------------------------------------------------------
void exit_init(INTN_enum int_n,INT_MODE_enum mode)
{
	if(INT0_P32 == int_n)
	{
		IT0 = mode;
		EX0 = 1; 		//使能INT0中断
	}

	if(INT1_P33 == int_n)
	{
		IT1 = mode;
		EX1 = 1; 		//使能INT1中断
	}
	
	if(INT2_P36 == int_n)
	{
		INTCLKO |= 1<<4;	//使能INT2中断
	}
	
	if(INT3_P37 == int_n)
	{
		INTCLKO |= 1<<5;	//使能INT3中断
	}
	
	if(INT4_P30 == int_n)
	{
		INTCLKO |= 1<<6;	//使能INT4中断
	}

}