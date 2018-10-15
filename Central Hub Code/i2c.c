/*
Code to implement I2C
-----------------------------------------
EE459 - Embedded Systems Capstone Project

Team Name: Garmin - Team 4
Group Members: Ricardo Bazurto, Victor Fu, Chang Joon Kang, Nick Peng
Spring 2017


*/

#include "i2c.h"


#define F_SCL 100000UL // SCL frequency
#define Prescaler 1
#define TWBR_val ((((F_CPU / F_SCL) / Prescaler) - 16 ) / 2)

void i2c_init(void)
{
    TWBR = (uint8_t)TWBR_val;
}

uint8_t i2c_start(uint8_t address)
{
    // reset TWI control register
    TWCR = 0;
    // transmit START condition 
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    // wait for end of transmission
    while( !(TWCR & (1<<TWINT)) );
    
    // check if the start condition was successfully transmitted
    if((TWSR & 0xF8) != TW_START){ return 1; }
    
    // load slave address into data register
    TWDR = address;
    // start transmission of address
    TWCR = (1<<TWINT) | (1<<TWEN);
    // wait for end of transmission
    while( !(TWCR & (1<<TWINT)) );
    
    // check if the device has acknowledged the READ / WRITE mode
    uint8_t twst = TW_STATUS & 0xF8;
    if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;
    
    return 0;
}

uint8_t i2c_write(uint8_t data)
{
    // load data into data register
    TWDR = data;
    // start transmission of data
    TWCR = (1<<TWINT) | (1<<TWEN);
    // wait for end of transmission
    while( !(TWCR & (1<<TWINT)) );
    
    if( (TWSR & 0xF8) != TW_MT_DATA_ACK ){ return 1; }
    
    return 0;
}

void i2c_stop(void) {
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
    while(TWCR & (1<<TWSTO));
}

unsigned char i2c_readAck(void)
{
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    while(!(TWCR & (1<<TWINT)));    

    return TWDR;

}

void i2c_start_wait(unsigned char address)
{
    uint8_t   twst;


    while ( 1 )
    {
        // send START condition
        TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    
        // wait until transmission completed
        while(!(TWCR & (1<<TWINT)));
    
        // check value of TWI Status Register. Mask prescaler bits.
        twst = TW_STATUS & 0xF8;
        if ( (twst != TW_START) && (twst != TW_REP_START)) continue;
    
        // send device address
        TWDR = address;
        TWCR = (1<<TWINT) | (1<<TWEN);
    
        // wail until transmission completed
        while(!(TWCR & (1<<TWINT)));
    
        // check value of TWI Status Register. Mask prescaler bits.
        twst = TW_STATUS & 0xF8;
        if ( (twst == TW_MT_SLA_NACK )||(twst ==TW_MR_DATA_NACK) ) 
        {           
            /* device busy, send stop condition to terminate write operation */
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
            
            // wait until stop condition is executed and bus released
            while(TWCR & (1<<TWSTO));
            
            continue;
        }
        //if( twst != TW_MT_SLA_ACK) return 1;
        break;
     }

}

unsigned char i2c_rep_start(unsigned char address)
{
    return i2c_start( address );

}

unsigned char i2c_readNak(void)
{
    TWCR = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
    
    return TWDR;

}
