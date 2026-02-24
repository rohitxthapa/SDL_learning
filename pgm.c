#include "polygons.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

double delta = 0.016;
int window_w = 1920;
int window_h = 1080;
int grid_len = 1024;
int chunk_pool = 81;
SDL_FRect screen_rect;
float zoom = 1.0f;

typedef struct {
  SDL_FRect block;
  int gridx;
  int gridy;
  bool player_move_grid;
  float speed;
} character;

typedef struct {
  int gridx;
  int gridy;
  bool present;
} gridpos;

typedef struct {
  int gx;
  int gy;
  bool active;
  polygons polygon[5];
  int no_of_polygons;
} chunks;

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

int hash_pos(int x, int y) {
  long long num = ((long long)x * 73856093LL) ^ ((long long)y * 19349669LL);
  int mod = (int)(num % chunk_pool);
  if (mod < 0) {
    mod += chunk_pool;
  }
  return mod;
}
void load_chunks(chunks *chunks, character *player) {
  for (int i = 0; i < chunk_pool; i++) {
    if (chunks[i].active) {
      if (abs(chunks[i].gx - player->gridx) > 4 ||
          abs(chunks[i].gy - player->gridy) > 4) {
        chunks[i].active = false;
        for (int j = 0; j < chunks[i].no_of_polygons; j++) {
          free_polygon(&chunks[i].polygon[j]);
        }
        chunks[i].no_of_polygons = 0;
      }
    }
  }
  int sqrt_pool = 9;
  for (int i = 0; i < sqrt_pool; i++) {
    for (int j = 0; j < sqrt_pool; j++) {
      int gx = player->gridx - 4 + i;
      int gy = player->gridy - 4 + j;
      int index = hash_pos(gx, gy);
      if (!chunks[index].active) {
        chunks[index].gx = gx;
        chunks[index].gy = gy;
        chunks[index].active = true;
      } else {
        int start = index;
        while (chunks[index].active &&
               ((chunks[index].gx != gx) || (chunks[index].gy != gy))) {
          index++;
          index %= chunk_pool;
          if (index == start) {
            SDL_Log("i borke here");
            SDL_Log("%d %d %d ", index, player->gridx, player->gridy);
            break;
          }
        }
        chunks[index].gx = gx;
        chunks[index].gy = gy;
        chunks[index].active = true;
      }
    }
  }
}

void load_polygons(chunks *chunks, character *player, SDL_Renderer *renderer) {
  for (int i = 0; i < chunk_pool; i++) {
    if (chunks[i].active && chunks[i].polygon[0].texture == NULL) {
      chunks[i].no_of_polygons = 5;
      for (int j = 0; j < chunks[i].no_of_polygons; j++) {
        get_polygons(&chunks[i].polygon[j], chunks[i].gx, chunks[i].gy);
        get_polygons_texture(&chunks[i].polygon[j], renderer);
      }
    }
  }
}

void update(character *player) {

  const bool *keys = SDL_GetKeyboardState(NULL);
  if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
    player->block.y += player->speed * delta;
  }
  if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
    player->block.y -= player->speed * delta;
  }
  if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
    player->block.x += player->speed * delta;
  }
  if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
    player->block.x -= player->speed * delta;
  }
  if (player->block.x < 0) {
    player->block.x = grid_len;
    player->gridx--;
    player->player_move_grid = true;
  } else if (player->block.x > grid_len) {
    player->block.x = 0;
    player->gridx++;
    player->player_move_grid = true;
  }
  if (player->block.y < 0) {
    player->block.y = grid_len;
    player->gridy--;
    player->player_move_grid = true;
  } else if (player->block.y > grid_len) {
    player->block.y = 0;
    player->gridy++;
    player->player_move_grid = true;
  }
}

void render_polygons(chunks *chunks, character *player,
                     SDL_Renderer *renderer) {
  for (int i = -2; i <= 2; i++) {
    for (int j = -2; j <= 2; j++) {
      if (abs(i) + abs(j) <= 2) {
        int x = player->gridx + i;
        int y = player->gridy + j;
        int index = hash_pos(x, y);
        int start = index;
        if (chunks[index].gx != x || chunks[index].gy != y) {
          while (chunks[index].active) {
            index++;
            if (index == start) {
              index = -1;
              break;
            }
          }
        }
        if (index >= 0) {
          for (int k = 0; k < chunks[index].no_of_polygons; k++) {
            SDL_FRect destination = {
                .x = (i * grid_len + chunks[index].polygon[k].x -
                      player->block.x),
                .y = (j * grid_len + chunks[index].polygon[k].y +
                      player->block.y),
                .w = chunks[index].polygon[k].size * 2 + 2,
                .h = chunks[index].polygon[k].size * 2 + 2};
            // SDL_FRect demo = {1300.0, 800.0, 200.0, 200.0};
            SDL_RenderTexture(renderer, chunks[index].polygon[k].texture, NULL,
                              &destination);
          }
        }
      }
    }
  }
}

void render(SDL_Renderer *renderer, character *player, chunks *chunks) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  render_polygons(chunks, player, renderer);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  // something need to be done here replace design by screen_rect
  screen_rect.x = ((float)window_w / 2) - (player->block.w / 2);
  screen_rect.y = ((float)window_h / 2) - (player->block.h / 2);
  screen_rect.w = player->block.w;
  screen_rect.h = player->block.h;
  SDL_RenderFillRect(renderer, &screen_rect);

  SDL_RenderDebugTextFormat(renderer, 100.0f, 100.0f, "%d %d %f %f ",
                            player->gridx, player->gridy, player->block.x,
                            player->block.y);
  SDL_RenderPresent(renderer);
}

int main() {
  SDL_Window *window;
  SDL_Renderer *renderer;
  if (!init(&window, &renderer)) {
    return 1;
  }

  srand(13439);

  character player;
  player.block = (SDL_FRect){0, 0, 100, 100};
  player.gridx = rand() % 100;
  player.gridy = rand() % 100;
  player.speed = 1000;
  player.player_move_grid = true;

  chunks loadedchunks[chunk_pool];
  for (int i = 0; i < chunk_pool; i++) {
    loadedchunks[i].active = false;
    loadedchunks[i].gx = 0;
    loadedchunks[i].gy = 0;
    loadedchunks[i].no_of_polygons = 0;
    for (int t = 0; t < 5; t++) {
      loadedchunks[i].polygon[t].texture = NULL;
    }
  }

  bool running = true;
  Uint64 frame_start_ticks = SDL_GetPerformanceCounter(), frame_end_ticks;
  SDL_Event event;

  while (running) {
    frame_end_ticks = SDL_GetPerformanceCounter();
    delta = (double)(frame_end_ticks - frame_start_ticks) /
            (double)SDL_GetPerformanceFrequency();
    frame_start_ticks = frame_end_ticks;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }

    if (player.player_move_grid) {
      load_chunks(loadedchunks, &player);
      load_polygons(loadedchunks, &player, renderer);
      player.player_move_grid = false;
    }
    update(&player);
    render(renderer, &player, loadedchunks);
  }

  for (int i = 0; i < chunk_pool; i++) {
    for (int j = 0; j < loadedchunks[i].no_of_polygons; j++) {
      free_polygon(&loadedchunks[i].polygon[j]);
    }
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}
