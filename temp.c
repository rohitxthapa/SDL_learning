#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define POS_X 1
#define NEG_X 2
#define POS_Y 4
#define NEG_Y 8

SDL_Window *window;
SDL_Renderer *renderer;

long int seed = 13439;
double delta = 0.016;
int window_w = 1920;
int window_h = 1080;
int grid_len = 4096;
int render_pixel = 2048;
int chunk_pool = 25;
int chunk_len = 5;
SDL_FRect screen_rect = {0, 0, 0, 0};

typedef struct {
  SDL_FRect block;
  int gridx;
  int gridy;
  int player_move_grid;
  float speed;
} character;

typedef struct {
  int sides;
  SDL_Vertex *vertices;
  float x, y;
  float size;
  float degreeradian;
  float degreeradiantoxaxis;
  SDL_Texture *texture;
} polygons;

typedef struct {
  int gx;
  int gy;
  polygons *polygon;
  int no_of_polygons;
} chunks;

bool init() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL could not initilize : %s", SDL_GetError());
    return false;
  }
  window = SDL_CreateWindow("shapes", 1920, 1080, SDL_WINDOW_RESIZABLE);
  if (window == NULL) {
    SDL_Log("SDL_Window could not initilize : %s", SDL_GetError());
    return false;
  }
  renderer = SDL_CreateRenderer(window, NULL);
  if (renderer == NULL) {
    SDL_Log("SDL_Renderer could not initilize : %s", SDL_GetError());
    return false;
  }
  if (SDL_SetRenderVSync(renderer, 1)) {
    SDL_Log("VSync activated");
  } else {
    SDL_Log("SDL_VSync couldn't activate %s", SDL_GetError());
  }
  return true;
}

void get_polygons(polygons *block) {
  block->sides = 3 + rand() % 15;
  block->vertices = malloc(sizeof(SDL_Vertex) * (block->sides));
  block->degreeradian = ((block->sides - 2) * 3.14159253589 / block->sides);
  block->size = 50 + rand() % 50;
  block->x = rand() % chunk_len;
  block->y = rand() % chunk_len;
  block->degreeradiantoxaxis = (rand() % 360) * (3.141592653589) / 180;
}

void get_polygons_texture(polygons *block, SDL_Renderer *renderer) {
  for (int i = 0; i < block->sides; i++) {
    float theta =
        block->degreeradiantoxaxis + (2.0f * 3.141592653589 * i) / block->sides;
    block->vertices[i].position.x = block->size * (1 + cos(theta)) + 1;
    block->vertices[i].position.y = block->size * (1 + sin(theta)) + 1;
  }
  for (int i = 0; i < block->sides; i++)
    block->vertices[i].color = (SDL_FColor){0.6, 0.8, 0.4, 1};

  int triangles = block->sides - 2;
  int incides[triangles * 3];
  int n = 0;
  for (int i = 0; i < triangles; i++) {

    for (int j = 0; j < 3; j++) {
      incides[i * 3 + j] = (j == 0) ? 0 : j + n;
    }
    n++;
  }
  int dimention = (int)(block->size * 2) + 2;
  block->texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, dimention, dimention);
  SDL_SetTextureBlendMode(block->texture, SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, block->texture);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  SDL_RenderGeometry(renderer, NULL, block->vertices, block->sides, incides,
                     triangles * 3);
  SDL_SetRenderTarget(renderer, NULL);
}
void free_polygon(polygons *block) {
  if (block->vertices) {
    free(block->vertices);
    block->vertices = NULL;
  }

  if (block->texture) {
    SDL_DestroyTexture(block->texture);
    block->texture = NULL;
  }
}

void generate_chunk(chunks *chunk, int gx, int gy) {
  chunk->gx = gx;
  chunk->gy = gy;
  srand(seed + ((gx * 73856093) ^ (gy * 19349663)));
  for (int i = 0; i < chunk->no_of_polygons; i++) {
    get_polygons(&chunk->polygon[i]);
    get_polygons_texture(&chunk->polygon[i], renderer);
  }
}

void free_chunk(chunks *chunk) {
  for (int i = 0; i < chunk->no_of_polygons; i++) {
    free_polygon(&chunk->polygon[i]);
  }
  chunk->gx = chunk->gy = chunk->no_of_polygons = 0;
}

void load_chunks(chunks (*chunks)[chunk_len], character *player) {
  int center_x = player->gridx;
  int center_y = player->gridy;

  if (player->player_move_grid == -1) {
    for (int i = 0; i < chunk_len; i++) {
      for (int j = 0; j < chunk_len; j++) {
        generate_chunk(&chunks[i][j], center_x + i - 2, center_y + j - 2);
      }
    }
    return;
  }

  int del_x, add_x;
  int del_y, add_y;
  int unit_x = 0, unit_y = 0;
  if (player->player_move_grid & POS_X) {
    del_x = 0;
    add_x = 4;
    unit_x = 1;
  } else if (player->player_move_grid & NEG_X) {
    del_x = 4;
    add_x = 0;
    unit_x = -1;
  }
  if (player->player_move_grid & POS_Y) {
    del_y = 4;
    add_y = 0;
    unit_y = -1;
  } else if (player->player_move_grid & NEG_Y) {
    del_y = 0;
    add_y = 4;
    unit_y = 1;
  }
  int temp;
  if (unit_x != 0) {
    for (int i = 0; i < chunk_len; i++) {
      free_chunk(&chunks[del_x][i]);
    }
    while (del_x + unit_x != add_x) {
      temp = del_x + unit_x;
      for (int i = 0; i < chunk_len; i++) {
        chunks[del_x][i] = chunks[temp][i];
      }
      del_x = temp;
    }
    for (int i = 0; i < chunk_len; i++) {
      generate_chunk(&chunks[add_x][i], center_x + add_x - 2, center_y + i - 2);
    }
  }
  if (unit_y != 0) {
    for (int i = 0; i < chunk_len; i++) {
      free_chunk(&chunks[i][del_y]);
    }
    while (del_y + unit_y != add_y) {
      temp = del_y + unit_y;
      for (int i = 0; i < chunk_len; i++) {
        chunks[i][del_y] = chunks[i][temp];
      }
      del_y = temp;
    }
    for (int i = 0; i < chunk_len; i++) {
      generate_chunk(&chunks[i][add_y], center_x + i - 2, center_y + add_y - 2);
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
    player->player_move_grid |= NEG_X;
  } else if (player->block.x > grid_len) {
    player->block.x = 0;
    player->gridx++;
    player->player_move_grid |= POS_X;
  }
  if (player->block.y < 0) {
    player->block.y = grid_len;
    player->gridy--;
    player->player_move_grid |= NEG_Y;
  } else if (player->block.y > grid_len) {
    player->block.y = 0;
    player->gridy++;
    player->player_move_grid |= POS_Y;
  }
}

void render(character *player, chunks (*chunks)[chunk_len]) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
  int x_pos = player->block.x + player->block.w / 2;
  int y_pos = player->block.y + player->block.h / 2;
  int x_flag = (x_pos > render_pixel) ? 1 : -1;
  int y_flag = (y_pos > render_pixel) ? 1 : -1;

  for (int i = 0; i <= 1; i++) {
    for (int j = 0; j <= 1; j++) {
      int x = 2 + i * x_flag;
      int y = 2 + j * y_flag;
      for (int k = 0; k < chunks[x][y].no_of_polygons; k++) {
        int poly_pos_x =
            x * chunk_len + chunks[x][y].polygon[k].x - 2 * chunk_len - x_pos;
        int poly_pos_y =
            y * chunk_len + chunks[x][y].polygon[k].y - 2 * chunk_len - y_pos;
        if (abs(poly_pos_x) < render_pixel && abs(poly_pos_y) < render_pixel) {
          int size = chunks[x][y].polygon[k].size * 2 + 2;
          SDL_FRect destination = {poly_pos_x, poly_pos_y, size, size};
          SDL_FRect demo = {300, 800, size, size};
          SDL_RenderTexture(renderer, chunks[x][y].polygon[k].texture, NULL,
                            &demo);
        }
      }
    }
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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

int main(int argc, char *argv[]) {

  if (!init()) {
    return 1;
  }
  srand(seed);

  character player;
  player.block = (SDL_FRect){0, 0, 100, 100};
  player.gridx = 0;
  player.gridy = 0;
  player.speed = 1000.0f;
  player.player_move_grid = -1;

  chunks chunks[chunk_len][chunk_len];
  for (int i = 0; i < chunk_len; i++) {
    for (int j = 0; j < chunk_len; j++) {
      chunks[i][j].no_of_polygons = 20;
      chunks[i][j].polygon =
          malloc(chunks[i][j].no_of_polygons * sizeof(polygons));
      for (int k = 0; k < 5; k++) {
        chunks[i][j].polygon[k].texture = NULL;
        chunks[i][j].polygon[k].vertices = NULL;
      }
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

    if (player.player_move_grid != 0) {
      load_chunks(chunks, &player);
      player.player_move_grid = 0;
    }
    update(&player);
    render(&player, chunks);
  }
  for (int i = 0; i < chunk_len; i++) {
    for (int j = 0; j < chunk_len; j++) {
      for (int k = 0; k < chunks[i][j].no_of_polygons; k++) {
        free_chunk(&chunks[i][j]);
      }
      free(chunks[i][j].polygon);
    }
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}
