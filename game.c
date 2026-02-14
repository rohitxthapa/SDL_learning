#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int windowbreadth;
  int windowheight;
  double delta;
  int animationspeed;
  SDL_Texture *map;
  SDL_FRect swindow;
  SDL_FRect dwindow;
  Uint64 framestarttick, frameendtick;
} Gamestate;

typedef struct {
  SDL_FRect drect;
  SDL_FRect srect;
  SDL_Texture *texture;
  int dx, dy;
  int ax, ay;
  int framecount;
  float playerspeed;
  enum animationstate {
    IDLE = 0,
    STARTINGTOFLY = 1,
    FLYING = 2,
    STOPPINGFROMFLY = 3
  } state;
} Character;

typedef struct {
  SDL_FRect position;
  struct {
    short int redval;
    short int greenval;
    short int blueval;
    short int trans;
  } colour;
} Object;

void init(SDL_Window **window, SDL_Renderer **renderer, Gamestate *state);
void loadmedia(SDL_Renderer *renderer, SDL_Texture **player, SDL_Texture **map);
void handleinput(Character *player);
void update(Character *player, Object *objs, int size, Gamestate *state);
void render(SDL_Renderer *renderer, Character *player, Object *objs, int size,
            Gamestate *state);
void cleanup(const char *error, void *win, void *ren, void *tex);

int main(int argv, char *argc[]) {

  Gamestate state = {
      .windowbreadth = 1000,
      .windowheight = 800,
      .delta = 0.016,
      .animationspeed = 125,
      .swindow = {300, 300, state.windowbreadth / 3, state.windowheight / 3},
      .framestarttick = SDL_GetTicks(),
      .dwindow = {0, 0, .w = state.windowbreadth, .h = state.windowheight}};
  SDL_Window *window;
  SDL_Renderer *renderer;
  init(&window, &renderer, &state);

  Character player = {.srect = {0, 32, 32, 32},
                      .drect = {(state.windowbreadth >> 1) - player.srect.w / 2,
                                (state.windowheight >> 1) - player.srect.h / 2,
                                64, 64},
                      .dx = 0,
                      .dy = 0,
                      .ax = 0,
                      .ay = 0,
                      .framecount = 0,
                      .playerspeed = 360.0f,
                      .state = IDLE};
  loadmedia(renderer, &player.texture, &state.map);

  Object objs[10] = {
      {{800 - state.swindow.x, 800 - state.swindow.y, 100, 100},
       {0xff, 0x00, 0x00, 0xff}},
      {{600 - state.swindow.x, 800 - state.swindow.y, 100, 100},
       {0x00, 0xff, 0x00, 0xff}},
  };

  int running = 1;
  SDL_Event event;

  Uint64 start = SDL_GetPerformanceCounter(), end;
  while (running) {

    end = SDL_GetPerformanceCounter();
    state.delta = (double)(end - start) / (double)SDL_GetPerformanceFrequency();
    start = end;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        running = 0;
      case SDL_EVENT_WINDOW_RESIZED:
        SDL_GetWindowSize(window, &state.windowbreadth, &state.windowheight);
        state.swindow.w = state.windowbreadth / 3;
        state.swindow.h = state.windowheight / 3;
        state.dwindow.w = state.windowbreadth;
        state.dwindow.h = state.windowheight;
      }
      if (event.type == SDL_EVENT_QUIT) {
        running = 0;
      }
    }

    handleinput(&player);
    update(&player, objs, 2, &state);
    render(renderer, &player, objs, 2, &state);
  }

  const char *message = "execution completed";
  cleanup(message, window, renderer, &player.texture);
  SDL_Quit();
  return 0;
}

void init(SDL_Window **window, SDL_Renderer **renderer, Gamestate *state) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    exit(1);
  }

  *window = SDL_CreateWindow("hello window", state->windowbreadth,
                             state->windowheight, SDL_WINDOW_RESIZABLE);
  if (*window == NULL) {
    const char *err = SDL_GetError();
    cleanup(err, *window, NULL, NULL);
    exit(1);
  }
  *renderer = SDL_CreateRenderer(*window, NULL);
  if (*renderer == NULL) {
    const char *err = SDL_GetError();
    cleanup(err, *window, *renderer, NULL);
    SDL_Quit();
    exit(1);
  }
  if (SDL_SetRenderVSync(*renderer, 1)) {
    SDL_Log("vsync activated");
  } else {
    SDL_Log("vsync is not activated");
  }
}

void loadmedia(SDL_Renderer *renderer, SDL_Texture **player,
               SDL_Texture **map) {
  SDL_Surface *surface = IMG_Load("playersprite.png");
  if (surface == NULL) {
    const char *err = SDL_GetError();
    cleanup(err, NULL, renderer, NULL);
    SDL_Quit();
    exit(1);
  }
  *player = SDL_CreateTextureFromSurface(renderer, surface);
  surface = IMG_Load("greenscreen.png");
  *map = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_DestroySurface(surface);
  if (*player == NULL || *map == NULL) {
    const char *err = SDL_GetError();
    cleanup(err, NULL, renderer, NULL);
    SDL_Quit();
    exit(1);
  }
}

void handleinput(Character *player) {

  const bool *keys = SDL_GetKeyboardState(NULL);

  player->ax = 0;
  player->ay = 0;
  if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
    player->ay += -1;
  }
  if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
    player->ay += 1;
  }
  if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
    player->ax += 1;
  }
  if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
    player->ax += -1;
  }
  player->dx += player->ax;
  player->dy += player->ay;
}

void update(Character *player, Object *objs, int size, Gamestate *state) {
  bool moving = (player->dx != 0 || player->dy != 0);

  if (moving) {
    float xpixeltoadd, ypixeltoadd;
    xpixeltoadd = player->dx * player->playerspeed * state->delta;
    // player->drect.x += xpixeltoadd;
    state->swindow.x += xpixeltoadd;
    ypixeltoadd = player->dy * player->playerspeed * state->delta;
    // player->drect.y += ypixeltoadd;
    state->swindow.y += ypixeltoadd;

    for (int i = 0; i < size; i++) {
      objs[i].position.x -= 3 * xpixeltoadd;
      objs[i].position.y -= 3 * ypixeltoadd;
    }

    if (player->dx != 0) {
      player->dx += (player->dx < 0) ? 1 : -1;
    }
    if (player->dy != 0) {
      player->dy += (player->dy < 0) ? 1 : -1;
    }
  }

  state->frameendtick = SDL_GetTicks();
  if (((state->frameendtick - state->framestarttick) / state->animationspeed) !=
      0) {
    if (moving) {
      if (player->state == IDLE) {
        player->framecount = 0;
        player->state = STARTINGTOFLY;
      } else if (player->state == STARTINGTOFLY && player->framecount == 7) {
        player->state = FLYING;
        player->drect.y -= 20;
      }
    } else {
      if (player->state == FLYING) {
        player->state = STOPPINGFROMFLY;
        player->drect.y += 20;
      } else if (player->state != IDLE && player->framecount == 7) {
        player->state = IDLE;
      }
    }
    player->srect.x = (player->framecount) * 32;
    player->srect.y = player->state * 32;

    player->framecount = (player->framecount + 1) % 8;
    state->framestarttick = state->frameendtick;
  }

  if (objs) {
    for (int i = 0; i < size; i++) {
      if (player->drect.y + player->drect.h >= objs[i].position.y) {
        if (player->drect.y <= objs[i].position.y + objs[i].position.h) {
          if (player->drect.x + player->drect.w >= objs[i].position.x) {
            if (player->drect.x <= objs[i].position.x + objs[i].position.w) {

              player->dx -= player->ax * 8;
              player->dy -= player->ay * 8;
            }
          }
        }
      }
    }
  }

  if (state->swindow.x < 0) {
    state->swindow.x = 0;
  }
  if (state->swindow.y < 0) {
    state->swindow.y = 0;
  }
  if (state->swindow.x + state->swindow.w > 3200) {
    state->swindow.x = 3200 - state->swindow.w;
  }
  if (state->swindow.y + state->swindow.h > 1800) {
    state->swindow.y = 1800 - state->swindow.h;
  }
}

void render(SDL_Renderer *renderer, Character *player, Object *objs, int size,
            Gamestate *state) {
  SDL_RenderClear(renderer);
  SDL_RenderTexture(renderer, state->map, &state->swindow, &state->dwindow);
  for (int i = 0; i < size; i++) {

    SDL_SetRenderDrawColor(renderer, objs[i].colour.redval,
                           objs[i].colour.greenval, objs[i].colour.blueval,
                           objs[i].colour.trans);
    SDL_RenderFillRect(renderer, &objs[i].position);
  }
  SDL_RenderTexture(renderer, player->texture, &player->srect, &player->drect);

  SDL_RenderPresent(renderer);
}

void cleanup(const char *error, void *win, void *ren, void *tex) {
  printf("%s", error);
  if (tex) {
    SDL_DestroyTexture(tex);
  }
  if (ren) {
    SDL_DestroyRenderer(ren);
  }
  if (win) {
    SDL_DestroyWindow(win);
  }
  SDL_Quit();
}
