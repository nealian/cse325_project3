/*******************************************************************************
 *
 * filename: p3.c
 *
 * description: A simulation of the synchronization problem of several
 *              producers and consumers sharing a single fixed-size buffer.
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
#define ERR_MSG "p3"
#define ERR_CODE 1

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
    fprintf(stderr, "Usage:\t%s <number of producer threads> <number of consumer threads> <fifo(0) or filo(1)>\n",argv[0]);
    exit(ERR_CODE);
  }

  char *end_p, *end_c, *end_f;
  num_producers = strtol(argv[1], &end_p, 10);
  num_consumers = strtol(argv[2], &end_c, 10);
  is_FILO = strtol(argv[3], &end_f, 2);

  if(end_p == argv[1] || *end_p != '\0' || num_producers < 1
     || end_c == argv[2] || *end_c != '\0' || num_consumers < 1
     || end_f == argv[3] || *end_f != '\0' || !(is_FILO == 0 || is_FILO == 1)) {
    fprintf(stderr, "Error parsing command-line arguments.\n");
    exit(ERR_CODE);
  }

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
      perror(ERR_MSG);
      exit(ERR_CODE);
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
      perror(ERR_MSG);
      exit(ERR_CODE);
    }
  }

  /* 5. Sleep 300 seconds*/
  if(sleep(300)) {
    fprintf(stderr, "Main thread sleep interrupted.\n");
  }

  /* Cancel remaining blocking threads and clean up thread resources */
  for(i = 0; i < num_producers; i++) {
    pthread_cancel(producer_threads[i]);
    pthread_join(producer_threads[i], NULL);
  }
  for(i = 0; i < num_consumers; i++) {
    pthread_cancel(consumer_threads[i]);
    pthread_join(consumer_threads[i], NULL);
  }

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

  if(usleep(1000 * (rand() % 1000))) {
    fprintf(stderr, "Producer thread sleep interrupted.\n");
  }

  if(!sem_wait(&(q->producer))) {
    /* Lock production semaphore */
    /* This causes the thread to block until there's space in the buffer */

    if(push_queue(q, elem) < 0) {
      // Uh, Mr. The Plague, uh, something weird is happening on the net
      fprintf(stderr, "Error in producer thread: can't write to buffer.\n");
      return (void *) ERR_CODE;
    }

    /* Print description of what's up */
    printf("item %i added by producer %i: buffer = ", elem, producer_number);
    print_queue(q);
    putchar('\n');

    /* Unlock consumption semaphore to signal that the buffer has elements */
    if(sem_post(&(q->consumer))) {
      /* Handle semaphore post failure */
      perror("producer semaphore post");
      return (void *) ERR_CODE;
    }
  } else {
    /* Handle semaphore wait failure */
    perror("producer semaphore wait");
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

  if(usleep(1000 * (rand() % 1000))) {
    fprintf(stderr, "Consumer thread sleep interrupted.\n");
  }

  if(!sem_wait(&(q->consumer))) {
    /* Lock consumption semaphore. */
    /* This causes the thread to block until there is an item in the queue */

    if((elem = pop_queue(q)) < 0) {
      // Uh, Mr. The Plague, uh, something weird is happening on the net
      fprintf(stderr, "Error in consumer thread: can't read from buffer.\n");
      return (void *) ERR_CODE;
    }

    /* Check it out y'all, check it check it out */
    printf("item %i removed by consumer %i: buffer = ", elem, consumer_number);
    print_queue(q);
    putchar('\n');

    /* Unlock production semaphore */
    if(sem_post(&(q->producer))) {
      /* Handle semaphore post failure */
      perror("consumer semaphore post");
      return (void *) ERR_CODE;
    }
  } else {
    /* Handle semaphore wait failure */
    perror("consumer semaphore wait");
  }

  return NULL;
}
