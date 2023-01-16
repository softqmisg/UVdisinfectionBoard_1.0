#include "sevenseg.h"
#include <stdio.h>

char string_7seg[9];
uint8_t string_index=0;
uint8_t seg = DIGIT1; 
bool screen_blink_state=FALSE,screen_blink=FALSE;
uint16_t blink_counter=0;

/*=============================================================================*/
void sevenseg_init()
{
  for(uint8_t i=0;i<9;i++) string_7seg[i]=0;
}
/*=============================================================================*/
void Write_7seg(uint8_t x)
{
  uint8_t mask[8]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
  GPIO_WriteLow(SEG_CLK_PORT,SEG_CLK_PIN);
  for(uint8_t i=0;i<8;i++)
  {
    if(mask[i]&x)
      GPIO_WriteHigh(SEG_DAT_PORT,SEG_DAT_PIN);
    else
      GPIO_WriteLow(SEG_DAT_PORT,SEG_DAT_PIN);
    GPIO_WriteLow(SEG_CLK_PORT,SEG_CLK_PIN);
    GPIO_WriteHigh(SEG_CLK_PORT,SEG_CLK_PIN);
  }
}
/*=============================================================================*/
///because of routing of segment connected to 164D, the value send to SHOW_SEG different the A to G
// it is custom as:
// QA=G, QB=C,QC=DP, QD=D, QE=E, QF=A,QG=F, QH=B//
//     ==A==
//    |     |
//    F      B
//    |     |
//    |==G==
//    |     |
//    E     C
//    |     |
//     ==D==    .Dp
void SHOW_SEG(char x,uint8_t dp)
{
  uint8_t data;

  switch((char) x)
  {
  case '0':
    data=5;
    break;
  case '1':
    data=125;
    break;
  case '2':
    data=70;
    break;
  case '3':
    data=84;
    break;
  case '4':
    data=60;
    break;
  case '5':
    data=148;
    break;
  case '6':
    data=132;
    break;
  case '7':
    data=93;
    break;
  case '8':
    data=4;
    break;
  case '9':
    data=20;
    break;	
  case 'S':
    data=148;//0x76;
    break;
  case 'A':
    data=12;//0x76;
    break;
  case 'P':
    data=14;//0x76;
    break;
  case 'L':
    data=167;//0x76;
    break;
  case 'D':
    data=100;//0x76;
    break;
  case 'N':
    data=236;//0x76;
    break;
  case 'H':
    data=44;
    break;
  case 'U':
    data=37;
    break;
  case 'M':
    data=204;
    break;
  case 'I':
    data=175;
    break;
  case 'T':
    data=166;
    break;
  case 'E':
    data=134;
    break;
  case 'R':
    data=238;
    break;
  case 'O':
    data=228;
    break;
  case 'F':
    data=142;
    break;
  case 'C':
    data=135;
    break;
  case '-':
    data=254;
    break;
  case '\'':
    data=127;
    break;
  case '\"':
    data=63;
    break;                
  default:
    data=255;
    break;
  }
   if(dp) data&=0xfb;   
//#ifdef CC_SEGMEMNT
//  if(dp) data&=0x7F;
//#endif
  Write_7seg(data);
}
/*=============================================================================*/
void Blink_7seg(bool state)
{
  screen_blink=state;
}
/*=============================================================================*/
void print_7SEG(char *string)
{
  //TIM1_Cmd(DISABLE);
//  for(  i=0;(string[i]!=\0')&&(3-i<=dotp)  ;i++)
//      string_7seg[i]=string[i];
//  string_7seg[i]='.';
//  while(string[i]!=\0') 
//        string_7seg[i]=string[i];
  sprintf(string_7seg,"%s",string);
  //TIM1_Cmd(ENABLE);
}
/*=============================================================================*/


/*=============================================================================*/
//evey  2.5ms, blinking 400ms
extern volatile uint8_t key_timercounter[4];
extern uint8_t key_timerstart[4];

#pragma vector =TIM1_OVR_UIF_vector 
__interrupt void TIM1_UPD_OVF_BRK_IRQHandler(void)
//void TIM1_UPD_OVF_BRK_IRQHandler()
{

  //	///////////////////////////KEY STATE///////////////////////
  for(uint8_t i=0;i<4;i++)
  {
    if(key_timerstart[i])
      key_timercounter[i]++;
  }
  ///////////////////////////////////segments/////////s///////////////////////////////////////////
  //    off  previous digit
  
  switch(seg)
  {
  case 2:
    GPIO_WriteHigh(DIGIT1_PORT,DIGIT1_PIN);	
    break;
  case 3:
    GPIO_WriteHigh(DIGIT2_PORT,DIGIT2_PIN);	
    break;
  case 4:
    GPIO_WriteHigh(DIGIT3_PORT,DIGIT3_PIN);	
    break;
  case 1:
    GPIO_WriteHigh(DIGIT4_PORT,DIGIT4_PIN);	
    break;
  }
  //show digits
  blink_counter++;
  if((blink_counter>BLINK_ON_DELAY) && (screen_blink_state==FALSE))
  {
    blink_counter=0;
    screen_blink_state=TRUE;
  }
  else if(blink_counter>BLINK_OFF_DELAY && (screen_blink_state==TRUE))
  {
    blink_counter=0;
    screen_blink_state=FALSE;    
  }
    
  if((screen_blink_state==FALSE && screen_blink==TRUE )|| (screen_blink==FALSE))
  {
    if(string_7seg[string_index+1]=='.')
    {
      SHOW_SEG(string_7seg[string_index],TRUE);
      string_index++;
    }
    else
    {
      SHOW_SEG(string_7seg[string_index],FALSE);
    }

    switch(seg)
    {
    case 1:
      GPIO_WriteLow(DIGIT1_PORT,DIGIT1_PIN);	
      break;
    case 2:
      GPIO_WriteLow(DIGIT2_PORT,DIGIT2_PIN);	
      break;
    case 3:
      GPIO_WriteLow(DIGIT3_PORT,DIGIT3_PIN);	
      break;
    case 4:
      GPIO_WriteLow(DIGIT4_PORT,DIGIT4_PIN);	
      break;
    } 
    seg++;
    if(seg==DIGITALL) seg=DIGIT1;     
    string_index++;
    if(string_index>8 || (string_7seg[string_index]=='\0')) 
    {
      string_index=0;
      seg=DIGIT1;
    }
  }
  else
  {
    string_index=0;
    seg=DIGIT1;
  }
  //////////////////////////////////////////////////////////////////////////////
  TIM1_ClearFlag(TIM1_FLAG_UPDATE);
}
/////////////////////////////////////////////////////////////////////////
//void num2str(int16_t data, uint8_t d_digit,uint8_t line)
//{
//  char string[3]={' ',' ',' '};
//  char minus=0;
//  if(line==ROW1)
//  {
//    if(data<0) 
//    {
//      minus=1;
//      data=-data;
//    }
//    
//    string[2]=(char)((uint16_t)data%10)+'0';
//    data=data/10;
//    string[1]=(char)(data%10)+'0';
//    
//    data=data/10;
//    if(data==0) 
//      string[0]=' ';	
//    else
//      string[0]=(char)data+'0';	
//    
//    if(minus) string[0]='-';
//  }
//  if(line==ROW2)
//  {
//    string[1]=(char)((uint16_t)data%10)+'0';
//    data=data/10;
//    string[0]=(char)(data%10)+'0';
//  }
//  print_7SEG(string,d_digit,line);	
//  
//}
