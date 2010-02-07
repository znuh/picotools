#include <stdio.h>
#include <SDL/SDL_ttf.h>
#include <assert.h>
#include "wview.h"
#include "mmap.h"
#include "sdl_display.h"
#include "scrollbar.h"
#include "wvfile.h"

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

TTF_Font *font = NULL;

void render_text(char *text, int x, int y, SDL_Color color) {
	SDL_Color bg = {0, 0, 0};
	SDL_Surface *txt_sf = TTF_RenderText_Shaded(font, text, color, bg);
	SDL_Rect dst_rect;
	
	assert(txt_sf);
	
	if(x < 0)
		x = sdl.screen->w - txt_sf->w + x;
	
	if(y < 0)
		y = sdl.screen->h - txt_sf->h + y;
	
	dst_rect.x = x;
	dst_rect.y = y;
	
	dst_rect.h = txt_sf->h;
	dst_rect.w = txt_sf->w;
	
	SDL_BlitSurface(txt_sf, NULL, sdl.screen, &dst_rect);
	SDL_FreeSurface(txt_sf);
}

#define H_DIVS		8
#define V_DIVS		8

#define ABS(x)		((x) >= 0 ? (x) : -(x))

void print_time(char *dst, float ns) {
	if(ABS(ns) > 1000000)
		sprintf(dst,"%.3f ms",ns/1000000);
	else if(ABS(ns) > 1000)
		sprintf(dst,"%.3f us",ns/1000);
	else
		sprintf(dst,"%3.0f ns",ns);
}

void draw_text(wview_t *wv) {
	SDL_Color color = {128, 128, 128};
	char buf[200];
	float val;
	
	// start
	val = wv->wi->pre;
	val *= wv->wi->ns;
	val *= -1;
	print_time(buf,val);
	render_text(buf, 2, -1, color);
	
	// end
	val = wv->wi->scnt;
	val -= wv->wi->pre;
	val *= wv->wi->ns;
	print_time(buf,val);
	render_text(buf, -1, -1, color);
	
	// selected start
	val = wv->x_pos;
	val -= wv->wi->pre;
	val *= wv->wi->ns;
	print_time(buf,val);
	render_text(buf, wv->x_ofs, wv->y_ofs + wv->target_h, color);
	
	// selected end
	val = wv->x_pos + wv->x_cnt;
	val -= wv->wi->pre;
	val *= wv->wi->ns;
	print_time(buf,val);
	render_text(buf, -10, wv->y_ofs + wv->target_h, color);
	
	// time/div
	val = wv->x_cnt;
	val *= wv->wi->ns;
	val /= H_DIVS;
	print_time(buf,val);
	strcat(buf,"/div");
	render_text(buf, wv->x_ofs + (wv->target_w*4) / 9, wv->y_ofs + wv->target_h, color);
	
	// date
	render_text(ctime(&(wv->wi->capture_time)), wv->x_ofs, 2, color);
	
	// capture counter
	sprintf(buf,"%6ld",wv->wi->capture_cnt);
	render_text(buf, -10, 2, color);
}

void wview_redraw(wview_t * wv)
{
	uint32_t color[] = { 0x8080ffff, 0xff8080ff };
	int y_ofs = wv->y_ofs;
	unsigned long scnt;
	int samples_per_pixel = wv->x_cnt / wv->target_w;
	int buf_cnt;
	int trigger_done = 0;
	int x;

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

	draw_text(wv);

	// foreach sample buffer
	for (buf_cnt = 0; buf_cnt < wv->sbuf_cnt; buf_cnt++) {
		samplebuf_t *sbuf = &(wv->sbuf[buf_cnt]);
		float max_val = sbuf->min_val;
		float min_val = sbuf->max_val;
		int last_lower = -1;
		int last_upper = -1;
		
		x = 0;

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
				
				// trigger
				if((!trigger_done) && (wv->x_pos + scnt >= wv->wi->pre)) {
					vlineColor(sdl.screen, wv->x_ofs + x, y_ofs + 1, y_ofs + wv->target_h - 2, 0x80ff8080);
					trigger_done = 1;
				}

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
	
	// trigger
	if(!trigger_done) {
		vlineColor(sdl.screen, wv->x_ofs + x, y_ofs + 1, y_ofs + wv->target_h - 2, 0x80ff8080);
		trigger_done = 1;
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

/* wview_create

*/

int load_wave(wview_t *wv, char *fname) {
	waveinfo_t *wi;
	samplebuf_t *sbuf;
	mf_t mf;
	
	assert(!(map_file(&mf, fname, 0, 0)));
		
	wv->mf = mf;
	
	wi = (waveinfo_t *)mf.ptr;
	wv->wi = wi;
	
	assert(wi->magic == WVINFO_MAGIC);
	
	// skip header
	mf.ptr += sizeof(waveinfo_t);
	
	// 1 or 2 channels?
	assert(wi->ch_config & 3);
	if((wi->ch_config & 3) == 3)
		wv->sbuf_cnt = 2;
	else
		wv->sbuf_cnt = 1;
	
	assert((sbuf = malloc(sizeof(samplebuf_t) * wv->sbuf_cnt)));
	wv->sbuf = sbuf;
	
	// 1st channel
	sbuf[0].d = mf.ptr;
		
	sbuf[0].y_ofs = 256;	// channel y offset
	
	sbuf[0].max_val = 32768;
	sbuf[0].min_val = -32768;
	sbuf[0].dtype = INT16;
	
	if(wv->sbuf_cnt > 1) {
		sbuf[1].d = mf.ptr + wi->scnt * 2; // CHANGEME: short -> byte
		
		sbuf[1].y_ofs = 512;	// channel y offset
		
		sbuf[1].max_val = 32768;
		sbuf[1].min_val = -32768;
		sbuf[1].dtype = INT16;
	}
	
	return 0;
}

// wview_destroy - unmap file, free

int main(int argc, char **argv)
{
	wview_t wview;
	scrollbar_t *sb;

	assert(argc > 1);
	
	load_wave(&wview, argv[1]);

	// x zoom: min (show all samples)
	wview.x_pos = 0;
	wview.x_cnt = wview.wi->scnt;

	// main window
	wview.x_ofs = 10;
	wview.y_ofs = 10;
	wview.target_w = 1024;
	wview.target_h = 512;

	//printf("%ld samples, target_w %ld\n", wview.samples, wview.target_w);

	sdl_init(wview.target_w + wview.x_ofs * 2,
		 wview.target_h + wview.y_ofs * 2 + 50);

	assert(!TTF_Init());

	assert((font = TTF_OpenFont("/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono.ttf", 12)));

	assert((sb =
		scrollbar_create(sdl.screen, 0,
				 wview.target_h + wview.y_ofs * 2 + 15,
				 wview.target_w + 20, 12, wview.target_w,
				 wview.wi->scnt)));

	event_loop(&wview, sb);

	scrollbar_destroy(sb);

	sdl_destroy();

	return 0;
}
