#include "polygons.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double delta;

typedef struct {
  float x, y;
} chunks;

typedef struct {
  SDL_FRect block;
} viewpoint;

typedef struct {
  SDL_FRect block;
  int gridx;
  int gridy;
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
void update(character *player, viewpoint *camera) {

  const bool *keys = SDL_GetKeyboardState(NULL);
  if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
    player->block.y += -1;
    camera->block.y += -1;
  }
  if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
    player->block.y += 1;
    camera->block.y += 1;
  }
  if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
    player->block.x += 1;
    camera->block.x += 1;
  }
  if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
    player->block.x += -1;
    player->block.x += -1;
  }
}

void render(SDL_Renderer *renderer, character *player) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderFillRect(renderer, &player->block);

  SDL_RenderPresent(renderer);
}

int main() {
  SDL_Window *window;
  SDL_Renderer *renderer;
  if (!init(&window, &renderer)) {
    return 1;
  }

  srand(123456789);

  chunks loadedchunks;
  loadedchunks.chunks = malloc(9 * 8);
  viewpoint camera;
  camera.block = (SDL_FRect){0, 0, 1920, 1080};
  character player;
  player.block = (SDL_FRect){6000, 5000, 200, 200};
  player.gridx = player.block.x / chunklength;
  player.gridy = player.block.y / chunklength;

  bool running = true;
  Uint64 frame_start_ticks = SDL_GetPerformanceCounter(), frame_end_ticks;
  SDL_Event event;

  while (running) {

    frame_end_ticks = SDL_GetPerformanceCounter();
    delta = (double)(frame_end_ticks - frame_start_ticks) /
            SDL_GetPerformanceFrequency();
    frame_start_ticks = frame_end_ticks;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }
    update(&player, &camera);
    render(renderer, &player);
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}
