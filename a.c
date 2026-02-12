#include <SDL3/SDL.h>
#include <assert.h>
#include <stdio.h>

struct particle {};

int main(int argc, char *argv[]) {
  // 1. INITIALIZE SDL
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());

    return -1;
  }

  // 2. CREATE WINDOW
  SDL_Window *window =
      SDL_CreateWindow("title i gave", 500, 700, SDL_WINDOW_RESIZABLE);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

  if (!window) {
    SDL_Log("Window creation failed: %s", SDL_GetError());
    return -1;
  }
  if (renderer == NULL) {
    assert(0 && "not able tyo create accelerated rendere ");
  }

  SDL_SetRenderDrawColor(renderer, 0x00, 0xAA, 0xff, 0xff);

  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);

  SDL_RenderLine(renderer, 0.0f, 0.0f, 100.0f, 50.0f);

  SDL_FRect rect = {.x = 100, .y = 50, .w = 100, .h = 140};

  SDL_RenderRect(renderer, &rect);
  // SDL_RenderPoints(renderer, ) SDL_RenderPresent(renderer);

  // 3. SET MOUSE POSITION
  //  int w, h;
  //   SDL_GetWindowSize(window, &w, &h);
  //  SDL_WarpMouseInWindow(window, w / 2, h / 2);

  // 4. LOAD ASSET (The BMP file)
  // SDL_Surface* msurface = SDL_LoadSurface("./deadpool2.bmp");
  // if (msurface == NULL) {
  // This will stop the program and tell you the path is wrong
  //   assert(0 && "Improper file path: Could not find deadpool2.bmp");
  //   return -1;
  //  }

  // 5. DRAW TO WINDOW SURFACE
  // Get the surface that belongs to the window
  //  SDL_Surface* windowsurface = SDL_GetWindowSurface(window);
  //  if (windowsurface != NULL) {
  // Copy msurface onto windowsurface
  //  SDL_BlitSurface(msurface, NULL, windowsurface, NULL);
  // In SDL3, when using GetWindowSurface, you often need to update it to show
  // changes
  // SDL_UpdateWindowSurface(window);
  //} else {
  //  SDL_Log("Could not get window surface: %s", SDL_GetError());
  //  }

  // 6. MAIN LOGIC / GAME LOOP
  SDL_Event event;
  bool running = true;
  Uint64 lasttick = SDL_GetTicks();
  Uint64 currenttick;

  while (running) {
    // Check for Input
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }

    // Frame Timing (Cap to ~60 FPS)
    currenttick = SDL_GetTicks();
    Uint64 elapsed = currenttick - lasttick;
    if (elapsed < 17) {
      SDL_Delay(17 - elapsed);
    }
    lasttick = SDL_GetTicks();
  }

  // 7. CLEANUP
  //   SDL_DestroySurface(msurface); // Free the image memory
  SDL_DestroyWindow(window); // Close the window
  SDL_Quit();                // Shut down SDL

  return 0;
}
