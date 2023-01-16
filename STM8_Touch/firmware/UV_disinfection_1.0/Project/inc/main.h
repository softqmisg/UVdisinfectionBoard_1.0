#ifndef __MAIN_H__
#define __MAIN_H__
#include <iostm8s005k6.h>
#include "stm8s.h"
//#define __WATCHDOG_ENABLE__
/*======================Define Ports & Pins===================================*/
#define TOUCH_CLK_PORT          GPIOF
#define TOUCH_CLK_PIN           GPIO_PIN_4      //2MHZ speed(2M),output(O)
#define TOUCH_DAT_PORT          GPIOE
#define TOUCH_DAT_PIN           GPIO_PIN_5      //2M,Input(I)

#define DIGIT1_PORT             GPIOB   
#define DIGIT1_PIN              GPIO_PIN_5      //2M,O
#define DIGIT2_PORT             GPIOB
#define DIGIT2_PIN              GPIO_PIN_4      //2M,O
#define DIGIT3_PORT             GPIOB
#define DIGIT3_PIN              GPIO_PIN_2      //2M,O
#define DIGIT4_PORT             GPIOB
#define DIGIT4_PIN              GPIO_PIN_3      //2M,O
#define DIGIT_TIK_PORT          GPIOC
#define DIGIT_TIK_PIN           GPIO_PIN_6      //10MHz Speed Programable(10MP),O

#define DS18_NTC_PORT           GPIOB   
#define DS18_NTC_PIN            GPIO_PIN_1      //AIN1

#define LED_SS_PORT             GPIOC
#define LED_SS_PIN              GPIO_PIN_1      //PWM_T1C1,10MP,O
#define LED_TT_PORT             GPIOC
#define LED_TT_PIN              GPIO_PIN_2      //PWM_T1_C2,10MP,O
#define LED_PLUS_PORT           GPIOC
#define LED_PLUS_PIN            GPIO_PIN_3      //PWM_T1_C3,10MP,O
#define LED_MINUS_PORT          GPIOC
#define LED_MINUS_PIN           GPIO_PIN_4      //PWM_T1_C4,10MP,O
#define LED_LOGO_PORT           GPIOD
#define LED_LOGO_PIN            GPIO_PIN_3      //10MP,O

#define SEG_CLK_PORT            GPIOC
#define SEG_CLK_PIN             GPIO_PIN_5      //10MP,O
#define SEG_DAT_PORT            GPIOD
#define SEG_DAT_PIN             GPIO_PIN_7      //10M,O

#define DOOR_KEY_PORT           GPIOC           
#define DOOR_KEY_PIN            GPIO_PIN_7      //10M,I

#define FAN_PID_PORT            GPIOD
#define FAN_PID_PIN             GPIO_PIN_0      //PWM_T3_C2,10M,O
#define THERMAL_PID_PORT        GPIOD
#define THERMAL_PID_PIN         GPIO_PIN_2      //PWM_T3_C1,10M,O
#define BUZZER_PORT             GPIOD
#define BUZZER_PIN              GPIO_PIN_4      //Beeper,10M,O
/*============================================================================*/
#define TIK_SPD    86400   //second per day
typedef struct
{
  int8_t hour;
  int8_t minute;
  int8_t second;
} TIME_t;
/*============================================================================*/
void delay_ms(uint8_t delay);
void delay_us(uint8_t delay);
#endif