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
#ifndef SCOPE_H
#define SCOPE_H

#include <stdint.h>

#include <libps5000/ps5000Api.h>

#define SCOPE_CHANGED_CH1		(1<<0)
#define SCOPE_CHANGED_CH2		(1<<1)
#define SCOPE_CHANGED_SAMPLES	(1<<2)
#define SCOPE_CHANGED_TIMEBASE	(1<<3)
#define SCOPE_CHANGED_TRIG_COND	(1<<4)
#define SCOPE_CHANGED_TRIG_DIR		(1<<5)
#define SCOPE_CHANGED_TRIG_PROP	(1<<6)
#define SCOPE_CHANGED_TRIG_OFS	(1<<7)
#define SCOPE_DATA_CB			(1<<8)
#define SCOPE_CHANGED_RUN		(1<<9)
#define SCOPE_CHANGED_SIGGEN	(1<<10)

typedef enum {
	SCOPE_NONE,
	SCOPE_PS5203 = 5203,
	SCOPE_PS5204 = 5204
} SCOPE_TYPE_t;

typedef struct scope_config_s {

	// channels
	PS5000_RANGE range[2];
	float f_range[2];
	uint8_t channel_config;	// (AC/DC, enabled) * 2

	// number of samples
	uint32_t samples;

	// timebase (srate)
	uint32_t timebase;

	PS5000_CHANNEL trig_ch;
	uint32_t trig_enabled;
	int16_t trig_level;

	// depends on trig. src
	TRIGGER_CONDITIONS trig_cond;

	// depends on trig. edge
	THRESHOLD_DIRECTION trig_dir;

	// depends on trig. src + trg. voltage
	TRIGGER_CHANNEL_PROPERTIES trig_prop;

	// trigger ofs
	uint32_t trig_ofs;
	uint32_t pre_trig;
	uint32_t post_trig;

	/*
	   chA
	   chB
	   number of samples
	   timebase
	   trig_cond
	   trig_dir
	   trig_prop
	   trig_ofs
	 */
	int run;
	uint32_t changed;
} scope_config_t;

int scope_open(int dryrun);
void scope_close(void);
int scope_channel_config(int ch);
int scope_sample_config(uint32_t *tbase, uint32_t *buflen);
int scope_run(int single);
void scope_stop(void);
int scope_trigger_config(void);
void viewer_close(void);
int scope_siggen_config(int32_t ofs, uint32_t pk2pk, float f, int16_t wform);

#endif
