#include <stdio.h>
#include <assert.h>
#include "wview.h"
#include "mmap.h"
#include "wvfile.h"

int main(int argc, char **argv)
{
	wview_t *wv = NULL;
	mf_t mf;

	assert(argc > 1);

	assert(!(map_file(&mf, argv[1], 0, 0)));

	assert((wv = wview_init(1024, 512)));

	load_wave(wv, mf.ptr);

	event_loop(wv);

	// wview_destroy - unmap file, free

	return 0;
}
