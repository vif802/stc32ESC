#ifndef _SEEKFREE_WIRELESS
#define _SEEKFREE_WIRELESS

#include "common.h"


#define WIRELESS_UART        		UART_4         //无线转串口模块 所使用到的串口     
#define WIRELESS_UART_TX_PIN    	UART4_TX_P03
#define WIRELESS_UART_RX_PIN    	UART4_RX_P02
#define WIRELESS_TIMER_N    		TIM_2
#define WIRELESS_UART_BAUD   		115200
#define WIRELESS_DATA_BUF           S4BUF



#define WIRELESS_RTS_PIN P07 			//定义流控位引脚  指示当前模块是否可以接受数据  0可以继续接收  1不可以继续接收
//#define WIRELESS_CMD_PIN P05 			//定义命令引脚

#define WIRELESS_BUFFER_SIZE       64
#define WIRELESS_TIMEOUT_COUNT     500


void wireless_uart_init(void);
void wireless_uart_callback(void);

uint32 wireless_uart_send_buff(uint8 *buff, uint16 len);
uint32 wireless_uart_read_buff(uint8 *buff, uint32 len);

#endif 
