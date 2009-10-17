#include <stdio.h>
#include <assert.h>
#include "wview.h"
#include "mmap.h"
#include "sdl_display.h"

float samplebuf_get_sample(samplebuf_t *s, unsigned long sample) {
	void *d = s->d;
	float res=0;
	
	switch(s->dtype) {
		case INT8:
			res = ((int8_t *)d)[sample];
			break;
		case UINT8: 
			res = ((uint8_t *)d)[sample];
			break;
		case INT16:
			res = ((int16_t *)d)[sample];
			break;
		case UINT16:
			res = ((uint8_t *)d)[sample];
			break;
		case INT32:
			res = ((int32_t *)d)[sample];
			break;
		case UINT32:
			res = ((uint32_t *)d)[sample];
			break;
		case FLOAT:
			res = ((float *)d)[sample];
			break;
		case DOUBLE:
			res = ((double *)d)[sample];
			break;
		default: assert(0);
	}
	
	return res;
}

// method: zoom@(x,y,delta_zoom)

// TODO: 10x alphablended uebermalen

extern struct sdl_ctx sdl;

int last = -1;

void sample_draw(wview_t *wv, int x, float y) {
	//printf("%f\n",y);
	if(last >= 0)
		aalineColor(sdl.screen,x-1,last,x,256-y, 0x8080ffff);
	last = 256-y;
	//pixelColor(sdl.screen, x, 256-y, 0xffffffff);
}

void wview_redraw(wview_t *wv) {
	float x_zoom = (float)wv->target_w / (float)wv->x_cnt;
	unsigned long scnt;
	unsigned long samples_per_pixel = wv->x_cnt / wv->target_w;
	int buf_cnt;
	int x = 0;
	
	//printf("x_zoom %f\n",x_zoom);
	//printf("%ld samples/pixel\n",samples_per_pixel);
	
	last = -1;
	
	for(buf_cnt = 0; buf_cnt < wv->sbuf_cnt; buf_cnt++) {
		samplebuf_t *sbuf = wv->sbuf+buf_cnt;
		float avg = 0;
		
		for(scnt=0; scnt < wv->x_cnt; scnt++) {
			float val = samplebuf_get_sample(sbuf, wv->x_pos + scnt);
			
			avg += val;
			
			// TODO: max/min?
			
			// draw every m'th pixel with alpha
			if(!((scnt+1) % samples_per_pixel)) {
				//printf("%f\n",avg/samples_per_pixel);
				sample_draw(wv, x++, avg/samples_per_pixel);
				// TODO: scale y, draw line from last
				avg=0;
			}
		}
	}
	sdl_flip();
}

//unsigned long pixel_to_sample(wview_t *wv, unsigned long x_pixel) {
	
//}

void event_loop(wview_t *wv) {
	while(1) {
		SDL_Event event;
		
		SDL_WaitEvent(&event);
		
		switch(event.type) {
			case SDL_MOUSEMOTION:
				//printf("Mouse moved by %d,%d to (%d,%d)\n", 
				//		event.motion.xrel, event.motion.yrel,
				//		event.motion.x, event.motion.y);
				wv->x_cnt = (wv->samples / sdl.w) * event.motion.x;
				
				if(wv->x_cnt < sdl.w)
					wv->x_cnt = sdl.w;
				
				wview_redraw(wv);
				break;
			case SDL_QUIT:
				return;
		}
	}
}

int main(int argc, char **argv) {
	samplebuf_t sbuf;
	wview_t wview;
	mf_t mf;
	
	assert(argc>1);
	assert(!(map_file(&mf, argv[1], 0, 0)));
	
	wview.sbuf = &sbuf;
	wview.sbuf_cnt = 1;
	
	sbuf.y_ofs = 100;
	sbuf.min_val = 0;
	sbuf.max_val = 255;
	sbuf.dtype = UINT8;
	sbuf.d = mf.ptr + 164;
	
	wview.samples = mf.len - 164;
	
	wview.x_pos = 0;
	wview.y_pos = 0;
	wview.x_cnt = wview.samples;
	wview.y_cnt = 100; // ?????
	
	wview.target_w = 1000;
	wview.target_h = 100; // ????
	
	printf("%ld samples, target_w %ld\n",wview.samples,wview.target_w);
	
	sdl_init(wview.target_w,768);
	
	wview_redraw(&wview);
	
	event_loop(&wview);
	
	unmap_file(&mf);
	
	sdl_destroy();
	
	return 0;
}