#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

// This file contains the definition of MAX_ITERATIONS, among other things.
#include "samples.h"


// Delay in nanoseconds (1 millisecond)
#define DELAY 1000000

// Number of samples that do_work() handles
#define PROCESSING_INTERVAL  256

#define N_BUFFERS 2

// Could be a local variable, but you may need it as a static variable
// here when you modify this file according to the lab instructions.
static int sample_buffer[N_BUFFERS][PROCESSING_INTERVAL];
sem_t activateDoWork;
int buffer;

void do_work(int *samples)
{
        int i;

        //  A busy loop. (In a real system this would do something
        //  more interesting such as an FFT or a particle filter,
        //  etc...)
        volatile int dummy; // A compiler warning is ok here
        for(i=0; i < 20000000;i++){
                dummy=i;
        }

        // Write out the samples.
        for(i=0; i < PROCESSING_INTERVAL; i++){
                write_sample(0,samples[i]);
        }

}

struct timespec firsttime;
/*void *maintask(void *arg)
{
        int channel = 0;
        struct timespec current;
        int i;
        int samplectr = 0;
        current = firsttime;


        for(i=0; i < MAX_ITERATIONS; i++){
                clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &current, NULL);


                sample_buffer[samplectr] = read_sample(channel);
                samplectr++;
                if(samplectr == PROCESSING_INTERVAL){
                        samplectr = 0;
                        do_work(sample_buffer);
                }

                // Increment current time point
                current.tv_nsec +=  DELAY;
                if(current.tv_nsec >= 1000000000){
                        current.tv_nsec -= 1000000000;
                        current.tv_sec++;
                }


        }
        return NULL;
}*/

void *sampleTask(void *arg) {

  int channel = 0;
  struct timespec current;
  int i;
  int samplectr = 0;
  current = firsttime;
  buffer = 0;

  struct sched_param sp;
  sp.sched_priority = 10;

  if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp)) {
    fprintf(stderr, "Waning: failed to set sched param to real time priority. \n");
  }

  for(i=0; i < MAX_ITERATIONS; i++){
          clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &current, NULL);


          sample_buffer[buffer][samplectr] = read_sample(channel);
          samplectr++;
          if(samplectr == PROCESSING_INTERVAL){
                  samplectr = 0;
                  sem_post(&activateDoWork);
                  if (buffer == 0) {buffer++;}
                  else{buffer = 0;}
          }

          // Increment current time point
          current.tv_nsec +=  DELAY;
          if(current.tv_nsec >= 1000000000){
                  current.tv_nsec -= 1000000000;
                  current.tv_sec++;
          }


  }
  return NULL;
}

void *doWorkTask(void *arg) {

  int i;

  struct sched_param sp;
  sp.sched_priority = 5;

  if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp)) {
    fprintf(stderr, "Waning: failed to set sched param to real time priority. \n");
  }

  for (i = 0; i < MAX_ITERATIONS / PROCESSING_INTERVAL; i++ ) {
    sem_wait(&activateDoWork);
    if (buffer == 0) {
      do_work(sample_buffer[1]);
    }
    else {
      do_work(sample_buffer[0]);
    }
  }
  return NULL;
}


int main(int argc,char **argv)
{
        sem_init(&activateDoWork, 0 ,0 );

        clock_gettime(CLOCK_MONOTONIC, &firsttime);

        if (mlockall(MCL_FUTURE|MCL_CURRENT)) {
          fprintf(stderr, "Warning: Failed to lock memory! \n");
        }
        //pthread_t thread0;
        pthread_t sample_thread;
        pthread_t doWork_thread;
        pthread_attr_t attr;
        pthread_attr_t attr1;



        // Start the sampling at an even multiple of a second (to make
        // the sample times easy to analyze by hand if necessary)
        firsttime.tv_sec+=2;
        firsttime.tv_nsec = 0;
        printf("Starting sampling at about t+2 seconds\n");

        samples_init(&firsttime);

        if(pthread_attr_init(&attr)){
                perror("pthread_attr_init");
        }
        /*if(pthread_attr_init(&attr1)){
                perror("pthread_attr1_init");
        }*/

        // Set default stacksize to 64 KiB (should be plenty)
        if(pthread_attr_setstacksize(&attr, 65536)){
                perror("pthread_attr_setstacksize()");
        }
        /*if(pthread_attr_setstacksize(&attr1, 65536)){
                perror("pthread_attr1_setstacksize()");
        }*/

        //pthread_create(&thread0, &attr,mainTask, NULL);
        pthread_create(&sample_thread, &attr, sampleTask, NULL);
        pthread_create(&doWork_thread, &attr, doWorkTask, NULL);

        //pthread_join(thread0, NULL);
        pthread_join(sample_thread, NULL);
        pthread_join(doWork_thread, NULL);

        // Dump output data which will be used by the analyze.m script
        dump_outdata();
        dump_sample_times();
        return 0;
}
