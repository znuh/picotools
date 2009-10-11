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
#include <gtk/gtk.h>
#include "scope.h"

GtkLabel *samples_lbl;
GtkLabel *srate_lbl;
GtkLabel *trig_volt_lbl;
GtkLabel *trig_pre_lbl;
GtkLabel *trig_post_lbl;
GtkLabel *time_lbl;

GtkToggleButton *single_btn;

guint reconf_timer=-1;
int reconf_timer_active=0;

extern scope_config_t scope_config;

void format_time(char *buf, float time_ns) {
	char mult='n';
	
	if(time_ns>1000) {
		time_ns/=1000;
		mult='u';
	}
	if(time_ns>1000) {
		time_ns/=1000;
		mult='m';
	}
	if(time_ns>1000) {
		time_ns/=1000;
		mult=' ';
	}
	if(mult != ' ')
		sprintf(buf,"%.3f %cs",time_ns,mult);
	else
		sprintf(buf,"%.3f s",time_ns);
}

void single_done(void) {
	gtk_toggle_button_set_active(single_btn,0);
}

/***************** srate/buf reconf timer **************************/

unsigned long sbuf_len=0;
unsigned long tbase=0;
unsigned long ns=0;
float srate=0;

gboolean timeout(gpointer data) {
	int res =	scope_sample_config(&tbase, &sbuf_len);
	
	// TODO
	scope_config.timebase = tbase;
	scope_config.samples = sbuf_len;
	
	printf("%d: timebase %ld buf_len %ld\n",res,tbase,sbuf_len);
	
	reconf_timer_active=0;
	return FALSE;
}

void schedule_reconfig(void) {
	if(reconf_timer_active)
		g_source_remove(reconf_timer);
	reconf_timer=g_timeout_add(100,timeout,NULL);
	reconf_timer_active=1;
}

/***************** action buttons **************************/

void on_single_btn_toggled(GtkWidget * w, gpointer priv)
{
	int val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
	int res;
	
	if(val)
		res = scope_run(1);
	else
		scope_stop();
}

void on_auto_btn_toggled(GtkWidget * w, gpointer priv)
{
	// ignore auto mode while dumping to textfiles... (bad idea)
	/*
	int val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
	int res;
 
	if(val)
		res = scope_run(0);
	*/
}

/***************** channel config **************************/

PS5000_RANGE scope_range[] = {
	PS5000_100MV,
        PS5000_200MV,
        PS5000_500MV,
        PS5000_1V,
        PS5000_2V,
        PS5000_5V,
        PS5000_10V,
        PS5000_20V
};

float f_scope_range[] = {
	0.1,
	0.2,
	0.5,
	1,
	2,
	5,
	10,
	20
};

void update_trigger_voltage(void) {
	char buf[64];
	float level, range = 20.0; // ext
	
	if(scope_config.trig_ch == PS5000_CHANNEL_A)
		range = scope_config.f_range[0];
	
	else if(scope_config.trig_ch == PS5000_CHANNEL_B)
		range = scope_config.f_range[1];
	
	level = (range * scope_config.trig_level) / PS5000_MAX_VALUE;
	
	sprintf(buf,"threshold: %.6fV",level);
	gtk_label_set_text(trig_volt_lbl, buf);
}

void update_trigger_offset() {
	char buf[64], tbuf[64];
	unsigned long long scaled_val = sbuf_len;
	float pre, post;
	
	scaled_val *= scope_config.trig_ofs;
	scaled_val /= 128000000;
		
	post = sbuf_len - scaled_val;
	pre = sbuf_len - post;
	
	format_time(tbuf,pre*ns);
	sprintf(buf,"pre  %.3fMS (%s)",pre/1000000,tbuf);
	gtk_label_set_text(trig_pre_lbl,buf);
	
	format_time(tbuf,post*ns);
	sprintf(buf,"post %.3fMS (%s)",post/1000000,tbuf);
	gtk_label_set_text(trig_post_lbl,buf);
}

void on_ch1_range_cbox_changed(GtkWidget * w, gpointer priv)
{
	int val = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
	//printf("%d\n",val);
	scope_config.range[0] = scope_range[val];
	scope_config.f_range[0] = f_scope_range[val];
	scope_channel_config(0);
	if(scope_config.trig_ch == PS5000_CHANNEL_A)
		update_trigger_voltage();
}

void on_ch2_range_cbox_changed(GtkWidget * w, gpointer priv)
{
	int val = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
	//printf("%d\n",val);
	scope_config.range[1] = scope_range[val];
	scope_config.f_range[1] = f_scope_range[val];
	scope_channel_config(1);
	if(scope_config.trig_ch == PS5000_CHANNEL_B)
		update_trigger_voltage();
}

void on_ch1_btn_toggled(GtkWidget * w, gpointer priv)
{
	int val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
	scope_config.channel_config &= ~(1<<0);
	scope_config.channel_config |= (val<<0);
	scope_channel_config(0);
}

void on_ch2_btn_toggled(GtkWidget * w, gpointer priv)
{
	int val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
	scope_config.channel_config &= ~(1<<1);
	scope_config.channel_config |= (val<<1);
	scope_channel_config(1);
}

void on_ch1_cpl_cbox_changed(GtkWidget * w, gpointer priv)
{
	int val = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
	//printf("%d\n",val);
	scope_config.channel_config &= ~(1<<2);
	scope_config.channel_config |= (val<<2);
	scope_channel_config(0);
}

void on_ch2_cpl_cbox_changed(GtkWidget * w, gpointer priv)
{
	int val = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
	//printf("%d\n",val);
	scope_config.channel_config &= ~(1<<3);
	scope_config.channel_config |= (val<<3);
	scope_channel_config(1);
}

/***************** srate/buffer config **************************/

void update_time(void) {
	char buf[64], tbuf[64];
	float stime = ns;
	
	if((ns == 0) || (sbuf_len == 0))
		return;
	
	format_time(tbuf,stime * sbuf_len);
	
	sprintf(buf,"time: %s",tbuf);
	gtk_label_set_text(time_lbl,buf);
}

void on_samples_scale_value_changed(GtkWidget * w, gpointer priv)
{
	char buf[64];
	sbuf_len = gtk_range_get_value(GTK_RANGE(w))*1000;
	sprintf(buf,"%.3f Msamples",((float)sbuf_len)/1000000);
	gtk_label_set_text(samples_lbl,buf);
	update_time();
	update_trigger_offset();
	schedule_reconfig();
}

void on_srate_scale_value_changed(GtkWidget * w, gpointer priv)
{
	char buf[64];
	tbase = 100-gtk_range_get_value(GTK_RANGE(w))/10000;
	
	if(tbase <= 2) {
		ns=(1<<tbase);
	}
	else {
		ns=(tbase-2)*8;
	}
	srate = 1000.0;
	srate/=ns;
	sprintf(buf,"srate %.2f MS/s (%ld ns)",srate,ns);
	gtk_label_set_text(srate_lbl,buf);
	update_time();
	update_trigger_offset();
	schedule_reconfig();
}

/***************** trigger config **************************/

void on_trig_volt_scale_value_changed(GtkWidget * w, gpointer priv)
{
	scope_config.trig_level = gtk_range_get_value(GTK_RANGE(w));	
	update_trigger_voltage();
	// TODO: config
}

PS5000_CHANNEL trig_channel[] = {
	PS5000_CHANNEL_A,
	PS5000_CHANNEL_B,
	PS5000_EXTERNAL
};

void on_trig_src_cbox_changed(GtkWidget * w, gpointer priv)
{
	int val = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
	scope_config.trig_enabled = val;
	if(val)
		scope_config.trig_ch = trig_channel[val-1];
	update_trigger_voltage();
	// TODO: config
}

void on_trig_ofs_scale_value_changed(GtkWidget * w, gpointer priv)
{
	scope_config.trig_ofs = gtk_range_get_value(GTK_RANGE(w));
	update_trigger_offset();
	// TODO: config
}

THRESHOLD_DIRECTION edge[] = {
	RISING,
	FALLING,
	RISING_OR_FALLING
};

void on_trig_edge_cbox_changed(GtkWidget * w, gpointer priv)
{
	int val = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
	scope_config.trig_dir = edge[val];
	// TODO: config
}
