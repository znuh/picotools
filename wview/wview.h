#ifndef WVIEW_H
#define WVIEW_H

#include <stdint.h>

#include "mmap.h"
#include "wvfile.h"

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

typedef struct samplebuf_s {
	void *d;
	DTYPE_t dtype;

	int y_ofs;

	float min_val;
	float max_val;

	uint8_t invert_y;

	//int v_range;
	//int coupling;
	//int scale;
} samplebuf_t;

typedef struct wview_s {
	//mf_t mf;

	samplebuf_t *sbuf;
	int sbuf_cnt;

	waveinfo_t *wi;

	//unsigned long samples;

	//unsigned long timebase;

	//unsigned long trigger_ofs;

	unsigned long x_pos, y_pos;
	unsigned long x_cnt, y_cnt;

	unsigned long x_ofs, y_ofs;
	unsigned long target_w, target_h;
} wview_t;

wview_t *wview_init(int w, int h);
void event_loop(wview_t * wv);
int load_wave(wview_t * wv, uint8_t * ptr);

#endif
