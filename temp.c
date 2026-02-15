#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const int movementspeed = 100; // pxl per sec
double delta = 0.016;
Uint64 framestart, frameend;

typedef struct {
  float x;
  float y;
} position;
typedef struct {
  short int degree;
} direction;
typedef struct {
  float dx, dy;
  float speed;
  direction goingtoward;
} velocity;

typedef struct {
  int windowbread
} map;

typedef struct {
  position positioninmainmap;
  velocity movement;
  direction pointing;
} Character;

bool init(SDL_Window **window, SDL_Renderer **renderer) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL could not initilize : %s", SDL_GetError());
    return false;
  }
  *window = SDL_CreateWindow("shapes", 640, 360, SDL_WINDOW_RESIZABLE);
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

int main(int argc, char *argv[]) {
  SDL_Window *window;
  SDL_Renderer *renderer;
  init(&window, &renderer);

  SDL_FRect camera = {0, 0, 100, 100};

  SDL_Event event;
  bool running = true;
  while (running) {

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
      if (event.type == SDL_EVENT_WINDOW_RESIZED) {
        SDL_GetWindowSize(window, &camera.w, &camera.h);
      }
    }
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}
