#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>

int windowbreadth = 1000;
int windowheight = 800;
int animationspeed = 100;
double delta = 0.016;
float playerspeed = 120.0f;

typedef struct {
  int drectheight, drectbreadth;
  SDL_FRect drect;
  int srectheight, srectbreadth;
  SDL_FRect srect;
  SDL_Texture *texture;
  int dx, dy;
  int ax, ay;
  int framecount;
  Uint64 framestarttick, frameendtick;
} Character;

typedef struct {
  SDL_FRect position;
  short int redval;
  short int greenval;
  short int blueval;
  short int trans;

} Object;

void init(SDL_Window **window, SDL_Renderer **renderer);
void loadmedia(SDL_Renderer *renderer, Character *player);
void handleinput(Character *player);
void update(Character *player, Object *objs, int size);
void render(SDL_Renderer *renderer, Character *player, Object *objs, int size);
void cleanup(const char *error, void *win, void *ren, void *tex);

int main(int argv, char *argc[]) {

  SDL_Window *window;
  SDL_Renderer *renderer;
  init(&window, &renderer);

  Character player = {
      .drectbreadth = 100,
      .drectheight = 100,
      .drect = {100, 100, player.drectbreadth, player.drectheight},
      .srectbreadth = 48,
      .srectheight = 48,
      .srect = {0, 0, player.srectbreadth, player.srectheight},
      .dx = 0,
      .dy = 0,
      .ax = 0,
      .ay = 0,
      .framecount = 0,
      .framestarttick = SDL_GetTicks()};
  loadmedia(renderer, &player);

  Object objs[10] = {
      {{400, 400, 100, 100}, 0xff, 0x00, 0x00, 0xff},
      {{600, 600, 100, 100}, 0x00, 0xff, 0x00, 0xff},
  };

  int running = 1;
  SDL_Event event;

  Uint64 start = SDL_GetPerformanceCounter(), end;
  while (running) {

    end = SDL_GetPerformanceCounter();
    delta = (double)(end - start) / (double)SDL_GetPerformanceFrequency();
    start = end;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = 0;
      }
    }

    handleinput(&player);
    update(&player, objs, 2);
    render(renderer, &player, objs, 2);
  }

  const char *message = "execution completed";
  cleanup(message, window, renderer, &player.texture);
  SDL_Quit();
  return 0;
}

void init(SDL_Window **window, SDL_Renderer **renderer) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    exit(1);
  }

  *window = SDL_CreateWindow("hello window", windowbreadth, windowheight,
                             SDL_WINDOW_RESIZABLE);
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

void loadmedia(SDL_Renderer *renderer, Character *player) {
  SDL_Surface *surface = SDL_LoadBMP("firsttry.bmp");
  if (surface == NULL) {
    const char *err = SDL_GetError();
    cleanup(err, NULL, renderer, NULL);
    SDL_Quit();
    exit(1);
  }
  player->texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_DestroySurface(surface);
  if (player->texture == NULL) {
    const char *err = SDL_GetError();
    cleanup(err, NULL, renderer, player->texture);
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

void update(Character *player, Object *objs, int size) {
  bool moving = (player->dx != 0 || player->dy != 0);

  if (moving) {
    player->drect.x += player->dx * playerspeed * delta;
    player->drect.y += player->dy * playerspeed * delta;
    if (player->dx != 0) {
      player->dx += (player->dx < 0) ? 1 : -1;
    }
    if (player->dy != 0) {
      player->dy += (player->dy < 0) ? 1 : -1;
    }
  }

  player->frameendtick = SDL_GetTicks();
  if (animationspeed < (player->frameendtick - player->framestarttick)) {
    if (moving) {
      player->framecount = (player->framecount + 1) % 4;
      player->srect.x = player->framecount * 96;
    } else {
      player->framecount = 0;
      player->srect.x = 0;
    }
    player->framestarttick = player->frameendtick;
  }

  if (objs) {
    for (int i = 0; i < size; i++) {
      if (player->drect.y + player->drectheight >= objs[i].position.y) {
        if (player->drect.y <= objs[i].position.y + objs[i].position.h) {
          if (player->drect.x + player->drectbreadth >= objs[i].position.x) {
            if (player->drect.x <= objs[i].position.x + objs[i].position.w) {

              // player->dx -= player->ax * 18;
              // player->dy -= player->ay * 18;
            }
          }
        }
      }
    }
  }
  if (player->drect.x < 0) {
    player->drect.x = 0;
    player->dx = 0;
    // player->dx += 20;
  }
  if (player->drect.y < 0) {
    player->drect.y = 0;
    player->dy = 0;
    // player->dy += 20;
  }
  if ((player->drect.x + player->drectbreadth) > windowbreadth) {
    player->drect.x = windowbreadth - player->drectbreadth;
    player->dx = 0;
    // player->dx -= 20;
  }
  if ((player->drect.y + player->drectheight) > windowheight) {
    player->drect.y = windowheight - player->drectheight;
    player->dx = 0;
    // player->dy -= 20;
  }
}

void render(SDL_Renderer *renderer, Character *player, Object *objs, int size) {

  SDL_SetRenderDrawColor(renderer, 0xff, 0x0f, 0xa0, 0xff);
  SDL_RenderClear(renderer);
  for (int i = 0; i < size; i++) {

    SDL_SetRenderDrawColor(renderer, objs[i].redval, objs[i].greenval,
                           objs[i].blueval, objs[i].trans);
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
