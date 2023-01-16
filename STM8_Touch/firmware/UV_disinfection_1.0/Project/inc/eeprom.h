#ifndef _EEPROM_H
#define _EEPROM_H
#include "stm8s.h"
#define BASEADDRESS	0x4000

#define TEMPERATURE_ADD    BASEADDRESS+0  //0:100 2byte  
#define HOUR_ADD   BASEADDRESS+2  //1:99 1byte
#define MINUTE_ADD BASEADDRESS+4  //1:99 1byte
#define FIRST_LOAD_ADD      BASEADDRESS+6

#define EEPROM_BYTES    5
////////////////////////////////////////////////////////////////////////////////////////
#define TEMPERATURE_DEFAULT    600  //custom  *10
#define HOUR_DEFAULT   10  //1:99 1byte
#define MINUTE_DEFAULT 0  //1:99 1byte

////////////////////////////////////////////////////////////////////////////////////////
void eeprom_init(void);
void eeprom_write(uint16_t value,unsigned int add, uint8_t bytes);

int16_t eeprom_read(unsigned int add, uint8_t bytes);

#endif