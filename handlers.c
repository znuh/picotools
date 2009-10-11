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
GtkLabel *trig_ofs_lbl;
GtkLabel *time_lbl;

GtkToggleButton *single_btn;

guint reconf_timer=-1;
int reconf_timer_active=0;

extern scope_config_t scope_config;

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

void on_ch1_range_cbox_changed(GtkWidget * w, gpointer priv)
{
	int val = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
	//printf("%d\n",val);
	scope_config.range[0] = scope_range[val];
	scope_channel_config(0);
}

void on_ch2_range_cbox_changed(GtkWidget * w, gpointer priv)
{
	int val = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
	//printf("%d\n",val);
	scope_config.range[1] = scope_range[val];
	scope_channel_config(1);
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
	char buf[64];
	char mult = 'n';
	float stime = ns;
	
	if((ns == 0) || (sbuf_len == 0))
		return;
	
	stime *= sbuf_len;
	
	if(stime>1000) {
		stime/=1000;
		mult='u';
	}
	if(stime>1000) {
		stime/=1000;
		mult='m';
	}
	if(stime>1000) {
		stime/=1000;
		mult=' ';
	}
	
	sprintf(buf,"time: %.3f %cs",stime,mult);
	gtk_label_set_text(time_lbl,buf);
}

void on_samples_scale_value_changed(GtkWidget * w, gpointer priv)
{
	char buf[64];
	sbuf_len = gtk_range_get_value(GTK_RANGE(w))*1000;
	sprintf(buf,"%.3f Msamples",((float)sbuf_len)/1000000);
	gtk_label_set_text(samples_lbl,buf);
	update_time();
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
	schedule_reconfig();
}

/***************** trigger config **************************/

void on_trig_volt_scale_value_changed(GtkWidget * w, gpointer priv)
{
	float val = gtk_range_get_value(GTK_RANGE(w));
	printf("%f\n",val);
}

void on_trig_src_cbox_changed(GtkWidget * w, gpointer priv)
{
}

void on_trig_ofs_scale_value_changed(GtkWidget * w, gpointer priv)
{
	float val = gtk_range_get_value(GTK_RANGE(w));
	printf("%f\n",val);
}

void on_trig_edge_cbox_changed(GtkWidget * w, gpointer priv)
{
}
