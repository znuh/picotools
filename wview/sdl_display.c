#include "sdl_display.h"
#include <assert.h>

struct sdl_ctx sdl;

int sdl_init(int w, int h)
{
	Uint8 video_bpp;
	Uint32 videoflags;

	sdl.w = w;
	sdl.h = h;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "can't initialize SDL video\n");
		exit(1);
	}
	atexit(SDL_Quit);

	video_bpp = 32;
	videoflags = SDL_HWSURFACE | SDL_DOUBLEBUF /*| SDL_FULLSCREEN */ ;

	if ((sdl.screen =
	     SDL_SetVideoMode(sdl.w, sdl.h, video_bpp, videoflags)) == NULL) {
		fprintf(stderr, "can't set video mode %dx%d\n", sdl.w, sdl.h);
		exit(2);
	}

	SDL_WM_SetCaption("wview", "wview");

	return 0;
}
