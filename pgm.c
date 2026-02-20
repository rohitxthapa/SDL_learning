#include "polygons.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

double delta;
int grid_len = 1028;
int chunk_pool = 81;

typedef struct {
  SDL_FRect block;
  int gridx;
  int gridy;
  bool player_move_grid;
  float speed;
} character;

typedef struct {
  SDL_FRect block;
  int gridx;
  int gridy;
} viewpoint;

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
void destroy_polygons(polygons *polygons, int size) {
  for (int i = 0; i < size; i++) {
    if (polygons[i].texture) {
      SDL_DestroyTexture(polygons->texture);
      polygons[i].texture = NULL;
    }
  }
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
      if (abs(chunks[i].gx - player->gridx) > 2 ||
          abs(chunks[i].gy - player->gridy > 2)) {
        chunks[i].active = false;
        destroy_polygons(chunks[i].polygon, chunks[i].no_of_polygons);
        chunks[i].no_of_polygons = 0;
      }
    }
  }
  int sqrt_pool = 7;
  for (int i = 0; i < sqrt_pool; i++) {
    for (int j = 0; j < sqrt_pool; j++) {
      int gx = player->gridx - 3 + i;
      int gy = player->gridy - 3 + j;
      int index = hash_pos(gx, gy);
      if (!chunks[index].active) {
        chunks[index].gx = gx;
        chunks[index].gy = gy;
        chunks[index].active = true;
      } else {
        int start = index;
        while (chunks[index].active &&
               ((chunks[index].gx != player->gridx + i) ||
                (chunks[index].gy != player->gridy + j))) {
          index++;
          index %= chunk_pool;
          if (index == start) {
            SDL_Log("i borke here");
            SDL_Log("%d %d %d ", index, player->gridx, player->gridy);
            break;
          }
        }
        chunks[index].gx = player->gridx + i;
        chunks[index].gy = player->gridy + j;
        chunks[index].active = true;
      }
    }
  }
}

void load_polygons(chunks *chunks, character *player, SDL_Renderer *renderer) {
  for (int i = 0; i < chunk_pool; i++) {
    if (chunks->active && chunks->polygon[0].texture == NULL) {
      chunks[i].no_of_polygons = 5;
      for (int i = 0; i < chunks->no_of_polygons; i++) {
        get_polygons(&chunks->polygon[i], chunks->gx, chunks->gy);
        get_polygons_texture(&chunks->polygon[i], renderer);
      }
    }
  }
}

void update(character *player, viewpoint *camera) {

  const bool *keys = SDL_GetKeyboardState(NULL);
  if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
    player->block.y += player->speed * delta;
    camera->block.y += player->speed * delta;
  }
  if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
    player->block.y -= player->speed * delta;
    camera->block.y -= player->speed * delta;
  }
  if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
    player->block.x += player->speed * delta;
    camera->block.x += player->speed * delta;
  }
  if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
    player->block.x -= player->speed * delta;
    camera->block.x -= player->speed * delta;
  }
  if (player->block.x < 0) {
    player->block.x = grid_len;
    player->gridx--;
    player->player_move_grid = true;
  } else if (player->block.x > grid_len) {
    player->block.x = 0;
    player->gridx++;
  }
  if (player->block.y < 0) {
    player->block.y = grid_len;
    player->gridy--;
    player->player_move_grid = true;
  } else if (player->block.y > grid_len) {
    player->block.y = 0;
    player->gridy++;
  }

  if (camera->block.x < 0) {
    camera->block.x = grid_len;
    camera->gridx--;
  } else if (camera->block.x > grid_len) {
    camera->block.x = 0;
    camera->gridx++;
  }
  if (camera->block.y < 0) {
    camera->block.y = grid_len;
    camera->gridy--;
  } else if (camera->block.y > grid_len) {
    camera->block.y = 0;
    camera->gridy++;
  }
}

void render(SDL_Renderer *renderer, character *player, viewpoint *camera) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_FRect design = {960, 540, .w = player->block.w, .h = player->block.h};
  SDL_RenderFillRect(renderer, &design);

  SDL_RenderDebugTextFormat(
      renderer, 100.0f, 100.0f, "%d %d %f %f %d %d %f %f ", player->gridx,
      player->gridy, player->block.x, player->block.y, camera->gridx,
      camera->gridy, camera->block.x, camera->block.y);
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
  player.block = (SDL_FRect){960 - 100, 540 - 100, 200, 200};
  player.gridx = rand() % 100;
  player.gridy = rand() % 100;
  player.speed = 4000;
  player.player_move_grid = true;

  viewpoint camera;
  camera.block = (SDL_FRect){.x = 0, .y = 0, .w = 1920, .h = 1080};
  camera.gridx = player.gridx;
  camera.gridy = player.gridy;

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
    update(&player, &camera);
    render(renderer, &player, &camera);
  }

  for (int i = 0; i < chunk_pool; i++) {
    destroy_polygons(loadedchunks[i].polygon, loadedchunks[i].no_of_polygons);
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}
