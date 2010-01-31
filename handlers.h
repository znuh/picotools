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
#ifndef HANDLERS_H
#define HANDLERS_H

#include <gtk/gtk.h>
#include <glade/glade.h>

GtkLabel *samples_lbl;
GtkLabel *srate_lbl;
GtkLabel *trig_volt_lbl;
GtkLabel *trig_pre_lbl;
GtkLabel *trig_post_lbl;
GtkLabel *time_lbl;

GtkToggleButton *single_btn;

void on_ch1_range_cbox_changed(GtkWidget * w, gpointer priv);
void on_ch2_btn_toggled(GtkWidget * w, gpointer priv);
void on_samples_scale_value_changed(GtkWidget * w, gpointer priv);
void on_trig_volt_scale_value_changed(GtkWidget * w, gpointer priv);
void on_ch2_cpl_cbox_changed(GtkWidget * w, gpointer priv);
void on_ch1_btn_toggled(GtkWidget * w, gpointer priv);
void on_srate_scale_value_changed(GtkWidget * w, gpointer priv);
void on_ch1_cpl_cbox_changed(GtkWidget * w, gpointer priv);
void on_trig_src_cbox_changed(GtkWidget * w, gpointer priv);
void on_trig_ofs_scale_value_changed(GtkWidget * w, gpointer priv);
void on_trig_edge_cbox_changed(GtkWidget * w, gpointer priv);
void on_ch2_range_cbox_changed(GtkWidget * w, gpointer priv);
void on_auto_btn_toggled(GtkWidget * w, gpointer priv);
void on_single_btn_toggled(GtkWidget * w, gpointer priv);
void init(void);
#endif
