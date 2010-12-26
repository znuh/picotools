#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <SDL/SDL_ttf.h>
#include <stdlib.h>
#include <assert.h>
#include "wview.h"
#include "mmap.h"
#include "sdl_display.h"
#include "scrollbar.h"
#include "wvfile.h"

long int u32_to_long(uint32_t in) {
	long int res = in;
	return res;
}

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

	assert(val >= sbuf->min_val);
	assert(val <= sbuf->max_val);

	val *= 256;		//(wv->y_cnt);
	val /= (sbuf->max_val - sbuf->min_val);

	if (sbuf->invert_y)
		val = sbuf->y_ofs + val;
	else
		val = sbuf->y_ofs - val;

	// TODO: round?
	pixel = val;

	return pixel;
}

TTF_Font *font = NULL;

void render_text(char *text, int x, int y, SDL_Color color)
{
	SDL_Color bg = { 0, 0, 0 };
	SDL_Surface *txt_sf = TTF_RenderText_Shaded(font, text, color, bg);
	SDL_Rect dst_rect;

	assert(txt_sf);

	if (x < 0)
		x = sdl.screen->w - txt_sf->w + x;

	if (y < 0)
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

void print_time(char *dst, float ns)
{
	if (ABS(ns) > 1000000)
		sprintf(dst, "%.3f ms", ns / 1000000);
	else if (ABS(ns) > 1000)
		sprintf(dst, "%.3f us", ns / 1000);
	else
		sprintf(dst, "%3.0f ns", ns);
}

void print_volt(char *dst, float volt)
{
	if (volt >= 1)
		sprintf(dst, "%.3f V", volt);
	else
		sprintf(dst, "%.3f mV", volt * 1000);
}

void draw_text(wview_t * wv)
{
	SDL_Color color = { 128, 128, 128 };
	char buf[200];
	float val;
	time_t tbuf;

	// start
	val = wv->wi->pre;
	val *= wv->wi->ns;
	val *= -1;
	print_time(buf, val);
	render_text(buf, 2, -1, color);

	// end
	val = wv->wi->scnt;
	val -= wv->wi->pre;
	val *= wv->wi->ns;
	print_time(buf, val);
	render_text(buf, -1, -1, color);

	// selected start
	val = wv->x_pos;
	val -= wv->wi->pre;
	val *= wv->wi->ns;
	print_time(buf, val);
	render_text(buf, wv->x_ofs, wv->y_ofs + wv->target_h, color);

	// selected end
	val = wv->x_pos + wv->x_cnt;
	val -= wv->wi->pre;
	val *= wv->wi->ns;
	print_time(buf, val);
	render_text(buf, -10, wv->y_ofs + wv->target_h, color);

	// time/div
	val = wv->x_cnt;
	val *= wv->wi->ns;
	val /= H_DIVS;
	print_time(buf, val);
	strcat(buf, "/div");
	render_text(buf, wv->x_ofs + (wv->target_w * 4) / 9,
		    wv->y_ofs + wv->target_h, color);

	// date
	tbuf = wv->wi->capture_time;
	strcpy(buf, ctime(&tbuf));
	buf[strlen(buf) - 1] = 0;
	render_text(buf, wv->x_ofs, 2, color);

	// samples/s
	val = 1000;
	val /= (float)wv->wi->ns;
	if (val >= 1000)
		sprintf(buf, "%.3f GS/s", val / (float)1000);
	else
		sprintf(buf, "%.3f MS/s", val);
	render_text(buf, wv->x_ofs + 800, 2, color);

	// capture counter
	sprintf(buf, "%6ld", u32_to_long(wv->wi->capture_cnt));
	render_text(buf, -10, 2, color);

	// V/div ch 1
	val = ((float)wv->target_h * wv->wi->scale[0]) / (float)V_DIVS;
	print_volt(buf, val);
	color.r = 0x80;
	color.g = 0x80;
	color.b = 0xff;
	strcat(buf, "/div ");
	if (wv->wi->ch_config & 4)
		strcat(buf, "DC");
	else
		strcat(buf, "AC");

	if (wv->sbuf[0].invert_y)
		strcat(buf, " INV");

	render_text(buf, wv->x_ofs + 300, 2, color);

	// V/div ch 2
	if (wv->wi->scnt > 1) {
		val = ((float)wv->target_h * wv->wi->scale[1]) / (float)V_DIVS;
		print_volt(buf, val);
		color.r = 0xff;
		color.g = 0x80;
		color.b = 0x80;
		strcat(buf, "/div ");
		if (wv->wi->ch_config & 8)
			strcat(buf, "DC");
		else
			strcat(buf, "AC");

		if (wv->sbuf[1].invert_y)
			strcat(buf, " INV");

		render_text(buf, wv->x_ofs + 500, 2, color);
	}

}

void draw_grid(wview_t * wv)
{
	int y_ofs = wv->y_ofs;
	int scnt;

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
}

void wview_redraw(wview_t * wv)
{
	uint32_t color[] = { 0x8080ffff, 0xff8080ff };
	int y_ofs = wv->y_ofs;
	unsigned long scnt;
	float pixels_per_sample = ((float)wv->target_w / (float)wv->x_cnt);
	float x_f = 0;
	int buf_cnt;
	int trigger_done = 0;
	int x = 0;

	//printf("%f\n",step);
	//printf("x_cnt %d\n",wv->x_cnt);
	
	draw_grid(wv);
	draw_text(wv);

	// foreach sample buffer
	for (buf_cnt = 0; buf_cnt < wv->sbuf_cnt; buf_cnt++) {
		samplebuf_t *sbuf = &(wv->sbuf[buf_cnt]);
		float max_val = sbuf->min_val;
		float min_val = sbuf->max_val;
		int last_lower = -1;
		int last_upper = -1;

		// zero offset
		hlineColor(sdl.screen, 0, wv->x_ofs - 1, y_ofs + sbuf->y_ofs,
			   color[buf_cnt]);

		x = 0;
		x_f = 0.0;

		// foreach sample
		for (scnt = 0; scnt < wv->x_cnt; scnt++) {
			float val =
			    samplebuf_get_sample(sbuf, wv->x_pos + scnt);
			int lower, upper;

			if (val > max_val)
				max_val = val;

			if (val < min_val)
				min_val = val;

			x_f += pixels_per_sample;

			if(((int)x_f) > x) {
				int did_vline = 0;

				// trigger
				if ((!trigger_done)
				    && (wv->x_pos + scnt >= wv->wi->pre)) {
					vlineColor(sdl.screen, wv->x_ofs + x,
						   y_ofs + 1,
						   y_ofs + wv->target_h - 2,
						   0x80ff8080);
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

				x = (int)x_f;
			} // draw
			
		} // foreach sample
		
		//printf("x: %d\n", x);
		
	} // foreach sample buffer

	// trigger
	if (!trigger_done) {
		vlineColor(sdl.screen, wv->x_ofs + x, y_ofs + 1,
			   y_ofs + wv->target_h - 2, 0x80ff8080);
		trigger_done = 1;
	}
}

scrollbar_t *sb;

void wv_save_wave(wview_t * wv)
{
	uint8_t *dst, *ptr;
	int fd, len = wv->wi->scnt;
	char buf[64];

	if (wv->sbuf_cnt == 2)
		len *= 2;

	sprintf(buf, "%ld.wv", u32_to_long(wv->wi->capture_time));
	fd = open(buf, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0)
		return;

	if (ftruncate(fd, len + sizeof(waveinfo_t)))
		goto done;

	dst =
	    mmap(NULL, len + sizeof(waveinfo_t), PROT_WRITE, MAP_SHARED, fd, 0);
	if (dst == MAP_FAILED)
		goto done;

	memcpy(dst, wv->wi, sizeof(waveinfo_t));
	ptr = dst + sizeof(waveinfo_t);

	memcpy(ptr, wv->sbuf[0].d, wv->wi->scnt);
	ptr += wv->wi->scnt;

	if (wv->sbuf_cnt == 2)
		memcpy(ptr, wv->sbuf[1].d, wv->wi->scnt);

	munmap(dst, len + sizeof(waveinfo_t));

 done:
	close(fd);
	return;
}

int load_wave(wview_t * wv, uint8_t * ptr)
{
	waveinfo_t *wi;

	wi = (waveinfo_t *) ptr;
	wv->wi = wi;

	assert(wi->magic == WVINFO_MAGIC);

	// new max len
	if (sb->max_len != wi->scnt) {
		sb->pos = 0;
		sb->len = sb->max_len = wi->scnt;
	}
	// skip header
	ptr += sizeof(waveinfo_t);

	// 1 or 2 channels?
	assert(wi->ch_config & 3);
	if ((wi->ch_config & 3) == 3)
		wv->sbuf_cnt = 2;
	else
		wv->sbuf_cnt = 1;

	wv->sbuf[0].d = ptr;

	if (wv->sbuf_cnt > 1)
		wv->sbuf[1].d = ptr + wi->scnt;

	return 0;
}

extern int request_wave(uint8_t ** ptr);
extern void release_wave(uint8_t * ptr);

void event_loop(wview_t * wv)
{
	//SDL_Color fg = { 0x80, 0x80, 0x80 };
	uint8_t *wavedata = NULL;
	int redraw = SB_VALS_CHANGED;
	int initialized = 0;

	assert(sb);

	//render_text("waiting for data", wv->target_w/3, wv->target_h/2, fg);
	draw_grid(wv);
	SDL_Flip(sdl.screen);

	while (1) {
		SDL_Event event;

		SDL_WaitEvent(NULL);

		// aggregate events
		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT)
				return;

			//if ((wv->wi) && (wv->wi->magic == WVINFO_MAGIC)
			//   && (wv->sbuf_cnt > 0))
			//      initialized = 1;
			if (initialized) {
				// HACK: save file
				if (event.type == SDL_MOUSEBUTTONDOWN) {
					int m_x = event.button.x;
					int m_y = event.button.y;
					if ((m_x <= 20) && (m_y <= 20))
						wv_save_wave(wv);
				}
				// HACK: invert channel y
				if (event.type == SDL_MOUSEBUTTONDOWN) {
					int m_x = event.button.x;
					int m_y = event.button.y;
					if ((m_x >= wv->x_ofs + 300)
					    && (m_x < wv->x_ofs + 500)
					    && (m_y <= 20)) {
						wv->sbuf[0].invert_y ^= 1;
						redraw |= 1;
					}
				}
				// HACK: invert channel y
				if (event.type == SDL_MOUSEBUTTONDOWN) {
					int m_x = event.button.x;
					int m_y = event.button.y;
					if ((m_x >= wv->x_ofs + 500)
					    && (m_x < wv->x_ofs + 800)
					    && (m_y <= 20)) {
						wv->sbuf[1].invert_y ^= 1;
						redraw |= 1;
					}
				}
				// scrollbar
				if (((event.type == SDL_MOUSEMOTION)
				     || (event.type == SDL_MOUSEBUTTONDOWN)
				     || (event.type == SDL_MOUSEBUTTONUP))) {
					redraw |= scrollbar_event(sb, &event);
				}
			}
		}

		// new wave data?
		if (request_wave(&wavedata)) {
			load_wave(wv, wavedata);
			initialized = 1;
			redraw = 1;
		}

		if (!(initialized))
			continue;

		wv->x_cnt = sb->len;
		wv->x_pos = sb->pos;

		// TODO: redraw only if minimum delay of 1/60 sec. passed
		if (redraw) {
			// TODO: don't redraw everything all the time
			SDL_FillRect(sdl.screen, NULL, 0xff000000);

			//if(redraw & SB_CHANGED)
			// always redraw scrollbar otherwise it'll disappear
			scrollbar_draw(sb);

			//if (redraw & SB_VALS_CHANGED)
			wview_redraw(wv);

			release_wave(wavedata);

			SDL_Flip(sdl.screen);
		}
		redraw = 0;
	}
}

wview_t *wview_init(int w, int h)
{
	wview_t *wv = malloc(sizeof(wview_t));
	samplebuf_t *sbuf;
	assert(wv);

	wv->wi = NULL;

	// x zoom: min (show all samples)
	wv->x_pos = 0;
	wv->x_cnt = wv->target_w;	//wv->wi->scnt; //TODO

	// main window
	wv->x_ofs = 10;
	wv->y_ofs = 20;
	wv->target_w = w;
	wv->target_h = h;

	assert((sbuf = malloc(sizeof(samplebuf_t) * 2)));
	wv->sbuf = sbuf;
	wv->sbuf_cnt = 0;

	// 1st channel
//      sbuf[0].d = mf.ptr;

	sbuf[0].y_ofs = 128;	// channel y offset

	sbuf[0].invert_y = 0;

	sbuf[0].max_val = 127;
	sbuf[0].min_val = -127;
	sbuf[0].dtype = INT8;

	// 2nd channel
//      sbuf[1].d = mf.ptr + wi->scnt;  // * 2;  // CHANGEME: short -> byte

	sbuf[1].y_ofs = 256 + 128;	// channel y offset

	sbuf[1].invert_y = 0;

	sbuf[1].max_val = 127;
	sbuf[1].min_val = -127;
	sbuf[1].dtype = INT8;

	sdl_init(wv->target_w + wv->x_ofs * 2,
		 wv->target_h + wv->y_ofs * 2 + 30);

	assert(!TTF_Init());

	assert((font =
		TTF_OpenFont
		("/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono.ttf",
		 12)));

	assert((sb = scrollbar_create(sdl.screen, 0, wv->target_h + wv->y_ofs * 2, wv->target_w + 20, 12, wv->target_w, wv->target_w + 1)));	//TODO

	return wv;
}
