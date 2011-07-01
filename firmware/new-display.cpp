#include "mbed.h"

#define _I2C_ADDRESS 0x04
#define _I2C_HOME 0xFE
#define _I2C_CLS 0xFF


I2C i2c(p9, p10);        // sda, scl
Serial pc(USBTX, USBRX); // tx, rx

void put(char *s)
{
  i2c.write(_I2C_ADDRESS, s++, strlen(s));
  return;
  while(*s)
  {
    i2c.write(_I2C_ADDRESS, s++, 1);
    //wait(0.01);
  }
}

void home()
{
  char c = _I2C_HOME;    
  i2c.write(_I2C_ADDRESS,&c, 1);      
}

void cls()
{
  char c = _I2C_CLS;    
  i2c.write(_I2C_ADDRESS,&c, 1);      
}


/**
*** Main function
**/
int main() {
    char c[2],d,key, result;
    char str[64];
    wait(0.5);
    
    i2c.frequency(100000);
    pc.baud(115200); 
    sprintf(str, "Hello world!");
    printf("Hello MBED!\n");
        
    while(1) 
    {
      wait(0.1);
      result = i2c.read(_I2C_ADDRESS,&key, 1);
      wait(0.1);
      
      
      sprintf(str, "\tHello World\r"); 
      put(str);    
  
      sprintf(str, "%4.4d: %4.4d, %1.1d", (int)d++, (int)key, (int)result); 
      wait(0.01);
      put(str);    
      printf(str);
      printf("\n\r");
      wait(1);
      cls();
    }
}
