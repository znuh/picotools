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

#include <libps5000-1.3/ps5000Api.h>

#define SCOPE_CHANGED_CH1		(1<<0)
#define SCOPE_CHANGED_CH2		(1<<1)
#define SCOPE_CHANGED_SAMPLES	(1<<2)
#define SCOPE_CHANGED_TIMEBASE	(1<<3)
#define SCOPE_CHANGED_TRIG_COND	(1<<4)
#define SCOPE_CHANGED_TRIG_DIR		(1<<5)
#define SCOPE_CHANGED_TRIG_PROP	(1<<6)
#define SCOPE_CHANGED_TRIG_OFS	(1<<7)

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
	unsigned long samples;

	// timebase (srate)
	unsigned long timebase;

	PS5000_CHANNEL trig_ch;
	int trig_enabled;
	short trig_level;

	// depends on trig. src
	TRIGGER_CONDITIONS trig_cond;

	// depends on trig. edge
	THRESHOLD_DIRECTION trig_dir;

	// depends on trig. src + trg. voltage
	TRIGGER_CHANNEL_PROPERTIES trig_prop;

	// trigger ofs
	unsigned long trig_ofs;
	unsigned long pre_trig;
	unsigned long post_trig;

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
	unsigned long changed;	// TODO
} scope_config_t;

int scope_open(int dryrun);
void scope_close(void);
int scope_channel_config(int ch);
int scope_sample_config(unsigned long *tbase, unsigned long *buflen);
int scope_run(int single);
void scope_stop(void);
int scope_trigger_config(void);
void viewer_close(void);

#endif
