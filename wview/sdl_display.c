#include "sdl_display.h"
#include "mmap.h"
#include <assert.h>

#define FPS 60

struct sdl_ctx sdl;

mf_t font;

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
//      SDL_SetAlpha(sdl.screen, SDL_SRCALPHA, 0);

	SDL_WM_SetCaption("wview", "wview");

	assert(!(map_file(&font, "Fonts/8x13.fnt", 0, 0)));
	gfxPrimitivesSetFont(font.ptr, 8, 13);

	return 0;
}

void sdl_flip(void)
{
	SDL_Flip(sdl.screen);
	SDL_FillRect(sdl.screen, NULL, 0xff000000);
}

void sdl_destroy(void)
{
	unmap_file(&font);
}
