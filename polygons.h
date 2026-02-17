#ifndef POLYGONS_H
#define POLYGONS_H

#include <stdbool.h>
#include <SDL3/SDL.h>


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
