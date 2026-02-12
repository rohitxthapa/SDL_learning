#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

void *initialization();

int main(int argv, char *argc[]) {
  // 1. INITIALIZATION
  // Start the SDL Video Subsystem, which is needed for windows and rendering.
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  // Create a resizable window.
  int windowbreadth = 640, windowheight = 400;
  SDL_Window *window = SDL_CreateWindow("hello window", windowbreadth,
                                        windowheight, SDL_WINDOW_RESIZABLE);
  if (window == NULL) {
    printf("window could not be created ! SDL_Error : %s", SDL_GetError());
    SDL_Quit();
    return 1;
  }
  // Create a hardware-accelerated renderer for the window.
  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
  if (renderer == NULL) {
    printf("renderer could no tbe created ! SDL_Error : %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  if (SDL_SetRenderVSync(renderer, 1)) {
    SDL_Log("vsync activated");
  } else {
    SDL_Log("vsync is not activated");
  }
  // 2. RESOURCE LOADING
  // Load the BMP image from the disk into a CPU_based surface.
  SDL_Surface *surface = SDL_LoadBMP("firsttry.bmp");
  if (surface == NULL) {
    printf("unable to load image to surface! SDL_Error: %s\n", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  // Create a GPU_based texture from the surface. Textures are much faster to
  // draw
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  // Surface is no longer needed
  SDL_DestroySurface(surface);
  if (texture == NULL) {
    printf("unable to create texture from surface! SDL_Error: %s\n",
           SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  // 3.GAME STATE SETUP
  //  Destination: Where to draw on the screen (x,y) and wwhat size (w,h).
  int drectheight = 100, drectbreadth = 100;
  SDL_FRect drect = {100, 100, drectbreadth, drectheight};
  int srectheight = 48, srectbreadth = 48;
  SDL_FRect srect = {0, 0, srectbreadth, srectheight};
  // to show animation
  int anicount = 0;
  Uint64 anistartticks = SDL_GetTicks(), aniendticks;
  // Source: What part of the texture to draw.
  int running = 1;
  SDL_Event event;

  // variables for fame capping
  Uint64 start = SDL_GetPerformanceCounter(), end;
  double delta;
  int i = 0;

  // 4. MAIN GAME LOOP
  while (running) {
    // SDL_Log("%d", i++); // to test teh fps
    //  Get a snapshot of the current state of all keyboard keys.
    const bool *keys = SDL_GetKeyboardState(NULL);

    // Process all pending events in the queue.
    while (SDL_PollEvent(&event)) {
      // Exit the loop if the user closes the window.
      if (event.type == SDL_EVENT_QUIT) {
        running = 0;
      }
    }

    // UPDATE game state based on input.
    // This is to keep the  character inside the window.
    // Tried inplementing movement with delta
    bool moving = keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W] ||
                  keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S] ||
                  keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A] ||
                  keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D];

    aniendticks = SDL_GetTicks();
    if (1 < ((aniendticks - anistartticks) / 69)) {
      if (moving) {
        anicount++;
        if (anicount < 4) {
          srect.x += 96;
        } else {
          anicount = 0;
          srect.x = 0;
        }
      } else {
        anicount = 0;
        srect.x = 0;
      }
      anistartticks = aniendticks;
    }

    if (moving) {
      if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
        if (drect.y > 0)
          drect.y -= 120 * delta;
      }
      if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
        if (drect.y + drectheight < windowheight)
          drect.y += 120 * delta;
      }
      if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
        if (drect.x + drectbreadth < windowbreadth)
          drect.x += 120 * delta;
      }
      if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
        if (drect.x > 0)
          drect.x -= 120 * delta;
      }
    }

    // RENDER the scene
    // Dark blue background
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);
    // Clear the screen with the draw color.
    SDL_RenderClear(renderer);
    // Draw the texutre piece defined by srect to the screen location definded
    // by drect.
    SDL_RenderTexture(renderer, texture, &srect, &drect);
    // Show the rendered frame.
    SDL_RenderPresent(renderer);

    // frame rate capping logic
    end = SDL_GetPerformanceCounter();

    delta = (double)(end - start) / (double)(SDL_GetPerformanceFrequency());
    if (delta < 0.0167) {
      SDL_Delay(1000 * (0.0167 - delta));
    }
    // this is so we can get correct delta
    start = end;
  }
  // 5. CLEANUP
  // Destroy all the created resource to free memory
  SDL_DestroyRenderer(renderer);
  SDL_DestroyTexture(texture);
  SDL_DestroyWindow(window);
  // Shut all the subsystems.
  SDL_Quit();

  return 0;
}
