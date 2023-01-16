#include "onewire.h"
#include "ds18b20.h"

void DelayDs18b20_us(uint8_t us)
{
  delay_us(us);
  
}
void DelayDs18b20_ms(uint16_t ms)
{
  delay_ms(ms);
}
////////////////////////////////////////////////////////////
bool DS18B20_init(void) 
{ 
  bool r;
  uint8_t counter=0;
  DS18B20_INPUT(); 
  DelayDs18b20_us(10); //480	
  while((r=onewire_reset()) && counter<10) 
  {
    IWDG_RESET();
    onewire_write(skip_ROM);
    onewire_write(write_scratchpad);
    onewire_write(0x00);//TH
    onewire_write(0x00);//TL		
    onewire_write(0x7F);//config: 0 R1 R0 1 1 1 1 1 ,R1R0:00=>9bit,01=>10bit,10=>11bit,11=>12bit			
    DelayDs18b20_us(200);
    counter++;
  }
  if(r==0)
    return TRUE; 
  else
    return FALSE;
}
////////////////////////////////////////////////////////////
  uint8_t lsb,msb;
bool DS18B20_get_temperature(int16_t *temperature) 
{ 
  
  bool  minus = 0;
  uint16_t decimal;

  float temp;
  if(!onewire_reset())
  {	
    onewire_write(skip_ROM);
    onewire_write(convert_T); 
    
    DelayDs18b20_ms(800); 	
    
    if(!onewire_reset())
    {		
      onewire_write(skip_ROM); 
      onewire_write(read_scratchpad); 
      lsb=onewire_read();	
      DelayDs18b20_us(10);
      msb=onewire_read();
      decimal=((uint16_t)msb<<8);
      decimal+=(uint16_t)lsb;
      
      if(msb & 0x80)// negative
      {
        decimal=~(uint16_t)decimal+1;
        minus=1;
      }
  
     temp= (float)decimal*0.625;
      if (minus) 
        temp = 0.0 - (float)temp;
      *temperature=(int16_t)temp;
      return TRUE;// temp is multiplied by 10
    }
  }
  //	*temperature=HIGH_TEMPERATURE_OFF;
  return FALSE;
}