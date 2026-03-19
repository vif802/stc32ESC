 #ifndef _SEEKFREE_IIC_h
#define _SEEKFREE_IIC_h

#include "common.h"
#define SIMIIC_SCL_PIN P40
#define SIMIIC_SDA_PIN P41 


typedef enum IIC       //DAC模块
{
    SIMIIC,
    SCCB
} IIC_type;



void  simiic_start(void);
void  simiic_stop(void);
void  simiic_ack_main(uint8 ack_main);
void  send_ch(uint8 c);
uint8 read_ch(uint8 ack);
void  simiic_write_reg(uint8 dev_add, uint8 reg, uint8 dat);
uint8 simiic_read_reg(uint8 dev_add, uint8 reg, IIC_type type);
void  simiic_read_regs(uint8 dev_add, uint8 reg, uint8 *dat_add, uint8 num, IIC_type type);
void  simiic_init(void);











#endif

