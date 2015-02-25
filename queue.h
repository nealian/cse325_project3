/**
 * queue.h
 * Wrapper for Linux tail queues
 */

#ifndef _BUFFER_QUEUE_
#define _BUFFER_QUEUE_

#include <sys/queue.h> // TAILQ macros
#include <pthread.h> // pthread_mutex
#include <semaphore.h>
#include <stdbool.h> // bool, true, false

typedef struct {
  bool is_FILO; // True => FILO queue, false => FIFO.
  TAILQ_HEAD(q_head, q_entry) head; // head element, used internally
  int size; // kept accurate with bookkeeping
  int capacity; // max number of elements in the queue
  pthread_mutex_t buffer_mutex; // Mutex to synchronize buffer operations
  sem_t producer, consumer; // Producer/consumer semaphores
} queue;

struct q_entry {
  int value;
  TAILQ_ENTRY(q_entry) entries;
};

/**
 * Create a new empty FIFO or FILO instance. Will allocate space, so the queue
 * must be freed with free_queue when finished.
 *
 * @param is_FILO   If true, the created queue is FILO, if false then FIFO.
 * @param capacity  Max number of elements in the queue.
 * @return          Pointer to new queue.
 */
queue* new_queue(bool is_FILO, int capacity);

/**
 * Add an element to the queue. FILO queues add elements to the head of the
 * queue, and FIFO queues add them to the tail.
 *
 * Note! This function blocks until the buffer mutex is open AND the queue is
 * not full
 *
 * @param q     Initialized queue to which the new element will be added.
 * @param item  Element to add to the queue.
 * @return      Number of elements now in the queue, or <0 on failure.
 */
int push_queue(queue* q, int item);

/**
 * Get the next element in the queue.
 *
 * Note! This function blocks until the buffer mutex is open AND there are
 * elements to pop from the queue.
 *
 * @param q  Initialized queue from which to get an element.
 * @return   The next element in the queue, or <0 on failure.
 */
int pop_queue(queue* q);

/**
 * Free all space allocated to the queue, and the queue wrapper itself.
 *
 * Note! This function blocks until the buffer mutex is open.
 *
 * @param q  Queue to be freed.
 */
void free_queue(queue* q);

/**
 * Print human-readable representation of queue to stdout. Does not print
 * a trailing newline character!
 *
 * Note! This function blocks until the buffer mutex is open.
 *
 * @param q  Queue to print out.
 */
void print_queue(queue* q);

#endif
