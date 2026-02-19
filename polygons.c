#include "polygons.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdlib.h>

void get_polygons(polygons *block, int x, int y) {
  srand((x * 73856093) ^ (y * 19349663));
  block->sides = 3 + rand() % 15;
  block->vertices = malloc(sizeof(SDL_Vertex) * (block->sides));
  block->degreeradian =
      ((block->sides - 2) * 180.0 / block->sides) * (3.141592653589) / 180;
  block->size = 50 + rand() % 50;
  block->x = block->size + 1;
  block->y = block->size + 1;
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
  int dimention = (int)(block->size * 2) + 2;
  block->texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, dimention, dimention);
  SDL_SetTextureBlendMode(block->texture, SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, block->texture);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  SDL_RenderGeometry(renderer, NULL, block->vertices, block->sides, incides,
                     triangles * 3);
  SDL_SetRenderTarget(renderer, NULL);
}
void free_polygon(polygons *block) {
  if (block->vertices)
    free(block->vertices);
  if (block->texture)
    SDL_DestroyTexture(block->texture);
}
