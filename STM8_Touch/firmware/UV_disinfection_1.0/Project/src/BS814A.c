#include "bs814a.h"

uint8_t key_shortpressed[4]={0,0,0,0};
uint8_t key_longpressed[4]={0,0,0,0};
volatile uint8_t key_timercounter[4]={0,0,0,0};
uint8_t key_timerstart[4]={0,0,0,0};
uint8_t key_pressed[4]={0,0,0,0};
uint8_t key_released[4]={1,1,1,1};
uint8_t data[4]={1,1,1,1};

/*=============================================================================*/
uint8_t flag_touch=0;
#pragma vector=EXTI4_vector
__interrupt void EXTI4_IRQHandler(void)
{
  flag_touch=1;
}
/*============================================================================*/
void EXTI_setup(void)
{
  ITC_DeInit(); 
  ITC_SetSoftwarePriority(ITC_IRQ_PORTE, ITC_PRIORITYLEVEL_0); 
  EXTI_DeInit(); 
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOE, EXTI_SENSITIVITY_FALL_ONLY); 
  EXTI_SetTLISensitivity(EXTI_TLISENSITIVITY_FALL_ONLY);
}
/*=============================================================================*/
void bs814a_init()
{
  GPIO_Init(TOUCH_CLK_PORT, TOUCH_CLK_PIN ,GPIO_MODE_OUT_PP_HIGH_SLOW); 
  GPIO_Init(TOUCH_DAT_PORT, TOUCH_DAT_PIN ,GPIO_MODE_IN_PU_IT); 
  EXTI_setup();
}
/*******************************************************************************/
uint8_t read_touch2(uint8_t *data)
{
  uint8_t mask=1,input=0,cnt=0,result=1;
  ITC_DeInit();
  EXTI_DeInit();
  GPIO_Init(TOUCH_DAT_PORT, TOUCH_DAT_PIN ,GPIO_MODE_IN_FL_NO_IT); 
  input=0;
  for(uint8_t i=0;i<8;i++)
  {
    GPIO_WriteLow(TOUCH_CLK_PORT,TOUCH_CLK_PIN);
    delay_us(25);
    GPIO_WriteHigh(TOUCH_CLK_PORT,TOUCH_CLK_PIN);
    if(GPIO_ReadInputPin(TOUCH_DAT_PORT,TOUCH_DAT_PIN)!=FALSE)
    {
      input|=mask;
    }
    mask<<=1;
    delay_us(25);
  }
  if(!(input&0x01)) cnt++;
  if(!(input&0x02)) cnt++;
  if(!(input&0x04)) cnt++;
  if(!(input&0x08)) cnt++;
  if((input&0x01))data[0]=1; else data[0]=0;
  if((input&0x02)) data[1]=1; else data[1]=0;
  if((input&0x04))data[2]=1; else data[2]=0;
  if((input&0x08))data[3]=1; else data[3]=0;
  
  if(cnt !=((input>>4)&0x07))
  {
    delay_ms(10);
    result=0;
  }
  else
  {
    if(input&0x80) 
      result=1;
    else 
      result=0;
  }
  bs814a_init(); 
  return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void key_scan(void)
{
  if(flag_touch)
  {
    flag_touch=0;
    if(read_touch2(data))
    {
      for(uint8_t i=0;i<4;i++)
      {
        if((data[i]==1) && (key_pressed[i]==1))
        {
          key_released[i]=1;key_timerstart[i]=0;key_timercounter[i]=0;
        }
        else if(data[i]==0)
        {
          key_pressed[i]=1;key_released[i]=0;key_timerstart[i]=1;key_timercounter[i]=0;
        }
      }
    }
  }
  for(uint8_t i=0;i<4;i++)
  {
    if( (key_timerstart[i]==1) && (key_timercounter[i]>LIMIT_SHORT) && (key_pressed[i]==1))
    {
      key_longpressed[i]=1;key_timercounter[i]=0;
    }
    else if( (key_timercounter[i]<LIMIT_SHORT) && (key_pressed[i]==1) && (key_released[i]==1))
    {
      key_pressed[i]=0;
      key_shortpressed[i]=1;
    }
  }      
  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
///*=============================================================================*/
//uint8_t read_touch(uint8_t *data)
//{
//  uint8_t input[8]={0,0,0,0,0,0,0,0},cnt=0,result=1;
//  ITC_DeInit();
//  EXTI_DeInit();
//  GPIO_Init(TOUCH_DAT_PORT, TOUCH_DAT_PIN ,GPIO_MODE_IN_FL_NO_IT); 
//  for(uint8_t i=0;i<8;i++)
//  {
//    GPIO_WriteLow(TOUCH_CLK_PORT,TOUCH_CLK_PIN);
//    delay_us(30);
//    GPIO_WriteHigh(TOUCH_CLK_PORT,TOUCH_CLK_PIN);
//    if(GPIO_ReadInputPin(TOUCH_DAT_PORT,TOUCH_DAT_PIN)!=FALSE)
//    {
//      input[i]=1;
//    }
//    delay_us(30);
//  }
//  if(!input[0]) cnt++;
//  if(!input[1]) cnt++;
//  if(!input[2]) cnt++;
//  if(!input[3]) cnt++;
//  data[0]=input[0];data[1]=input[1];data[2]=input[2];data[3]=input[3];
//  uint8_t check=input[6]<<2+input[5]<<1+input[4];
//  if(cnt !=check)
//  {
//    delay_ms(10);
//    result=0;
//  }
//  else
//  {
//    if(input[7])
//      result=1;
//    else
//      result=0;
//  }
//  bs814a_init(); 
//  return result;
//}
///*******************************************************************************/
//uint8_t read_touch1(uint8_t *data)
//{
//  uint8_t mask=1,input=0,cnt=0,result=1;
//  ITC_DeInit();
//  EXTI_DeInit();
//  GPIO_Init(TOUCH_DAT_PORT, TOUCH_DAT_PIN ,GPIO_MODE_IN_FL_NO_IT); 
//  input=0;
//  for(uint8_t i=0;i<8;i++)
//  {
//    GPIO_WriteLow(TOUCH_CLK_PORT,TOUCH_CLK_PIN);
//    delay_us(25);
//    GPIO_WriteHigh(TOUCH_CLK_PORT,TOUCH_CLK_PIN);
//    if(GPIO_ReadInputPin(TOUCH_DAT_PORT,TOUCH_DAT_PIN)!=FALSE)
//    {
//      input|=mask;
//    }
//    mask<<=1;
//    delay_us(25);
//  }
//  if(input&0x01) cnt++;
//  if(input&0x02) cnt++;
//  if(input&0x04) cnt++;
//  if(input&0x08) cnt++;
//  *data=input;
//  if(cnt !=((input>>4)&0x07))
//  {
//    delay_ms(10);
//    result=0;
//  }
//  else
//  {
//    if(input&0x80) 
//      result=1;
//    else 
//      result=0;
//  }
//  bs814a_init(); 
//  return result;
//}