#include "onewire.h"
void delsyus_onewire(uint16_t us)
{
  do
  {
    __asm("nop");
    __asm("nop");
    __asm("nop");		
    __asm("nop");
    __asm("nop");
    __asm("nop");		
    __asm("nop");
    __asm("nop");		
    __asm("nop");
    __asm("nop");
    __asm("nop");		
    __asm("nop");
    __asm("nop");
    __asm("nop");
    us--;
  }while(us);
}
////////////////////////////////////////////////////////////
bool onewire_reset(void) 
{ 
  bool res = FALSE; 
  DS18B20_OUTPUT(); 
  DS18B20_OUT_LOW(); 
  delsyus_onewire(600); //480
  DS18B20_INPUT();	
  delsyus_onewire(70);
  res = DS18B20_IN(); 
  delsyus_onewire(600); //480
  return res; 
  
} 
////////////////////////////////////////////////////////////
void onewire_write(unsigned char value) 
{ 
  unsigned char s=0x01;
  disableInterrupts();
  DS18B20_INPUT();
  while(s)
  {
    DS18B20_OUTPUT(); 
    DS18B20_OUT_LOW(); 
    if(value&s) 
    { 
      //__asm("nop");
      delsyus_onewire(1);
      DS18B20_INPUT();
      delsyus_onewire(80);
    } 
    else
    {
      delsyus_onewire(80);//60 us<TX “0”<120 us 
      DS18B20_INPUT(); 
      delsyus_onewire(5);//1 us< tREC <...
    }
    s<<=1;
  }
  enableInterrupts();        

} 
////////////////////////////////////////////////////////////
unsigned char onewire_read(void) 
{ 
  uint8_t s=0x01;
  unsigned char value = 0x00; 
  disableInterrupts();
  while(s)
  {
    DS18B20_OUTPUT(); 
    DS18B20_OUT_LOW(); 
    delsyus_onewire(1);
    DS18B20_INPUT();
    if(DS18B20_IN())
      value|=s;
    
    s<<=1;		
    delsyus_onewire(60);
  }
  enableInterrupts();
  
  delsyus_onewire(50);
  return value;
}
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
//void delsyus_onewire(uint16_t us)
//{
//  if(us>255)
//  {
//    while(us)
//    {
//      delay_us(200);
//      us-=200;
//    }
//  }
//  else if(us<5)
//  {
//    delay_us(us);
//  }
//  else
//  {
//    delay_us(us)
//  }

//  us=(uint16_t)10*us;
//  while(us)
//  {
//    __asm("nop");
//    us--;
//  }
//}
//////////////////////////////////////////////////////////////
//bool onewire_reset(void) 
//{ 
//  bool res = FALSE; 
//  DS18B20_OUTPUT(); 
//  DS18B20_OUT_LOW(); 
//  delsyus_onewire(600); //480
//  DS18B20_INPUT();	
//  delsyus_onewire(100);
//  res = DS18B20_IN(); 
//  delsyus_onewire(600); //480
//  return res; 
//  
//} 
//////////////////////////////////////////////////////////////
//void onewire_write(unsigned char value) 
//{ 
//  unsigned char s=0x01;
//  DS18B20_INPUT();
//  while(s)
//  {
//    DS18B20_OUTPUT(); 
//    DS18B20_OUT_LOW(); 
//    if(value&s) 
//    { 
//      delsyus_onewire(5);
//      DS18B20_INPUT();
//      delsyus_onewire(80);
//    } 
//    else
//    {
//      delsyus_onewire(80);//60 us<TX “0”<120 us 
//      DS18B20_INPUT(); 
//      delsyus_onewire(2);//1 us< tREC <...
//    }
//    s<<=1;
//    
//  }
//} 
//////////////////////////////////////////////////////////////
//unsigned char onewire_read(void) 
//{ 
//  uint8_t s=0x01;
//  unsigned char value = 0x00; 
//  while(s)
//  {
//    DS18B20_OUTPUT(); 
//    DS18B20_OUT_LOW(); 
//    delsyus_onewire(2);
//    DS18B20_INPUT();
//    delsyus_onewire(1);
//    if(GPIO_ReadInputPin(DS18_NTC_PORT, DS18_NTC_PIN)!=FALSE)
//      value|=s;
//    s<<=1;		
//    delsyus_onewire(80);
//  }
//  return value;
//}
//////////////////////////////////////////////////////////////
