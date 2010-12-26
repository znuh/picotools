#ifndef WVIEW_H
#define WVIEW_H

#include <stdint.h>

#include "mmap.h"
#include "wvfile.h"
#include "sdl_display.h"

typedef enum {
	INT8,
	UINT8,
	INT16,
	UINT16,
	INT32,
	UINT32,
	FLOAT,
	DOUBLE
} DTYPE_t;

typedef struct __attribute__((__packed__)) samplebuf_s {
	void *d;
	DTYPE_t dtype;

	int32_t y_ofs;

	float min_val;
	float max_val;

	uint8_t invert_y;

	//int v_range;
	//int coupling;
	//int scale;
} samplebuf_t;

typedef struct __attribute__((__packed__)) wview_s {
	//mf_t mf;

	samplebuf_t *sbuf;
	int32_t sbuf_cnt;

	waveinfo_t *wi;

	//unsigned long samples;

	//unsigned long timebase;

	//unsigned long trigger_ofs;

	uint32_t x_pos, y_pos;
	uint32_t x_cnt, y_cnt;

	uint32_t x_ofs, y_ofs;
	uint32_t target_w, target_h;

	SDL_Surface *wv_sf;
	SDL_Rect wv_rect;
}  wview_t;

wview_t *wview_init(int w, int h);
void event_loop(wview_t * wv);
int load_wave(wview_t * wv, uint8_t * ptr);

#endif
