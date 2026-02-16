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
  float degreeradiantoxaxis;
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

  block->x = 900;
  block->y = 500;
  block->sides = 3 + rand() % 15;
  block->vertices = malloc(sizeof(SDL_Vertex) * (block->sides));
  block->degree = (block->sides - 2) * 180.0 / block->sides;
  block->degreeradian = block->degree * (3.141592653589) / 180;
  block->size = 50 + rand() % 50;
  block->degreeradiantoxaxis = (rand() % 360) * (3.141592653589) / 180;

  return;
}

void updateblocks(Blocks *block) {
  // suggested by opencode agent
  for (int i = 0; i < block->sides; i++) {
    float theta =
        block->degreeradiantoxaxis + (2.0f * 3.141592653589 * i) / block->sides;
    block->vertices[i].position.x = block->x + block->size * cos(theta);
    block->vertices[i].position.y = block->y - block->size * sin(theta);
  }

  for (int i = 0; i < block->sides; i++)
    block->vertices[i].color = (SDL_FColor){1, 0, 0, 1};
  return;
}

// void updateblocks(Blocks *block) {
//   double tempx, tempy;
//   double ix, iy, fx, fy;

//   ix = block->x;
//   fx = ix + (block->size * cos(block->degreeradiantoxaxis));
//   iy = block->y;
//   fy = iy - (block->size * sin(block->degreeradiantoxaxis));

//   block->vertices[0].position = (SDL_FPoint){ix, iy};
//   block->vertices[1].position = (SDL_FPoint){fx, fy};
//   // suggested by opencode agent
//   for (int i = 2; i < block->sides; i++) {

//     tempx = fx + (ix - fx) * cos(block->degreeradian) -
//             (iy - fy) * sin(block->degreeradian);
//     tempy = fy + (ix - fx) * sin(block->degreeradian) +
//             (iy - fy) * cos(block->degreeradian);
//     block->vertices[i].position.x = tempx;
//     block->vertices[i].position.y = tempy;
//     ix = fx;
//     iy = fy;
//     fx = tempx;
//     fy = tempy;
//   }

//   for (int i = 0; i < block->sides - 1; i++)
//     block->vertices[i].color = (SDL_FColor){1, 0, 0, 1};
//   return;
// }

void renderblocks(Blocks *block, SDL_Renderer *renderer) {
  int triangles = block->sides - 2;
  int incides[triangles * 3];
  int n = 0;
  for (int i = 0; i < triangles; i++) {

    for (int j = 0; j < 3; j++) {
      incides[i * 3 + j] = (j == 0) ? 0 : j + n;
    }
    n++;
  }
  SDL_RenderGeometry(renderer, NULL, block->vertices, block->sides, incides,
                     triangles * 3);
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
  srand(12345);
  while (running) {
    SDL_RenderClear(renderer);
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);

    getblocksdimentions(&block);

    updateblocks(&block);

    renderblocks((&block), renderer);
    free(block.vertices);
    SDL_RenderPresent(renderer);
    SDL_Delay(1000);
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}
