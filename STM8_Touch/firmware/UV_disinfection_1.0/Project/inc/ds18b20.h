#ifndef __DS18B20_H__
#define __DS18B20_H__
#include <stdbool.h>

#define DS18B20

#define convert_T 0x44 
#define read_scratchpad 0xBE 
#define write_scratchpad 0x4E 
//	#define copy_scratchpad 0x48 
//	#define recall_E2 0xB8 
//	#define read_power_supply 0xB4 
#define skip_ROM 0xCC 
//#define read_ROM	0x33
//	#define resolution 12 
bool DS18B20_init(void); 
bool DS18B20_get_temperature(int16_t *temperature) ;
extern void delay_us(uint8_t us);
extern void delay_ms(uint8_t us);
extern void IWDG_RESET(void);
//	void DS18B20_Read_ROM(void);
//	extern uint8_t ds18b20_rom[8];
#endif