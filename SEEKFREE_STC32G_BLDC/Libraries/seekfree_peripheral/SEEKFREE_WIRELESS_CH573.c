#include "SEEKFREE_WIRELESS_CH573.h"
#include "zf_uart.h"
#include "zf_fifo.h"


static  fifo_struct     wireless_ch573_fifo;
static  uint8           wireless_ch573_buffer[WIRELESS_CH573_BUFFER_SIZE];  // 数据存放数组

static          uint8   wireless_ch573_data;


//-------------------------------------------------------------------------------------------------------------------
//  @brief      573转串口模块回调函数
//  @param      NULL
//  @return     void					
//  @since      v1.0
//  Sample usage:	
//  @note       
//-------------------------------------------------------------------------------------------------------------------
void wireless_ch573_callback(void)
{
    //接到一个字节后单片机将会进入串口中断，通过在此处读取wireless_ch573_data可以取走数据
    wireless_ch573_data = WIRELESS_CH573_DATA_BUF;
    fifo_write_buffer(&wireless_ch573_fifo, &wireless_ch573_data, 1);       // 存入 FIFO
}




//-------------------------------------------------------------------------------------------------------------------
//  @brief      573转串口模块 发送函数
//  @param      buff        需要发送的数据地址
//  @param      len         发送长度
//  @return     uint32      剩余未发送的字节数   
//  @since      v1.0
//  Sample usage:	
//  @note       
//-------------------------------------------------------------------------------------------------------------------
uint32 wireless_ch573_send_buff(uint8 *buff, uint16 len)
{
    while(len>30)
    {
        if(WIRELESS_CH573_RTS_PIN == 1)  
        {
            return len;//模块忙,如果允许当前程序使用while等待 则可以使用后面注释的while等待语句替换本if语句
        }
        //while(RTS_PIN);  //如果RTS为低电平，则继续发送数据
        uart_putbuff(WIRELESS_CH573_UART,buff,30);

        buff += 30; //地址偏移
        len -= 30;//数量
    }
    
    if(WIRELESS_CH573_RTS_PIN == 1)  
    {
        return len;//模块忙,如果允许当前程序使用while等待 则可以使用后面注释的while等待语句替换本if语句
    }
    //while(WIRELESS_CH573_RTS_PIN);  //如果RTS为低电平，则继续发送数据
    uart_putbuff(WIRELESS_CH573_UART,buff,len);//发送最后的数据
    
    return 0;
}


//-------------------------------------------------------------------------------------------------------------------
//  @brief      无线转串口模块 读取函数
//  @param      buff            存储的数据地址
//  @param      len             长度
//  @return     uint32          实际读取字节数
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
uint32 wireless_ch573_read_buff (uint8 *buff, uint32 len)
{
    uint32 data_len = len;
    fifo_read_buffer(&wireless_ch573_fifo, buff, &data_len, FIFO_READ_AND_CLEAN);
    return data_len;
}





//-------------------------------------------------------------------------------------------------------------------
//  @brief      573转串口模块初始化
//  @param      NULL
//  @return     void					
//  @since      v1.0
//  Sample usage:	
//  @note       
//-------------------------------------------------------------------------------------------------------------------
void wireless_ch573_init(void)
{
    WIRELESS_CH573_RTS_PIN = 0;
    wireless_type = WIRELESS_CH573;
    //本函数使用的波特率为115200，为573转串口模块的默认波特率，如需其他波特率请自行配置模块并修改串口的波特率
    fifo_init(&wireless_ch573_fifo, wireless_ch573_buffer, WIRELESS_CH573_BUFFER_SIZE);
    uart_init(WIRELESS_CH573_UART, WIRELESS_CH573_UART_RX_PIN, WIRELESS_CH573_UART_TX_PIN, WIRELESS_CH573_UART_BAUD, WIRELESS_CH573_TIMER_N);	//初始化串口    
    
}
