#include <stdio.h>
#include <assert.h>
#include "wview.h"
#include "mmap.h"
#include "wvfile.h"

int main(int argc, char **argv)
{
	wview_t *wv = NULL;
	
	assert(argc > 1);

	assert((wv = wview_init(1024, 512)));
	
	load_wave(wv, argv[1]);

	event_loop(wv);
	
	// wview_destroy - unmap file, free

	return 0;
}
