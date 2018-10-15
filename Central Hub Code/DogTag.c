/*
Code for each of the dog tags
-----------------------------------------
EE459 - Embedded Systems Capstone Project

Team Name: Garmin - Team 4
Group Members: Ricardo Bazurto, Victor Fu, Chang Joon Kang, Nick Peng
Spring 2017


*/


#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/pgmspace.h>


#define fosc 7372800
#define BAUD 9600
#define UBBR fosc/16/BAUD - 1

void alarm_buttonpress(){

	//Flash LED
	int repeat = 0;

    while (repeat < 10){
    	PORTC |= 1 << PC0;      // Set PC0 to a 1  

		_delay_ms(100);
		int count = 0;
		while (count <100){
			PORTC |= 1 <<PC2;
			_delay_us(100);
			PORTC &= ~(1 << PC2);
			_delay_us(100);
			count++;
		}	
		PORTC &= ~(1 << PC0);   // Set PC0 to a 0
		_delay_ms(100);			
		repeat++;

	}
	
}

int main(void){

	DDRC |= 1 << DDC0;          // Set PORTC bit 0 for output
	PORTC &= ~(1<<PC0);			// Turn off LED
	DDRC &= ~(1 << PC1);		// Set button as input
	PORTC |= (0 << PC1);		// Initialization of button to high

	DDRC |= 1 << DDC2;
	PORTC &= ~(1<<PC2);

	 while (1) {
		
		//If alarm button is pressed
    	if (PINC & (1<<PC1)){
    		
			alarm_buttonpress();

    	} 







    }
	return 0;
}