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
#ifndef PSCOPE_H
#define PSCOPE_H

#include <stdint.h>
#include <libps5000-1.3/ps5000Api.h>

typedef enum {
	SCOPE_NONE,
	SCOPE_PS5203 = 5203,
	SCOPE_PS5204 = 5204
} SCOPE_TYPE_t;

typedef enum {
	CH_ENABLE,
	CH_VOLTAGE_RANGE,
	CH_VOLTAGE_OFFSET,
	CH_COUPLING,
	SAMPLE_BUF_LEN,
	SAMPLE_RATE,
	TRIG_SOURCE,
	TRIG_EDGE,
	TRIG_THRESHOLD,
	TRIG_OFFSET,
	SIGGEN,
} CFG_ELEM_t;

typedef struct ps_ch_cfg_s {	
	short enabled;
	short dc;
	PS5000_RANGE range;
} ps_ch_cfg_t;

typedef struct ps_tbase_cfg_s {
	unsigned long tbase;	// timebase
	long samples;			// number of samples
	long timeval;			// time interval in ns
	long max_samples;		// max samples at given timebase
} ps_tbase_cfg_t;

typedef struct ps_trig_cfg_s {
	short enable;
	PS5000_CHANNEL ch;
	short level;
	THRESHOLD_DIRECTION dir;
//	TRIGGER_CONDITIONS cond;			// depends on trig. src
//	THRESHOLD_DIRECTION dir;			// depends on trig. edge
//	TRIGGER_CHANNEL_PROPERTIES prop;	// depends on trig. src + trg. voltage
} ps_trig_cfg_t;

typedef struct ps_siggen_cfg_s {
	long offset;
	unsigned long pk2pk;
	short wvtype;
	float freq;
} ps_siggen_cfg_t;

// TODO: siggen

typedef struct ps_cfg_s {

	// channels
	ps_ch_cfg_t ch[2];

	// timebase
	ps_tbase_cfg_t tbase;

	ps_trig_cfg_t trig;

	ps_siggen_cfg_t siggen;

	// trigger ofs
//	unsigned long trig_ofs;
//	unsigned long pre_trig;
//	unsigned long post_trig;

} ps_cfg_t;

/*
int scope_open(int dryrun);
void scope_close(void);
int scope_channel_config(int ch);
int scope_sample_config(unsigned long *tbase, unsigned long *buflen);
int scope_run(int single);
void scope_stop(void);
int scope_trigger_config(void);
void viewer_close(void);
int scope_siggen_config(long ofs, unsigned long pk2pk, float f, short wform);
*/

#endif
