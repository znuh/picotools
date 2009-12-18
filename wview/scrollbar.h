#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "sdl_display.h"

typedef struct scrollbar_s {
	/* surface stuff */
	SDL_Surface *dst;
	int sf_xofs;
	int sf_yofs;
	int sf_w;
	int sf_h;
	
	/* params */
	unsigned long max_len;
	
	/* user modifyable params */
	unsigned long pos;
	unsigned long len;
} scrollbar_t;

scrollbar_t *scrollbar_create(SDL_Surface *sf, int sf_xofs, int sf_yofs, int sf_w, int sf_h, unsigned long max_len);

void scrollbar_destroy(scrollbar_t *sb);

void scrollbar_draw(scrollbar_t *sb);

void scrollbar_event(scrollbar_t *sb, SDL_Event *evt);

#endif
