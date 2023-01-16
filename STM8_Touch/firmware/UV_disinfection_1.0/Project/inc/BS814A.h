#ifndef __BS814A_H__
#define __BS814A_H__
#include "main.h"
#define LIMIT_SHORT	200
#define LONGPRESS_TIME  500//LIMIT_SHORT*2.5ms
#define LOCK_DELAY      10 //20*250ms=~5.625s

#define TC_SS          0
#define TC_TT          1
#define TC_PLUS        2
#define TC_MINUS       3

extern uint8_t flag_touch;
void bs814a_init();
//uint8_t read_touch(uint8_t *data);
//uint8_t read_touch1(uint8_t *data);
uint8_t read_touch2(uint8_t *data);

void key_scan(void);
extern void delay_ms(uint8_t delay);
extern void delay_us(uint8_t delay);
extern uint8_t key_shortpressed[4];
extern uint8_t key_longpressed[4];
#endif