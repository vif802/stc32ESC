#ifndef _SEEKFREE_WIRELESS_CH573_H_
#define _SEEKFREE_WIRELESS_CH573_H_


#include "common.h"

#define WIRELESS_CH573_UART        		UART_4         //CH573转串口模块 所使用到的串口     
#define WIRELESS_CH573_UART_TX_PIN    	UART4_TX_P03
#define WIRELESS_CH573_UART_RX_PIN    	UART4_RX_P02
#define WIRELESS_CH573_TIMER_N    		TIM_2
#define WIRELESS_CH573_UART_BAUD   		115200
#define WIRELESS_CH573_DATA_BUF         S4BUF



#define WIRELESS_CH573_RTS_PIN P07 			//定义流控位引脚  指示当前模块是否可以接受数据  0可以继续接收  1不可以继续接收
//#define WIRELESS_CH573_CMD_PIN P05 			//定义命令引脚


#define WIRELESS_CH573_BUFFER_SIZE       64
#define WIRELESS_CH573_TIMEOUT_COUNT     500


void wireless_ch573_init(void);
void wireless_ch573_callback(void);

uint32 wireless_ch573_send_buff(uint8 *buff, uint16 len);
uint32 wireless_ch573_read_buff (uint8 *buff, uint32 len);


#endif
