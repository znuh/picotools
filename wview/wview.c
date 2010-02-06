#include <stdio.h>
#include <assert.h>
#include "wview.h"
#include "mmap.h"
#include "sdl_display.h"
#include "scrollbar.h"

float samplebuf_get_sample(samplebuf_t * s, unsigned long sample)
{
	void *d = s->d;
	float res = 0;

	switch (s->dtype) {
	case INT8:
		res = ((int8_t *) d)[sample];
		break;
	case UINT8:
		res = ((uint8_t *) d)[sample];
		break;
	case INT16:
		res = ((int16_t *) d)[sample];
		break;
	case UINT16:
		res = ((uint8_t *) d)[sample];
		break;
	case INT32:
		res = ((int32_t *) d)[sample];
		break;
	case UINT32:
		res = ((uint32_t *) d)[sample];
		break;
	case FLOAT:
		res = ((float *)d)[sample];
		break;
	case DOUBLE:
		res = ((double *)d)[sample];
		break;
	default:
		assert(0);
	}

	return res;
}

extern struct sdl_ctx sdl;

int pixel_from_sample(samplebuf_t * sbuf, float sample)
{
	float val = sample;
	int pixel;

	val *= 256;		//(wv->y_cnt);
	val /= (sbuf->max_val - sbuf->min_val);
	val = sbuf->y_ofs - val;

	// TODO: round?
	pixel = val;

	return pixel;
}

#define H_DIVS		8
#define V_DIVS		8

void wview_redraw(wview_t * wv)
{
	uint32_t color[] = { 0x8080ffff, 0xff8080ff };
	int y_ofs = wv->y_ofs;
	unsigned long scnt;
	int samples_per_pixel = wv->x_cnt / wv->target_w;
	int buf_cnt;

	//printf("x_cnt %d\n",wv->x_cnt);
	//printf("%ld samples/pixel\n",samples_per_pixel);

	// frame
	rectangleColor(sdl.screen, wv->x_ofs, y_ofs,
		       wv->target_w + wv->x_ofs - 1,
		       wv->y_ofs + wv->target_h - 1, 0xffffff40);

	// vertical divisions
	for (scnt = wv->target_h / V_DIVS; scnt < wv->target_h;
	     scnt += wv->target_h / V_DIVS)
		hlineColor(sdl.screen, wv->x_ofs + 1,
			   wv->target_w + wv->x_ofs - 2, y_ofs + scnt,
			   0xffffff40);

	// horizontal divisions
	for (scnt = wv->target_w / H_DIVS; scnt < wv->target_w;
	     scnt += wv->target_w / H_DIVS)
		vlineColor(sdl.screen, wv->x_ofs + scnt, y_ofs + 1,
			   y_ofs + wv->target_h - 2, 0xffffff40);

	// foreach sample buffer
	for (buf_cnt = 0; buf_cnt < wv->sbuf_cnt; buf_cnt++) {
		samplebuf_t *sbuf = &(wv->sbuf[buf_cnt]);
		float max_val = sbuf->min_val;
		float min_val = sbuf->max_val;
		int last_lower = -1;
		int last_upper = -1;
		int x = 0;

		// foreach sample
		for (scnt = 0; scnt < wv->x_cnt; scnt++) {
			float val =
			    samplebuf_get_sample(sbuf, wv->x_pos + scnt);
			int lower, upper;

			if (val > max_val)
				max_val = val;

			if (val < min_val)
				min_val = val;

			// TODO: draw every m'th pixel with alpha

			if (!((scnt + 1) % samples_per_pixel)) {
				int did_vline = 0;

				//float val = avg / (float)samples_per_pixel;
				//printf("%f\n",avg/samples_per_pixel);

				// lower pixel >= upper pixel
				lower = pixel_from_sample(sbuf, min_val);
				upper = pixel_from_sample(sbuf, max_val);

				if (lower != upper) {
					vlineColor(sdl.screen, wv->x_ofs + x,
						   wv->y_ofs + lower,
						   wv->y_ofs + upper,
						   color[buf_cnt]);
					did_vline = 1;
				}
				// last valid?
				if (last_lower >= 0) {

					if (lower < last_upper) {
						// rising
						aalineColor(sdl.screen,
							    wv->x_ofs + x - 1,
							    wv->y_ofs +
							    last_upper,
							    wv->x_ofs + x,
							    wv->y_ofs +
							    lower,
							    color[buf_cnt]);
					} else if ((upper > last_lower)
						   || (!did_vline)) {
						// falling OR no line done yet
						aalineColor(sdl.screen,
							    wv->x_ofs + x - 1,
							    wv->y_ofs +
							    last_lower,
							    wv->x_ofs + x,
							    wv->y_ofs +
							    upper,
							    color[buf_cnt]);
					}
				}	// last valid

				last_lower = lower;
				last_upper = upper;

				// TODO: scale y, draw line from last
				max_val = sbuf->min_val;
				min_val = sbuf->max_val;

				x++;
			}
			// cropping seems to be a better option than using non-constant samples_per_pixel
			if (x == wv->target_w)
				break;
		}
		//printf("x: %d\n", x);
	}
}

void event_loop(wview_t * wv, scrollbar_t * sb)
{
	int redraw = SB_VALS_CHANGED;

	while (1) {
		SDL_Event event;

		SDL_WaitEvent(NULL);

		while (SDL_PollEvent(&event)) {
			if ((event.type == SDL_MOUSEMOTION)
			    || (event.type == SDL_MOUSEBUTTONDOWN)
			    || (event.type == SDL_MOUSEBUTTONUP)) {
				redraw |= scrollbar_event(sb, &event);
			}

			if (event.type == SDL_QUIT)
				return;
		}

		wv->x_cnt = sb->len;
		wv->x_pos = sb->pos;

		if (redraw) {
			//if(redraw & SB_CHANGED)
			// always redraw scrollbar otherwise it'll disappear
			scrollbar_draw(sb);

			if (redraw & SB_VALS_CHANGED)
				wview_redraw(wv);
			sdl_flip();
		}
		redraw = 0;
	}
}

int main(int argc, char **argv)
{
	samplebuf_t sbuf[2];
	wview_t wview;
	scrollbar_t *sb;
	mf_t mf[2];

	assert(argc > 1);
	assert(!(map_file(mf, argv[1], 0, 0)));

	// 1 channel for now
	wview.sbuf = sbuf;
	wview.sbuf_cnt = 1;

	if (argc > 2) {
		if (!((map_file(mf + 1, argv[2], 0, 0))))
			wview.sbuf_cnt = 2;
	}

	sbuf[0].y_ofs = 256;	// channel y offset
	sbuf[1].y_ofs = 512;	// channel y offset

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

	///// ugly test
	if (!(strcmp(argv[1] + strlen(argv[1]) - 3, ".wv"))) {
		sbuf[0].max_val = 32768;
		sbuf[0].min_val = -32768;
		sbuf[0].dtype = INT16;
		wview.samples /= 2;
	}
	// x zoom: min (show all samples)
	wview.x_pos = 0;
	wview.x_cnt = wview.samples;

	// main window
	wview.x_ofs = 10;
	wview.y_ofs = 10;
	wview.target_w = 1024;
	wview.target_h = 512;

	printf("%ld samples, target_w %ld\n", wview.samples, wview.target_w);

	sdl_init(wview.target_w + wview.x_ofs * 2,
		 wview.target_h + wview.y_ofs * 2 + 20);

	assert((sb =
		scrollbar_create(sdl.screen, 0,
				 wview.target_h + wview.y_ofs * 2 + 5,
				 wview.target_w + 20, 12, wview.target_w,
				 wview.samples)));

	event_loop(&wview, sb);

	unmap_file(mf);
	if (wview.sbuf_cnt == 2)
		unmap_file(mf + 1);

	scrollbar_destroy(sb);

	sdl_destroy();

	return 0;
}
