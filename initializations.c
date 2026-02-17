#include "declarations.h"
#include <SDL3/SDL.h>
#include <math.h>
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

void get_polygons(polygons *block, float x, float y) {
  block->x = 900;
  block->y = 500;
  block->sides = 3 + rand() % 15;
  block->vertices = malloc(sizeof(SDL_Vertex) * (block->sides));
  block->degreeradian =
      ((block->sides - 2) * 180.0 / block->sides) * (3.141592653589) / 180;
  block->size = 50 + rand() % 50;
  block->degreeradiantoxaxis = (rand() % 360) * (3.141592653589) / 180;
}

void get_polygons_texture(polygons *block, SDL_Renderer *renderer) {
  for (int i = 0; i < block->sides; i++) {
    float theta =
        block->degreeradiantoxaxis + (2.0f * 3.141592653589 * i) / block->sides;
    block->vertices[i].position.x = block->x + block->size * cos(theta);
    block->vertices[i].position.y = block->y - block->size * sin(theta);
  }
  for (int i = 0; i < block->sides; i++)
    block->vertices[i].color = (SDL_FColor){1, 0, 0, 1};

  int triangles = block->sides - 2;
  int incides[triangles * 3];
  int n = 0;
  for (int i = 0; i < triangles; i++) {

    for (int j = 0; j < 3; j++) {
      incides[i * 3 + j] = (j == 0) ? 0 : j + n;
    }
    n++;
  }
  block->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_TARGET, block->size * 2,
                                     block->size * 2);
  SDL_SetRenderTarget(renderer, block->texture);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  SDL_RenderGeometry(renderer, NULL, block->vertices, block->sides, incides,
                     triangles * 3);
  SDL_SetRenderTarget(renderer, NULL);
}
