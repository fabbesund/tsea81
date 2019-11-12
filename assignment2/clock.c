#include "clock.h"

#include <pthread.h>
#include <semaphore.h>

//time data type
typedef struct
{
  int hours;
  int minutes;
  int seconds;
} timeDataType;

//clock data type
typedef struct
{
  //current time
  timeDataType time;
  //alarm time
  timeDataType alarmTime;
  //alarm enable flag
  int alarmEnabled;
  //semaphore for mutal exclusion
  pthread_mutex_t mutex;
  //semaphore for alarm activation
  //sem_t startAlarm;
} clockDataType;

// the actual clock
static clockDataType Clock;

//clock initialise clock
void clockInit(void)
{
  //initialise time and alarmTime
  Clock.time.hours = 0;
  Clock.time.minutes = 0;
  Clock.time.seconds = 0;

  Clock.alarmTime.hours = 0;
  Clock.alarmTime.minutes = 0;
  Clock.alarmTime.seconds = 0;

  //alarm is nod enabled
  Clock.alarmEnabled = 0;

  //initialise semaphores
  pthread_mutex_init(&Clock.mutex, NULL);
  //sem_init(&Clock.startAlarm, 0, 0);
}

//clockSetTime: set current time to hours minutes and seconds
void clockSetTime(int hours, int minutes, int seconds)
{
  pthread_mutex_lock(&Clock.mutex);
  Clock.time.hours = hours;
  Clock.time.minutes = minutes;
  Clock.time.seconds = seconds;
  pthread_mutex_unlock(&Clock.mutex);
}

//clockSetAlarmTime sets the alarmtime in hours minutes and seconds
void clockSetAlarmTime(int hours, int minutes, int seconds)
{
  pthread_mutex_lock(&Clock.mutex);
  Clock.alarmTime.hours = hours;
  Clock.alarmTime.minutes = minutes;
  Clock.alarmTime.seconds = seconds;
  Clock.alarmEnabled = 1;
  pthread_mutex_unlock(&Clock.mutex);
}

//Increment time: increments the current time with one second
void incrementTime(void)
{
    /* reserve clock variables */
    pthread_mutex_lock(&Clock.mutex);

    /* increment time */
    Clock.time.seconds++;
    if (Clock.time.seconds > 59)
    {
        Clock.time.seconds = 0;
        Clock.time.minutes++;
        if (Clock.time.minutes > 59)
        {
            Clock.time.minutes = 0;
            Clock.time.hours++;
            if (Clock.time.hours > 12)
            {
                Clock.time.hours = 1;
            }
        }
    }

    /* release clock variables */
    pthread_mutex_unlock(&Clock.mutex);
}

/* getTime: read time from common clock variables */
void getTime(int *hours, int *minutes, int *seconds)
{
    /* reserve clock variables */
    pthread_mutex_lock(&Clock.mutex);

    /* read values */
    *hours = Clock.time.hours;
    *minutes = Clock.time.minutes;
    *seconds = Clock.time.seconds;

    /* release clock variables */
    pthread_mutex_unlock(&Clock.mutex);
}

/* getAlarmTime: read alarm time from common clock variables */
void getAlarmTime(int *hours, int *minutes, int *seconds)
{
    /* reserve clock variables */
    pthread_mutex_lock(&Clock.mutex);

    /* read values */
    *hours = Clock.alarmTime.hours;
    *minutes = Clock.alarmTime.minutes;
    *seconds = Clock.alarmTime.seconds;

    /* release clock variables */
    pthread_mutex_unlock(&Clock.mutex);
}

/* check if time is ok*/
int timeOK(int hours, int minutes, int seconds)
{
  return hours >= 1 && hours <= 12 && minutes >= 0 && minutes <= 59 &&
  seconds <= 59 && seconds >= 0;
}

void resetAlarm()
{
  Clock.alarmTime.hours = 0;
  Clock.alarmTime.minutes = 0;
  Clock.alarmTime.seconds = 0;

  //alarm is not enabled
  Clock.alarmEnabled = 0;
}

int alarmEnabled()
{
  return Clock.alarmEnabled;
}














//
