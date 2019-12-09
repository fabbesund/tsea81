#include "lift.h"

/* Simple_OS include */
#include <pthread.h>

/* drawing module */
#include "draw.h"

/* standard includes */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* panic function, to be called when fatal errors occur */
/*static void lift_panic(const char message[])
{
    printf("LIFT_PANIC!!! ");
    printf("%s", message);
    printf("\n");
    exit(0);
}*/

/* --- monitor data type for lift and operations for create and delete START --- */

/* lift_create: creates and initialises a variable of type lift_type */
lift_type lift_create(void)
{
    /* the lift to be initialised */
    lift_type lift;

    /* floor counter */
    int floor;

    /* loop counter */
    int i;

    /* allocate memory */
    lift = (lift_type) malloc(sizeof(lift_data_type));

    /* initialise variables */

    /* initialise floor */
    lift->floor = 0;

    /* set direction of lift travel to up */
    lift->up = 1;

    /* the lift is not moving */
    lift->moving = 0;

    /* initialise person information */
    for (floor = 0; floor < N_FLOORS; floor++)
    {
        for (i = 0; i < MAX_N_PERSONS; i++)
        {
            lift->persons_to_enter[floor][i].id = NO_ID;
            lift->persons_to_enter[floor][i].to_floor = NO_FLOOR;
        }
    }

    /* initialise passenger information */
    for (i = 0; i < MAX_N_PASSENGERS; i++)
    {
        lift->passengers_in_lift[i].id = NO_ID;
        lift->passengers_in_lift[i].to_floor = NO_FLOOR;
    }

    /* initialise mutex and event variable */
    pthread_mutex_init(&lift->mutex,NULL);
    pthread_cond_init(&lift->change,NULL);

    return lift;
}

/* lift_delete: deallocates memory for lift */
void lift_delete(lift_type lift)
{
    free(lift);
}


/* --- monitor data type for lift and operations for create and delete END --- */


/* --- functions related to lift task START --- */

/* MONITOR function lift_next_floor: computes the floor to which the lift
   shall travel. The parameter *change_direction indicates if the direction
   shall be changed */
void lift_next_floor(lift_type lift, int *next_floor, int *change_direction)
{
    int up = lift->up;
    if(up) {
      if(lift->floor == N_FLOORS - 2){
        *change_direction = 1;
      }
      else {
        *change_direction = 0;
      }
      *next_floor = lift->floor + 1;
    }
    else if(!up) {
      if(lift->floor == 1){
        *change_direction = 1;
      }
      else {
        *change_direction = 0;
      }
      *next_floor = lift->floor - 1;
    }
}

void lift_move(lift_type lift, int next_floor, int change_direction)
{
  lift->floor = next_floor;
  if (change_direction) {
    lift->up = !lift->up;
  }
}


/* this function is used also by the person tasks */
int n_passengers_in_lift(lift_type lift)
{
    int n_passengers = 0;
    int i;

    for (i = 0; i < MAX_N_PASSENGERS; i++)
    {
        if (lift->passengers_in_lift[i].id != NO_ID)
        {
            n_passengers++;
        }
    }
    return n_passengers;
}

void enter_floor(lift_type lift, int id, int floor, int to_floor) {
  int found = 0;
  int i;
  for (i = 0; i < MAX_N_PERSONS && !found; i++) {
    if (lift->persons_to_enter[floor][i].id == NO_ID) {
      found = 1;
      lift->persons_to_enter[floor][i].id = id;
      lift->persons_to_enter[floor][i].to_floor = to_floor;
    }
  }
}

void leave_floor(lift_type lift, int id, int enter_floor) {
  int found = 0;
  int i;
  for(i = 0; i < MAX_N_PERSONS && !found; i++) {
    if (lift->persons_to_enter[enter_floor][i].id == id) {
      found = 1;
      lift->persons_to_enter[enter_floor][i].id = NO_ID;
      lift->persons_to_enter[enter_floor][i].to_floor = NO_FLOOR;
    }
  }
}

void enter_lift(lift_type lift, int id, int to_floor) {
  int found = 0;
  int i;
  for(i = 0; i < MAX_N_PERSONS && !found; i++) {
    if(lift->passengers_in_lift[i].id == NO_ID) {
      found = 1;
      lift->passengers_in_lift[i].id = id;
      lift->passengers_in_lift[i].to_floor = to_floor;
    }
  }
}
