
/*
Header File for GPS
-----------------------------------------
EE459 - Embedded Systems Capstone Project

Team Name: Garmin - Team 4
Group Members: Ricardo Bazurto, Victor Fu, Chang Joon Kang, Nick Peng
Spring 2017


*/
#ifndef gps.c
#define gps.c

unsigned char* converttime();
unsigned char* convertlat();
unsigned char* convertlong();

float process_lat(unsigned char lat);
float process_long(unsigned char coord);
int calcdistance(unsigned char* lat1, unsigned char* lon1, unsigned char* lat2, unsigned char* lon2);

#endif