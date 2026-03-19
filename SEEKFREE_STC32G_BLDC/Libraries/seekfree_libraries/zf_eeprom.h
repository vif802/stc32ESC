#ifndef __ZF_EEPROM_H
#define __ZF_EEPROM_H

#include "common.h"



void iap_init(void);
void iap_idle(void);
void iap_set_tps(void);
uint8 iap_get_cmd_state(void);
void iap_read_bytes(uint16 addr, uint8 *buf, uint16 len);
void iap_write_bytes(uint16 addr, uint8 *buf, uint16 len);
void iap_erase_page(uint16 addr);
void extern_iap_write_bytes(uint16 addr, uint8 *buf, uint16 len);


#endif


