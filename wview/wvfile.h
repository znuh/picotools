#ifndef WVFILE_H
#define WVFILE_H

#include <time.h>

#define WVINFO_MAGIC	0xb94b781c

typedef struct waveinfo_s {
	unsigned long magic;
	time_t capture_time;
	unsigned long capture_cnt;
	unsigned long scnt;
	unsigned long pre;
	unsigned long ns;
	unsigned long ch_config;
	float scale[2];
} waveinfo_t;

#endif
