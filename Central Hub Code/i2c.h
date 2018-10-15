/*
Header file to implement I2C
-----------------------------------------
EE459 - Embedded Systems Capstone Project

Team Name: Garmin - Team 4
Group Members: Ricardo Bazurto, Victor Fu, Chang Joon Kang, Nick Peng
Spring 2017


*/
#ifndef i2c_h

#define i2c_h 


#include <stdint.h>
#include <util/twi.h>

#define SCL_CLOCK  100000L


void i2c_init(void);
uint8_t i2c_start(uint8_t address);
uint8_t i2c_write(uint8_t data);
void i2c_stop(void);
unsigned char i2c_readAck(void);
void i2c_start_wait(unsigned char address);
unsigned char i2c_rep_start(unsigned char address);
unsigned char i2c_readNak(void);

#endif