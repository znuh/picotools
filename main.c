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
#include <glade/glade.h>
#include <string.h>
#include <assert.h>
#include "handlers.h"
#include "scope.h"

GladeXML *glade;
extern SCOPE_TYPE_t scope_type;

int main(int argc, char **argv)
{
	int dryrun = 0;

	// http://tadeboro.blogspot.com/2009/06/multi-threaded-gtk-applications.html
	if (!(g_thread_supported()))
		g_thread_init(NULL);

	gdk_threads_init();

	gdk_threads_enter();

	gtk_init(&argc, &argv);
	glade_init();

	assert((glade = glade_xml_new("pico.glade", NULL, NULL)));
	glade_xml_signal_autoconnect(glade);

	assert((samples_lbl =
		GTK_LABEL(glade_xml_get_widget(glade, "samples_lbl"))));
	assert((srate_lbl =
		GTK_LABEL(glade_xml_get_widget(glade, "srate_lbl"))));
	assert((time_lbl = GTK_LABEL(glade_xml_get_widget(glade, "time_lbl"))));
	assert((trig_volt_lbl =
		GTK_LABEL(glade_xml_get_widget(glade, "trig_volt_lbl"))));
	assert((trig_pre_lbl =
		GTK_LABEL(glade_xml_get_widget(glade, "trig_pre_lbl"))));
	assert((trig_post_lbl =
		GTK_LABEL(glade_xml_get_widget(glade, "trig_post_lbl"))));
	assert((single_btn =
		GTK_TOGGLE_BUTTON(glade_xml_get_widget(glade, "single_btn"))));

	if (argc > 1) {
		if (!(strcmp(argv[1], "-dryrun")))
			dryrun = 1;
	}

	scope_open(dryrun);

	GtkWindow *w = GTK_WINDOW(glade_xml_get_widget(glade, "window1"));
	if (scope_type == SCOPE_PS5204)
		gtk_window_set_title(w, "pscope - PS5204");
	else if (scope_type == SCOPE_PS5203)
		gtk_window_set_title(w, "pscope - PS5203");
	else
		gtk_window_set_title(w, "pscope - NONE");

	init();

	gtk_main();

	gdk_threads_leave();

	return 0;
}
