/*******************************************************************************
 *
 * filename: p3.c
 *
 * description:
 *
 * group: SANIC TEEM
 * authors: Ian Neal, Rob Kelly
 * class: CSE 325
 * instructor: Zheng
 * assignment: Lab Project 3
 * assigned: Feb 11 2015
 * due: Feb 25 2015
 ******************************************************************************/

// *INCLUDES*
#include <unistd.h> // usleep, sleep
#include <stdlib.h> // rand, srand
#include <time.h> // time (for srand)
#include <stdbool.h> // bool, true, false
#include <stdio.h>
#include "queue.h"


// *CONSTANTS*
#define BUF_CAPACITY 10


// *TYPES AND STRUCTS*
typedef struct {
  unsigned char number; // The producer or consumer number
  queue *q;
} thread_args;


// *FUNCTION PROTOTYPES*
void *producer_thread(void *args);
void *consumer_thread(void *args);


// *MAIN PROGRAM*
int main(int argc, char *argv[]){

  int num_producers, num_consumers;
  bool is_FILO;
  queue *q;
  int i;
  srand(time(NULL));


  /* 1. Get command line arguments */
  if (argc != 4) {
    printf("Usage:\t%s <number of producer threads> <number of consumer threads> <fifo(0) or filo(1)>\n",argv[0]);
    exit(1);
  }
  num_producers = atoi(argv[1]);
  num_consumers = atoi(argv[2]);
  is_FILO = atoi(argv[3]);

  /* 2. Initialize buffer entries with -1*/
  q = new_queue(is_FILO, BUF_CAPACITY);

  /* 3. Create producer thread(s) */
  pthread_t producer_threads[num_producers];
  for(i=0; i<num_producers; i++) {
    thread_args *this_thread_args = (thread_args *) calloc(1, sizeof(thread_args));
    this_thread_args->number = i + 1;
    this_thread_args->q = q;

    if(pthread_create(&(producer_threads[i]), NULL, producer_thread, (void *) this_thread_args) != 0) {
      // Uh, Mr. The Plague, uh, something weird is happening on the net
    }
  }

  /* 4. Create consumer thread(s) */
  pthread_t consumer_threads[num_consumers];
  for(i=0; i<num_consumers; i++) {
    thread_args *this_thread_args = (thread_args *) calloc(1, sizeof(thread_args));
    this_thread_args->number = i + 1;
    this_thread_args->q = q;

    if(pthread_create(&(consumer_threads[i]), NULL, consumer_thread, (void *) this_thread_args) != 0) {
      // Uh, Mr. The Plague, uh, something weird is happening on the net
    }
  }

  /* 5. Sleep 300 seconds*/
  sleep(300);

  /* 6. Exit */
  free_queue(q);
  return 0;
}


/**
 * The thread handler for the producers. Waits a random-ish amount of time, then
 * pushes a random number onto the queue, and finally prints the results.
 *
 * @param args   A pointer to the thread_args struct for thread info
 * @return       Pointer to return values (or, in this case, always NULL)
 */
void *producer_thread(void *args)
{
  unsigned char producer_number = ((thread_args *) args)->number;
  queue *q = ((thread_args *) args)->q;
  free(args);
  args = NULL;

  int elem = rand();
  usleep(1000 * (rand() % 1000));

  if(!sem_wait(&(q->producer))) {
    /* Lock production semaphore */
    /* This causes the thread to block until there's space in the buffer */

    if(push_queue(q, elem) < 0) {
      // Uh, Mr. The Plague, uh, something weird is happening on the net
    }

    /* Print description of what's up */
    printf("item %i added by producer %i: buffer = ", elem, producer_number);
    print_queue(q);
    putchar('\n');

    /* Unlock consumption semaphore to signal that the buffer has elements */
    if(sem_post(&(q->consumer))) {
      /* Handle semaphore post failure */
      perror("producer semaphore"); // TODO
    }
  } else {
    /* Handle semaphore wait failure */
    perror("producer semaphore"); // TODO
  }

  return NULL;
}


/**
 * The thread handler for the consumers. Waits a random-ish amount of time, then
 * pops a random number off of the queue, and finally prints the results.
 *
 * @param args   A pointer to the thread_args struct for thread info
 * @return       Pointer to return values (or, in this case, always NULL)
 */
void *consumer_thread(void *args)
{
  unsigned char consumer_number = ((thread_args *) args)->number;
  queue *q = ((thread_args *) args)->q;
  free(args);
  args = NULL;

  int elem;
  usleep(1000 * (rand() % 1000));

  if(!sem_wait(&(q->consumer))) {
    /* Lock consumption semaphore. */
    /* This causes the thread to block until there is an item in the queue */

    if((elem = pop_queue(q)) < 0) {
      // Uh, Mr. The Plague, uh, something weird is happening on the net
    }

    /* Check it out y'all, check it check it out */
    printf("item %i removed by consumer %i: buffer = ", elem, consumer_number);
    print_queue(q);
    putchar('\n');

    /* Unlock production semaphore */
    if(sem_post(&(q->producer))) {
      /* Handle semaphore post failure */
      perror("consumer semaphore"); // TODO
    }
  } else {
    /* Handle semaphore wait failure */
    perror("consumer semaphore"); // TODO
  }

  return NULL;
}
