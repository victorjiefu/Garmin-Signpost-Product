/*
EE459 - Embedded Systems Capstone Project

Team Name: Garmin - Team 4
Group Members: Ricardo Bazurto, Victor Fu, Chang Joon Kang, Nick Peng
Spring 2017

*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>
#include "gps.h"
#include <stdint.h>
#include "xbee.h"

#define fosc 7372800
#define BAUD 9600
#define UBBR fosc/16/BAUD - 1


unsigned char gpstime[9], latitude[12], longitude[11];
unsigned char direction1[2], direction2[2], altitude[5], west;
unsigned  char sat_num[2], DOP[5];
unsigned char time[];

unsigned char testlat[] = "34.012498 N";
unsigned char testlong[] = "118.174443 W";

/*
  The NIBBLE_HIGH condition determines whether data bits 4-7 or 0-3 are used
  to send the four bit nibble to the LCD.
  If NIBBLE_HIGH is declared, use data bits 4-7.
  If NIBBLE_HIGH not declared, use data bits 0-3.
*/
#define NIBBLE_HIGH                 // Use bits 4-7 for talking to LCD

void initialize(void);
void strout(int, unsigned char *);
void cmdout(unsigned char, unsigned char);
void datout(unsigned char);
void nibout(unsigned char, unsigned char);
void busywt(void);

/*
  Use the "PROGMEM" attribute to store the strings in the ROM
  insteat of in RAM.
#ifdef NIBBLE_HIGH
const unsigned char str1[] PROGMEM = ">> at328-5.c hi <<901234";
#else
const unsigned char str1[] PROGMEM = ">> at328-5.c lo <<901234";
#endif
const unsigned char str2[] PROGMEM = ">> USC EE459L <<78901234";
*/

#define LCD_RS          0b00000010
#define LCD_RW          0b10000000
#define LCD_E           0b00000001
#define LCD_Bits        (LCD_RS|LCD_RW|LCD_E)

#ifdef NIBBLE_HIGH
#define LCD_Data_D     0xf0     // Bits in Port D for LCD data
#define LCD_Status     0x80     // Bit in Port D for LCD busy status
#else
#define LCD_Data_B     0x03     // Bits in Port B for LCD data
#define LCD_Data_D     0x0c     // Bits in Port D for LCD data
#define LCD_Status     0x08     // Bit in Port D for LCD busy status
#endif

#define WAIT           1
#define NOWAIT         0

/*
  strout - Print the contents of the character string "s" starting at LCD
  RAM location "x".  The string must be terminated by a zero byte.
*/
void strout(int x, unsigned char *s)
{
    unsigned char ch;

    cmdout(x | 0x80, WAIT);     // Make A contain a Set Display Address command

    /* Use the "pgm_read_byte()" routine to read the date from ROM */
    while ((ch = *s++) != (unsigned char) '\0') {
        datout(ch);             // Output the next character
    }
}

/*
  datout - Output a byte to the LCD display data register (the display)
  and wait for the busy flag to reset.
*/
void datout(unsigned char x)
{
    PORTB &= ~(LCD_RW|LCD_E);   // Set R/W=0, E=0, RS=1
    PORTB |= LCD_RS;
    nibout(x, NOWAIT);
    nibout(x << 4, WAIT);
}

/*
  cmdout - Output a byte to the LCD display instruction register.  If
  "wait" is non-zero, wait for the busy flag to reset before returning.
  If "wait" is zero, return immediately since the BUSY flag isn't
  working during initialization.
*/
void cmdout(unsigned char x, unsigned char wait)
{
    PORTB &= ~(LCD_RW | LCD_E | LCD_RS);         // Set R/W=0, E=0, RS=0
    nibout(x, NOWAIT);
    nibout(x << 4, wait);
}

/*
  nibout - Puts bits 4-7 from x into the four bits that we're
  using to talk to the LCD.  The other bits of the port are unchanged.
  Toggle the E control line low-high-low.
*/
void nibout(unsigned char x, unsigned char wait)
{
#ifdef NIBBLE_HIGH
    PORTD |= (x & LCD_Data_D);  // Put high 4 bits of data in PORTD
    PORTD &= (x | ~LCD_Data_D);
    PORTD &= (x | ~LCD_Data_D);
#else
    PORTB |= (x & LCD_Data_B);  // Put low 2 bits of data in PORTB
    PORTB &= (x | ~LCD_Data_B);
    PORTD |= (x & LCD_Data_D);  // Put high 2 bits of data in PORTD
    PORTD &= (x | ~LCD_Data_D);
#endif
    PORTB |= LCD_E;             // Set E to 1
    PORTB &= ~LCD_E;            // Set E to 0
    if (wait)
        busywt();               // Wait for BUSY flag to reset
}

/*
  initialize - Do various things to force a initialization of the LCD
  display by instructions, and then set up the display parameters and
  turn the display on.
*/
void initialize()
{
    _delay_ms(15);              // Delay at least 15ms

    nibout(0x30, NOWAIT);       // Send a 0x30
    _delay_ms(4);               // Delay at least 4msec

    nibout(0x30, NOWAIT);       // Send a 0x30
    _delay_us(120);             // Delay at least 100usec

    nibout(0x30, NOWAIT);       // Send a 0x30

    nibout(0x20, WAIT);         // Function Set: 4-bit interface
    
    cmdout(0x28, WAIT);         // Function Set: 4-bit interface, 2 lines

    cmdout(0x0c, WAIT);         // Display and cursor off
}

/*
  busywt - Wait for the BUSY flag to reset
*/
void busywt()
{
    unsigned char bf;

#ifdef NIBBLE_HIGH
    PORTD &= ~LCD_Data_D;       // Set for no pull ups
    DDRD &= ~LCD_Data_D;        // Set for input
#else
    PORTB &= ~LCD_Data_B;       // Set for no pull ups
    PORTD &= ~LCD_Data_D;
    DDRB &= ~LCD_Data_B;        // Set for input
    DDRD &= ~LCD_Data_D;
#endif

	PORTB &= ~(LCD_E | LCD_RS);
    PORTB |= LCD_RW;   // Set E=0, R/W=1, RS=0

    do {
        PORTB |= LCD_E;         // Set E=1
        _delay_us(1);           // Wait for signal to appear
        bf = PIND & LCD_Status; // Read status register high bits
        PORTB &= ~(LCD_E);        // Set E=0
				PORTB |= LCD_E;         // Need to clock E a second time to fake
				PORTB &= ~(LCD_E);        //   getting the status register low bits
    } while (bf != 0);          // If Busy (PORTD, bit 7 = 1), loop

#ifdef NIBBLE_HIGH
    DDRD |= LCD_Data_D;         // Set PORTD bits for output
#else
    DDRB |= LCD_Data_B;         // Set PORTB, PORTD bits for output
    DDRD |= LCD_Data_D;
#endif
}

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
	//clear all array buffers
	//memset(latitude,0,sizeof (latitude));
	//memset(longitude,0,sizeof(longitude));
	
	//memset(direction1,0,2);
	//memset(direction2,0,2);
	memset(gpstime,0,sizeof(gpstime));
	/*
	strcpy(latitude, "0");
	strcpy(longitude, "0");
	strcpy(direction1, "N");
	strcpy(direction2, "W");*/

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
									i=1;
									while(gps_temp != ','){
										
										gps_temp = readGPS();

										if (gps_temp == ','){
											//latitude[i] = '\0';
										}
										else{

											latitude[i] = gps_temp;
											i++;
											
										}
									}

									//store N or S
									gps_temp = readGPS();
									direction1[0] = gps_temp;
									direction1[1] = '\0';
									//skip comma
									gps_temp = readGPS();

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



void alarm_buttonpress(bool flag){

	//Flash LED
	int repeat = 0;
    while (flag){
    	PORTC |= 1 << PC0;      // Set PC0 to a 1  
    	PORTD &= ~(1<<PD3);  
		_delay_ms(50);
			
		PORTC &= ~(1 << PC0);   // Set PC0 to a 0
		_delay_ms(50);			
		repeat++;

		if ((PINC & (1<<PC2))||(PINC & (1<<PC3))){
			flag = false;
			PORTD |= 1 << PD3;
			break;
			
		}


	}

	
}





int main(void){

	DDRC |= 1 << DDC0;          // Set PORTC bit 0 for output
	PORTC &= ~(1<<PC0);			// Turn off LED
	
	DDRC &= ~(1 << PC1);		// Set button as input
	PORTC |= (0 << PC1);		// Initialization of button to low
	
	DDRD |= 1 << DDD3;			//Set buzzer as output
	PORTD |= 1 << PD3;			//Set to high as off

	DDRD &= ~(1<<PD6); 			//Set pin as input from other MCU
	PORTD |= (0 << PD6);

	DDRD |= 1 << DDD7;
	PORTD &= ~(0<<PD7);

	// Set up Pin Interrupts for button debounce
	PCMSK0 |= (1 << PCINT11);	
	PCMSK1 |= (1 << PCINT12);
	
	int menu_select=0; // Variable used for menu selection
	
	serial_init(UBBR);
	char* letter = "Start\n";	//Start of serial out
    stringout(letter);
	

	#ifdef NIBBLE_HIGH
        DDRD |= LCD_Data_D;         // Set PORTD bits 4-7 for output
	#else
		DDRB |= LCD_Data_B;         // Set PORTB bits 0-1 for output
		DDRD |= LCD_Data_D;         // Set PORTD bits 2-3 for output
	#endif

    DDRB |= LCD_Bits;           // Set PORTB bits 2, 3 and 4 for output

    initialize();               // Initialize the LCD display
    cmdout(0x01, WAIT);			//Clear LCD
    
    bool alarmflag = false;
   

    while (1) {
    	_delay_ms(100);
    	
    	//If alarm button is pressed
    	if ((PINC & (1<<PC1))){
			
			//Clears LCD
    		strout(0x00, "                    ");
			strout(0x40, "                    "); // Print string on line 2
			strout(0x14, "                    ");
			strout(0x54, "                    ");
			//End Clear
			if (alarmflag==false){
				alarm_buttonpress(false);
				menu_select=5;

			}
			else{
				menu_select = 0;
				alarmflag = false;
			}
			
    	}
    	else if ((PIND & (1<<PD6))){

    		//Clears LCD
    		strout(0x00, "                    ");
			strout(0x40, "                    "); // Print string on line 2
			strout(0x14, "                    ");
			strout(0x54, "                    ");

			strout(0x00, "       WARNING:     ");    // Print string on line 2
			strout(0x14, "Tag 1 is in trouble!");    // Print string on line 1
			if (alarmflag==false){
				alarm_buttonpress(false);
				menu_select=5;

			}
			else{
				menu_select = 0;
				alarmflag = false;
			}

    	}
		//Right Button Function
    	else if (PINC & (1<<PC2)){
			_delay_ms(100);
			//Clears LCD
			//cmdout(0x01, WAIT);	
    		strout(0x00, "                    ");
			strout(0x40, "                    "); // Print string on line 2
			strout(0x14, "                    ");
			strout(0x54, "                    ");
			//End Clear
			if(menu_select==4){
				menu_select=0;
			}
			else if(menu_select==5){
				menu_select=0;
			}
			else{
				menu_select++;
			}
    	}
		//Left Button Function
    	else if (PINC & (1<<PC3)){
			_delay_ms(100);			
			//Clears LCD
			cmdout(0x01, WAIT);	
    		strout(0x00, "                    ");
			strout(0x40, "                    "); // Print string on line 2
			strout(0x14, "                    ");
			strout(0x54, "                    ");
			//End Clear
			if(menu_select==0){
				menu_select=4;
			}
			else if(menu_select==5){
				menu_select=0;
			}
			else{
				menu_select--;
			}
    	}		
    	//retrive own gps data
    	get_gpsdata();

    	unsigned char* timeout = converttime(gpstime);
    	
    	unsigned char latbuf[10];
    	unsigned char longbuf[11];

    	unsigned char* newlat = convertlat(latitude);
    	
		//memset(latbuf, 0 , 9);
		strcpy(latbuf, newlat);
	
    	
    	unsigned char* newlong = convertlong(longitude);
    	//memset(longbuf, 0, 10);
    	strcpy(longbuf, newlong);
    	
    	/*int p = 0;
    	unsigned char recdata[25];
    	while(p < 25){
    		recdata[p] = serial_in();
    		p++;
    	}
    	stringout(recdata);*/
    	

		if(menu_select==0){
    		strout(0x00, "Welcome to SignPost ");

    		unsigned char time[20] = "    ";
			strcat(time, timeout);
    		strout(0x40, time);    // Print string on line 1

			strout(0x54, "<-Tag Location GPS->"); // Print string on line 4
			
		}
		
		else if(menu_select==1){
			
			_delay_ms(100);

			unsigned char lat[20] = "Lat = ";
			
			if (latitude[0] == ','){
				strcat(lat, "NO FIX");
				
			}
			else{
				
				strcat(lat, newlat);
				strcat(lat, direction1);
			}
			
			unsigned char longi[20] = "Long = ";
			if (longitude[0] == ','){
				strcat(longi, "NO FIX");
				
			}
			else{
				
				strcat(longi, newlong);
				strcat(longi, "W");
			
			}
			if (sat_num[0] == ','){
				sat_num[0] = 'X';
				sat_num[1] = 'X';
			}

			unsigned char alt[20] = "Altitude = ";
			if (altitude[0] == ','){
				strcat(alt, "NO FIX");
			
			}
			else{
				strcat(alt, altitude);
				strcat(alt, " m");
			}
			unsigned char gpstitle[20] = "      GPS Data   ";
    		strcat(gpstitle, sat_num);
    		strout(0x0, gpstitle);

			strout(0x40, lat); // Print string on line 1

			strout(0x14, longi); // Print string on line 2

			strout(0x54, alt);
			_delay_ms(100);
		}
		else if(menu_select==2){
			_delay_ms(100);
			
			unsigned char dspalt[20] = "Altitude = ";
			strcat(dspalt, altitude);

    		strout(0x0,  "    Weather Info    ");
			
			strout(0x40, "Temperature =  72   ");    // Print string on line 1

			strout(0x14, "Pressure =          "); // Print string on line 2

			strout(0x54, dspalt); // Print string on line 3
		}
		else if(menu_select==4){
			
			_delay_ms(100);

			char distance[6];
			
			/*float l = 34.007878342;
			unsigned char test[10];
			dtostrf(l,10,10,test);
			stringout(test);*/

			int d;
			if (newlat[0] != 0){

				d = abs(calcdistance(newlat, newlong, testlat, testlong));
				itoa(d, distance, 6);
			
			}

			unsigned char dout[20];
			memset(dout,0,20);
			strcpy(dout, "Distance = ");


			
			strcat(dout, distance);
			strcat(dout, " m");	
			
			unsigned char dir[20] = "Direction = NW";
			strcat(dir, find_dir(newlat, newlong, testlat, testlong));

    		strout(0x0,  "   Tag Locations    ");
			
			strout(0x40, "      Tag 1       ");    // Print string on line 1

			strout(0x14, dout); // Print string on line 2

			strout(0x54, "Direction = "); // Print string on line 3
		}
		else if (menu_select == 3){


			unsigned char gpstitle[20] = "   Tag1 GPS Data    ";
			unsigned char lat[20] = "Lat = ";
			unsigned char longi[20] = "Long = ";

			PORTD |= 1 << PD7;

			/*int l=0;
			char readdata = serial_in();
			testlat[0] = readdata;
			l++;
			while (readdata != ','){
				
				readdata = serial_in();
				testlat[l] = readdata;
				l++;

			}
			l=0;
			readdata = serial_in();
			testlong[0] = readdata;
			l++;
			while (readdata != ','){
				
				readdata = serial_in();
				testlong[l] = readdata;
				l++;

			}*/
			
			strcat(lat, testlat);
			strcat(longi, testlong);

			strout(0x0, gpstitle);

			strout(0x14, lat); // Print string on line 1

			strout(0x54, longi); // Print string on line 2

			PORTD &= ~(1<<PD7);
		}
		else if(menu_select==5){
			_delay_ms(100);
			alarmflag = true;
    		strout(0x0,  "   SOS Triggered   ");
			
			strout(0x40, "Help is on the way! ");    // Print string on line 1

			strout(0x54, "<- Return to menu ->"); // Print string on line 3
			alarm_buttonpress(true);
			
			
		}

    }
	

	return 0;

}

