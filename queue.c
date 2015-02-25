#include <stdlib.h> // malloc
#include <stdio.h> // printf
#include "queue.h"
#include <semaphore.h>

// TODO

queue* new_queue(bool is_FIFO, int capacity) {
  queue* q = malloc(sizeof(queue));
  q->is_FIFO = is_FIFO;
  
  TAILQ_INIT(&(q->head));
    
  q->size = 0;
  q->capacity = capacity;

  pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  q->buffer_mutex = m;

  if(sem_init(&(q->producer), 0, capacity) == -1
      || sem_init(&(q->consumer), 0, 0) == -1) {
    // Error handling for producer & consumer semaphore initialization
    perror("queue"); // TODO: maybe synchronize error messages across project
    exit(-1);
  }
  
  return q;
}

void print_queue(queue* q) {
  struct q_entry *entry;

  printf("[");
  TAILQ_FOREACH(entry, &(q->head), entries) {
    printf(" %d", entry->value);
  }
  printf(" ]");
}
