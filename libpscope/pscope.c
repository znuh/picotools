#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "pscope.h"

#include <assert.h>

short 			handle;

SCOPE_TYPE_t 		scope_type 			= SCOPE_NONE;

scope_config_t 	active_cfg, new_cfg;


pthread_mutex_t	scope_mutex 			= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t 	data_cb_cond			= PTHREAD_COND_INITIALIZER;

pthread_t			data_cb_pthread;

int 			scope_running			= 0;
int				drop_values			= 1;

pthread_mutex_t	reconf_mutex 			= PTHREAD_MUTEX_INITIALIZER;
int				reconf_active = 0;

scope_config_t scope_config;

void run(void);

void reconf_start(void) {

	pthread_mutex_lock(&reconf_mutex);
	reconf_active = 1;
	pthread_mutex_unlock(&reconf_mutex);
	
	pthread_mutex_lock(&scope_mutex);
	
	drop_values = 1;
	
	// stop
	if(scope_running)
		ps5000Stop(handle);
}

void reconf_done(void) {

	pthread_mutex_lock(&reconf_mutex);
	reconf_active = 0;
	pthread_mutex_unlock(&reconf_mutex);
	
	// reenable if stopped
	if(scope_running)
		run();
	
	pthread_mutex_unlock(&scope_mutex);
}

/*
 - create copy of old config
 - modify config
 - try set config + run
 - success: return
 - fail: restore old config, reverse *p, rerun, return
*/

#define CFG_CHANNEL			1
#define CFG_TIMEBASE		2
#define CFG_TRIG_CH_PROP	4
#define CFG_TRIG_CH_COND	8
#define CFG_TRIG_CH_DIR		0x10

static uint32_t cfg_update[] = {
	CFG_CHANNEL | CFG_TIMEBASE, 		//CH_ENABLE,
	CFG_CHANNEL, 		//CH_VOLTAGE_RANGE,
	CFG_CHANNEL, 		//CH_COUPLING,
	CFG_TIMEBASE, 		//SAMPLE_BUF_LEN,
	CFG_TIMEBASE, 		//SAMPLE_RATE,
	CFG_TRIG_CH_PROP | CFG_TRIG_CH_COND | CFG_TRIG_CH_DIR, //TRIG_SOURCE,
	CFG_TRIG_CH_DIR, 	//TRIG_EDGE,
	CFG_TRIG_CH_PROP, 	//TRIG_THRESHOLD,
	0, 					//TRIG_OFFSET,
};

int reconf(CFG_ELEM_t elem, int ch, void *p) {
	PICO_STATUS status;
	int res;
	
	reconf_start();

	switch(elem) {
		case CH_ENABLE:
			break;
		case CH_VOLTAGE_RANGE:
			break;
		case CH_COUPLING:
			break;
			
		case SAMPLE_BUF_LEN:
			break;
		case SAMPLE_RATE:
			break;
		
		case TRIG_SOURCE:
			break;
		case TRIG_EDGE:
			break;
		case TRIG_THRESHOLD:
			break;
		case TRIG_OFFSET:
			break;
		
		default:
			return -1;
	}

	if(cfg_update[elem] & CFG_CHANNEL) {
		//status = ps5000SetChannel(handle, scope_ch, enable, dc, range);
		if(status != PICO_OK)
			goto error;
	}
	if(cfg_update[elem] & CFG_TIMEBASE) {
		//status = ps5000GetTimebase(handle, *tbase, *buflen, &ns, 0, &samples, 0);
		/* TODO: give reduced srate + sbuf back to user (ptr args?) */
		if(status != PICO_OK)
			goto error;
	}
	if(cfg_update[elem] & CFG_TRIG_CH_PROP) {
		//status = ps5000SetTriggerChannelProperties
		if(status != PICO_OK)
			goto error;
	}
	if(cfg_update[elem] & CFG_TRIG_CH_COND) {
		//status = ps5000SetTriggerChannelConditions
		if(status != PICO_OK)
			goto error;
	}
	if(cfg_update[elem] & CFG_TRIG_CH_DIR) {
		//status = ps5000SetTriggerChannelDirections
		if(status != PICO_OK)
			goto error;
	}

	// try run

	// check status

error:
	// rollback if necessary

	// run old cfg if necessary
	
	reconf_done();
	
	return 0;
}

#if 0
int ps_ch_set_vrange(int ch, uint32_t *vrange) {
	reconf_start();
	
	reconf_done();
	return 0;
}

int ps_ch_enable(int ch, int *en) {
	reconf_start();

	//return ((ps5000SetChannel(handle, scope_ch, enable, dc, range) ==
	//	 PICO_OK) ? 0 : -1);
	
	reconf_done();
	return 0;
}

int ps_ch_set_cp(int ch, int *cpl) {
	reconf_start();
	
	//return ((ps5000SetChannel(handle, scope_ch, enable, dc, range) ==
	//	 PICO_OK) ? 0 : -1);
	
	reconf_done();
	return 0;
}

int ps_set_sbuf(uint32_t *len) {
	reconf_start();
	
	//res = ps5000GetTimebase(handle, *tbase, *buflen, &ns, 0, &samples, 0);
	// tbase, buflen for Run, ...
	
	reconf_done();
	return 0;
}

int ps_set_srate(float *srate) {
	reconf_start();
	
	//res = ps5000GetTimebase(handle, *tbase, *buflen, &ns, 0, &samples, 0);
	// tbase, buflen for Run, ...
	
	reconf_done();
	return 0;
}

int ps_trig_set_src(int src) {
	reconf_start();
	
	//scope_trigger_config
	
	reconf_done();
	return 0;
}

int ps_trig_set_edge(int edge) {
	reconf_start();
	
	//ps5000SetTriggerChannelDirections
	
	reconf_done();
	return 0;
}

int ps_trig_set_thres(uint32_t *thres) {
	reconf_start();
	
	//ps5000SetTriggerChannelProperties
	
	reconf_done();
	return 0;
}

int ps_trig_set_ofs(float *ofs) {
	reconf_start();
	
	// for Run
	
	reconf_done();
	return 0;
}
#endif

int ps_run(int mode) {
	return 0;
}

int ps_stop(void) {
	return 0;
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

//	viewer_init();
//      res = ps5000SetSigGenBuiltIn(handle, 0, 3000000, 0, (float)1000000.0, (float)1000000., 0, 0, 0, 0, 0, 0, 0, 0, 0);
//      printf("%ld %lx\n",res,res);
//      assert(res == PICO_OK);
	return 0;
}

void PREF4 CallBackBlock (short handle, PICO_STATUS status, void * pParameter) {
	int reconf_active_copy;
	
	pthread_mutex_lock(&reconf_mutex);
	reconf_active_copy = reconf_active;
	pthread_mutex_unlock(&reconf_mutex);

	if(reconf_active_copy)
		return;
	
	//	data_ready = 1;
	//printf("cb\n");
	
	pthread_mutex_lock(&scope_mutex);
	drop_values = 0;
	pthread_cond_signal(&data_cb_cond);
	pthread_mutex_unlock(&scope_mutex);
	
//	printf("cb done\n");
}

void scope_close(void)
{
	if (!scope_type)
		return;
	ps5000CloseUnit(handle);
	//scope_type = SCOPE_NONE;

	//viewer_destroy();
}

void run(void) {
	assert(ps5000RunBlock(handle, 0, 100000, 100, 0, NULL, 0,
			   CallBackBlock, NULL) == PICO_OK);
}

void test(void) {
	assert( ps5000SetChannel(handle, PS5000_CHANNEL_A, 1, 1, PS5000_100MV) == PICO_OK);
	run();	
}

short buf[100000];
int dcnt = 0;

int max = 60;

volatile int done = 0;

void data_cb(int *bla) {
	unsigned long cnt;
	pthread_mutex_lock(&scope_mutex);
	printf("data_cb enter\n");
	while(1) {
		pthread_cond_wait(&data_cb_cond, &scope_mutex);
		if(drop_values)
			continue;
//		printf("data_cb %d\n",dcnt);
//		ps5000Stop(handle);
		cnt = 100000;
		ps5000SetDataBuffer(handle, PS5000_CHANNEL_A, buf, cnt);
		ps5000GetValues(handle, 0, (unsigned long*) &cnt, 1, RATIO_MODE_NONE, 0, NULL);
//		ps5000Stop(handle);
	//	printf("%d\n",cnt);
		if(!dcnt) {
			for(cnt=0; cnt<100000; cnt++)
				fprintf(stderr,"%d\n",buf[cnt]);
		}
		dcnt++;
		if(dcnt == max)
			break;
		run();
	}
	pthread_mutex_unlock(&scope_mutex);
	done = 1;
}

int main(int argc, char **argv) {

	if(argc > 1)
		max = atoi(argv[1]);
	
	assert(scope_open(0) == 0);

	pthread_create(&data_cb_pthread, NULL, (void *) data_cb, NULL);
	
	test();
	while(!done) {}

	pthread_join(data_cb_pthread, NULL);
	
	scope_close();
	
	return 0;
}
