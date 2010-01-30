#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "sdl_display.h"

#define	GRIP_SIZE	7
#define	GRIP_SPACE	3

#define UPPER_LIMIT(a,b)	((a) = (((a) > (b)) ? (b) : (a)))
#define LOWER_LIMIT(a,b)	((a) = (((a) < (b)) ? (b) : (a)))

typedef enum MouseMode {
	SB_OUT,
	SB_START,
	SB_POS,
	SB_END
} sb_MouseMode;

typedef struct scrollbar_s {
	/* surface stuff */
	SDL_Surface *dst;
	int sf_xofs;
	int sf_yofs;
	int sf_w;
	int sf_h;

	/* mouse handling */
	sb_MouseMode mouse_mode;
	int mouse_ofs;

	/* UI element positions */
	int grip_start;
	int grip_end;

	/* params */
	unsigned long max_len;

	/* user modifyable params */
	int pos;
	int len;

} scrollbar_t;

scrollbar_t *scrollbar_create(SDL_Surface * sf, int sf_xofs, int sf_yofs,
			      int sf_w, int sf_h, unsigned long max_len);

void scrollbar_destroy(scrollbar_t * sb);

void scrollbar_draw(scrollbar_t * sb);

void scrollbar_event(scrollbar_t * sb, SDL_Event * evt);

#endif
