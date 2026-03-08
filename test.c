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
#define NO_MOVE 0
#define INITIAL_LOAD -1

SDL_Window *window;
SDL_Renderer *renderer;

long int seed = 13439;
double delta = 0.016;
int window_w = 1920;
int window_h = 1080;
int grid_len = 4096;
int render_pixel = 1024;
int chunk_pool = 25;
int chunk_len = 5;

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
  bool active;
} chunks;

chunks temp;

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
  block->x = rand() % grid_len;
  block->y = rand() % grid_len;
  block->degreeradiantoxaxis = (rand() % 360) * (3.141592653589) / 180;
}

void get_polygons_texture(polygons *block, SDL_Renderer *renderer) {
  for (int i = 0; i < block->sides; i++) {
    float theta =
        block->degreeradiantoxaxis + (2.0f * 3.141592653589 * i) / block->sides;
    block->vertices[i].position.x = block->size * (1 + cos(theta)) + 1;
    block->vertices[i].position.y = block->size * (1 + sin(theta)) + 1;
    block->vertices[i].color = (SDL_FColor){0.8f,0.5f,0.2f,1};
  }

  int triangles = block->sides - 2;
  int incides[triangles * 3];
  for (int i = 0; i < triangles; i++) {
      incides[i * 3 + 0] = 0;
      incides[i * 3 + 1] = i + 1;
      incides[i * 3 + 2] = i + 2;
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

void generate_chunk(chunks *chunk, int gx, int gy) {
        chunk->gx = gx;
        chunk->gy = gy;
        srand(seed + ((gx * 73856093) ^ (gy * 19349663)));
        for (int i = 0; i < chunk->no_of_polygons; i++) {
          get_polygons(&chunk->polygon[i]);
          get_polygons_texture(&chunk->polygon[i], renderer);
        }
        chunk->active = true ;
}

void free_chunk(chunks *chunk) {
    if(chunk->active){
  for (int i = 0; i < chunk->no_of_polygons; i++) {
    if (chunk->polygon[i].vertices) {
      free(chunk->polygon[i].vertices);
      chunk->polygon[i].vertices = NULL;
    }
    if (chunk->polygon[i].texture) {
      SDL_DestroyTexture(chunk->polygon[i].texture);
      chunk->polygon[i].texture = NULL;
    }
  }
  chunk->active = false ;
    }
}

void load_chunks(chunks (*chunks)[chunk_len], character *player) {
  int center_x = player->gridx;
  int center_y = player->gridy;

  if (player->player_move_grid == INITIAL_LOAD) {
    for (int i = 0; i < chunk_len; i++) {
      for (int j = 0; j < chunk_len; j++) {
        generate_chunk(&chunks[i][j], center_x + i - 2, center_y + j - 2);
      }
    }
    return;
  }

  int state = player->player_move_grid;

  if (state & POS_X || state & NEG_X) {
    int unit = (state & POS_X) ? 1 : -1;
    int start = (state & POS_X) ? 0 : chunk_len - 1;
    int end = abs(chunk_len - 1 - start);

    for (int j = 0; j < chunk_len; j++) {
      free_chunk(&chunks[start][j]);

      for (int i = start; i != end; i += unit) {
        temp = chunks[i][j];
        chunks[i][j] = chunks[i + unit][j];
        chunks[i + unit][j] = temp;
      }
      generate_chunk(&chunks[end][j], center_x + end - 2, center_y + j - 2);
    }
  }

  if (state & POS_Y || state & NEG_Y) {
    int unit = (state & POS_Y) ? 1 : -1;
    int start = (state & POS_Y) ? 0 : chunk_len - 1;
    int end = abs(chunk_len - 1 - start);

    for (int i = 0; i < chunk_len; i++) {
      free_chunk(&chunks[i][start]);

      for (int j = start; j != end; j += unit) {
        temp = chunks[i][j];
        chunks[i][j] = chunks[i][j + unit];
        chunks[i][j + unit] = temp;
      }
      generate_chunk(&chunks[i][end], center_x + i - 2, center_y + end - 2);
    }
  }
}

void update(character *player) {

  const bool *keys = SDL_GetKeyboardState(NULL);
  int dir_x=0,dir_y=0;
  if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
      dir_y = 1;
  }
  if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
      dir_y = -1;
  }
  if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
      dir_x = 1 ;
  }
  if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
      dir_x = -1 ;
  }
  float state = ((dir_x!=0)&&(dir_y!=0))? 0.7071067f : 1.0f;
  player->block.x += dir_x * state * player->speed * delta ;
  player->block.y += dir_y * state * player->speed * delta ;

  if (player->block.x < 0) {
    player->block.x = grid_len;
    player->gridx--;
    player->player_move_grid |= NEG_X;
  }
  if (player->block.x > grid_len) {
    player->block.x = 0;
    player->gridx++;
    player->player_move_grid |= POS_X;
  }
  if (player->block.y < 0) {
    player->block.y = grid_len;
    player->gridy--;
    player->player_move_grid |= NEG_Y;
  }
  if (player->block.y > grid_len) {
    player->block.y = 0;
    player->gridy++;
    player->player_move_grid |= POS_Y;
  }
}

void render(character *player, chunks (*chunks)[5]) {
  SDL_SetRenderDrawColor(renderer, 30, 60, 30, 255);
  SDL_RenderClear(renderer);

  long long int world_x = (player->gridx * grid_len) + player->block.x;
  long long int world_y = (player->gridy * grid_len) + player->block.y;

  for (int i = 0; i < chunk_len; i++) {
    for (int j = 0; j < chunk_len; j++) {
      for (int k = 0; k < chunks[i][j].no_of_polygons; k++) {
        polygons *poly = &chunks[i][j].polygon[k];

        long long int p_world_x = (chunks[i][j].gx * grid_len) + poly->x;
        long long int p_world_y = (chunks[i][j].gy * grid_len) + poly->y;

        float screen_x = (window_w / 2.0f) + p_world_x - world_x ;
        float screen_y = (window_h / 2.0f) - p_world_y + world_y ;

        if (screen_x > -100 && screen_x < window_w + 100 && screen_y > -100 &&
            screen_y < window_h + 100) {
          SDL_FRect dest = {screen_x - poly->size, screen_y - poly->size,
                            poly->size * 2, poly->size * 2};
          SDL_RenderTexture(renderer, poly->texture, NULL, &dest);
        }
      }
    }
  }

  SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
  SDL_FRect p_rect = {(window_w / 2.0f) - 25, (window_h / 2.0f) - 25, 50, 50};
  SDL_RenderFillRect(renderer, &p_rect);

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
  player.speed = 4000.0f;
  player.player_move_grid = -1;

  chunks chunks[chunk_len][chunk_len];
  for (int i = 0; i < chunk_len; i++) {
    for (int j = 0; j < chunk_len; j++) {
        chunks[i][j].active = false ;
      chunks[i][j].no_of_polygons = 20;
      chunks[i][j].polygon =
          malloc(chunks[i][j].no_of_polygons * sizeof(polygons));
      for (int k = 0; k < chunks[i][j].no_of_polygons; k++) {
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

    if (player.player_move_grid != NO_MOVE) {
      load_chunks(chunks, &player);
      player.player_move_grid = NO_MOVE;
    }
    update(&player);
    render(&player, chunks);
  }
  for (int i = 0; i < chunk_len; i++) {
    for (int j = 0; j < chunk_len; j++) {
        free_chunk(&chunks[i][j]);
    }
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}
