#include <stdlib.h> // malloc
#include <stdio.h> // printf
#include "queue.h"
#include <semaphore.h>

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
    /* Error handling for producer & consumer semaphore initialization */
    perror("queue"); // TODO: maybe synchronize error messages across project
    exit(-1);
  }
  
  return q;
}

int push_queue(queue* q, int item) {
  struct q_entry *new_entry = malloc(sizeof(struct q_entry));

  if(new_entry == NULL) {
    /* Error handling for that malloc */
    perror("queue push"); // TODO
    return -1;
  }

  new_entry->value = item;

  /* Critical section. Wait on producer semaphore, lock buffer,
     insert, unlock buffer, and post consumer semaphore. */
  
  if(sem_wait(&(q->producer))) {
    /* Lock production semaphore */
    /* This causes the thread to block until there's space in the queue */
    
    if(pthread_mutex_lock(&(q->buffer_mutex))) {
      /* Lock buffer mutex */
      /* This ensures that buffer modifications don't interfere */
      
      if(q->is_FIFO) {
        /* Insert at head */
        TAILQ_INSERT_HEAD(&(q->head), new_entry, entries);
      } else {
        /* Insert at tail */
        TAILQ_INSERT_TAIL(&(q->head), new_entry, entries);
      }
      
      q->size++;

      if(!pthread_mutex_unlock(&(q->buffer_mutex))) {
        /* Handle buffer mutex unlock failure */
        perror("queue push"); // TODO
        return -1;
      }
    } else {
      /* Handle buffer mutex lock failure */
      perror("queue push"); // TODO
      return -1;
    }
    
    if(sem_post(&(q->consumer))) {
      /* Unlock consumption semaphore */
      return q->size;
    } else {
      /* Handle semaphore post failure */
      perror("queue push"); // TODO
      return -1;
    }
  } else {
    /* Handle semaphore wait failure */
    perror("queue push"); // TODO
    return -1;
  }
}

int pop_queue(queue* q) {
  // TODO
}

void free_queue(queue* q) {
  // TODO
}

void print_queue(queue* q) {
  struct q_entry *entry;

  if(pthread_mutex_lock(&(q->buffer_mutex))) {
    /* Lock buffer mutex */
    
    printf("[");
    TAILQ_FOREACH(entry, &(q->head), entries) {
      printf(" %d", entry->value);
    }
    printf(" ]");

    if(!pthread_mutex_unlock(&(q->buffer_mutex))) {
      /* Handle buffer mutex unlock failure */
      perror("queue print"); // TODO
    }
  } else {
    /* Handle buffer mutex lock failure */
    perror("queue print"); // TODO
  }
}
