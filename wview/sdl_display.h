#ifndef SDL_DISPLAY_H
#define SDL_DISPLAY_H

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

struct sdl_ctx {
	SDL_Surface *screen;
	int h, w;
};

int sdl_init(int w, int h);
void sdl_destroy(void);
void sdl_flip(void);

#endif
