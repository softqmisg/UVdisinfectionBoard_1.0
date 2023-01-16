#include "eeprom.h"
extern void delay_ms(uint8_t ms);
void delayeeprom_ms(uint16_t ms)
{
  delay_ms(ms);
}
void eeprom_init(void)
{
  FLASH_DeInit();
}
void eeprom_write(uint16_t value, unsigned int add, uint8_t bytes)
{
  while(bytes)
  {
    FLASH_Unlock(FLASH_MEMTYPE_DATA);
    FLASH_EraseByte(add);
    delayeeprom_ms(20); 
    FLASH_ProgramByte(add, (uint8_t)(value&0xFF)); 
    delayeeprom_ms(20); 
    FLASH_Lock(FLASH_MEMTYPE_DATA);
    value>>=8;
    add++;
    bytes--;
  }
  
}
/*
void eeprom_read(uint8_t *bytes)
{
uint8_t i;
for(i=0;i<EEPROM_BYTES;i++)
{
*(bytes+i)=FLASH_ReadByte(BASEADDRESS+i);	
delay_ms(10); 
    }

}
*/
int16_t eeprom_read(unsigned int add, uint8_t bytes)
{
  uint8_t tmp;
  tmp=FLASH_ReadByte(add);
  if(bytes==1) return (int8_t) tmp;
  return (int16_t) (tmp|(FLASH_ReadByte(add+1)<<8));
}