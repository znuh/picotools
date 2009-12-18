#include <stdio.h>
#include <assert.h>
#include "wview.h"
#include "mmap.h"
#include "sdl_display.h"
#include "scrollbar.h"

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

void sample_draw(wview_t *wv, int x, float y, uint32_t color) {
	// TODO: y range checks (top/bottom)
	//printf("%f\n",y);
	if(last >= 0)
		aalineColor(sdl.screen,x-1,last,x,y, color);
	last = y;
	//pixelColor(sdl.screen, x, 256-y, 0xffffffff);
}

scrollbar_t *sb;

void wview_redraw(wview_t *wv) {
	uint32_t color[] = {0x8080ffff, 0xff8080ff};
	float x_zoom = (float)wv->target_w / (float)wv->x_cnt;
	unsigned long scnt;
	int samples_per_pixel = wv->x_cnt / wv->target_w;
	int buf_cnt;
	
	//printf("x_zoom %f\n",x_zoom);
	//printf("%ld samples/pixel\n",samples_per_pixel);
	
	for(buf_cnt = 0; buf_cnt < wv->sbuf_cnt; buf_cnt++) {
		samplebuf_t *sbuf = &(wv->sbuf[buf_cnt]);
		float avg = 0;
		int x = 0;
		
		last = -1;
		
		for(scnt=0; scnt < wv->x_cnt; scnt++) {
			float val = samplebuf_get_sample(sbuf, wv->x_pos + scnt);
			
			avg += val;
			
			// TODO: max/min?
			
			// draw every m'th pixel with alpha
			if(!((scnt+1) % samples_per_pixel)) {
				float val = avg/(float)samples_per_pixel;
				val *= 256; //(wv->y_cnt);
				val /= (sbuf->max_val - sbuf->min_val);
				val = sbuf->y_ofs - val;
				//printf("%f\n",avg/samples_per_pixel);
				sample_draw(wv, x++, val, color[buf_cnt]);
				// TODO: scale y, draw line from last
				avg=0;
			}
		}
	}
	scrollbar_draw(sb);
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
	samplebuf_t sbuf[2];
	wview_t wview;
	mf_t mf[2];
	
	assert(argc>1);
	assert(!(map_file(mf, argv[1], 0, 0)));
	
	// 1 channel for now
	wview.sbuf = sbuf;
	wview.sbuf_cnt = 1;
	
	if(argc>2) {
		if(!((map_file(mf+1, argv[2], 0, 0))))
			wview.sbuf_cnt=2;
	}
	
	sbuf[0].y_ofs = 256; // channel y offset
	sbuf[1].y_ofs = 512; // channel y offset
	
	// channel min/max
	sbuf[0].min_val = 0;
	sbuf[0].max_val = 255;
	
	// channel min/max
	sbuf[1].min_val = 0;
	sbuf[1].max_val = 255;
	
	// uint8
	sbuf[0].dtype = UINT8;
	sbuf[0].d = mf[0].ptr + 164;
	
	// uint8
	sbuf[1].dtype = UINT8;
	sbuf[1].d = mf[1].ptr + 164;
	
	wview.samples = mf[0].len - 164;
	
	// x zoom: min (show all samples)
	wview.x_pos = 0;
	wview.x_cnt = wview.samples;
	
	// TODO: y zoom: min (show all)
	//wview.y_pos = 0;
	//wview.y_cnt = sbuf.max_val + 1;
	
	// main window
	wview.target_w = 1000;
	wview.target_h = 768;
	
	printf("%ld samples, target_w %ld\n",wview.samples,wview.target_w);
	
	sdl_init(wview.target_w, wview.target_h);
	
	assert((sb = scrollbar_create(sdl.screen, 0, wview.target_h-12, wview.target_w, 12, wview.samples)));
	
	wview_redraw(&wview);
	
	event_loop(&wview);
	
	unmap_file(mf);
	if(wview.sbuf_cnt==2)
		unmap_file(mf+1);
	
	scrollbar_destroy(sb);
	
	sdl_destroy();
	
	return 0;
}
