#ifndef _SEVEN_SEGMENT_H
#define _SEVEN_SEGMENT_H
#include "main.h"
#include <stdbool.h>

#define CA_SEGMENT

#define DIGITNO		0//0b00000
#define DIGIT1		1//0b1000
#define DIGIT2		2//0b0100
#define DIGIT3		3//0b0010
#define DIGIT4		4//0b0001
#define DIGITALL	5//0b1111

#define FREQ_TIME1		400 //Hz
#define MS_TIME1		2.5 //1000/FREQ_TIME2


//BLINK_DELAY*2.5ms= blink delays
#define BLINK_ON_DELAY 400//400/MS_TIME1 //400ms
#define BLINK_OFF_DELAY 100//400/MS_TIME1 //400ms


extern  char Delaystart_Flag;
extern bool flag_2sec;
///////////////////////////////////////////
void sevenseg_init(void);
void Blink_7seg(bool state);
//void print_7SEG(char *string);
void print_7SEG(char *string);
void num2str(int16_t data, uint8_t d_digit,uint8_t line);
void Write_7seg(uint8_t x);
void SHOW_SEG(char x,uint8_t dp);
//void TIM1_UPD_OVF_BRK_IRQHandler();
#endif