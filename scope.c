/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "scope.h"
#include "wview/wvfile.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "wview/wview.h"

static unsigned long capture_cnt = 0;

scope_config_t scope_config;

short handle;

SCOPE_TYPE_t scope_type = SCOPE_NONE;

extern unsigned long ns;

static short *d = NULL;
uint8_t *waves = NULL;

int wave_size = 0;

wview_t *wv = NULL;

// TODO: cleanup code - seperate SDL/GTK/scope parts
#include <gtk/gtk.h>
#include <SDL/SDL.h>

GMutex *mutex = NULL;
GCond *cond = NULL;

void wview_thread(void)
{
	g_mutex_lock(mutex);
	assert((wv = wview_init(1024, 512)));

	// tell the parent wview is ready
	g_cond_broadcast(cond);
	g_mutex_unlock(mutex);

	event_loop(wv);

	SDL_Quit();
}

GThread *viewer_thread = NULL;

int viewer_init(void)
{
	int max_samples = 1024 * 1024 * (scope_type == SCOPE_PS5204 ? 128 : 64);

	assert((d = malloc(max_samples * sizeof(short))));

	wave_size = max_samples + sizeof(waveinfo_t);

	assert((waves = malloc(wave_size * 3)));

	mutex = g_mutex_new();
	cond = g_cond_new();

	g_mutex_lock(mutex);

	viewer_thread =
	    g_thread_create((GThreadFunc) wview_thread, NULL, TRUE, NULL);

	// wait for wview to come up
	while (!(wv))
		g_cond_wait(cond, mutex);
	g_mutex_unlock(mutex);

	return 0;
}

void viewer_close(void)
{
	SDL_Event ev;

	ev.type = SDL_QUIT;
	SDL_PushEvent(&ev);
	g_thread_join(viewer_thread);
}

void viewer_destroy(void)
{
	if (d) {
		free(d);
		d = NULL;
	}
	if (waves) {
		free(waves);
		waves = NULL;
	}
}

int new_wave = 0;

void notify_viewer(uint8_t * ptr)
{
	SDL_Event ev;

	ev.type = SDL_USEREVENT;
	//ev.user.data1 = ptr;
	new_wave = 1;
	SDL_PushEvent(&ev);
}

int request_wave(uint8_t ** ptr)
{
	*ptr = waves;
	if (new_wave) {
		new_wave = 0;
		return 1;
	}
	return 0;
}

void release_wave(uint8_t * ptr)
{
}

int scope_open(int dryrun)
{
	char line[80];
	short i, r = 0;
	PICO_STATUS res = PICO_OK;

	if (!dryrun) {

		res = ps5000OpenUnit(&handle);
		assert(res == PICO_OK);

		for (i = 0; i < 5; i++) {
			ps5000GetUnitInfo(handle, line, sizeof(line), &r, i);
			if (i == 3)
				scope_type = atoi(line);
			//printf("%s\n", line);
		}
		assert((scope_type == SCOPE_PS5204)
		       || (scope_type == SCOPE_PS5203));

	}

	viewer_init();
//      res = ps5000SetSigGenBuiltIn(handle, 0, 3000000, 0, (float)1000000.0, (float)1000000., 0, 0, 0, 0, 0, 0, 0, 0, 0);
//      printf("%ld %lx\n",res,res);
//      assert(res == PICO_OK);
	return 0;
}

void scope_close(void)
{
	if (!scope_type)
		return;
	ps5000CloseUnit(handle);
	scope_type = SCOPE_NONE;

	viewer_destroy();
}

int scope_channel_config(int ch)
{
	PS5000_CHANNEL scope_ch = ch ? PS5000_CHANNEL_B : PS5000_CHANNEL_A;
	short enable = (scope_config.channel_config >> ch) & 1;
	short dc = (scope_config.channel_config >> (ch + 2)) & 1;
	PS5000_RANGE range = scope_config.range[ch];

	printf("ch %d: %s %s %d\n", scope_ch, enable ? "enable" : "",
	       dc ? "DC" : "AC", range);

	if (!scope_type)
		return 0;

	return ((ps5000SetChannel(handle, scope_ch, enable, dc, range) ==
		 PICO_OK) ? 0 : -1);
}

int scope_sample_config(unsigned long *tbase, unsigned long *buflen)
{
	long ns;
	long samples;
	PICO_STATUS res;

	if (!scope_type)
		return 0;

	res = ps5000GetTimebase(handle, *tbase, *buflen, &ns, 0, &samples, 0);

	printf("%ld: %ld ns %ld samples\n", res, ns, samples);

	return ((res == PICO_OK) ? 0 : -1);
}

int scope_done(void);

void copy_wave(uint8_t * dst, short *d1, short *d2, waveinfo_t * wi)
{
	uint8_t *d;
	int cnt;

	memcpy(dst, wi, sizeof(waveinfo_t));
	dst += sizeof(waveinfo_t);

	// compress to 8 bit
	if (d1) {
		d = (uint8_t *) d1;
		d++;
		for (cnt = 0; cnt < wi->scnt; cnt++) {
			*dst = *d;
			dst++;
			d += 2;
		}
	}

	if (d2) {
		d = (uint8_t *) d2;
		d++;
		for (cnt = 0; cnt < wi->scnt; cnt++) {
			*dst = *d;
			dst++;
			d += 2;
		}
	}
}

void save_wave(char *fname, short *d1, short *d2, waveinfo_t * wi)
{
	int fd = open(fname, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);
	int len = sizeof(waveinfo_t) + wi->scnt;
	uint8_t *mapped;

	if (fd < 0)
		return;

	if ((d1) && (d2))
		len += wi->scnt;

	assert(!(ftruncate(fd, len)));

	assert((mapped =
		mmap(NULL, len, PROT_WRITE, MAP_SHARED, fd, 0)) != MAP_FAILED);

	copy_wave(mapped, d1, d2, wi);

	munmap(mapped, len);
	close(fd);

}

void save_ascii(char *fname, short *d1, short *d2, waveinfo_t * wi)
{
	FILE *fl = fopen(fname, "w");
	int cnt;

	for (cnt = 0; cnt < wi->scnt; cnt++) {
		float scaled_a = 0.0, scaled_b = 0.0, vala, valb;
		double tstamp = cnt;

		if (wi->ch_config & 1) {
			vala = d1[cnt];
			scaled_a = vala * wi->scale[0];	// scale to Volts
		}

		if (wi->ch_config & 2) {
			valb = d2[cnt];
			scaled_b = valb * wi->scale[1];	// scale to Volts
		}

		tstamp -= wi->pre;
		tstamp *= wi->ns;
		tstamp /= 1000000000;

		if ((wi->ch_config & 3) == 3)
			fprintf(fl, "%f %f %f\n", tstamp, scaled_a, scaled_b);
		else if ((wi->ch_config & 3) == 1)
			fprintf(fl, "%f %f\n", tstamp, scaled_a);
		else if ((wi->ch_config & 3) == 2)
			fprintf(fl, "%f %f\n", tstamp, scaled_b);
	}

	fclose(fl);
}

void scope_stop(void)
{
	if (!scope_type)
		return;
	//printf("stop\n");
	ps5000Stop(handle);
}

void rerun_thread(void)
{
	//usleep(1000);         //TODO: fixme - use auto mode
	scope_run(0);
}

void PREF4 CallBackBlock(short handle, PICO_STATUS status, void *pParameter)
{
	//char fname[64], buf[128] = "./wview/wview ";
	waveinfo_t wi;
	unsigned long scnt = scope_config.samples;
	time_t now = time(NULL);
	short *d1 = d, *d2 = d;
	int channels = scope_config.channel_config;
	int run_again = 0;

	// notify gui
	run_again = scope_done();

	scope_stop();

	//printf("done scnt %ld status %ld\n", scnt, status);

	if ((channels & 3) == 3)
		d2 = d1 + scope_config.samples;

	// 1st channel active
	if ((channels >> 0) & 1) {
		assert(ps5000SetDataBuffer
		       (handle, PS5000_CHANNEL_A, d1,
			scope_config.samples) == PICO_OK);
	}
	// 2nd channel active
	if ((channels >> 1) & 1) {
		assert(ps5000SetDataBuffer
		       (handle, PS5000_CHANNEL_B, d2,
			scope_config.samples) == PICO_OK);
	}

	assert(ps5000GetValues
	       (handle, 0, &scnt, 1, RATIO_MODE_NONE, 0, NULL) == PICO_OK);

	wi.magic = WVINFO_MAGIC;
	wi.capture_time = now;
	wi.capture_cnt = ++capture_cnt;

	wi.scnt = scnt;

	if (scope_config.trig_enabled)
		wi.pre = scope_config.pre_trig;
	else
		wi.pre = 0;

	wi.ns = ns;

	wi.scale[0] = scope_config.f_range[0] / (PS5000_MAX_VALUE >> 8);
	wi.scale[1] = scope_config.f_range[1] / (PS5000_MAX_VALUE >> 8);

	wi.ch_config = scope_config.channel_config;

	/*
	   sprintf(fname, "%ld.txt", now);
	   save_ascii(fname, d1, d2, &wi);
	 */
	/*
	   sprintf(fname, "%ld.wv", now);
	   save_wave(fname, d1, d2, &wi);

	   strcat(buf, fname);
	   strcat(buf, " &");
	   system(buf);
	 */
	copy_wave(waves, d1, d2, &wi);	// TODO: triplebuffer w/ locking
	notify_viewer(waves);

	if (run_again)
		g_thread_create((GThreadFunc) rerun_thread, NULL, FALSE, NULL);

}

int scope_run(int single)
{
	PICO_STATUS res;
	unsigned long pre = scope_config.pre_trig, post =
	    scope_config.post_trig;

	if (!scope_type)
		return 0;

	if (!(scope_config.trig_enabled)) {
		post += pre;
		pre = 0;
	}
	//printf("%ld %ld\n", pre, post);
	res =
	    ps5000RunBlock(handle, pre, post, scope_config.timebase, 0, NULL, 0,
			   CallBackBlock, NULL);

	if (res != PICO_OK)
		printf("run FAIL: %lx\n", res);

	return ((res == PICO_OK) ? 0 : -1);
}

int scope_trigger_config(void)
{
	PICO_STATUS res = PICO_OK;

	if (!scope_type)
		return res;

	if (scope_config.changed & SCOPE_CHANGED_TRIG_PROP) {
		// depends on channel & voltage
		scope_config.trig_prop.channel = scope_config.trig_ch;
		scope_config.trig_prop.thresholdMode = LEVEL;
		scope_config.trig_prop.hysteresis = 0;
		scope_config.trig_prop.thresholdMinor = scope_config.trig_level;
		scope_config.trig_prop.thresholdMajor = scope_config.trig_level;
		res =
		    ps5000SetTriggerChannelProperties(handle,
						      &(scope_config.trig_prop),
						      (scope_config.trig_enabled
						       ? 1 : 0), 1, 0);
		if (res != PICO_OK) {
			printf("SetTriggerChannelProperties: %ld\n", res);
			goto error;
		}
	}

	if (scope_config.changed & SCOPE_CHANGED_TRIG_COND) {
		// depends on channel

		// defaults
		scope_config.trig_cond.channelA = CONDITION_DONT_CARE;
		scope_config.trig_cond.channelB = CONDITION_DONT_CARE;
		scope_config.trig_cond.channelC = CONDITION_DONT_CARE;
		scope_config.trig_cond.channelD = CONDITION_DONT_CARE;
		scope_config.trig_cond.external = CONDITION_DONT_CARE;
		scope_config.trig_cond.aux = CONDITION_DONT_CARE;
		scope_config.trig_cond.pulseWidthQualifier =
		    CONDITION_DONT_CARE;

		if (scope_config.trig_ch == PS5000_CHANNEL_A)
			scope_config.trig_cond.channelA = CONDITION_TRUE;
		else if (scope_config.trig_ch == PS5000_CHANNEL_B)
			scope_config.trig_cond.channelB = CONDITION_TRUE;
		else if (scope_config.trig_ch == PS5000_EXTERNAL)
			scope_config.trig_cond.external = CONDITION_TRUE;
		else if (scope_config.trig_enabled) {
			printf("IARGH!?!?!\n");
		}

		res =
		    ps5000SetTriggerChannelConditions(handle,
						      &(scope_config.trig_cond),
						      (scope_config.trig_enabled
						       ? 1 : 0));
		if (res != PICO_OK) {
			printf("SetTriggerChannelConditions: %ld\n", res);
			goto error;
		}
	}

	if (scope_config.changed & SCOPE_CHANGED_TRIG_DIR) {
		THRESHOLD_DIRECTION dir[6] =
		    { NONE, NONE, NONE, NONE, NONE, NONE };

		//printf("ch %d edge %d\n",scope_config.trig_ch, scope_config.trig_dir);

		if (scope_config.trig_ch == PS5000_CHANNEL_A)
			dir[0] = scope_config.trig_dir;
		else if (scope_config.trig_ch == PS5000_CHANNEL_B)
			dir[1] = scope_config.trig_dir;
		else if (scope_config.trig_ch == PS5000_EXTERNAL)
			dir[4] = scope_config.trig_dir;
		else if (scope_config.trig_enabled) {
			printf("IARGH!?!?!\n");
		}
		//printf("%d %d %d\n",dir[0],dir[1],dir[2]);

		res =
		    ps5000SetTriggerChannelDirections(handle, dir[0], dir[1],
						      dir[2], dir[3], dir[4],
						      dir[5]);
		if (res != PICO_OK) {
			printf("SetTriggerChannelDirections: %ld\n", res);
			goto error;
		}
	}

	scope_config.changed &=
	    ~(SCOPE_CHANGED_TRIG_PROP | SCOPE_CHANGED_TRIG_DIR |
	      SCOPE_CHANGED_TRIG_DIR);

	//printf("trigger cfg fine???\n");
	return res;

 error:
	printf("trigger cfg error\n");
	return res;
}

int scope_siggen_config(long ofs, unsigned long pk2pk, float f, short wform)
{
	int ret = 0;
	PICO_STATUS res;

	if (!scope_type)
		return ret;

	res =
	    ps5000SetSigGenBuiltIn(handle, ofs, pk2pk, wform, f, f, 0, 0, 0, 0,
				   0, 0, 0, 0, 0);

	if (res == PICO_OK)
		return ret;

	switch (res) {
	case PICO_SIG_GEN_PARAM:
		ret = -1;
		break;
	case PICO_SIGGEN_OFFSET_VOLTAGE:
		ret = -2;
		break;
	case PICO_SIGGEN_PK_TO_PK:
		ret = -3;
		break;
	case PICO_SIGGEN_OUTPUT_OVER_VOLTAGE:
		ret = -4;
		break;
	default:
		ret = -5;
	}

	return ret;
}
