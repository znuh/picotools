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
} ps_trig_cfg_t;

typedef struct ps_siggen_cfg_s {
	long offset;
	unsigned long pk2pk;
	short wvtype;
	float freq;
} ps_siggen_cfg_t;

typedef struct ps_cfg_s {

	// channels
	ps_ch_cfg_t ch[2];

	// timebase
	ps_tbase_cfg_t tbase;

	ps_trig_cfg_t trig;

	ps_siggen_cfg_t siggen;

} ps_cfg_t;

#endif
