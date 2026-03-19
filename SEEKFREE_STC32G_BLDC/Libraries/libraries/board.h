#ifndef __BOARD_H
#define __BOARD_H
#include "common.h"


//FOSC可选值:35000000, 33177600, 30000000, 27000000. 24000000, 22118400

#define FOSC					0			// FOSC的值设置为0，则内核频率通过寄存器强制设置为35Mhz，
											// 不管STC-ISP软件下载时候选择多少，他都是35Mhz。
											
//#define FOSC          		33177600	// FOSC的值设置为33.1776Mhz,
											// 使用STC-ISP软件下载的时候，
											// 此频率需要跟STC-ISP软件中的 <输入用户程序运行时的IRC频率>选项的频率一致。


#define EXTERNAL_CRYSTA_ENABLE 	0			// 使用外部晶振，0为不使用，1为使用（建议使用内部晶振）
#define PRINTF_ENABLE			1			// printf使能，0为失能，1为使能
#define ENABLE_IAP 				1			// 使能软件一键下载功能，0为失能，1为使能

#define DEBUG_UART 			  	UART_1
#define DEBUG_UART_BAUD 	  	115200
#define DEBUG_UART_RX_PIN  		UART1_RX_P30
#define DEBUG_UART_TX_PIN  		UART1_TX_P31
#define DEBUG_UART_TIM			TIM_2

#if (1==PRINTF_ENABLE)
	char putchar(char c);
#endif

#define SET_P54_RESRT 	  (RSTCFG |= 1<<4)	//设置P54为复位引脚

extern int32 sys_clk;

void board_init(void);
void DisableGlobalIRQ(void);
void EnableGlobalIRQ(void);

#endif

