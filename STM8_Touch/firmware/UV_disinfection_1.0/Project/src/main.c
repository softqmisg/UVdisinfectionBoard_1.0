#include "main.h"
#include "pid_controller.h"
#include "ds18b20.h"
#include "sevenseg.h"
#include "eeprom.h"
#include "bs814a.h"
#include "retarget.h"
/*=============================Define=======================================================*/
#define GOMAINMENU      10//Second
#define LED_SS          0
#define LED_TT          1
#define LED_PLUS        2
#define LED_MINUS       3

#define TEMPERATURE_LIMITHIGH       80.0
#define TEMPERATURE_LIMITLOW        40.0

#define HOUR_LIMIT       99

#define CHANGE_DISP_DELAY       10

/*=============================Global variable===============================================*/
enum {POWERUP=0,IDLE,TEMP,TIME,START} menu_state=POWERUP;
/*===============================setup peripherial functions=================================*/
void TIM1_setup(void);
void TIM2_setup(void);
void TIM3_setup(void);
void TIM4_setup(void);
void beeper_setup(void);
void UART2_setup(void);
void GPIO_setup(void);
void clk_setup(void);
void IWDG_setup(void);
void IWDG_RESET(void);
/*===============================user functions=============================================*/
void fade_apply(char num);
void fade_led(char num,char onoff,uint8_t fixedpercent);
TIME_t decreaseTime(TIME_t t,TIME_t d);
TIME_t IncreaseTime(TIME_t t,TIME_t d);
void Buz_short();

/*=============================fade_led=============================================*/
uint16_t counter_fade[4]={20,20,20,20};
bool channel_fade[4]={0,0,0,0};
bool fade_state[4]={0,0,0,0};
void fade_apply(char num);
volatile bool flag_20ms=0;
#pragma vector=TIM3_OVR_UIF_vector
__interrupt void TIM3_UPD_IRQHandler(void)
{
  flag_20ms=1;
  
  for(uint8_t ch=0;ch<4;ch++)
  {
    if(channel_fade[ch])
      fade_apply(ch);
  }
  TIM3_ClearFlag(TIM3_FLAG_UPDATE);
}
void fade_apply(char num)
{
  switch(num)
  {
  case LED_SS:
    TIM1_SetCompare1(counter_fade[0]);
    break;
  case LED_TT:
    TIM1_SetCompare2(counter_fade[1]);
    break;
  case LED_PLUS:
    TIM1_SetCompare3(counter_fade[2]);
    break;
  case LED_MINUS:
    TIM1_SetCompare4(counter_fade[3]);
    break;
  }
  if(fade_state[num]==0)
  {
    counter_fade[num]=(uint16_t)counter_fade[num]+20;
    if(counter_fade[num]>900) 
    {
      fade_state[num]=1;
    }
  }
  else
  {
    counter_fade[num]=(uint16_t)counter_fade[num]-20;
    if(counter_fade[num]<=20)
    {
      fade_state[num]=0;
    }
  }
}
void fade_led(char num,char onoff,uint8_t fixed_percent)
{
  if(onoff)
  {
    counter_fade[num]=20;
    channel_fade[num]=1;
    return;
  }
  channel_fade[num]=0;
  counter_fade[num]=(uint16_t)fixed_percent*10;
  fade_apply(num);
}
/*=============================================================================*/
volatile bool flag_tim4_ovf=0;
#pragma vector =TIM4_OVR_UIF_vector 
__interrupt void TIM4_UPD_IRQHandler (void)
{
  flag_tim4_ovf=1;
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  
}
void delay_ms(uint8_t delay)
{
  TIM4_TimeBaseInit(TIM4_PRESCALER_128,124);// f=16Mhz/(128*125)=1KHZ
  TIM4_SetCounter(0);
  TIM4_Cmd(ENABLE);
  while(delay)
  {
    while(!flag_tim4_ovf);flag_tim4_ovf=0;
    delay--;
  }
  TIM4_Cmd(DISABLE);
}
void delay_us(uint8_t delay)
{
  TIM4_TimeBaseInit(TIM4_PRESCALER_16,delay);// f=16Mhz/(16*delay)=1MHZ
  TIM4_SetCounter(0);
  TIM4_Cmd(ENABLE);
  while(!flag_tim4_ovf);flag_tim4_ovf=0;
  TIM4_Cmd(DISABLE);
}
/*============================================================================*/
volatile  bool flag_1s=0;
uint16_t  counter_change_disp=0;
volatile bool  flag_change_disp=0;
uint16_t GOMAINMENU_counter=0;
enum {DispTime, DispText} DispState=DispTime;
#pragma vector =TIM2_OVR_UIF_vector 
__interrupt void TIM2_UPD_IRQHandler (void)
{
  flag_1s=1;
  counter_change_disp++;
  if(counter_change_disp>CHANGE_DISP_DELAY)
  {
    counter_change_disp=0;
    flag_change_disp=1;
  }
  if(GOMAINMENU_counter>0)
    GOMAINMENU_counter--;  
  ///blink second segment if show time
  if(DispState==DispTime)
    GPIO_WriteReverse(DIGIT_TIK_PORT,DIGIT_TIK_PIN);
  else
    GPIO_WriteHigh(DIGIT_TIK_PORT,DIGIT_TIK_PIN);
  
  TIM2_ClearFlag(TIM2_FLAG_UPDATE); 
}
TIME_t decreaseTime(TIME_t t,TIME_t d)
{
  TIME_t r=t;
  r.second-=d.second;
  if(r.second<0)
  {
    r.minute--;
    r.second+=60;
  }
  r.minute-=d.minute;
  if(r.minute<0)
  {
    r.hour--;
    r.minute+=60;
  }
  r.hour-=d.hour;
  return r;
}
TIME_t IncreaseTime(TIME_t t,TIME_t d)
{
  t.second+=d.second;
  if(t.second>59)
  {
    t.minute++;
    t.second-=60;
  }
  t.minute+=d.minute;
  if(t.minute>59)
  {
    t.hour++;
    t.minute-=60;
  }
  t.hour+=d.hour;
  return t;
}
/*============================================================================*/
void Buz_short()
{
  BEEP_Init(BEEP_FREQUENCY_4KHZ);
  BEEP_Cmd(ENABLE);
  delay_ms(50);
  BEEP_Cmd(DISABLE);
}
/*============================================================================*/
void Buz_long()
{
  BEEP_Init(BEEP_FREQUENCY_4KHZ);
  BEEP_Cmd(ENABLE);
  delay_ms(500);
  BEEP_Cmd(DISABLE);
}
/*============================================================================*/
double TEMPERATURE_Value;
int8_t  HOUR_Value;  //1:99 1byte
int8_t  MINUTE_Value;  //1:99 1byte
void update_value()
{
  uint16_t tmp=eeprom_read(TEMPERATURE_ADD,2);
  TEMPERATURE_Value=(double)tmp/10.0;
  HOUR_Value=(int8_t)eeprom_read(HOUR_ADD,1);
  MINUTE_Value=(int8_t)eeprom_read(MINUTE_ADD,1);
}
/*============================================================================*/
int16_t cur_temp;
void main()
{ 
  float pid_kp=1.0,pid_kd=0.0,pid_ki=0.0;
  char str[10],tmp_str[30];

  PIDControl pid_temperature;
  TIME_t cur_time;
  TIME_t onesec_time={
    .hour=0,
    .minute=0,
    .second=1,
  };
  bool wait=false;
  bool lock_ss=false;
  uint8_t counter_lock_p=0,counter_lock_m=0;
  bool sensor_valid=TRUE;
  //////////////////////////setup peripheral///////////////////////////////////////
  clk_setup();
  GPIO_setup();
  TIM1_setup();//for PWM of LEDS(16bit)&7 SEGS ,freq=400HZ, interupt
  TIM2_setup();//for tik of 1 second (16bit),freq=1Hz, interrupt
  TIM3_setup();//for PWM of PID (16bit) ,freq=50Hz,  interrupt
  TIM4_setup();//for tik of us (8bit),freq=1MHZ,no interrupt
  beeper_setup();
  UART2_setup();
  IWDG_setup();
  //////////////////////////Device peripheral///////////////////////////////////////
  bs814a_init();
  eeprom_init();
  enableInterrupts();	
  sevenseg_init();
  GPIO_WriteHigh(DIGIT_TIK_PORT,DIGIT_TIK_PIN);
  DispState=DispText;
  //////////////////read EEPROM///////////////////
  uint8_t first_load=eeprom_read(FIRST_LOAD_ADD,1);
  if(first_load!=0x55)
  {
    eeprom_write(0x55,FIRST_LOAD_ADD,1);
    delay_ms(50);
    eeprom_write(TEMPERATURE_DEFAULT,TEMPERATURE_ADD,2);
    delay_ms(50);
    eeprom_write(HOUR_DEFAULT,HOUR_ADD,1);
    delay_ms(50);
    eeprom_write(MINUTE_DEFAULT,MINUTE_ADD,2);
    delay_ms(50);    
  }
  update_value();
  //////////////////Initialize befor main loop//////////////////////////////////
  menu_state=POWERUP;
  GPIO_WriteHigh(LED_LOGO_PORT,LED_LOGO_PIN);
  GPIO_WriteHigh(DIGIT_TIK_PORT,DIGIT_TIK_PIN);
  DispState=DispText;
  
  if(!DS18B20_init())
  {
    DispState=DispText;
    print_7SEG("SERR");
    while(1)
    {
      IWDG_RESET();
    }
  }
  fade_led(LED_SS,0,0);
  fade_led(LED_TT,1,0);
  fade_led(LED_PLUS,0,0);
  fade_led(LED_MINUS,0,0);
  Buz_short();
  Blink_7seg(TRUE);
  DispState=DispText;
  print_7SEG("----");
  menu_state=IDLE;
  ///////////////////main loop//////////////////////////////////////////////////
  while(1)
  {
    /////////////////always run function in loop////////////////////////////////
    key_scan();
    
    ////////////////state machine of menu///////////////////////////////////////
    switch(menu_state)
    {
      ////////////////IDLE STATE////////////////////////
    case IDLE:
      if(key_shortpressed[TC_PLUS])
      {
        key_shortpressed[TC_PLUS]=0;
      }
      if(key_longpressed[TC_PLUS])
      {
        key_longpressed[TC_PLUS]=0;
      }    
      if(key_shortpressed[TC_MINUS])
      {
        key_shortpressed[TC_MINUS]=0;
      }
      if(key_longpressed[TC_MINUS])
      {
        key_longpressed[TC_MINUS]=0;
      }   
      if(key_shortpressed[TC_SS])
      {
        key_shortpressed[TC_SS]=0;
      }
      if(key_longpressed[TC_SS])
      {
        key_longpressed[TC_SS]=0;
      }            
      if(key_shortpressed[TC_TT]||key_longpressed[TC_TT])
      {
        key_longpressed[TC_TT]=0;
      }
      if(key_shortpressed[TC_TT]||key_longpressed[TC_TT])
      {
        if(key_shortpressed[TC_TT])
          key_shortpressed[TC_TT]=0;
        
        GPIO_WriteHigh(DIGIT_TIK_PORT,DIGIT_TIK_PIN);
        DispState=DispText;
        sprintf(tmp_str,"%4.1fC",TEMPERATURE_Value);
        print_7SEG(tmp_str);        
        Blink_7seg(TRUE);
        fade_led(LED_PLUS,0,100);
        fade_led(LED_MINUS,0,100);
        fade_led(LED_TT,1,0);
        fade_led(LED_SS,0,0);
        //        Buz_short();
        menu_state=TEMP;
      }
      break;
      ////////////////TEMP STATE////////////////////////
    case TEMP:
      if(key_shortpressed[TC_PLUS]||key_longpressed[TC_PLUS])
      {
        if(key_shortpressed[TC_PLUS])
          key_shortpressed[TC_PLUS]=0;
        else
          key_longpressed[TC_PLUS]=0;
        TEMPERATURE_Value+=0.5;
        if(TEMPERATURE_Value>TEMPERATURE_LIMITHIGH)
          TEMPERATURE_Value=TEMPERATURE_LIMITHIGH;
        GPIO_WriteHigh(DIGIT_TIK_PORT,DIGIT_TIK_PIN);
        DispState=DispText;
        sprintf(tmp_str,"%4.1fC",TEMPERATURE_Value);
        print_7SEG(tmp_str);          
        //        Buz_short();
      }
      
      if(key_shortpressed[TC_MINUS]||key_longpressed[TC_MINUS])
      {
        if(key_shortpressed[TC_MINUS])
          key_shortpressed[TC_MINUS]=0;
        else
          key_longpressed[TC_MINUS]=0;
        TEMPERATURE_Value-=0.5;
        if(TEMPERATURE_Value<TEMPERATURE_LIMITLOW)
          TEMPERATURE_Value=TEMPERATURE_LIMITLOW;
        GPIO_WriteHigh(DIGIT_TIK_PORT,DIGIT_TIK_PIN);
        DispState=DispText;
        sprintf(tmp_str,"%4.1fC",TEMPERATURE_Value);
        print_7SEG(tmp_str);           
        //        Buz_short();
      }
      if(key_shortpressed[TC_TT])
      {
        key_shortpressed[TC_TT]=0;
        DispState=DispTime;
        sprintf(tmp_str,"%02d%02d",HOUR_Value,MINUTE_Value);
        print_7SEG(tmp_str);        
        Blink_7seg(TRUE);
        fade_led(LED_PLUS,0,100);
        fade_led(LED_MINUS,0,100);
        fade_led(LED_TT,0,10);
        fade_led(LED_SS,1,0);
        
        //        Buz_short();
        menu_state=TIME;
        uint16_t tmp=(uint16_t)((double)TEMPERATURE_Value*10.0);
        eeprom_write(tmp,TEMPERATURE_ADD,2);
        delay_ms(50);
       
      }
      if(key_longpressed[TC_TT])
      {
        key_longpressed[TC_TT]=0;
      } 
      if(key_shortpressed[TC_SS])
      {
        key_shortpressed[TC_SS]=0;
      }
      if(key_longpressed[TC_SS])
      {
        key_longpressed[TC_SS]=0;
      }      
      break;
      ////////////////TIME STATE////////////////////////
    case TIME:
      if(key_shortpressed[TC_PLUS]||key_longpressed[TC_PLUS])
      {
        if(key_shortpressed[TC_PLUS])
          key_shortpressed[TC_PLUS]=0;
        else
          key_longpressed[TC_PLUS]=0;
        MINUTE_Value+=10;
        if(MINUTE_Value>59)
        {
          MINUTE_Value=0;
          HOUR_Value++;
          if(HOUR_Value>99) HOUR_Value=0;
        }
        DispState=DispTime;
        sprintf(tmp_str,"%02d%02d",HOUR_Value,MINUTE_Value);
        print_7SEG(tmp_str);   
        //        Buz_short();
      }
      
      if(key_shortpressed[TC_MINUS]||key_longpressed[TC_MINUS])
      {
        if(key_shortpressed[TC_MINUS])
          key_shortpressed[TC_MINUS]=0;
        else
          key_longpressed[TC_MINUS]=0;
        MINUTE_Value-=10;
        if(MINUTE_Value<0)
        {
          MINUTE_Value=50;
          HOUR_Value--;
          if(HOUR_Value<0) HOUR_Value=99;
        }
        DispState=DispTime;
        sprintf(tmp_str,"%02d%02d",HOUR_Value,MINUTE_Value);
        print_7SEG(tmp_str);   
        //        Buz_short();
      }
      if(key_shortpressed[TC_TT])
      {
        key_shortpressed[TC_TT]=0;
        GPIO_WriteHigh(DIGIT_TIK_PORT,DIGIT_TIK_PIN);
        DispState=DispText;
        sprintf(tmp_str,"%4.1fC",TEMPERATURE_Value);
        print_7SEG(tmp_str);        
        Blink_7seg(TRUE);
        fade_led(LED_PLUS,0,100);
        fade_led(LED_MINUS,0,100);
        fade_led(LED_TT,1,0);
        fade_led(LED_SS,0,0);
        //        Buz_short();
        menu_state=TEMP;
        eeprom_write((uint16_t)HOUR_Value,HOUR_ADD,1);
        delay_ms(50);
        eeprom_write((uint16_t)MINUTE_Value,MINUTE_ADD,2);
        delay_ms(50);         
      }
      if(key_longpressed[TC_TT])
      {
        key_longpressed[TC_TT]=0;
      } 
      if(key_shortpressed[TC_SS])
      {
        key_shortpressed[TC_SS]=0;
        fade_led(LED_PLUS,0,0);
        fade_led(LED_MINUS,0,0);
        fade_led(LED_TT,0,0);
        fade_led(LED_SS,0,10); 
        flag_change_disp=1;
        counter_change_disp=0;
        Blink_7seg(FALSE);
        cur_time.hour=HOUR_Value;cur_time.minute=MINUTE_Value;cur_time.second=0;
        PIDInit(&pid_temperature, pid_kp,pid_ki,pid_kd,  1.0, 0, 624,  AUTOMATIC, DIRECT);  
        TIM3_SetCompare2(624);//start FAN
        Buz_short();
        counter_lock_p=0;counter_lock_m=0;wait=false;
        menu_state=START;
        eeprom_write((uint16_t)HOUR_Value,HOUR_ADD,1);
        delay_ms(50);
        eeprom_write((uint16_t)MINUTE_Value,MINUTE_ADD,2);
        delay_ms(50);           
      }
      if(key_longpressed[TC_SS])
      {
        key_longpressed[TC_SS]=0;
      }      
      
      break;
      ////////////////START STATE////////////////////////
    case START:
      if(flag_change_disp && !wait)
      {
        counter_change_disp=0;
        flag_change_disp=0;
         
        switch(DispState)
        {
        case DispText:
          DispState=DispTime;
          sprintf(tmp_str,"%02d%02d",cur_time.hour,cur_time.minute);
          print_7SEG(tmp_str);            
          break;
        case DispTime:
          GPIO_WriteHigh(DIGIT_TIK_PORT,DIGIT_TIK_PIN);
          DispState=DispText;
          if(sensor_valid)
            sprintf(tmp_str,"%4.1fC",(float)cur_temp/10.0);
          else
            sprintf(tmp_str,"SERR");
          print_7SEG(tmp_str);           
          break;
        }
      }
      if(flag_1s && !wait)
      {
        flag_1s=0;
        if(!DS18B20_get_temperature(&cur_temp))
        {
                 //sprintf(tmp_str,"%5.1f",temp/10.0);
          sensor_valid=FALSE;
        } 
        else
          sensor_valid=TRUE;
        cur_time=decreaseTime(cur_time,onesec_time);
        if(cur_time.hour<0)
        {
          ///stop all
          TIM3_SetCompare1(0);//stop heater
          TIM3_SetCompare2(0);//stop fan
          DispState=DispText;
          print_7SEG("----");
          Blink_7seg(TRUE);
          menu_state=IDLE; 
          Buz_long();
          fade_led(LED_SS,0,0);
          fade_led(LED_TT,1,0);
          fade_led(LED_PLUS,0,0);
          fade_led(LED_MINUS,0,0);          
        }
        else
        {
          if(sensor_valid)
          {
            PIDSetpointSet(&pid_temperature,(float)TEMPERATURE_Value);
            PIDInputSet(&pid_temperature,(float)cur_temp/10.0);
            PIDCompute(&pid_temperature);
            float output=PIDOutputGet(&pid_temperature);
            TIM3_SetCompare1((uint16_t) output);
            switch(DispState)
            {
            case DispTime:
              sprintf(tmp_str,"%02d%02d",cur_time.hour,cur_time.minute);
              print_7SEG(tmp_str);            
              break;
            case DispText:
              GPIO_WriteHigh(DIGIT_TIK_PORT,DIGIT_TIK_PIN);
              if(sensor_valid)
                sprintf(tmp_str,"%4.1fC",(float)cur_temp/10.0);
              else
                sprintf(tmp_str,"SERR");
              print_7SEG(tmp_str);           
              break;
            }
            
          }
        }
      }
      if(key_shortpressed[TC_PLUS])
      {
        key_shortpressed[TC_PLUS]=0;
      }
      if(key_shortpressed[TC_MINUS])
      {
        key_shortpressed[TC_MINUS]=0;
      }
      if(key_longpressed[TC_PLUS])
      {
        key_longpressed[TC_PLUS]=0;
        counter_lock_p++;
      }    
      if(key_longpressed[TC_MINUS])
      {
        key_longpressed[TC_MINUS]=0;
        counter_lock_m++;
      }   
      if(counter_lock_p >LOCK_DELAY &&counter_lock_m>LOCK_DELAY)
      {
        counter_lock_p=0;counter_lock_m=0;
        lock_ss=!lock_ss;
        if(lock_ss)
        {
          fade_led(LED_SS,0,0);
        }
        else
        {
          fade_led(LED_SS,0,10);
        }
      }
      if(key_shortpressed[TC_SS])
      {
        key_shortpressed[TC_SS]=0;
        if(!lock_ss)
        {
          if(wait)
          {
            wait=false;
            flag_1s=1;
            flag_change_disp=1;
            Blink_7seg(FALSE);
            TIM3_SetCompare2(1);//start again fan
          }
          else
          {
            ///wait all
            TIM3_SetCompare1(0);//stop heater
            TIM3_SetCompare2(0);//stop fan
            DispState=DispText;
            print_7SEG("STOP");
            Blink_7seg(TRUE);
            wait=true;
          }
          Buz_short();
        }
      }
      if(key_longpressed[TC_SS]) //manual; stop
      {
        if(!lock_ss)
        {
          key_longpressed[TC_SS]=0;
          TIM3_SetCompare1(0);//stop heater
          TIM3_SetCompare2(0);//stop fan
          DispState=DispText;
          print_7SEG("----");
          Blink_7seg(TRUE);
          fade_led(LED_SS,0,0);
          fade_led(LED_TT,1,0);
          fade_led(LED_PLUS,0,0);
          fade_led(LED_MINUS,0,0);          
          menu_state=IDLE;   
          Buz_long();
        }
      }            
      if(key_shortpressed[TC_TT])
      {
        key_shortpressed[TC_TT]=0;
      }
      if(key_longpressed[TC_TT])
      {
        key_longpressed[TC_TT]=0;
      }    
      break;
    }
    ////////////////////////////////////////////////////////////////////////////
    IWDG_RESET();
  }
}
/*===============================for PWM of LEDS(16bit)&7 SEGS & keys,freq=400HZ, interupt=============================================*/
void TIM1_setup(void) 
{ 
  TIM1_DeInit(); 
  TIM1_TimeBaseInit(39,TIM1_COUNTERMODE_UP,999, 0); // step=1s f=16Mhz/(39+1)/1000=400Hz
  TIM1_OC1Init(TIM1_OCMODE_PWM1,TIM1_OUTPUTSTATE_ENABLE,TIM1_OUTPUTNSTATE_DISABLE,0,TIM1_OCPOLARITY_LOW,TIM1_OCNPOLARITY_LOW,TIM1_OCIDLESTATE_SET,TIM1_OCNIDLESTATE_SET);
  TIM1_OC2Init(TIM1_OCMODE_PWM1,TIM1_OUTPUTSTATE_ENABLE,TIM1_OUTPUTNSTATE_DISABLE,0,TIM1_OCPOLARITY_LOW,TIM1_OCNPOLARITY_LOW,TIM1_OCIDLESTATE_SET,TIM1_OCNIDLESTATE_SET);
  TIM1_OC3Init(TIM1_OCMODE_PWM1,TIM1_OUTPUTSTATE_ENABLE,TIM1_OUTPUTNSTATE_DISABLE,0,TIM1_OCPOLARITY_LOW,TIM1_OCNPOLARITY_LOW,TIM1_OCIDLESTATE_SET,TIM1_OCNIDLESTATE_SET);
  TIM1_OC4Init(TIM1_OCMODE_PWM1,TIM1_OUTPUTSTATE_ENABLE,0,TIM1_OCPOLARITY_LOW,TIM1_OCIDLESTATE_SET);
  TIM1_CtrlPWMOutputs(ENABLE);
  TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE); 
  TIM1_SetCompare1(0);//on=1000
  TIM1_SetCompare2(0);//on=1000
  TIM1_SetCompare3(0);//on=1000
  TIM1_SetCompare4(0);//on=1000  
  TIM1_SetCounter(0);
  TIM1_Cmd(ENABLE); 
}
/*===============================for tik of 1 second (16bit),freq=1Hz, interrupt=============================================*/
void TIM2_setup(void) 
{ 
  TIM2_DeInit(); 
  TIM2_TimeBaseInit(TIM2_PRESCALER_1024, 15624); // f=16Mhz/(1024*15625)=1HZ
  TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE); 
  TIM2_Cmd(ENABLE); 
}
/*===============================for PWM of PID (16bit) ,freq=50Hz,  interrupt=============================================*/
void TIM3_setup(void) 
{ 
  TIM3_DeInit(); 
  TIM3_TimeBaseInit(TIM3_PRESCALER_512, 624); // f=16Mhz/(512*625)=50HZ
  TIM3_OC1Init(TIM3_OCMODE_PWM1,TIM3_OUTPUTSTATE_ENABLE,0,TIM3_OCPOLARITY_HIGH);
  TIM3_OC2Init(TIM3_OCMODE_PWM1,TIM3_OUTPUTSTATE_ENABLE,0,TIM3_OCPOLARITY_HIGH);
  TIM3_ITConfig(TIM3_IT_UPDATE,ENABLE);
  TIM3_SetCounter(0);
  TIM3_SetCompare1(0);
  TIM3_SetCompare2(0);
  
  TIM3_Cmd(ENABLE); 
}
/*==============================for tik of us (8bit),freq=1MHZ,no interrupt============================*/
void TIM4_setup(void)
{
  TIM4_DeInit();
  TIM4_TimeBaseInit(TIM4_PRESCALER_16,0);// f=16Mhz/(16*1)=0.1MHZ
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  TIM4_Cmd(DISABLE);
}
/*============================================================================*/
void beeper_setup(void)
{
  BEEP_DeInit();
  BEEP_LSICalibrationConfig(128000);
  BEEP_Init(BEEP_FREQUENCY_4KHZ);
}
/*============================================================================*/
void UART2_setup(void)
{
  UART2_DeInit();
  UART2_Init(9600,
             UART2_WORDLENGTH_8D,
             UART2_STOPBITS_1,
             UART2_PARITY_NO,
             UART2_SYNCMODE_CLOCK_DISABLE,
             UART2_MODE_TXRX_ENABLE);
  UART2_Cmd(ENABLE);
}

/*============================================================================*/
void GPIO_setup(void)
{
  GPIO_DeInit(GPIOA);
  GPIO_DeInit(GPIOB);
  GPIO_DeInit(GPIOC);
  GPIO_DeInit(GPIOD);
  GPIO_DeInit(GPIOE);
  GPIO_DeInit(GPIOF);
  
  //GPIO_Init(TOUCH_CLK_PORT, TOUCH_CLK_PIN ,GPIO_MODE_OUT_PP_HIGH_SLOW); 
  //GPIO_Init(TOUCH_DAT_PORT, TOUCH_DAT_PIN ,GPIO_MODE_IN_FL_NO_IT); 
  GPIO_Init(DIGIT1_PORT, DIGIT1_PIN ,GPIO_MODE_OUT_PP_HIGH_SLOW); 
  GPIO_Init(DIGIT2_PORT, DIGIT2_PIN ,GPIO_MODE_OUT_PP_HIGH_SLOW); 
  GPIO_Init(DIGIT3_PORT, DIGIT3_PIN ,GPIO_MODE_OUT_PP_HIGH_SLOW); 
  GPIO_Init(DIGIT4_PORT, DIGIT4_PIN ,GPIO_MODE_OUT_PP_HIGH_SLOW); 
  GPIO_Init(DIGIT_TIK_PORT, DIGIT_TIK_PIN ,GPIO_MODE_OUT_PP_HIGH_SLOW); 
  
  GPIO_Init(DS18_NTC_PORT, DS18_NTC_PIN ,GPIO_MODE_IN_FL_NO_IT); 
  
  GPIO_Init(LED_SS_PORT, LED_SS_PIN|LED_TT_PIN|LED_PLUS_PIN|LED_MINUS_PIN ,GPIO_MODE_OUT_PP_HIGH_FAST); 
  GPIO_Init(LED_LOGO_PORT, LED_LOGO_PIN ,GPIO_MODE_OUT_PP_HIGH_FAST); 
  
  GPIO_Init(SEG_CLK_PORT, SEG_CLK_PIN ,GPIO_MODE_OUT_PP_LOW_FAST); 
  GPIO_Init(SEG_DAT_PORT, SEG_DAT_PIN ,GPIO_MODE_OUT_PP_LOW_FAST); 
  
  GPIO_Init(DOOR_KEY_PORT, DOOR_KEY_PIN ,GPIO_MODE_IN_FL_IT); 
  GPIO_Init(FAN_PID_PORT, FAN_PID_PIN|THERMAL_PID_PIN,GPIO_MODE_OUT_PP_LOW_FAST); 
  GPIO_Init(BUZZER_PORT, BUZZER_PIN,GPIO_MODE_OUT_PP_HIGH_FAST); 
  GPIO_Init(GPIOD, GPIO_PIN_5, GPIO_MODE_OUT_PP_HIGH_FAST);//uart2 Tx 
  GPIO_Init(GPIOD, GPIO_PIN_6, GPIO_MODE_IN_PU_NO_IT);//uart2 Rx
}
/*============================================================================*/
void clk_setup(void)
{
  CLK_DeInit();
  CLK_HSECmd(DISABLE);
  CLK_LSICmd(ENABLE);
  while(CLK_GetFlagStatus(CLK_FLAG_LSIRDY)==FALSE);
  
  CLK_HSICmd(ENABLE);
  while(CLK_GetFlagStatus(CLK_FLAG_HSIRDY)==FALSE);
  CLK_ClockSwitchCmd(ENABLE);
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);//master clock=16MHZ/1=16MHZ
  CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);
  CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO,CLK_SOURCE_HSI,DISABLE,CLK_CURRENTCLOCKSTATE_ENABLE);
  
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C,DISABLE);
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_SPI,DISABLE);
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART2,ENABLE);
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_AWU,ENABLE);
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC,ENABLE);
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1,ENABLE);  //for PWM of LEDS(16bit)
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2,ENABLE);  //for tik of 1 second (16bit)
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER3,ENABLE);   //for PWM of PID (16bit)
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4,ENABLE);  //for tik of us (8bit)
}
/*============================================================================*/

void IWDG_setup(void)
{
#ifdef __WATCHDOG_ENABLE__
  
  IWDG_Enable();
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_SetPrescaler(IWDG_Prescaler_256); //128khz/256=500Hz==2ms
  IWDG_SetReload(0xFF); //watchdog duration=255*2ms=512ms;
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
#endif
}
/*============================================================================*/
void IWDG_RESET(void)
{
#ifdef __WATCHDOG_ENABLE__
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_ReloadCounter();
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
#endif
}
/*============================================================================*/

//  SHOW_SEG('R',1);
// //Write_7seg(0);
//  
//  while(1)
//  {
//    GPIO_WriteHigh(DIGIT4_PORT,DIGIT4_PIN);
//    GPIO_WriteLow(DIGIT1_PORT,DIGIT1_PIN);
//    GPIO_WriteHigh(LED_SS_PORT,LED_SS_PIN);
//    GPIO_WriteLow(LED_TT_PORT,LED_TT_PIN);
////    BEEP_Cmd(ENABLE);
//    while(!flag_1s);flag_1s=0;
//    GPIO_WriteHigh(DIGIT1_PORT,DIGIT1_PIN);
//    GPIO_WriteLow(DIGIT2_PORT,DIGIT2_PIN);
//    GPIO_WriteHigh(LED_TT_PORT,LED_TT_PIN);
//    GPIO_WriteLow(LED_PLUS_PORT,LED_PLUS_PIN);
////    BEEP_Cmd(DISABLE);
//    while(!flag_1s);flag_1s=0;
//    GPIO_WriteHigh(DIGIT2_PORT,DIGIT2_PIN);
//    GPIO_WriteLow(DIGIT3_PORT,DIGIT3_PIN);
//    GPIO_WriteHigh(LED_PLUS_PORT,LED_PLUS_PIN);
//    GPIO_WriteLow(LED_MINUS_PORT,LED_MINUS_PIN);  
////    BEEP_Cmd(ENABLE);
//    while(!flag_1s);flag_1s=0;
//    GPIO_WriteHigh(DIGIT3_PORT,DIGIT3_PIN);
//    GPIO_WriteLow(DIGIT4_PORT,DIGIT4_PIN);
//    GPIO_WriteHigh(LED_MINUS_PORT,LED_MINUS_PIN);
//    GPIO_WriteLow(LED_SS_PORT,LED_SS_PIN);    
////    BEEP_Cmd(DISABLE);
//    while(!flag_1s);flag_1s=0;
//  }
//  