#ifndef _SEEKFREE_AT24C02_H
#define _SEEKFREE_AT24C02_H

#include "common.h"



									//AT24C02模块支持256个8位数据存储
									//该模块使用的是IIC总线控制
#define AT24C02_DEV_ADDR 0xA0>>1	//IIC写入时的地址字节数据，+1为读取
									//AT24C02的地址是可变地址当A0 = 0 A1 = 0 A2 = 0的时候,
									//设备地址位A0，如果需要修改设备地址，请查看AT24C02手册进行修改

void at24c02_write_byte(uint8 byte_reg, uint8 dat);
uint8 at24c02_read_byte(uint8 byte_reg);
void at24c02_write_bytes(uint8 byte_reg, uint8 *dat_add, uint8 num);
void at24c02_read_bytes(uint8 byte_reg, uint8 *dat_add, uint8 num);

#endif 
