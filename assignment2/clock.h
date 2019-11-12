#ifndef CLOCK_H
#define CLOCK_H

//clockInit: initialise clock
void clockInit(void);

//clockSetTime: sets current time to hours; minutes; seconds
void clockSetTime(int hours, int minutes, int seconds);

//clockSetAlarmTime: sets alarmtime to hours, minutes and seconds
void clockSetAlarmTime(int hours, int minutes, int seconds);

//Increments the time with one second
void incrementTime(void);

//Read time from clock variables
void getTime(int *hours, int *minutes, int *seconds);

/* getAlarmTime: read alarm time from common clock variables */
void getAlarmTime(int *hours, int *minutes, int *seconds);

/* check if time is ok*/
int timeOK(int hours, int minutes, int seconds);

/*possibility to reset the alarm*/
void resetAlarm();

/* check to se if alarm is enabled*/
int alarmEnabled();

#endif
