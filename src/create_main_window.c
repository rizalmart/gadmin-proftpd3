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



#include "../config.h"
#include <gtk/gtk.h>
#include <string.h>
#include "gettext.h"
#include "support.h"
#include "widgets.h"
#include "credits_window.h"
#include "activate_button_clicked.h"
#include "deactivate_button_clicked.h"
#include "apply_button_clicked.h"
#include "show_shutdown.h"
#include "show_help.h"


void create_main_window(struct w *widgets)
{
    gchar *info, *utf8, *pixmap_directory;
    GtkCellRenderer *pixbuf_cell_renderer;
    GdkPixbuf *pixbuf;

    /* Create the main window */
    widgets->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(widgets->main_window), GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request(widgets->main_window, -1, 500);

    /* Set window information */
    info = g_strdup_printf("GADMIN-PROFTPD %s", VERSION);
    gtk_window_set_title(GTK_WINDOW(widgets->main_window), info);
    g_free(info);

    /* Set the main window icon */
    pixmap_directory = g_strdup_printf("%s/pixmaps/gadmin-proftpd", PACKAGE_DATA_DIR);
    add_pixmap_directory(pixmap_directory);
    g_free(pixmap_directory);

    pixbuf_cell_renderer = gtk_cell_renderer_pixbuf_new();
    pixbuf = create_pixbuf("gadmin-proftpd.png");
    g_object_set(pixbuf_cell_renderer, "pixbuf", pixbuf, NULL);
    gtk_window_set_icon(GTK_WINDOW(widgets->main_window), pixbuf);
    gdk_pixbuf_unref(pixbuf);

    widgets->main_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(widgets->main_window), widgets->main_vbox);

    /* Hboxes (down) */
    GtkWidget *toolbar_hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->main_vbox), toolbar_hbox, FALSE, FALSE, 0);

    GtkWidget *status_hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->main_vbox), status_hbox, FALSE, FALSE, 0);

    GtkWidget *status_hsep_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->main_vbox), status_hsep_hbox, FALSE, TRUE, 0);

    GtkWidget *notebook_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->main_vbox), notebook_hbox, TRUE, TRUE, 0);


    /* Create the main toolbar */
    GtkWidget *main_toolbar = gtk_toolbar_new();
    gtk_box_pack_start(GTK_BOX(toolbar_hbox), main_toolbar, TRUE, TRUE, 0);

    gtk_toolbar_set_style(GTK_TOOLBAR(main_toolbar), GTK_TOOLBAR_BOTH);



    /* Activate button */
    info = g_strdup_printf(_("Activate"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    
    /*
    GtkWidget *toolbar_icon_yes = gtk_image_new_from_stock("gtk-yes",
        gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *activate_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
        GTK_TOOLBAR_CHILD_BUTTON,
        NULL, utf8,
        NULL, NULL,
        toolbar_icon_yes, NULL, NULL);
	*/
	
    GtkToolItem *activate_button = gtk_tool_button_new(NULL, utf8);
	gtk_tool_button_set_icon_name(activate_button, "gtk-yes");
	gtk_toolbar_insert(GTK_TOOLBAR(main_toolbar), activate_button, -1);
	g_signal_connect_swapped(G_OBJECT(activate_button), "clicked",
                         G_CALLBACK(activate_button_clicked), widgets);
        
        
    if( utf8 != NULL )
        g_free(utf8);


    /* Deactivate button */
    info = g_strdup_printf(_("Deactivate"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    
    /*
    GtkWidget *toolbar_icon_no = gtk_image_new_from_stock("gtk-no",
        gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *deactivate_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
        GTK_TOOLBAR_CHILD_BUTTON,
        NULL, utf8,
        NULL, NULL,
        toolbar_icon_no, NULL, NULL);
    g_signal_connect_swapped(G_OBJECT(deactivate_button),
            "clicked", G_CALLBACK(deactivate_button_clicked), widgets);
   */
   
    GtkToolItem *deactivate_button = gtk_tool_button_new(NULL, utf8);
	gtk_tool_button_set_icon_name(deactivate_button, "gtk-no");
	gtk_toolbar_insert(GTK_TOOLBAR(main_toolbar), deactivate_button, -1);
	g_signal_connect_swapped(G_OBJECT(deactivate_button), "clicked",
                         G_CALLBACK(deactivate_button_clicked), widgets);            
            
    if( utf8 != NULL )
        g_free(utf8);


    /* Shutdown button */
    info = g_strdup_printf(_("Shutdown"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    
    /*
    GtkWidget *toolbar_icon_stop = gtk_image_new_from_stock("gtk-stop",
        gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *shutdown_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
        GTK_TOOLBAR_CHILD_BUTTON,
        NULL, utf8,
        NULL, NULL,
        toolbar_icon_stop, NULL, NULL);
    g_signal_connect_swapped(G_OBJECT(shutdown_button),
            "clicked", G_CALLBACK(show_shutdown), widgets);
    */
    
    GtkToolItem *shutdown_button = gtk_tool_button_new(NULL, utf8);
	gtk_tool_button_set_icon_name(shutdown_button, "gtk-stop");
	gtk_toolbar_insert(GTK_TOOLBAR(main_toolbar), shutdown_button, -1);
	g_signal_connect_swapped(G_OBJECT(shutdown_button), "clicked",
                         G_CALLBACK(show_shutdown), widgets);    
    
            
            
    if( utf8 != NULL )
        g_free(utf8);


    /* Help button */
    info = g_strdup_printf(_("Help"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    
    /*
    GtkWidget *toolbar_icon_help = gtk_image_new_from_stock("gtk-help",
                    gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *help_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
        GTK_TOOLBAR_CHILD_BUTTON,
        NULL, utf8,
        NULL, NULL,
        toolbar_icon_help, NULL, NULL);
    g_signal_connect_swapped(G_OBJECT(help_button),
            "clicked", G_CALLBACK(show_help), widgets);
    */
    
    GtkToolItem *help_button = gtk_tool_button_new(NULL, utf8);
	gtk_tool_button_set_icon_name(help_button, "gtk-help");
	gtk_toolbar_insert(GTK_TOOLBAR(main_toolbar), help_button, -1);
	g_signal_connect_swapped(G_OBJECT(help_button), "clicked",
                         G_CALLBACK(show_help), widgets);    
    
    
    if( utf8 != NULL )
        g_free(utf8);


    /* About button */
    info = g_strdup_printf(_("About"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    
    /*
    GtkWidget *toolbar_icon_about = gtk_image_new_from_stock("gtk-about",
                        gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *about_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
        GTK_TOOLBAR_CHILD_BUTTON,
        NULL, utf8,
        NULL, NULL,
        toolbar_icon_about, NULL, NULL);
    g_signal_connect_swapped(G_OBJECT(about_button), "clicked", G_CALLBACK(show_credits), widgets);
    */
    
    GtkToolItem *about_button = gtk_tool_button_new(NULL, utf8);
	gtk_tool_button_set_icon_name(about_button, "gtk-about");
	gtk_toolbar_insert(GTK_TOOLBAR(main_toolbar), about_button, -1);
	g_signal_connect_swapped(G_OBJECT(about_button), "clicked",
                         G_CALLBACK(show_credits), widgets);
    
    if( utf8 != NULL )
        g_free(utf8);


    /* Quit button */
    info = g_strdup_printf(_("Quit"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    
    /*
    GtkWidget *toolbar_icon_quit = gtk_image_new_from_stock("gtk-quit",
        gtk_toolbar_get_icon_size(GTK_TOOLBAR(main_toolbar)));
    GtkWidget *quit_button = gtk_toolbar_append_element(GTK_TOOLBAR(main_toolbar),
        GTK_TOOLBAR_CHILD_BUTTON,
        NULL, utf8,
        NULL, NULL,
        toolbar_icon_quit, NULL, NULL);
    g_signal_connect_swapped(G_OBJECT(quit_button),
            "clicked", G_CALLBACK(gtk_main_quit), NULL);
    */
 
    GtkToolItem *quit_button = gtk_tool_button_new(NULL, utf8);
	gtk_tool_button_set_icon_name(quit_button, "gtk-quit");
	gtk_toolbar_insert(GTK_TOOLBAR(main_toolbar), quit_button, -1);
	g_signal_connect_swapped(G_OBJECT(quit_button), "clicked",
                         G_CALLBACK(gtk_main_quit), widgets);   
    
    
    if( utf8 != NULL )
        g_free(utf8);

    /* The notebook */
    GtkWidget *notebook_vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(notebook_hbox), notebook_vbox, TRUE, TRUE, 0);

    widgets->notebook_vbox1 = gtk_vbox_new(FALSE, 0);
    widgets->notebook_vbox2 = gtk_vbox_new(FALSE, 0);
    widgets->notebook_vbox3 = gtk_vbox_new(FALSE, 0);
    widgets->notebook_vbox4 = gtk_vbox_new(FALSE, 0);
    widgets->notebook_vbox5 = gtk_vbox_new(FALSE, 0);
    widgets->notebook_vbox6 = gtk_vbox_new(FALSE, 0);
    widgets->notebook_vbox7 = gtk_vbox_new(FALSE, 0);

    GtkWidget *main_notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(notebook_vbox), main_notebook, TRUE, TRUE, 0);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(main_notebook), TRUE);

    info = g_strdup_printf(_("Servers"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    GtkWidget *server_label = gtk_label_new(utf8);
    if( utf8 != NULL )
        g_free(utf8);

    info = g_strdup_printf(_("Users"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    GtkWidget *user_label = gtk_label_new(utf8);
    if( utf8 != NULL )
        g_free(utf8);

    info = g_strdup_printf(_("Transfers"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    GtkWidget *transfer_label = gtk_label_new(utf8);
    if( utf8 != NULL )
        g_free(utf8);


    info = g_strdup_printf(_("Disc"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    GtkWidget *disc_label = gtk_label_new(utf8);
    if( utf8 != NULL )
        g_free(utf8);


    info = g_strdup_printf(_("Files"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    GtkWidget *file_label = gtk_label_new(utf8);
    if( utf8 != NULL )
        g_free(utf8);


    info = g_strdup_printf(_("Security"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    GtkWidget *security_label = gtk_label_new(utf8);
    if( utf8 != NULL )
        g_free(utf8);


    info = g_strdup_printf(_("Configuration"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    GtkWidget *conf_label = gtk_label_new(utf8);
    if( utf8 != NULL )
        g_free(utf8);


    gtk_notebook_insert_page(GTK_NOTEBOOK(main_notebook), widgets->notebook_vbox1, server_label, 0);
    gtk_notebook_insert_page(GTK_NOTEBOOK(main_notebook), widgets->notebook_vbox2, user_label, 1);
    gtk_notebook_insert_page(GTK_NOTEBOOK(main_notebook), widgets->notebook_vbox3, transfer_label, 2);
    gtk_notebook_insert_page(GTK_NOTEBOOK(main_notebook), widgets->notebook_vbox4, disc_label, 3);
    gtk_notebook_insert_page(GTK_NOTEBOOK(main_notebook), widgets->notebook_vbox5, file_label, 4);
    gtk_notebook_insert_page(GTK_NOTEBOOK(main_notebook), widgets->notebook_vbox6, security_label, 5);
    gtk_notebook_insert_page(GTK_NOTEBOOK(main_notebook), widgets->notebook_vbox7, conf_label, 6);


    /* Set version and status labels */
    info = g_strdup_printf(_("Information: cant read version"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    widgets->version_label = gtk_label_new(utf8);
    gtk_box_pack_start(GTK_BOX(status_hbox), widgets->version_label, FALSE, FALSE, 0);
    gtk_misc_set_alignment(GTK_MISC(widgets->version_label), 0, 0);
    if( utf8 != NULL )
        g_free(utf8);


    GtkWidget *status_spacer_label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(status_hbox), status_spacer_label, TRUE, TRUE, 0);
    gtk_misc_set_alignment(GTK_MISC(status_spacer_label), 0, 0);

    info = g_strdup_printf(_("Status: unknown"));
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    g_free(info);
    widgets->status_label = gtk_label_new(_("Status: unknown"));
    gtk_box_pack_start(GTK_BOX(status_hbox), widgets->status_label, FALSE, FALSE, 0);
    gtk_misc_set_alignment(GTK_MISC(widgets->status_label), 0, 0);
    if( utf8 != NULL )
        g_free(utf8);

    GtkWidget *status_hseparator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(status_hsep_hbox), status_hseparator, TRUE, TRUE, 0);
    gtk_widget_set_size_request(status_hseparator, 10, 10);


    gtk_widget_show_all(widgets->main_window);
}
