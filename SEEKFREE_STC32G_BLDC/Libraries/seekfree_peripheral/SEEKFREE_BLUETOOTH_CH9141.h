#ifndef SEEKFREE_BLUETOOTH_CH9141_H
#define SEEKFREE_BLUETOOTH_CH9141_H

#include "common.h"






#define BLUETOOTH_CH9141_INDEX              UART_4                              // 蓝牙模块 1 对应使用的串口号
#define BLUETOOTH_CH9141_TIMER              TIM_4                               // 蓝牙模块 1 对应使用的定时器
#define BLUETOOTH_CH9141_BUAD_RATE          115200                              // 蓝牙模块 1 对应使用的串口波特率
#define BLUETOOTH_CH9141_TX_PIN             UART4_RX_P02                         // 蓝牙模块 1 对应模块的 TX 要接到单片机的 RX
#define BLUETOOTH_CH9141_RX_PIN             UART4_TX_P03                        // 蓝牙模块 1 对应模块的 RX 要接到单片机的 TX
#define BLUETOOTH_CH9141_RTS_PIN            P07                                 // 蓝牙模块 1 对应模块的 RTS 引脚

#define BLUETOOTH_CH9141_DATA_BUF       	S4BUF

#define BLUETOOTH_CH9141_BUFFER_SIZE        64
#define BLUETOOTH_CH9141_TIMEOUT_COUNT      500



void        bluetooth_ch9141_uart_callback      (void);
uint8       bluetooth_ch9141_init               (void);

uint32      bluetooth_ch9141_send_buff          (uint8 *buff, uint32 len);
uint32      bluetooth_ch9141_read_buff          (uint8 *buff, uint32 len);



#endif
