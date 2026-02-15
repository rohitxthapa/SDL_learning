#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <stdio.h>

enum shapes {
  triangle,
  rectangle,
  pentagon,
  hexagon,
  heptagon,
  octagon,
  nonagon,
  decacgon
};

int main() {
  int n = 4;

  double degree = (n - 2) * (180.0 / n);
  printf("%f", degree);
  double degreeradian = degree * (3.1415926 / 180);
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window;
  window = SDL_CreateWindow("shapes", 1920, 1080, SDL_WINDOW_RESIZABLE);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
  SDL_SetRenderVSync(renderer, 1);
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);

  double ix = 600, iy = 800, fx = 700, fy = 800;
  double tx, ty;
  SDL_RenderLine(renderer, ix, iy, fx, fy);
  for (int i = 1; i < n; i++) {

    tx = fx + (ix - fx) * cos(degreeradian) - (iy - fy) * sin(degreeradian);
    ty = fy + (ix - fx) * sin(degreeradian) + (iy - fy) * cos(degreeradian);
    SDL_RenderLine(renderer, fx, fy, tx, ty);
    ix = fx;
    iy = fy;
    fx = tx;
    fy = ty;
  }
  SDL_RenderPresent(renderer);
  SDL_Delay(6000);
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}
