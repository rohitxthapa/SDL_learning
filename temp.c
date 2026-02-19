#include "declarations.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

float chunklength = 3000;
double delta;

typedef struct {
  queue **chunks;
  float x, y;
} chunks;

typedef struct {
  float x, y;
  float w, h;
} viewpoint;

typedef struct {
  float x, y;
  float w, h;
  SDL_FRect positioninmap;
} character;

bool init(SDL_Window **window, SDL_Renderer **renderer) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL could not initilize : %s", SDL_GetError());
    return false;
  }
  *window = SDL_CreateWindow("shapes", 1920, 1080, SDL_WINDOW_RESIZABLE);
  if (*window == NULL) {
    SDL_Log("SDL_Window could not initilize : %s", SDL_GetError());
    return false;
  }
  *renderer = SDL_CreateRenderer(*window, NULL);
  if (*renderer == NULL) {
    SDL_Log("SDL_Renderer could not initilize : %s", SDL_GetError());
    return false;
  }
  if (SDL_SetRenderVSync(*renderer, 1)) {
    SDL_Log("VSync activated");
  } else {
    SDL_Log("SDL_VSync couldn't activate %s", SDL_GetError());
  }
  return true;
}

int main() {
  SDL_Window *window;
  SDL_Renderer *renderer;
  if (!init(&window, &renderer)) {
    return 1;
  }

  srand(123456789);
  polygons *shapes;
  chunks loadedchunks;
  viewpoint camear;
  character player;

  loadedchunks.chunks = (queue **)malloc(25 * sizeof(queue *));
  loadedchunks.x = rand() % 10000;
  loadedchunks.y = rand() % 10000;

  bool running = true;
  Uint64 frame_start_ticks = SDL_GetPerformanceCounter(), frame_end_ticks;
  SDL_Event event;
  const bool *keys;

  player.positioninmap = (SDL_FRect){500, 500, 200, 200};
  while (running) {

    frame_end_ticks = SDL_GetPerformanceCounter();
    delta = (double)(frame_end_ticks - frame_start_ticks) /
            SDL_GetPerformanceFrequency();
    frame_start_ticks = frame_end_ticks;

    keys = SDL_GetKeyboardState(NULL);
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }

      SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
      SDL_RenderClear(renderer);

      SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
      SDL_RenderFillRect(renderer, &player.positioninmap);

      SDL_RenderPresent(renderer);
    }
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}

// void generatepolygons(gridpos* chunk){

// }

// int chunkmod(chunks *chunks, int x, int y) {
//   return (x + y + chunks->sideoftotalchunks / 2) % chunks->sideoftotalchunks;
// }
// void checkthechunks(chunks *chunks) {
//   int centerx, centery;
//   centerx = chunkmod(chunks, chunks->startx, 0);
//   centery = chunkmod(chunks, chunks->starty, 0);

//   for (int i = 1; i < chunks->sideoftotalchunks / 2; i++) {
//     if (!chunks
//              ->chunkspos[chunkmod(chunks, chunks->startx, i)]
//                         [chunks, chunks->starty, i]
//              .present) {
//       generatepolygons(&chunks->chunkspos[chunkmod(chunks, chunks->startx,
//       i)]
//                  [chunkmod(chunks, chunks->starty, i)]);
//     }
//   }
//   for (int i = -1; i < -(chunks->sideoftotalchunks / 2); i--) {
//     if (!chunks
//              ->chunkspos[chunkmod(chunks, chunks->startx, i)]
//                         [chunkmod(chunks, chunks->starty, i)]
//              .present) {
//       generatepolygons(&chunks->chunkspos[chunkmod(chunks, chunks->startx,
//       i)]
//                  [chunkmod(chunks, chunks->starty, i)],);
//     }
//   }
// }
//
//
// void checkthechunks(chunks *chunks) {
//   int x, y, centerx, centery, len;
//   x = chunks->startx;
//   y = chunks->starty;
//   len = chunks->len_chunk;
//   centerx = (x + len / 2) % len;
//   centery = (y + len / 2) % len;

//   for (int i = 1; i < chunks->len_chunk / 2; i++) {
//     int xc = (x + len / 2 + i) % len;
//     int yc = (y + len / 2 + i) % len;
//     int xf = (xc - len) % len;
//     int yf = (yc - len) % len;
//     if (!chunks->chunkspos[xc][yf].present) {
//       if ()
//     }
//   }
//   for (int i = -1; i < -(chunks->len_chunk / 2); i--) {
//     if (!chunks
//              ->chunkspos[chunkmod(chunks, chunks->startx, i)]
//                         [chunkmod(chunks, chunks->starty, i)]
//              .present) {
//       generatepolygons(&chunks->chunkspos[chunkmod(chunks, chunks->startx,
//       i)]
//                                          [chunkmod(chunks, chunks->starty,
//                                          i)]);
//     }
//   }
// }
