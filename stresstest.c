
/* gcc -lps5000 -Wall -o stresstest stresstest.c */

#include <stdio.h>
#include <assert.h>

#include <libps5000-1.3/ps5000Api.h>

short 			handle;

void PREF4 CallBackBlock (short handle, PICO_STATUS status, void * pParameter) {
	fputc('.', stderr);
}

/*
		assert(ps5000SetDataBuffer
		       (handle, PS5000_CHANNEL_A, d1,
			_scope_config.samples) == PICO_OK);

	res = ps5000GetValues
	       (handle, 0, &scnt, 1, RATIO_MODE_NONE, 0, NULL);
*/

void trigger_setup(void)
{
	PICO_STATUS res = PICO_OK;
	TRIGGER_CHANNEL_PROPERTIES prop;
	TRIGGER_CONDITIONS cond;
	THRESHOLD_DIRECTION dir[6] = { RISING, NONE, NONE, NONE, NONE, NONE };

	prop.channel = PS5000_CHANNEL_A;
	prop.thresholdMode = LEVEL;
	prop.hysteresis = 0;
	prop.thresholdMinor = 0;
	prop.thresholdMajor = 100;
	res = ps5000SetTriggerChannelProperties(handle, &prop, 1, 1, 0);
	assert(res == PICO_OK);

	cond.channelA = CONDITION_TRUE;
	cond.channelB = CONDITION_DONT_CARE;
	cond.channelC = CONDITION_DONT_CARE;
	cond.channelD = CONDITION_DONT_CARE;
	cond.external = CONDITION_DONT_CARE;
	cond.aux = CONDITION_DONT_CARE;
	cond.pulseWidthQualifier = CONDITION_DONT_CARE;

	res = ps5000SetTriggerChannelConditions(handle, &cond, 1);
	assert(res == PICO_OK);
	
	res = ps5000SetTriggerChannelDirections(handle, dir[0], dir[1],
						      dir[2], dir[3], dir[4],
						      dir[5]);
	assert(res == PICO_OK);
}

int main(int argc, char **argv) {
	long int tbase = 10;
	long int buflen = 128 * 1024;
	long int ns, samples;
	PICO_STATUS res;

	res = ps5000OpenUnit(&handle);
	assert(res == PICO_OK);

	res = ps5000SetChannel(handle, PS5000_CHANNEL_A, 1, 0, PS5000_20V);
	assert(res == PICO_OK);

	res = ps5000GetTimebase(handle, tbase, buflen, &ns, 0, &samples, 0);
	assert(res == PICO_OK);

	if(argc>1) {
		trigger_setup();
		fprintf(stderr,"trigger set\n");
	}

	fprintf(stderr,"press Ctrl+D to quit\n");
	
	while(!feof(stdin)) {

		fputc('+', stderr);
		fflush(stderr);
		
		res = ps5000RunBlock(handle, buflen/2, buflen-(buflen/2), tbase, 0, NULL, 0, CallBackBlock, NULL);
		assert(res == PICO_OK);

		fputc('*', stderr);
		fflush(stderr);

		res = ps5000Stop(handle);
		assert(res == PICO_OK);
		
	}

	ps5000CloseUnit(handle);

	return 0;
}
