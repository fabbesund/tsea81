/* A clock program with the possibility to
activate an alarm  */

//standard library includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

//file includes
#include "display.h"
#include "clock.h"
#include "si_ui.h"

pthread_mutex_t Mutex;
sem_t startAlarm;
struct timespec ts;
struct timespec alarmts;

/* time_from_set_message: extract time from set message from user interface */
void timeFromSetMessage(char message[], int *hours, int *minutes, int *seconds)
{
    sscanf(message,"set:%d:%d:%d",hours, minutes, seconds);
}

void alarmTimeFromSetMessage(char message[], int *hours, int *minutes, int * seconds)
{
  sscanf(message, "alarm:%d:%d:%d", hours, minutes, seconds);
}

/* clock_task: clock task */
void *clockThread(void *unused)
{
    /* local copies of the current time */
    int hours, minutes, seconds;
    int aHours, aMinutes, aSeconds;
    int delay = 1000*1000*1000; //one second in nanoseconds
    clock_gettime(CLOCK_MONOTONIC, &ts);

    /* infinite loop */
    while (1)
    {
        ts.tv_nsec += delay;
        if (ts.tv_nsec >= 1000*1000*1000) {
          ts.tv_nsec -= 1000*1000*1000;
          ts.tv_sec++;
        }
        /* read and display current time */
        getTime(&hours, &minutes, &seconds);
        display_time(hours, minutes, seconds);

        if (alarmEnabled())
        {
          getAlarmTime(&aHours, &aMinutes, &aSeconds);
          display_alarm_time(aHours, aMinutes, aSeconds);
          //sem_wait(getStartAlarm());
        }

        if (hours == aHours && minutes == aMinutes && seconds == aSeconds)
        {
          sem_post(&startAlarm);
        }

        /* increment time */
        incrementTime();

        /* wait untill one second */
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
    }
}

void *alarmThread(void *unused)
{
  int delayAlarm = 1500*1000*1000; //one and a half second in nanoseconds

  while(1)
  {
    sem_wait(&startAlarm);
    clock_gettime(CLOCK_MONOTONIC, &alarmts);
    while(alarmEnabled())
    {
      alarmts.tv_nsec += delayAlarm;
      if (alarmts.tv_nsec >= 2000*1000*1000) {
        alarmts.tv_nsec -= 2000*1000*1000;
        alarmts.tv_sec += 2;
      }
      if (alarmts.tv_nsec >= 1000*1000*1000) {
        alarmts.tv_nsec -= 1000*1000*1000;
        alarmts.tv_sec++;
      }
      display_alarm_text();
      clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &alarmts, NULL);
    }
  }
}

/* set_task: reads messages from the user interface, and
   sets the clock, or exits the program */
void * setThread(void *unused)
{
    /* message array */
    char message[SI_UI_MAX_MESSAGE_SIZE];

    /* time read from user interface */
    int hours, minutes, seconds;

    /* set GUI size */
    si_ui_set_size(400, 200);

    while(1)
    {
        /* read a message */
        si_ui_receive(message);
        /* check if it is a set message */
        if (strncmp(message, "set", 3) == 0)
        {
            timeFromSetMessage(message, &hours, &minutes, &seconds);
            if (timeOK(hours, minutes, seconds))
            {
                clockSetTime(hours, minutes, seconds);
            }
            else
            {
                si_ui_show_error("Illegal value for hours, minutes or seconds");
            }
        }
        //check if it is a alarm set message
        else if (strncmp(message, "alarm", 5) == 0)
        {
          alarmTimeFromSetMessage(message, &hours, &minutes, &seconds);
          if (timeOK(hours, minutes, seconds))
          {
            clockSetAlarmTime(hours, minutes, seconds);
          }
          else
          {
            si_ui_show_error("Illegal value for hours, minutes or seconds");
          }
        }
        //Check if it is a reset message
        else if (strcmp(message, "reset") == 0)
        {
          resetAlarm();
          erase_alarm_time();
          erase_alarm_text();
        }
        /* check if it is an exit message */
        else if (strcmp(message, "exit") == 0)
        {
            exit(0);
        }
        /* not a legal message */
        else
        {
            si_ui_show_error("unexpected message type");
        }
    }
}

int main(void)
{
  si_ui_init();
  clockInit();
  display_init();
  clockSetTime(12,59,30);
  sem_init(&startAlarm, 0, 0);

  //create tasks
  pthread_t clockThreadHandle;
  pthread_t alarmThreadHandle;
  pthread_t setThreadHandle;

  pthread_create(&clockThreadHandle, NULL, clockThread, 0);
  pthread_create(&alarmThreadHandle, NULL, alarmThread, 0);
  pthread_create(&setThreadHandle, NULL, setThread, 0);

  pthread_join(clockThreadHandle, NULL);
  pthread_join(alarmThreadHandle, NULL);
  pthread_join(setThreadHandle, NULL);

}
