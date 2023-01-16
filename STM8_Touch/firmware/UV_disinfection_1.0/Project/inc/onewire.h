#ifndef __ONE_WIRE_H__
#define __ONE_WIRE_H__

#include "main.h"

#define DS18B20_INPUT()         GPIO_Init(DS18_NTC_PORT, DS18_NTC_PIN, GPIO_MODE_IN_FL_NO_IT)
#define DS18B20_OUTPUT()        GPIO_Init(DS18_NTC_PORT, DS18_NTC_PIN, GPIO_MODE_OUT_PP_LOW_FAST)	

#define DS18B20_IN()            GPIO_ReadInputPin(DS18_NTC_PORT, DS18_NTC_PIN) 
#define DS18B20_OUT_LOW()       GPIO_WriteLow(DS18_NTC_PORT, DS18_NTC_PIN) 
#define DS18B20_OUT_HIGH()      GPIO_WriteHigh(DS18_NTC_PORT, DS18_NTC_PIN) 
bool onewire_reset(void); 

void onewire_write(unsigned char value); 
unsigned char onewire_read(void);
extern void delay_us(uint8_t us);

#endif