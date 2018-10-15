/*
Functions for GPS Module
-----------------------------------------
EE459 - Embedded Systems Capstone Project

Team Name: Garmin - Team 4
Group Members: Ricardo Bazurto, Victor Fu, Chang Joon Kang, Nick Peng
Spring 2017

*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

unsigned char gpsdata[20];
unsigned char newtime[13];
unsigned char nlat[9];
unsigned char nlong[12];
unsigned char dir[2];

void FloatToStringNew(char *str, float f, char size)

{

    char pos;  // position in string

    char len;  // length of decimal part of result

    char* curr;  // temp holder for next digit

    int value;  // decimal digit(s) to convert

    pos = 0;  // initialize pos, just to be sure

    value = (int)f;  // truncate the floating point number
    itoa(value,str,10);  // this is kinda dangerous depending on the length of str
    // now str array has the digits before the decimal

    if (f < 0 )  // handle negative numbers
    {
        f *= -1;
        value *= -1;
    }

     len = strlen(str);  // find out how big the integer part was
    pos = len;  // position the pointer to the end of the integer part
    str[pos++] = '.';  // add decimal point to string

    while(pos < (size + len + 1) )  // process remaining digits
    {
        f = f - (float)value;  // hack off the whole part of the number
        f *= 10;  // move next digit over
        value = (int)f;  // get next digit
        itoa(value, curr,10); // convert digit to string
        str[pos++] = *curr; // add digit to result string and increment pointer
    }
 }

unsigned char* converttime(unsigned char* gpstime){

	//memset(newtime,0,13);
	char hr[2];
	hr[0] = gpstime[0];
	hr[1] = gpstime[1];
	
	int hour = atoi(hr);
	hour -= 7;
	if (hour < 0){
		hour += 24;
	}


	unsigned char time[3]= "";
	itoa(hour,time,10);

	newtime[0] = gpstime[0];
	newtime[1] = gpstime[1];
	newtime[2] = ':';
	newtime[3] = gpstime[2];
	newtime[4] = gpstime[3];
	newtime[5] = ':';
	newtime[6] = gpstime[4];
	newtime[7] = gpstime[5];
	newtime[8] = ' ';
	newtime[9] = 'G';
	newtime[10] = 'M';
	newtime[11] = 'T';
	newtime[12] = '\0';

	return newtime;
}
unsigned char* convertlat(unsigned char* latitude){
	int count = 0;
	//memset(nlat, 0, 10);

	while (count < 9){
		nlat[count] = latitude[count];
		count++;
	}
	
	nlat[3] = latitude[2];
	nlat[4] = latitude[3];
	nlat[2] = '.';

	return nlat;
}
unsigned char* convertlong(unsigned char* longitude){

	int count = 0;
	//memset(nlong, 0, 12);
	while (count < 11){
		nlong[count] = longitude[count];
		count++;
	}
	
	nlong[4] = longitude[3];
	nlong[5] = longitude[4];
	nlong[3] = '.';
	
	return nlong;
}

float process_lat(unsigned char* coord){
	unsigned char degree[2], minutes[5];
	float loc = 0;
	degree[0] = coord[0];
	degree[1] = coord[1];
	
	int d = atoi(degree);

	minutes[0] = coord[3];
	minutes[1] = coord[4];
	minutes[2] = coord[5];
	minutes[3] = coord[6];
	minutes[4] = coord[7];
	minutes[5] = coord[8];
	//minutes[6] = '\0';
	int m = atoi(minutes)/10000.0;

	loc = (d*1.0 + m/60.0) *3.1415927/180.0;

	return loc;
}

float process_long(unsigned char* coord){
	
	
	unsigned char degree[3], minutes[5];
	float loc = 0;
	degree[0] = coord[0];
	degree[1] = coord[1];
	degree[2] = coord[2];

	int d = atoi(degree);

	minutes[0] = coord[4];
	minutes[1] = coord[5];
	minutes[2] = coord[6];
	minutes[3] = coord[7];
	minutes[4] = coord[8];
	minutes[5] = coord[9];

	int m = atoi(minutes)/10000.0;


	loc = (d + (m/60.0))*3.1415927/180.0;
	
	return loc;
}

//returns distance from tag in km
int calcdistance(unsigned char* lat1, unsigned char* lon1, unsigned char* lat2, unsigned char* lon2){

	float latnum1, lonnum1, latnum2, lonnum2;
	float a, d, dlat, dlong;


	latnum1 = process_lat(lat1);
	lonnum1 = process_long(lon1);
	latnum2 = process_lat(lat2);
	lonnum2 = process_long(lon2);
	
	dlat = (latnum2 - latnum1);
	dlong = (lonnum2 - lonnum1);

	unsigned char test[6];
	FloatToStringNew(test, latnum2, 6);
	//sprintf(test,"%f", lat1);
	//stringout (lat2);
	stringout(test);


	a = sin(dlat/2.0)*sin(dlat/2.0) + cos(latnum1)*cos(latnum2) *
		sin(dlong/2.0)*sin(dlong/2.0);

	d =  2.0 * 6371.0 * atan2(sqrt(a),sqrt(1.0-a));	

	return (int) d;

}

//find direction of tag
unsigned char* find_dir(unsigned char* lat1, unsigned char* lon1, unsigned char* lat2, unsigned char* lon2){

	int dlat = abs(lat2 - lat1);
	int dlong = abs(lon2 - lon1);

	
		if (lat1 > lat2){

			dir[0] = 'S';
			if  (lon2 > lon1){
				dir[1] = 'E';
			}
			else{
				dir[1] - 'W';
			}
		}
		else{
			dir[0] = 'N';
			if  (lon2 > lon1){
				dir[1] = 'E';
			}
			else{
				dir[1] - 'W';
			}
		}



	return dir;
}
