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
	wview_t *wv = NULL;
	mf_t mf;

	assert(argc > 1);

	assert(!(map_file(&mf, argv[1], 0, 0)));

	assert((wv = wview_init(1024, 512)));

	//load_wave(wv, mf.ptr);
	wptr = mf.ptr;

	event_loop(wv);

	// wview_destroy - unmap file, free

	return 0;
}
