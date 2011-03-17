#include <stdio.h>
#include <assert.h>
#include "wview.h"
#include "mmap.h"
#include "wvfile.h"

static int first_request = 1;

static uint8_t *wptr = NULL;

int request_wave(uint8_t ** ptr)
{
	*ptr = wptr;
	if (first_request) {
		first_request = 0;
		return 1;
	}
	return 0;
}

void release_wave(uint8_t * ptr)
{
}

int main(int argc, char **argv)
{
	waveinfo_t *wi;
	wview_t *wv = NULL;
	mf_t mf;

	assert(argc > 1);

	assert(!(map_file(&mf, argv[1], 0, 0)));

	assert((wv = wview_init(1024, 512)));

	wi = (waveinfo_t *) mf.ptr;
	assert(mf.len >= sizeof(waveinfo_t));

	if (wi->magic != WVINFO_MAGIC) {
		printf("loading raw file\n");
		wptr = malloc(mf.len + sizeof(waveinfo_t));
		wi = (waveinfo_t *) wptr;
		memcpy(wptr + sizeof(waveinfo_t), mf.ptr, mf.len);
		wi->magic = WVINFO_MAGIC;
		wi->capture_time = 0;
		wi->capture_cnt = 0;
		wi->scnt = mf.len;
		wi->pre = 0;
		wi->ns = 1;
		wi->ch_config = 1;
		wi->scale[0] = wi->scale[1] = 0.0;

		wv->sbuf[0].y_ofs = 256;	// channel y offset
		wv->sbuf[0].invert_y = 0;
		wv->sbuf[0].max_val = 255;
		wv->sbuf[0].min_val = 0;
		wv->sbuf[0].dtype = UINT8;

		unmap_file(&mf);
	}
	else {
		//load_wave(wv, mf.ptr);
		wptr = mf.ptr;
	}
	   
	event_loop(wv);

	// wview_destroy - unmap file, free

	return 0;
}
