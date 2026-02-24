#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Bitmask flags for movement
#define POS_X 1
#define NEG_X 2
#define POS_Y 4
#define NEG_Y 8
#define NO_MOVE 0
#define INITIAL_LOAD -1 // Specific flag for initial generation

SDL_Window *window;
SDL_Renderer *renderer;

long int seed = 13439;
double delta = 0.016;
int window_w = 1920;
int window_h = 1080;
int grid_len = 4096; // Size of a chunk in pixels
int render_pixel = 2048;
int chunk_len = 5; // 5x5 grid of chunks

typedef struct {
  SDL_FRect block;
  int gridx;
  int gridy;
  int player_move_grid; // Stores flags
  float speed;
} character;

typedef struct {
  int sides;
  SDL_Vertex *vertices;
  float x, y; // Local to chunk
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
    SDL_Log("SDL could not initialize : %s", SDL_GetError());
    return false;
  }
  window = SDL_CreateWindow("shapes", 1920, 1080, SDL_WINDOW_RESIZABLE);
  if (window == NULL) {
    SDL_Log("SDL_Window could not initialize : %s", SDL_GetError());
    return false;
  }
  renderer = SDL_CreateRenderer(window, NULL);
  if (renderer == NULL) {
    SDL_Log("SDL_Renderer could not initialize : %s", SDL_GetError());
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
  block->sides = 3 + rand() % 10;
  block->vertices = malloc(sizeof(SDL_Vertex) * (block->sides));
  block->size = 20 + rand() % 80;
  // Position within the chunk
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
    block->vertices[i].color =
        (SDL_FColor){0.6f + (float)(rand() % 100) / 200.0f, 0.8f, 0.4f, 1.0f};
    block->vertices[i].tex_coord = (SDL_FPoint){0.5f, 0.5f};
  }

  // Triangulation (Simple fan)
  int triangles = block->sides - 2;
  int indices[triangles * 3];
  for (int i = 0; i < triangles; i++) {
    indices[i * 3 + 0] = 0;
    indices[i * 3 + 1] = i + 1;
    indices[i * 3 + 2] = i + 2;
  }

  int dimension = (int)(block->size * 2) + 10;
  block->texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, dimension, dimension);
  SDL_SetTextureBlendMode(block->texture, SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, block->texture);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  // Center geometry in texture
  SDL_RenderGeometry(renderer, NULL, block->vertices, block->sides, indices,
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
  // Unique seed per chunk
  srand(seed + (unsigned int)((gx * 73856093) ^ (gy * 19349663)));
  for (int i = 0; i < chunk->no_of_polygons; i++) {
    get_polygons(&chunk->polygon[i]);
    get_polygons_texture(&chunk->polygon[i], renderer);
  }
}

void free_chunk(chunks *chunk) {
  for (int i = 0; i < chunk->no_of_polygons; i++) {
    free_polygon(&chunk->polygon[i]);
  }
}

void load_chunks(chunks (*chunks)[5], character *player) {
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

  // Shift logic
  if (player->player_move_grid & (POS_X | NEG_X)) {
    int unit_x = (player->player_move_grid & POS_X) ? 1 : -1;
    int start = (unit_x == 1) ? 0 : chunk_len - 1;
    int end = (unit_x == 1) ? chunk_len - 1 : 0;
    int step = (unit_x == 1) ? 1 : -1;

    // Free edge
    for (int i = 0; i < chunk_len; i++)
      free_chunk(&chunks[start][i]);

    // Shift
    for (int i = start; i != end; i += step) {
      for (int j = 0; j < chunk_len; j++)
        chunks[i][j] = chunks[i + step][j];
    }

    // Generate new
    for (int i = 0; i < chunk_len; i++) {
      generate_chunk(&chunks[end][i], center_x + (end - 2), center_y + (i - 2));
    }
  }

  if (player->player_move_grid & (POS_Y | NEG_Y)) {
    int unit_y = (player->player_move_grid & POS_Y) ? 1 : -1;
    int start = (unit_y == 1) ? 0 : chunk_len - 1;
    int end = (unit_y == 1) ? chunk_len - 1 : 0;
    int step = (unit_y == 1) ? 1 : -1;

    for (int i = 0; i < chunk_len; i++)
      free_chunk(&chunks[i][start]);
    for (int i = 0; i < chunk_len; i++) {
      for (int j = start; j != end; j += step)
        chunks[i][j] = chunks[i][j + step];
    }
    for (int i = 0; i < chunk_len; i++) {
      generate_chunk(&chunks[i][end], center_x + (i - 2), center_y + (end - 2));
    }
  }
}

void update(character *player) {
  const bool *keys = SDL_GetKeyboardState(NULL);
  float move_dist = player->speed * delta;
  if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W])
    player->block.y -= move_dist;
  if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S])
    player->block.y += move_dist;
  if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D])
    player->block.x += move_dist;
  if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A])
    player->block.x -= move_dist;

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

void render(character *player, chunks (*chunks)[5]) {
  SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
  SDL_RenderClear(renderer);

  // Calculate player world position relative to the 5x5 grid
  float world_x = (player->gridx * grid_len) + player->block.x;
  float world_y = (player->gridy * grid_len) + player->block.y;

  for (int i = 0; i < chunk_len; i++) {
    for (int j = 0; j < chunk_len; j++) {
      for (int k = 0; k < chunks[i][j].no_of_polygons; k++) {
        polygons *poly = &chunks[i][j].polygon[k];
        // Calculate absolute world position of polygon
        float p_world_x = (chunks[i][j].gx * grid_len) + poly->x;
        float p_world_y = (chunks[i][j].gy * grid_len) + poly->y;

        // Position relative to screen center
        float screen_x = p_world_x - world_x + (window_w / 2);
        float screen_y = p_world_y - world_y + (window_h / 2);

        // Culling
        if (screen_x > -100 && screen_x < window_w + 100 && screen_y > -100 &&
            screen_y < window_h + 100) {
          SDL_FRect dest = {screen_x - poly->size, screen_y - poly->size,
                            poly->size * 2, poly->size * 2};
          SDL_RenderTexture(renderer, poly->texture, NULL, &dest);
        }
      }
    }
  }

  // Draw Player
  SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
  SDL_FRect p_rect = {(window_w / 2) - 25, (window_h / 2) - 25, 50, 50};
  SDL_RenderFillRect(renderer, &p_rect);

  SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
  if (!init())
    return 1;
  srand(seed);

  character player = {0};
  player.block = (SDL_FRect){grid_len / 2, grid_len / 2, 50, 50};
  player.gridx = 0;
  player.gridy = 0;
  player.speed = 1000.0f;
  player.player_move_grid = INITIAL_LOAD;

  chunks chunk_grid[5][5];
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      chunk_grid[i][j].no_of_polygons = 15;
      chunk_grid[i][j].polygon =
          malloc(chunk_grid[i][j].no_of_polygons * sizeof(polygons));
      for (int k = 0; k < chunk_grid[i][j].no_of_polygons; k++) {
        chunk_grid[i][j].polygon[k].texture = NULL;
        chunk_grid[i][j].polygon[k].vertices = NULL;
      }
    }
  }

  bool running = true;
  Uint64 frame_start = SDL_GetPerformanceCounter();
  SDL_Event event;

  while (running) {
    Uint64 now = SDL_GetPerformanceCounter();
    delta = (double)(now - frame_start) / (double)SDL_GetPerformanceFrequency();
    frame_start = now;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT)
        running = false;
    }

    if (player.player_move_grid != NO_MOVE) {
      load_chunks(chunk_grid, &player);
      player.player_move_grid = NO_MOVE;
    }
    update(&player);
    render(&player, chunk_grid);
  }

  // Cleanup
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      free_chunk(&chunk_grid[i][j]);
      free(chunk_grid[i][j].polygon);
    }
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
