/**
 * queue.c
 * Implementation of the synchronized queue wrapper.
 */

#include <stdlib.h> // malloc
#include <stdio.h> // printf
#include "queue.h"

queue* new_queue(bool is_FILO, int capacity) {
  queue* q = malloc(sizeof(queue));
  q->is_FILO = is_FILO;
  
  TAILQ_INIT(&(q->head));

  q->size = 0;
  q->capacity = capacity;

  pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  q->buffer_mutex = m;

  if(sem_init(&(q->producer), 0, capacity) == -1
      || sem_init(&(q->consumer), 0, 0) == -1) {
    /* Error handling for producer & consumer semaphore initialization */
    perror("queue initialization");
    exit(1);
  }

  return q;
}

int push_queue(queue* q, int item) {
  struct q_entry *new_entry = malloc(sizeof(struct q_entry));

  if(new_entry == NULL) {
    /* Error handling for that malloc */
    perror("queue element allocation");
    return -1;
  }

  new_entry->value = item;

  /* Critical section. Lock buffer, insert, unlock buffer. */

  if(!pthread_mutex_lock(&(q->buffer_mutex))) {
    /* Lock buffer mutex */
    /* This ensures that buffer modifications don't interfere */
      
    TAILQ_INSERT_TAIL(&(q->head), new_entry, entries);
    q->size++;

    if(pthread_mutex_unlock(&(q->buffer_mutex))) {
      /* Handle buffer mutex unlock failure */
      perror("queue push");
      return q->size;
    }
  } else {
    /* Handle buffer mutex lock failure */
    perror("queue push");
    return -1;
  }

  return q->size;
}

int pop_queue(queue* q) {
  /* Critical section. Lock buffer, remove item, unlock buffer. */

  int item = -1;
  if(!pthread_mutex_lock(&(q->buffer_mutex))) {
    /* Lock buffer mutex */
    /* This ensures that buffer modifications don't interfere */

    struct q_entry *entry = NULL;

    if(q->is_FILO) {
      /* Pop from tail */
      entry = TAILQ_LAST(&(q->head), q_head);
    } else {
      /* Pop from head */
      entry = TAILQ_FIRST(&(q->head));
    }

    if(entry) {
      /* Get value of first element in queue, and remove+free */
      item = entry->value;
      TAILQ_REMOVE(&(q->head), entry, entries);
      free(entry);
    } else {
      /* Should be prevented by the semaphore, but handle error anyway. */
      perror("queue pop");
      return -1;
    }

    q->size--;

    if(pthread_mutex_unlock(&(q->buffer_mutex))) {
      /* Handle buffer mutex unlock failure */
      perror("queue pop");
      return item;
    }
  } else {
    /* Handle buffer mutex lock failure */
    perror("queue pop");
    return -1;
  }

  return item;
}

void free_queue(queue* q) {
  if(!pthread_mutex_lock(&(q->buffer_mutex))) {
    /* Lock buffer mutex (though at this point it should not matter) */

    /* Free all items remaining in the queue */
    struct q_entry *entry;
    while((entry = TAILQ_FIRST(&(q->head)))) {
      TAILQ_REMOVE(&(q->head), entry, entries);
      free(entry);
    }

    /* Destroy semaphores */
    if(sem_destroy(&(q->producer))
       || sem_destroy(&(q->consumer))) {
      /* Handle semaphore destruction failure */
      perror("queue semaphore destruction");
    }

    if(pthread_mutex_unlock(&(q->buffer_mutex))) {
      /* Handle buffer mutex unlock failure */
      perror("queue free");
    }
  } else {
    /* Handle buffer mutex lock failure */
    perror("queue free");
  }

  /* Destroy mutex */
  if(pthread_mutex_destroy(&(q->buffer_mutex))) {
    /* Handle buffer mutex destruction failure */
    perror("queue mutex destruction");
  }

  /* Finally, free queue itself */
  free(q);
}

void print_queue(queue* q) {
  struct q_entry *entry;

  if(!pthread_mutex_lock(&(q->buffer_mutex))) {
    /* Lock buffer mutex */

    printf("[");
    TAILQ_FOREACH(entry, &(q->head), entries) {
      printf(" %d", entry->value);
    }
    printf(" ]");

    if(pthread_mutex_unlock(&(q->buffer_mutex))) {
      /* Handle buffer mutex unlock failure */
      perror("queue print");
    }
  } else {
    /* Handle buffer mutex lock failure */
    perror("queue print");
  }
}
