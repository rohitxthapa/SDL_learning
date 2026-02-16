#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

float worldcoordinatex = 0.0f, worldcoordinatey = 0.0f;
float chunklengthx = 4000.0, chunklenthy = 4000.0;

typedef struct {
  int sides;
  SDL_Vertex *vertices;
  float x, y;
  float size;
  float degree;
  float degreeradian;
  float degreetoxaxis;
} Blocks;

bool init(SDL_Window **window, SDL_Renderer **renderer) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("couldnt initialize! : %s", SDL_GetError());
    return false;
  }
  *window = SDL_CreateWindow("shapes", 1920, 1080, SDL_WINDOW_RESIZABLE);
  if (*window == NULL) {
    SDL_Log("error in creating window ! : %s", SDL_GetError());
    return false;
  }
  *renderer = SDL_CreateRenderer(*window, NULL);
  if (*renderer == NULL) {
    SDL_Log("error in creating renderer ! : %s", SDL_GetError());
    SDL_DestroyWindow(*window);
    return false;
  }
  if (SDL_SetRenderVSync(*renderer, 1)) {
    SDL_Log("VSync activated");
  } else {
    SDL_Log("VSync couldn't activate");
  }
  return true;
}

void getblocksdimentions(Blocks *block) {
  block->sides = 3 + rand() % 5;
  block->vertices = malloc(sizeof(SDL_Vertex) * (block->sides + 1));

  block->x = 200;
  block->y = 200;
  block->degree = (block->sides - 2) * 180.0 / block->sides;
  block->degreeradian = block->degree * (3.141592653589) / 180;
  block->size = 50 + rand() % 50;
  block->degreetoxaxis = rand() % 360;

  return;
}

void updateblocks(Blocks *block) {
  float angle = block->degreetoxaxis * (3.141592653589) / 180.0f;
  float cx = block->x;
  float cy = block->y;

  for (int i = 0; i < block->sides; i++) {
    float theta = angle + (2.0f * 3.141592653589f * i) / block->sides;
    block->vertices[i].position.x = cx + block->size * cos(theta);
    block->vertices[i].position.y = cy - block->size * sin(theta);
  }

  block->vertices[block->sides].position.x = block->vertices[0].position.x;
  block->vertices[block->sides].position.y = block->vertices[0].position.y;

  for (int i = 0; i <= block->sides; i++)
    block->vertices[i].color = (SDL_FColor){1, 0, 0, 1};
  return;
}

void renderblocks(Blocks *block, SDL_Renderer *renderer) {
  int triangles = block->sides - 1;
  int *incides = malloc(sizeof(int) * triangles * 3);
  int n = 0;
  for (int i = 0; i < triangles; i++) {

    for (int j = 0; j < 3; j++) {
      incides[i * 3 + j] = (j == 0) ? 0 : j + n;
    }
    n++;
  }
  SDL_RenderGeometry(renderer, NULL, block->vertices, block->sides + 1, incides,
                     triangles * 3);
  free(incides);
}

int main() {
  SDL_Window *window;
  SDL_Renderer *renderer;
  if (!init(&window, &renderer)) {
    return 1;
  }
  Blocks block;

  SDL_Event event;
  bool running = true;
  while (running) {

    srand(1345);
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);

    getblocksdimentions(&block);

    updateblocks(&block);

    renderblocks((&block), renderer);

    SDL_RenderPresent(renderer);
  }

  free(block.vertices);
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}
