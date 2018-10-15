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

unsigned char gpstime[9], latitude[12], longitude[11];
unsigned char direction1[2], direction2[2], altitude[5], west;
unsigned  char sat_num[2], DOP[5];
unsigned char gpsdata[20];
unsigned char time[];

//Initialize the USART port
void serial_init(unsigned short ubrr){
	UBRR0 = ubrr;		//set buad rate
	UCSR0B |= (1 << TXEN0);		//Turn on transmitter
	UCSR0B |= (1 << RXEN0);		//Turn on receiver
	UCSR0C = (3 << UCSZ00);		//Set for async. operation
}

//Output a byte to the USART0 port
void serial_out(char ch){

	while ((UCSR0A & (1<<UDRE0)) == 0);
	UDR0 = ch;

}

//Read a byte from the USART0 and return it
char serial_in(void){
	while ( !(UCSR0A & (1 << RXC0)));
	
	return UDR0;
}

void stringout(char* str){

	int i = 0;
	for (i=0; i<strlen(str); i++){
		serial_out(str[i]);
	}

}

void arrayout(int* arr){

	int i = 0;
	for (i=0; i<10; i++){
		serial_out(arr[i]);
	}

}

char readGPS(){

	char gpsdata = serial_in();
	return gpsdata;
}

void get_gpsdata(){

	char gps_temp = readGPS();
	int GPGGA = 1;
	
	memset(gpstime,0,sizeof(gpstime));
	

	while (GPGGA){
		if (gps_temp == '$'){
			gps_temp = readGPS();
			if (gps_temp == 'G'){
				gps_temp = readGPS();
				if (gps_temp == 'P'){
					gps_temp = readGPS();
					if (gps_temp == 'G'){
						gps_temp = readGPS();
						if (gps_temp == 'G'){
							gps_temp = readGPS();
							if (gps_temp == 'A'){
								gps_temp = readGPS();

								if (gps_temp == ','){
									int i=1;
									int count = 0;
									//store time
									gps_temp = readGPS();
									gpstime[0] = gps_temp;
									

									while(gps_temp != ','){
										
										gps_temp = readGPS();
										if (gps_temp == ','){
											gpstime[i] = '\0';
										}
										else{
											gpstime[i] = gps_temp;

											i++;
										}
									}
									
									//store latitude
									gps_temp = readGPS();
									latitude[0] = gps_temp;
									gpsdata[count] = gps_temp;
									count++;
									i=1;
									while(gps_temp != ','){
										
										gps_temp = readGPS();

										if (gps_temp == ','){
											//latitude[i] = '\0';
										}
										else{

											latitude[i] = gps_temp;
											gpsdata[count] = gps_temp;
											count++; 
											i++;
											
										}
									}

									//store N or S
									gps_temp = readGPS();
									direction1[0] = gps_temp;
									direction1[1] = '\0';
									//skip comma
									gps_temp = readGPS();
									gpsdata[count] = gps_temp;
									count++;

									//store longitude
									gps_temp = readGPS();
									longitude[0] = gps_temp;
									i = 1;
									while(gps_temp != ','){
										
										gps_temp = readGPS();
										if (gps_temp == ','){
											longitude[i] = '\0';
										}
										else{
											longitude[i] = gps_temp;
											gpsdata[count] = gps_temp;
											count++; 
											i++;
										}
									}	
									
									//store E or W
									gps_temp = readGPS();
									
									direction2[0] = gps_temp;
									direction2[1] = '\0';
									//jump 
									gps_temp = readGPS();gps_temp = readGPS();gps_temp = readGPS();

									//store # of satellites
									gps_temp = readGPS();
									sat_num[0] = gps_temp;
									gps_temp = readGPS();
									sat_num[1] = gps_temp;
									
									gps_temp = readGPS();

									//HDOP
									gps_temp = readGPS();
									DOP[0] = gps_temp;
									i = 1;
									while(gps_temp != ','){
										
										gps_temp = readGPS();
										DOP[i] = gps_temp;
										i++;
									}	
									DOP[i] = '\0';

									//store altitude
									gps_temp = readGPS();
									altitude[0] = gps_temp;
										
									i = 1;
									while(gps_temp != ','){
										
										gps_temp = readGPS();
										if (gps_temp != ','){
											altitude[i] = gps_temp;
											i++;

										}
										else{
											altitude[i] = '\0';
										}

										
									}	
									


									GPGGA = 0;
								}								
							}
						}
					}
				}
			}
		}

		GPGGA = 0;
	}

}

void alarm_buttonpress(){

	//Flash LED
	int repeat = 0;

    while (repeat < 10){
    	PORTC |= 1 << PC0;      // Set PC0 to a 1  

		_delay_ms(100);
		
		PORTC &= ~(1 << PC2);	//Turn on buzzer
		PORTC |= 1 << PC3;		//Send 
	
		PORTC &= ~(1 << PC0);   // Set PC0 to a 0
		_delay_ms(100);			
		repeat++;

	}
	PORTC |= (1<<PC2);
	PORTC &= ~(1<<PC3);
	
}

int main(void){

	DDRC |= 1 << DDC0;          // Set PORTC bit 0 for output
	PORTC &= ~(1<<PC0);			// Turn off LED
	DDRC &= ~(1 << PC1);		// Set button as input
	PORTC &= ~(0 << PC1);		// Initialization of button to high

			
	DDRC |= 1 << DDC2;			//set output for buzzer
	PORTC |= (1<<PC2);	

	DDRC |= (1 << DDC3);		//Set output to MCU
	PORTC &= ~(1 << PC3);

	serial_init(UBBR);

	 while (1) {

		//If alarm button is pressed
    	if (PINC & (1<<PC1)){
    		
			alarm_buttonpress();


    	} 

    	get_gpsdata();
    	stringout("34.025000N, 118.054400W");


    }
	return 0;
}