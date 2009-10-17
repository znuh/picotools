#ifndef WVIEW_H
#define WVIEW_H

#include <stdint.h>

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
	
	//int v_range;
	//int coupling;
	//int scale;
} samplebuf_t;

typedef struct wview_s {
	samplebuf_t *sbuf;
	int sbuf_cnt;
	unsigned long samples;
	
	int timebase;
	
	unsigned long trigger_ofs;
	
	unsigned long x_pos, y_pos;
	unsigned long x_cnt, y_cnt;
	
	unsigned long target_w, target_h;
} wview_t;

#endif
