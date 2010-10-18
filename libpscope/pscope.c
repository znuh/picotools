#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "scope.h"
#include "pscope.h"

short 			handle;

SCOPE_TYPE_t 		scope_type 			= SCOPE_NONE;

ps_cfg_t 	ps_cfg;


pthread_mutex_t	scope_mutex 			= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t 	data_cb_cond			= PTHREAD_COND_INITIALIZER;

pthread_t			data_cb_pthread;

int 			scope_running			= 0;
int				drop_values			= 1;

pthread_mutex_t	reconf_mutex 			= PTHREAD_MUTEX_INITIALIZER;
int				reconf_active = 0;

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


/* TODO:
	- user config with changed flags
	- reconf: apply changed values from user config, mark changed values?
*/

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
#define CFG_SIGGEN			0x20

/* this maps cfg_changed_user to the scope functions that
   needs to be called for the scope */
static uint32_t cfg_to_cmds[] = {
	CFG_CHANNEL | CFG_TIMEBASE, 		//CH_ENABLE,
	CFG_CHANNEL, 		//CH_VOLTAGE_RANGE,
	0,					//CH_VOLTAGE_OFFSET,  //not supported by picoscope
	CFG_CHANNEL, 		//CH_COUPLING,
	CFG_TIMEBASE, 		//SAMPLE_BUF_LEN,
	CFG_TIMEBASE, 		//SAMPLE_RATE,
	CFG_TRIG_CH_PROP | CFG_TRIG_CH_COND | CFG_TRIG_CH_DIR, //TRIG_SOURCE,
	CFG_TRIG_CH_DIR, 	//TRIG_EDGE,
	CFG_TRIG_CH_PROP, 	//TRIG_THRESHOLD,
	0, 					//TRIG_OFFSET,
	CFG_SIGGEN,			//SIGGEN
};

int reconf(scope_cfg_t user_cfg) {
	uint32_t cfg_cmds = 0;
	PS5000_CHANNEL ps_ch = PS5000_CHANNEL_A; // TODO
	PICO_STATUS status;
	int cnt, res;

	/* collect all changes together first,
	   picoscope functions will be invoked afterwards */
	for(cnt=0; cnt<CFG_END; cnt++) {
		if(!(user_cfg.cfg_changed_user & (1<<cnt)))
			continue;

		cfg_cmds |= cfg_to_cmds[cnt];

		switch(cnt) {
			case CFG_CH_ENABLE:
				break;
			case CFG_CH_VOLTAGE_RANGE:
				break;
			case CFG_CH_COUPLING:
				break;
			case CFG_SAMPLE_BUF_LEN:
				break;
			case CFG_SAMPLE_RATE:
				break;
			case CFG_TRIG_SOURCE:
				break;
			case CFG_TRIG_EDGE:
				break;
			case CFG_TRIG_THRESHOLD:
				break;
			case CFG_TRIG_OFFSET:
				break;
			case CFG_SIGGEN:
				break;
			default:
				assert(0);
		}
	}
	
	reconf_start();

	if(cfg_cmds & CFG_CHANNEL) {
		/*
		status = ps5000SetChannel(handle,
		                          ps_ch,
		                          ps_cfg.ch[ch].enabled, 
		                          ps_cfg.ch[ch].enabled, 
		                          ps_cfg.ch[ch].range);
								  */
		if(status != PICO_OK)
			goto error;
	}
	if(cfg_cmds & CFG_TIMEBASE) {
		status = ps5000GetTimebase(handle, 
		                           ps_cfg.tbase.tbase, 
		                           ps_cfg.tbase.samples,
		                           &ps_cfg.tbase.timeval,
		                           0, 
		                           &ps_cfg.tbase.samples,
		                           0);
		/* TODO: give reduced srate + sbuf back to user (ptr args?) */
		if(status != PICO_OK)
			goto error;
	}
	if(cfg_cmds & CFG_TRIG_CH_PROP) {
//		status = ps5000SetTriggerChannelProperties(
		if(status != PICO_OK)
			goto error;
	}
	if(cfg_cmds & CFG_TRIG_CH_COND) {
		//status = ps5000SetTriggerChannelConditions
		if(status != PICO_OK)
			goto error;
	}
	if(cfg_cmds & CFG_TRIG_CH_DIR) {
		//status = ps5000SetTriggerChannelDirections
		if(status != PICO_OK)
			goto error;
	}
	if(cfg_cmds & CFG_SIGGEN) {
		status = ps5000SetSigGenBuiltIn(handle, 
		                                ps_cfg.siggen.offset, 
		                                ps_cfg.siggen.pk2pk, 
		                                ps_cfg.siggen.wvtype, 
		                                ps_cfg.siggen.freq, 
		                                ps_cfg.siggen.freq, 
		                                0, 0, 0, 0,  0, 0, 0, 0, 0);
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
