#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <SDL3/SDL.h>

typedef struct {
  float x;
  float y;
} data;

typedef struct {
  int start;
  int end;
  int size;
  data *points;
} queue;


bool queueinit(queue *que, int size);
bool queue_empty(queue *que);
bool queue_full(queue *que);
bool queue_enqueue(queue *que, data *point);
void queue_dequeue(queue *que);

typedef struct {
  int sides;
  SDL_Vertex *vertices;
  float x, y;
  float size;
  float degreeradian;
  float degreeradiantoxaxis;
  SDL_Texture *texture;
} polygons;
void get_polygons(polygons *block, float x , float y);

void get_polygons_texture(polygons *block, SDL_Renderer* renderer);


#endif
