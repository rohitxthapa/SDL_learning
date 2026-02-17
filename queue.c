#include "declarations.h"
#include <stdbool.h>
#include <stdlib.h>

bool queueinit(queue *que, int size) {
  que->start = 0;
  que->end = 0;
  que->size = size;
  que->points = malloc(size * sizeof(data));
  if (que->points == NULL) {
    return false;
  }
  return true;
}
bool queue_empty(queue *que) { return (que->start == que->end); }
bool queue_full(queue *que) {
  return (((que->end + 1) % que->size) == que->start);
}
bool queue_enqueue(queue *que, data *point) {
  if (queue_full(que)) {
    return false;
  }
  que->end = (que->end + 1) % que->size;
  que->points[que->end] = *point;
  return true;
}
void queue_dequeue(queue *que) {
  if (queue_empty(que)) {
    return;
  }
  que->start = (que->start + 1) % que->size;
  return;
}
