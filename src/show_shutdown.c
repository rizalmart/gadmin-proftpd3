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
#include "../config.h"
#include "support.h"
#include "gettext.h"
#include "widgets.h"
#include "shutdown_ok_clicked.h"


void show_shutdown(struct w *widgets)
{
    GtkWidget *shutdown_ok_button, *shutdown_cancel_button;

    GtkWidget *hseparator35, *hbox134, *label146;
    GtkWidget *label251, *vbox17, *label141;
    GtkWidget *hbox78, *label142;
    GtkWidget *hbox79, *label145, *label144;
    GtkWidget *label148, *hbox81, *label147;

    GtkWidget *label152, *image36, *label153;
    GtkWidget *hseparator49, *label252, *hseparator34;
    GtkWidget *alignment37, *hbox83, *image37;
    GtkWidget *label154, *hbuttonbox9;
    GtkWidget *alignment36, *hbox82;


    //GtkTooltips *tooltips;
    //tooltips = gtk_tooltips_new();

    widgets->shutdown_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width(GTK_CONTAINER(widgets->shutdown_window), 5);
    gtk_window_set_title(GTK_WINDOW(widgets->shutdown_window), _("Shutdown"));
    gtk_window_set_position(GTK_WINDOW(widgets->shutdown_window), GTK_WIN_POS_CENTER);

    vbox17 = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(widgets->shutdown_window), vbox17);

    label141 = gtk_label_new(_("The server will shut down according to these settings."));
    gtk_box_pack_start(GTK_BOX(vbox17), label141, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label141), GTK_JUSTIFY_LEFT);

    hseparator35 = gtk_hseparator_new();
    gtk_widget_set_name(hseparator35, "hseparator35");
    gtk_widget_show(hseparator35);
    gtk_box_pack_start(GTK_BOX(vbox17), hseparator35, TRUE, TRUE, 0);
    gtk_widget_set_size_request(hseparator35, -1, 5);

    hbox134 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox17), hbox134, TRUE, TRUE, 0);

    label146 = gtk_label_new(_("Time to real shutdown:"));
    gtk_box_pack_start(GTK_BOX(hbox134), label146, FALSE, FALSE, 0);
    gtk_widget_set_size_request(label146, 260, -1);
    gtk_label_set_justify(GTK_LABEL(label146), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment(GTK_MISC(label146), 0, 0.5);
    gtk_misc_set_padding(GTK_MISC(label146), 10, 0);

    widgets->real_shutdown_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox134), widgets->real_shutdown_entry, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text( widgets->real_shutdown_entry, _("now, +Minutes or HHMM"));
    gtk_entry_set_max_length(GTK_ENTRY(widgets->real_shutdown_entry), 1000);
    gtk_entry_set_text(GTK_ENTRY(widgets->real_shutdown_entry), "+30");

    label251 = gtk_label_new("");
    gtk_widget_set_name(label251, "label251");
    gtk_widget_show(label251);
    gtk_box_pack_start(GTK_BOX(hbox134), label251, FALSE, FALSE, 0);
    gtk_widget_set_size_request(label251, 80, 16);
    gtk_label_set_justify(GTK_LABEL(label251), GTK_JUSTIFY_LEFT);

    hbox78 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox17), hbox78, TRUE, TRUE, 0);

    label142 = gtk_label_new(_("New access disabled:"));
    gtk_box_pack_start(GTK_BOX(hbox78), label142, FALSE, FALSE, 0);
    gtk_widget_set_size_request(label142, 260, -1);
    gtk_label_set_justify(GTK_LABEL(label142), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment(GTK_MISC(label142), 0, 0.5);
    gtk_misc_set_padding(GTK_MISC(label142), 10, 0);

    widgets->new_acc_disabled_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox78), widgets->new_acc_disabled_entry, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text( widgets->new_acc_disabled_entry, _("Minutes before real shutdown."));
    gtk_entry_set_max_length(GTK_ENTRY(widgets->new_acc_disabled_entry), 1000);
    gtk_entry_set_text(GTK_ENTRY(widgets->new_acc_disabled_entry), "20");

    label144 = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(hbox78), label144, FALSE, FALSE, 0);
    gtk_widget_set_size_request(label144, 80, 16);
    gtk_label_set_justify(GTK_LABEL(label144), GTK_JUSTIFY_LEFT);

    hbox79 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox17), hbox79, TRUE, TRUE, 0);

    label145 = gtk_label_new(_("Existing users disconnected:"));
    gtk_box_pack_start(GTK_BOX(hbox79), label145, FALSE, FALSE, 0);
    gtk_widget_set_size_request(label145, 260, -1);
    gtk_label_set_justify(GTK_LABEL(label145), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment(GTK_MISC(label145), 0, 0.5);
    gtk_misc_set_padding(GTK_MISC(label145), 10, 0);

    widgets->existing_users_dc_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox79), widgets->existing_users_dc_entry, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text( widgets->existing_users_dc_entry, _("Minutes before real shutdown."));
    gtk_entry_set_max_length(GTK_ENTRY(widgets->existing_users_dc_entry), 1000);
    gtk_entry_set_text(GTK_ENTRY(widgets->existing_users_dc_entry), "10");

    label148 = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(hbox79), label148, FALSE, FALSE, 0);
    gtk_widget_set_size_request(label148, 80, 16);
    gtk_label_set_justify(GTK_LABEL(label148), GTK_JUSTIFY_LEFT);

    hbox81 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox17), hbox81, TRUE, TRUE, 0);

    label147 = gtk_label_new(_("Shutdown message:"));
    gtk_box_pack_start(GTK_BOX(hbox81), label147, FALSE, FALSE, 0);
    gtk_widget_set_size_request(label147, 180, -1);
    gtk_label_set_justify(GTK_LABEL(label147), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment(GTK_MISC(label147), 0.01, 0.5);
    gtk_misc_set_padding(GTK_MISC(label147), 10, 0);

    widgets->shutdown_msg_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox81), widgets->shutdown_msg_entry, TRUE, TRUE, 0);
    gtk_entry_set_max_length(GTK_ENTRY(widgets->shutdown_msg_entry), 1000);
    gtk_entry_set_text(GTK_ENTRY(widgets->shutdown_msg_entry), _("%s , Current connections will be dropped: %d"));

    label152 = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(hbox81), label152, FALSE, FALSE, 0);
    gtk_widget_set_size_request(label152, 10, 16);
    gtk_label_set_justify(GTK_LABEL(label152), GTK_JUSTIFY_LEFT);

    hseparator49 = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox17), hseparator49, TRUE, TRUE, 0);
    gtk_widget_set_size_request(hseparator49, -1, 10);

    label252 = gtk_label_new(_("Pressing the activate button will cancel the shutdown and let users login again."));
    gtk_box_pack_start(GTK_BOX(vbox17), label252, FALSE, FALSE, 0);
    gtk_widget_set_size_request(label252, 297, -1);
    gtk_label_set_justify(GTK_LABEL(label252), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment(GTK_MISC(label252), 0.04, 0.5);

    hseparator34 = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox17), hseparator34, TRUE, TRUE, 0);
    gtk_widget_set_size_request(hseparator34, -1, 10);

    hbuttonbox9 = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(vbox17), hbuttonbox9, TRUE, TRUE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox9), GTK_BUTTONBOX_SPREAD);

    shutdown_cancel_button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(hbuttonbox9), shutdown_cancel_button);
    //GTK_WIDGET_SET_FLAGS(shutdown_cancel_button, GTK_CAN_DEFAULT);
    gtk_widget_set_can_default(shutdown_cancel_button, TRUE);
    
    alignment37 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(shutdown_cancel_button), alignment37);

    hbox83 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(alignment37), hbox83);

    image37 = gtk_image_new_from_stock("gtk-cancel", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_name(image37, "image37");
    gtk_widget_show(image37);
    gtk_box_pack_start(GTK_BOX(hbox83), image37, TRUE, TRUE, 0);

    label154 = gtk_label_new_with_mnemonic(_("Cancel"));
    gtk_box_pack_start(GTK_BOX(hbox83), label154, TRUE, TRUE, 0);
    gtk_label_set_justify(GTK_LABEL(label154), GTK_JUSTIFY_LEFT);

    shutdown_ok_button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(hbuttonbox9), shutdown_ok_button);
    //GTK_WIDGET_SET_FLAGS(shutdown_ok_button, GTK_CAN_DEFAULT);
    gtk_widget_set_can_default(shutdown_ok_button, TRUE);

    alignment36 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(shutdown_ok_button), alignment36);

    hbox82 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(alignment36), hbox82);

    image36 = gtk_image_new_from_stock("gtk-apply", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(hbox82), image36, TRUE, TRUE, 0);

    label153 = gtk_label_new_with_mnemonic(_("OK"));
    gtk_box_pack_start(GTK_BOX(hbox82), label153, TRUE, TRUE, 0);
    gtk_label_set_justify(GTK_LABEL(label153), GTK_JUSTIFY_LEFT);

    g_signal_connect_swapped(shutdown_cancel_button,
            "clicked", G_CALLBACK(gtk_widget_destroy), G_OBJECT(widgets->shutdown_window));

    g_signal_connect_swapped(shutdown_ok_button,
            "clicked", G_CALLBACK(shutdown_ok_clicked), widgets);

    gtk_widget_show_all(widgets->shutdown_window);
}
