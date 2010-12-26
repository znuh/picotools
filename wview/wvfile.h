#ifndef WVFILE_H
#define WVFILE_H

#include <stdint.h>
#include <time.h>

#define WVINFO_MAGIC	0xb94b781c

typedef struct __attribute__((__packed__)) waveinfo_s {
	uint32_t magic;
	uint32_t capture_time;
	uint32_t capture_cnt;
	uint32_t scnt;
	uint32_t pre;
	uint32_t ns;
	uint32_t ch_config;
	float scale[2] __attribute__((__packed__));
} waveinfo_t;

#endif
