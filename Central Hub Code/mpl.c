/*
Code for Altimeter Sensor
-----------------------------------------
EE459 - Embedded Systems Capstone Project

Team Name: Garmin - Team 4
Group Members: Ricardo Bazurto, Victor Fu, Chang Joon Kang, Nick Peng
Spring 2017

*/
#include "mpl.h" 
#include "i2c.h"

#define MPL3115a2 0xC0 
#define I2C_WRITE 0
#define I2C_READ 1

uint8_t altStatus;
float altitude;
float pressure;
float temperature;

void mpl_init (void) 
{ 
   alt_set_mode();
   alt_set_eventFlags(); 
} 

void alt_set_mode (void)
{
  i2c_start_wait(MPL3115a2+I2C_WRITE);
  i2c_write(CTRL_REG1);
  i2c_write(0xB9);
  i2c_stop();
}

void alt_set_eventFlags (void)
{
  i2c_start_wait(MPL3115a2+I2C_WRITE);
  i2c_write(PT_DATA_CFG);
  i2c_write(0x07);      
  i2c_stop();
}

void alt_set_active (void)
{
  i2c_start_wait(MPL3115a2+I2C_WRITE);
  i2c_write(CTRL_REG1);
  i2c_write(0xB9); 
  i2c_stop();
}
void alt_set_standby(void){
  i2c_start_wait(MPL3115a2+I2C_WRITE);
  i2c_write(CTRL_REG1);
  i2c_write(0xB8); 
  i2c_stop();
}

uint8_t alt_get_status (void) 
{ 
  uint8_t altStatus = 0x00; 
  
  while (((altStatus & 0x08) == 0))
  { 
    i2c_start_wait(MPL3115a2+I2C_WRITE); 
    i2c_write(STATUS); 
    i2c_rep_start(MPL3115a2+I2C_READ); 
    altStatus = i2c_readNak(); 
    i2c_stop(); 
    _delay_us(100);
   } 
   return altStatus;
} 


int8_t mpl_getTemp(){
   
  int8_t temp = 0;

   
   alt_set_active();
   
   alt_get_status(); 
    
   int8_t msbA,csbA,lsbA,msbT,lsbT = 0x00; 

       
   i2c_start_wait(MPL3115a2+I2C_WRITE); 
   i2c_write(OUT_P_MSB); 
   i2c_rep_start(MPL3115a2+I2C_READ); 
   //_delay_ms(10);
   msbA = i2c_readAck(); 
   csbA = i2c_readAck(); 
   lsbA = i2c_readAck();
   msbT = i2c_readAck();
   lsbT = i2c_readNak(); 
   i2c_stop(); 
   

if(msbT > 0x7F) 
  {
    temp = ~(msbT << 8 | lsbT) + 1 ; // 2's complement
    temperature = (float) (temp >> 8) + (float)((lsbT >> 4)/16.0); // add whole and fractional degrees Centigrade
    temperature *= -1.;
  }
else 
  {
    temperature = (float) (msbT) + (float)((lsbT >> 4)/16.0); // add whole and fractional degrees Centigrade
  }
    
  
   
   return temp;
   
     
}


float mpl_getAlt () 
{    
   long temp = 0;

   
   alt_set_active();
   
   alt_get_status(); 
    
   int8_t msbA,csbA,lsbA,msbT,lsbT = 0x00; 

       
   i2c_start_wait(MPL3115a2+I2C_WRITE); 
   i2c_write(OUT_P_MSB); 
   i2c_rep_start(MPL3115a2+I2C_READ); 
   //_delay_ms(10);
   msbA = i2c_readAck(); 
   csbA = i2c_readAck(); 
   lsbA = i2c_readAck();
   msbT = i2c_readAck();
   lsbT = i2c_readNak(); 
   i2c_stop(); 
   
    if(msbA > 0x7F) 
  {
      temp = ~((long)msbA << 16 | (long)csbA << 8 | (long)lsbA) + 1; // 2's complement the data
      altitude = (float) (temp >> 8) + (float) ((lsbA >> 4)/16.0); // Whole number plus fraction altitude in meters for negative altitude
      altitude *= -1.;
    }
    else 
  {
      temp = ((msbA << 8) | csbA);
      altitude = (float) (temp) + (float) ((lsbA >> 4)/16.0);  // Whole number plus fraction altitude in meters
    }
  
  long pressure_whole =  ((long)msbA << 16 | (long)csbA << 8 | (long)lsbA) ; // Construct whole number pressure
  pressure_whole >>= 6;
  
  lsbA &= 0x30;
  lsbA >>= 4;
  float pressure_frac = (float) lsbA/4.0;

  pressure = (float) (pressure_whole) + pressure_frac;

if(msbT > 0x7F) 
  {
    temp = ~(msbT << 8 | lsbT) + 1 ; // 2's complement
    temperature = (float) (temp >> 8) + (float)((lsbT >> 4)/16.0); // add whole and fractional degrees Centigrade
    temperature *= -1.;
  }
else 
  {
    temperature = (float) (msbT) + (float)((lsbT >> 4)/16.0); // add whole and fractional degrees Centigrade
  }
    
   if (temperature < 20) 
   { 
       PINB = 0x30;
   } 
   
   return altitude;
} 