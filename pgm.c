#include "polygons.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
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
int hash_pos(int x, int y) {
  int num = (x * 7907) ^ (y * 7919);
  return num % chunk_pool;
}
void init_chunks(chunks *chunks, character *player) {
  int sqrt_pool = 9;
  for (int i = 0; i < sqrt_pool; i++) {
    for (int j = 0; j < sqrt_pool; j++) {
      int index = hash_pos(player->gridx - 4 + i, player->gridy - 4);
      if (!chunks[index].active) {
        chunks[index].gx = player->gridx + i;
        chunks[index].gy = player->gridy + j;
      } else {
        while (chunks[index].active ||
               ((chunks[index].gx != player->gridx + i) ||
                (chunks[index].gy != player->gridy + j))) {
          index++;
          index %= chunk_pool;
        }
        chunks[index].gx = player->gridx + i;
        chunks[index].gy = player->gridy + j;
      }
    }
  }
}
void get_polygons_array(chunks *chunks, int pos, SDL_Renderer *renderer) {
  for (int i = 0; i < chunks->no_of_polygons; i++) {
    get_polygons(&chunks->polygon[i], chunks->gx, chunks->gy);
    get_polygons_texture(&chunks->polygon[i], renderer);
  }
}
void loadchunks(chunks *chunks, character *player, SDL_Renderer *renderer) {
  for (int i = 0; i < chunk_pool; i++) {
    chunks[i].no_of_polygons = 5;
    get_polygons_array(chunks, i, renderer);
  }
}

void update(character *player, viewpoint *camera) {

  const bool *keys = SDL_GetKeyboardState(NULL);
  if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
    player->block.y += 100;
    camera->block.y += 100;
  }
  if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
    player->block.y += -100;
    camera->block.y += -100;
  }
  if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
    player->block.x += 100;
    camera->block.x += 100;
  }
  if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
    player->block.x += -100;
    camera->block.x += -100;
  }
  if (player->block.x < 0) {
    // mere we might need to decrease grid_len by one but i cant wrap my head
    // around it rn so next time
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
    // mere we might need to decrease grid_len by one but i cant wrap my head
    // around it rn so next time
    camera->block.x = grid_len;
    camera->gridx--;
  } else if (player->block.x > grid_len) {
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

  srand(12389);

  character player;
  player.block =
      (SDL_FRect){.x = 960 - 100, .y = 540 - 100, .w = 300, .h = 300};
  player.gridx = rand() % 1000;
  player.gridy = rand() % 1000;

  player.player_move_grid = true;
  viewpoint camera;
  camera.block = (SDL_FRect){.w = 1920, .h = 1080};
  camera.block.x = 0;
  camera.gridx = player.gridx;
  camera.block.y = 0;
  camera.gridy = player.gridy;

  chunks loadedchunks[chunk_pool];

  bool running = true;
  Uint64 frame_start_ticks = SDL_GetPerformanceCounter(), frame_end_ticks;
  SDL_Event event;
  loadchunks(loadedchunks, &player, renderer);

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
    if (player.player_move_grid) {
      init_chunks(loadedchunks, &player);
      player.player_move_grid = false;
    }
    update(&player, &camera);
    render(renderer, &player, &camera);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  }

  for (int i = 0; i < chunk_pool; i++) {
    for (int j = 0; j < loadedchunks[i].no_of_polygons; j++) {
      SDL_DestroyTexture(loadedchunks[i].polygon[j].texture);
    }
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}
