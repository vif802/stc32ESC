#include "zf_delay.h"
#include "SEEKFREE_IIC.h"
#include "SEEKFREE_AT24C02.h"


#pragma warning disable = 173

//-------------------------------------------------------------------------------------------------------------------
//  @brief      at24c02写一个字节
//  @param      byte_reg	at24c02地址(0-255)
//  @param      dat			需要写入的数据
//  @return     void					
//  @since      v1.0
//  Sample usage:	
//  @note       
//-------------------------------------------------------------------------------------------------------------------
void at24c02_write_byte(uint8 byte_reg, uint8 dat)
{
	delay_ms(6);
	simiic_write_reg(AT24C02_DEV_ADDR, byte_reg, dat);
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      at24c02读一个字节
//  @param      byte_reg	at24c02地址(0-255)
//  @return     uint8		返回读取到的字节数					
//  @since      v1.0
//  Sample usage:	
//  @note       
//-------------------------------------------------------------------------------------------------------------------
uint8 at24c02_read_byte(uint8 byte_reg)
{
	delay_ms(6);
	return simiic_read_reg(AT24C02_DEV_ADDR, byte_reg, SIMIIC);
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      at24c02写多个字节
//  @param      byte_reg	at24c02起始地址(0-255)
//  @param      dat_add		需要写入的数据指针
//  @param      num			需要写入多少个数据
//  @return     void					
//  @since      v1.0
//  Sample usage:	
//  @note       
//-------------------------------------------------------------------------------------------------------------------
void at24c02_write_bytes(uint8 byte_reg, uint8 *dat_add, uint8 num)
{
	delay_ms(6);
	simiic_start();
    send_ch( (AT24C02_DEV_ADDR<<1) | 0x00);
	send_ch( byte_reg );   				

    while(--num)
    {
		send_ch( *dat_add++ );   				
		byte_reg++;
		if((byte_reg % 8) == 0)
		{
			simiic_stop();
			delay_ms(6);
			simiic_start();
			send_ch( (AT24C02_DEV_ADDR<<1) | 0x00);
			send_ch( byte_reg );   				
		}
    }
	send_ch( *dat_add++ );   
	simiic_stop();
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      at24c02读多个字节
//  @param      byte_reg	at24c02起始地址(0-255)
//  @param      dat_add		需要读出的数据指针
//  @param      num			需要读出多少个数据
//  @return     void					
//  @since      v1.0
//  Sample usage:	
//  @note       
//-------------------------------------------------------------------------------------------------------------------
void at24c02_read_bytes(uint8 byte_reg, uint8 *dat_add, uint8 num)
{
	delay_ms(5);
	simiic_start();
    send_ch( (AT24C02_DEV_ADDR<<1) | 0x00);  //发送器件地址加写位
	send_ch( byte_reg );   					 //发送从机寄存器地址
	delay_ms(5);
	simiic_start();
	send_ch( (AT24C02_DEV_ADDR<<1) | 0x01);  //发送器件地址加读位
    while(--num)
    {
        *dat_add++ = read_ch(1); //读取数据
		byte_reg++;
    }
	*dat_add++ = read_ch(0); //读取数据
	simiic_stop();
}
