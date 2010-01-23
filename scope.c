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
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

scope_config_t scope_config;

short handle;
static int initialized = 0;

extern unsigned long ns;

int scope_open(void) {
	PICO_STATUS res = ps5000OpenUnit(&handle);
	assert(res == PICO_OK);
	initialized = 1;
	return 0;
}

void scope_close(void) {
	if(!initialized)
		return;
	ps5000CloseUnit(handle);
	initialized=0;
}

int scope_channel_config(int ch) {
	PS5000_CHANNEL scope_ch = ch ? PS5000_CHANNEL_B : PS5000_CHANNEL_A;
	short enable =(scope_config.channel_config>>ch)&1;
	short dc = (scope_config.channel_config>>(ch*2))&1;
	PS5000_RANGE range = scope_config.range[ch];

	printf("ch %d: %s %s %d\n",scope_ch,enable?"enable":"",dc?"DC":"AC",range);
	
	if(!initialized)
		return 0;
	
	return ((ps5000SetChannel(handle, scope_ch, enable, dc, range) == PICO_OK) ? 0 : -1);
}

int scope_sample_config(unsigned long *tbase, unsigned long *buflen) {
	long ns;
	long samples;
	PICO_STATUS res;
	
	if(!initialized)
		return 0;
	
	res = ps5000GetTimebase(handle, *tbase, *buflen, &ns, 0, &samples, 0);
	
	printf("%ld: %ld ns %ld samples\n",res,ns,samples);
	
	return ((res == PICO_OK) ? 0 : -1);
}

void single_done(void);

void PREF4 CallBackBlock(
                short handle,
                PICO_STATUS status,
                void * pParameter
)
{
	char fname[64];
	unsigned long scnt = scope_config.samples;
	FILE *fl;
	int cnt;
	short *d1=NULL, *d2=NULL;

	printf("done\n");
	
	// notify gui (gui will call scope_stop)
	// TODO: my feelings tell me there could be some threading issues?
	single_done();	
	
	// 1st channel active
	if((scope_config.channel_config>>0)&1) {
		assert((d1 = malloc(scope_config.samples * sizeof(short))));
		assert(ps5000SetDataBuffer(handle, PS5000_CHANNEL_A, d1, scope_config.samples) == PICO_OK);
		assert(ps5000GetValues(handle, 0, &scnt, 1, RATIO_MODE_NONE, 0, NULL) == PICO_OK);
	}
	
	// 2nd channel active
	if((scope_config.channel_config>>1)&1) {
		assert((d2 = malloc(scope_config.samples * sizeof(short))));
		assert(ps5000SetDataBuffer(handle, PS5000_CHANNEL_B, d2, scope_config.samples) == PICO_OK);
		assert(ps5000GetValues(handle, 0, &scnt, 1, RATIO_MODE_NONE, 0, NULL) == PICO_OK);
	}
	
	sprintf(fname,"%ld.txt",time(NULL));
	fl=fopen(fname,"w");
	
	for(cnt=0; cnt<scnt; cnt++) {
		float scaled_a, scaled_b, vala, valb;
		double tstamp = cnt;
		if(d1) {
			vala=d1[cnt];
			scaled_a = vala * scope_config.f_range[0]; // scale to Volts
			scaled_a /= PS5000_MAX_VALUE;	
		}
		if(d2) {
			valb=d2[cnt];
			scaled_b = valb * scope_config.f_range[1]; // scale to Volts
			scaled_b /= PS5000_MAX_VALUE;
		}
		tstamp *= ns;
		tstamp /= 1000000000;
		
		if((d1) && (d2))
			fprintf(fl,"%e %f %f\n",tstamp,scaled_a,scaled_b);
		else if(d1)
			fprintf(fl,"%e %f\n",tstamp,scaled_a);
		else if(d2)
			fprintf(fl,"%e %f\n",tstamp,scaled_b);
	}
	
	fclose(fl);
	
	free(d1);
	free(d2);
}

int scope_run(int single) {
	PICO_STATUS res;
	unsigned long pre = scope_config.pre_trig, post = scope_config.post_trig;
	
	if(!initialized)
		return 0;
	
	if(!(scope_config.trig_enabled)) {
		post+=pre;
		pre=0;
	}
	
	printf("%ld %ld\n",pre, post);
	res = ps5000RunBlock(handle, pre, post, scope_config.timebase, 0, NULL, 0, CallBackBlock, NULL);
	
	printf("run: %ld (%s)\n",res,(res == PICO_OK) ? "ok" : "FAIL");
	
	return ((res == PICO_OK) ? 0 : -1);
}

void scope_stop(void) {
	if(!initialized)
		return;
	ps5000Stop(handle);
}

int scope_trigger_config(void) {
	PICO_STATUS res = PICO_OK;

	if(!initialized)
		return res;
	
	if(scope_config.changed & SCOPE_CHANGED_TRIG_PROP) {
		// depends on channel & voltage
		scope_config.trig_prop.channel = scope_config.trig_ch;
		scope_config.trig_prop.thresholdMode = LEVEL;
		scope_config.trig_prop.hysteresis = 0;
		scope_config.trig_prop.thresholdMinor = scope_config.trig_level;
		scope_config.trig_prop.thresholdMajor = scope_config.trig_level;
		res = ps5000SetTriggerChannelProperties(handle, &(scope_config.trig_prop), (scope_config.trig_enabled ? 1 : 0), 1, 0);
		if (res != PICO_OK) {
			printf("SetTriggerChannelProperties: %ld\n",res);
			goto error;
		}
	}
	
	if(scope_config.changed & SCOPE_CHANGED_TRIG_COND) {
		// depends on channel
				
		// defaults
		scope_config.trig_cond.channelA = CONDITION_DONT_CARE;
		scope_config.trig_cond.channelB = CONDITION_DONT_CARE;
		scope_config.trig_cond.channelC = CONDITION_DONT_CARE;
		scope_config.trig_cond.channelD = CONDITION_DONT_CARE;
		scope_config.trig_cond.external = CONDITION_DONT_CARE;
		scope_config.trig_cond.aux = CONDITION_DONT_CARE;
		scope_config.trig_cond.pulseWidthQualifier = CONDITION_DONT_CARE;
		
		if(scope_config.trig_ch == PS5000_CHANNEL_A)
			scope_config.trig_cond.channelA = CONDITION_TRUE;
		else if(scope_config.trig_ch == PS5000_CHANNEL_B)
			scope_config.trig_cond.channelB = CONDITION_TRUE;
		else if(scope_config.trig_ch == PS5000_EXTERNAL)
			scope_config.trig_cond.external = CONDITION_TRUE;
		else if(scope_config.trig_enabled) {
			printf("IARGH!?!?!\n");
		}
		
		res = ps5000SetTriggerChannelConditions(handle, &(scope_config.trig_cond), (scope_config.trig_enabled ? 1 : 0));
		if (res != PICO_OK) {
			printf("SetTriggerChannelConditions: %ld\n",res);
			goto error;
		}
	}
	
	if(scope_config.changed & SCOPE_CHANGED_TRIG_DIR) {
		THRESHOLD_DIRECTION dir[6] = {NONE, NONE, NONE, NONE, NONE, NONE};
		
		//printf("ch %d edge %d\n",scope_config.trig_ch, scope_config.trig_dir);
		
		if(scope_config.trig_ch == PS5000_CHANNEL_A)
			dir[0] = scope_config.trig_dir;
		else if(scope_config.trig_ch == PS5000_CHANNEL_B)
			dir[1] = scope_config.trig_dir;
		else if(scope_config.trig_ch == PS5000_EXTERNAL)
			dir[4] = scope_config.trig_dir;
		else if(scope_config.trig_enabled) {
			printf("IARGH!?!?!\n");
		}
		
		res = ps5000SetTriggerChannelDirections(handle, dir[0], dir[1], dir[2], dir[3], dir[4], dir[5]);
		if (res != PICO_OK) {
			printf("SetTriggerChannelDirections: %ld\n",res);
			goto error;
		}
	}
		
	scope_config.changed &= ~(SCOPE_CHANGED_TRIG_PROP | SCOPE_CHANGED_TRIG_DIR | SCOPE_CHANGED_TRIG_DIR);
	
	printf("trigger cfg fine???\n");
	return res;	
	
error:
	printf("trigger cfg error\n");
	return res;
}
