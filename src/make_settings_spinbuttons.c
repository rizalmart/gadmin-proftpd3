/* GADMIN-PROFTPD - An easy to use GTK+ frontend for the ProFTPD standalone server.
 * Copyright (C) 2001 - 2013 Magnus Loef <magnus-swe@telia.com> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
*/



#include <gtk/gtk.h>
#include "make_settings_spinbuttons.h"


GtkWidget * make_spinbutton_with_label(GtkTable * table,
                            const gchar * label_text, gint left_attach,
                                    gint right_attach, gint top_attach,
                             gint bottom_attach, gint spinbutton_length)
{
    GtkWidget *spinbutton;
    gdouble min = 0;            /* minimum value */
    gdouble max = 999999;       /* max value */
    gdouble step = 1;           /* Steps per click */

    GtkWidget *label;
    label = gtk_label_new(label_text);

    spinbutton = gtk_spin_button_new_with_range(min, max, step);

    gtk_table_attach(table, label, left_attach, right_attach, top_attach, bottom_attach, GTK_FILL, GTK_EXPAND, 2, 2);
    gtk_table_attach(table, spinbutton, left_attach + 1, right_attach + 1, top_attach, bottom_attach,
                                                  GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 20, 2);

    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_misc_set_padding(GTK_MISC(label), 2, 2);
    gtk_widget_set_size_request(spinbutton, spinbutton_length, -1);

    gtk_widget_show(spinbutton);
    gtk_widget_show(label);

    return spinbutton;
}

GtkWidget * make_padded_spinbutton_with_label(GtkTable * table,
                                    const gchar * label_text, gint left_attach,
                                            gint right_attach, gint top_attach,
                                     gint bottom_attach, gint spinbutton_length)
{
    GtkWidget *spinbutton;
    gdouble min = 0;            /* minimum value */
    gdouble max = 999999;       /* max value */
    gdouble step = 1;           /* Steps per click */

    /* This function is only needed to make the passive port range from spinbutton a bit smaller */

    GtkWidget *label, *padlabel;
    GtkWidget *hbox;

    label = gtk_label_new(label_text);
    padlabel = gtk_label_new("");
    hbox = gtk_hbox_new(TRUE, 0);

    spinbutton = gtk_spin_button_new_with_range(min, max, step);

    gtk_table_attach(table, label, left_attach, right_attach, top_attach, bottom_attach, GTK_FILL, GTK_EXPAND, 2, 2);
    gtk_table_attach(table, hbox, left_attach + 1, right_attach + 1, top_attach, bottom_attach,
                                            GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 20, 2);

    gtk_box_pack_start(GTK_BOX(hbox), padlabel, TRUE, TRUE, 0);
    gtk_widget_set_size_request(padlabel, -1, -1);
    gtk_box_pack_start(GTK_BOX(hbox), spinbutton, TRUE, TRUE, 0);

    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_misc_set_padding(GTK_MISC(label), 2, 2);
    gtk_widget_set_size_request(spinbutton, spinbutton_length, -1);

    gtk_widget_show(spinbutton);
    gtk_widget_show(label);

    return spinbutton;
}

GtkWidget * make_short_spinbutton_with_label(GtkTable * table,
                                 const gchar * label_text, gint left_attach,
                                         gint right_attach, gint top_attach,
                                  gint bottom_attach, gint spinbutton_length)
{
    GtkWidget *spinbutton;
    gdouble min = 0;            /* minimum value */
    gdouble max = 999999;       /* max value */
    gdouble step = 1;           /* Steps per click */

    GtkWidget *label;
    label = gtk_label_new(label_text);

    spinbutton = gtk_spin_button_new_with_range(min, max, step);

    gtk_table_attach(table, label, left_attach, right_attach, top_attach, bottom_attach, GTK_FILL, GTK_EXPAND, 2, 2);
    gtk_table_attach(table, spinbutton, left_attach + 2, right_attach + 2, top_attach, bottom_attach,
                                                  GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 20, 2);

    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_misc_set_padding(GTK_MISC(label), 2, 2);
    gtk_widget_set_size_request(spinbutton, spinbutton_length, -1);

    gtk_widget_show(spinbutton);
    gtk_widget_show(label);

    return spinbutton;
}

GtkWidget * make_shortleft_spinbutton_with_label(GtkTable * table,
                                     const gchar * label_text, gint left_attach,
                                             gint right_attach, gint top_attach,
                                      gint bottom_attach, gint spinbutton_length)
{
    GtkWidget *spinbutton;
    gdouble min = 0;            /* minimum value */
    gdouble max = 999999;       /* max value */
    gdouble step = 1;           /* Steps per click */

    GtkWidget *label, *padlabel;
    GtkWidget *hbox;

    label = gtk_label_new(label_text);
    padlabel = gtk_label_new("");
    hbox = gtk_hbox_new(TRUE, 0);

    spinbutton = gtk_spin_button_new_with_range(min, max, step);

    gtk_table_attach(table, label, left_attach, right_attach, top_attach, bottom_attach, GTK_FILL, GTK_EXPAND, 2, 2);
    gtk_table_attach(table, hbox, left_attach + 1, right_attach + 1, top_attach, bottom_attach,
                                            GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 20, 2);

    gtk_box_pack_start(GTK_BOX(hbox), spinbutton, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), padlabel, TRUE, TRUE, 0);
    gtk_widget_set_size_request(padlabel, -1, -1);

    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_misc_set_padding(GTK_MISC(label), 2, 2);
    gtk_widget_set_size_request(spinbutton, spinbutton_length, -1);

    gtk_widget_show(spinbutton);
    gtk_widget_show(label);

    return spinbutton;
}

GtkWidget * make_spinbutton(GtkTable * table, gint left_attach, gint right_attach,
                 gint top_attach, gint bottom_attach, gint spinbutton_length)
{
    GtkWidget *spinbutton;
    gdouble min = 0;            /* minimum value */
    gdouble max = 999999;       /* max value */
    gdouble step = 1;           /* Steps per click */

    spinbutton = gtk_spin_button_new_with_range(min, max, step);

    gtk_table_attach(table, spinbutton, left_attach + 2, right_attach + 2, top_attach, bottom_attach,
                                                  GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 20, 2);

    gtk_widget_set_size_request(spinbutton, spinbutton_length, -1);

    gtk_widget_show(spinbutton);

    return spinbutton;
}
