/**
 * queue.h
 * Wrapper for Linux tail queues
 */

#include <sys/queue.h> // TAILQ macros
#include <stdbool.h> // bool, true, false

typedef struct {
  bool is_FIFO; // True => FIFO queue, false => FILO.
  struct q_entry *head; // Pointer to head element, used internally
  int size; // kept accurate with bookkeeping
} queue;

struct q_entry {
  int value;
  TAILQ_ENTRY(q_entry) entries;
};

queue* new_queue(bool is_FIFO);
void push_queue(queue* q, int item);
int pop_queue(queue* q);
void free_queue(queue* q);
void print_queue(queue* q);
