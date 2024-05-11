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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "gettext.h"
#include "widgets.h"
#include "create_user_settings.h"
#include "populate_shell_combo.h"
#include "make_settings_buttons.h"
#include "make_settings_entries.h"
#include "make_settings_combos.h"
#include "make_settings_spinbuttons.h"
#include "make_settings_checkbuttons.h"
#include "delete_user.h"
#include "show_info.h"
#include "apply_user.h"
#include "functions.h"


extern int use_ratio;
extern int use_quota;

int user_profiling_info_shown = 0;


/* Clears the user settings, sets default values and shows some info. */
void new_user(struct w *widgets)
{
    gchar *info;

    /* Clear the directory treeview */
    gtk_list_store_clear(GTK_LIST_STORE(widgets->directory_store));

    /* User */
    gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[0]), "user1");

    /* Password */
    gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[1]), "");

    /* Group */
    gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[2]), "group1");

    /* Comment */
    gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[3]), "User1");

    /* Shell */
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->user_set_combo[0]), 0);

    /* Require password */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[1]), TRUE);

    /* Show user in statistics */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[2]), TRUE);

    /* Max logins, set to 10 */
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[0]), 10);

    /* Allow logins from */
    gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[5]), "All");

    /* Ratio defaults to disabled */
    if( use_ratio )
    {
        /* Byte ratio */
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[1]), 0);

        /* Byte ratio credit */
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[2]), 0);

        /* File ratio */
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[3]), 0);

        /* File ratio credit */
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[4]), 0);
    }

    /* Fixme, Quota */
    if( use_quota )
    {


    }

    /* Only show profiling info once */
    if( user_profiling_info_shown )
        return;
    else
        user_profiling_info_shown = 1;

    info = g_strconcat(_("Write the new users name and other settings in the user profile below.\n"),
        _("Press apply to add this new user with the selected settings.\n\n"),
        _("You can also skip this step and directly alter an existing users name and\n"),
        _("settings and then press apply. This can save administration time.\n"),
        NULL);
    show_info(info);
    if( info != NULL )
        g_free(info);
}


void create_user_settings(struct w *widgets)
{
    //GtkTooltips *tooltips;
    GtkWidget *hbuttonbox;
    GtkWidget *delete_user_button;
    GtkWidget *new_user_button;
    GtkWidget *apply_user_button;

    int a = 0;
    int b = 1;
    int ent = 0;
    int comb = 0;
    int check = 0;
    int spin = 0;

    /* For the custom new user button */
    GtkWidget *new_user_hbox;
    GtkWidget *new_user_image;
    GtkWidget *new_user_label;
    GtkWidget *new_user_alignment;

    //tooltips = gtk_tooltips_new();

    hbuttonbox = gtk_hbutton_box_new();


    /* The delete, new user and apply user buttons */
    delete_user_button = gtk_button_new_from_stock(GTK_STOCK_DELETE);
    /* Custom new user button begin */
    new_user_button = gtk_button_new();
    new_user_alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(new_user_button), new_user_alignment);
    new_user_hbox = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(new_user_alignment), new_user_hbox);
    new_user_image = gtk_image_new_from_stock("gtk-add", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(new_user_hbox), new_user_image, FALSE, FALSE, 0);
    new_user_label = gtk_label_new_with_mnemonic(_("New user"));
    gtk_box_pack_start(GTK_BOX(new_user_hbox), new_user_label, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(new_user_label), GTK_JUSTIFY_LEFT);
    /* Custom new user button end */

    apply_user_button = gtk_button_new_from_stock(GTK_STOCK_APPLY);

    /* Add the buttons to the user_button_box created in create_user_tab.c */
    gtk_box_pack_start(GTK_BOX(widgets->user_button_box), delete_user_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->user_button_box), new_user_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->user_button_box), apply_user_button, FALSE, FALSE, 0);

    g_signal_connect_swapped(G_OBJECT(delete_user_button), "clicked", G_CALLBACK(delete_user), widgets);

    g_signal_connect_swapped(G_OBJECT(new_user_button), "clicked", G_CALLBACK(new_user), widgets);

    g_signal_connect_swapped(G_OBJECT(apply_user_button), "clicked", G_CALLBACK(apply_user), widgets);

    gtk_table_attach(GTK_TABLE(widgets->usr_set_table), hbuttonbox, 0, 3, a, b, 0, 0, 5, 5);
    a++;
    b++;


    /* Username. Max length and input 350 chars */
    GtkWidget *username_button;
    widgets->user_set_entry[ent] = gtk_entry_new();
    username_button = make_button_with_entry(GTK_TABLE(widgets->usr_set_table),
                   widgets->user_set_entry[ent], "gtk-refresh", _("Username:"),
                             _("Create a randomized username"), 0, 1, a, b, 350);
    a++;
    b++;
    ent++;

    g_signal_connect_swapped(G_OBJECT(username_button), "clicked", G_CALLBACK(randomize_username), widgets);

    /* Password  */
    GtkWidget *password_button;
    widgets->user_set_entry[ent] = gtk_entry_new();
    password_button = make_button_with_entry(GTK_TABLE(widgets->usr_set_table),
                   widgets->user_set_entry[ent], "gtk-refresh", _("Password:"),
                             _("Create a randomized password"), 0, 1, a, b, 350);
    a++;
    b++;
    ent++;

    g_signal_connect_swapped(G_OBJECT(password_button),
            "clicked", G_CALLBACK(randomize_password), widgets);


    /* Group */
    widgets->user_set_entry[ent] = make_entry_with_label(GTK_TABLE(widgets->usr_set_table),
                _("Group:"), 0, 1, a, b, 350);
    gtk_widget_set_tooltip_text(widgets->user_set_entry[ent],
                _("The group the user belongs to"));
    a++;
    b++;
    ent++;


    /* Comment */
    widgets->user_set_entry[ent] = make_entry_with_label(GTK_TABLE(widgets->usr_set_table),
                _("Comment:"), 0, 1, a, b, 350);
    gtk_widget_set_tooltip_text(widgets->user_set_entry[ent],
                _("A comment about the user"));
    a++;
    b++;
    ent++;


    a++;
    b++;
    ent++; // Let this be for now

    /* Shell combo */
    widgets->user_set_combo[comb] = make_label_textcombo_label(GTK_TABLE(widgets->usr_set_table),
                _("Shell:"), 0, 1, a, b, 80);
                
    populate_shell_combo(GTK_COMBO_BOX(widgets->user_set_combo[comb]));
    
    a++;
    b++;
    comb++;

    /* Account locked checkbutton */
    widgets->user_set_checkbutton[check] = make_checkbutton_with_label(GTK_TABLE(widgets->usr_set_table),
                _("Account locked:"), 0, 1, a, b);
    a++;
    b++;
    check++;

    /* Require password */
    widgets->user_set_checkbutton[check] = make_checkbutton_with_label(GTK_TABLE(widgets->usr_set_table),
                _("Require password:"), 0, 1, a, b);
    a++;
    b++;
    check++;

    /* Show user in statistics */
    widgets->user_set_checkbutton[check] = make_checkbutton_with_label(GTK_TABLE(widgets->usr_set_table),
                _("Show user in statistics:"), 0, 1, a, b);
    a++;
    b++;
    check++;

    /* Max logins */
    widgets->user_set_spinbutton[spin] = make_shortleft_spinbutton_with_label(GTK_TABLE(widgets->usr_set_table),
                _("Maximum logins:"), 0, 1, a, b, 80);
    gtk_widget_set_tooltip_text(widgets->user_set_spinbutton[spin],
                _("Maximum number of simultaneous logins this user can make"));
    a++;
    b++;
    spin++;

    /* Allow logins from */
    widgets->user_set_entry[ent] = make_entry_with_label(GTK_TABLE(widgets->usr_set_table),
                _("Allow logins from:"), 0, 1, a, b, 350);
    gtk_widget_set_tooltip_text(widgets->user_set_entry[ent],
                _("All, None, IP, DNS name or CIDR-Address"));
    a++;
    b++;
    ent++;


    /* Ratio settings */
    if( use_ratio )
    {
        /* Byte ratio */
        widgets->user_set_spinbutton[spin] = make_shortleft_spinbutton_with_label(GTK_TABLE(widgets->usr_set_table),
                _("Byte ratio:"), 0, 1, a, b, 80);
        gtk_widget_set_tooltip_text(widgets->user_set_spinbutton[spin],
                _("If set to 3 then the ratio is 3:1. 0 means unlimited."));
        a++;
        b++;
        spin++;

        /* Byte ratio credit */
        widgets->user_set_spinbutton[spin] = make_shortleft_spinbutton_with_label(GTK_TABLE(widgets->usr_set_table),
            _("Byte ratio credit:"), 0, 1, a, b, 80);
        gtk_widget_set_tooltip_text(widgets->user_set_spinbutton[spin],
            _("If set to 6 then the the user can download 6 times the byte ratio before uploading. 0 means no credit."));
        a++;
        b++;
        spin++;

        /* File ratio */
        widgets->user_set_spinbutton[spin] = make_shortleft_spinbutton_with_label(GTK_TABLE(widgets->usr_set_table),
            _("File ratio:"), 0, 1, a, b, 80);
        gtk_widget_set_tooltip_text(widgets->user_set_spinbutton[spin],
            _("If set to 9 then 9 files can be downloaded for every 1 file uploaded. 0 means unlimited."));
        a++;
        b++;
        spin++;

        /* File ratio credit */
        widgets->user_set_spinbutton[spin] = make_shortleft_spinbutton_with_label(GTK_TABLE(widgets->usr_set_table),
            _("File ratio credit:"), 0, 1, a, b, 80);
        gtk_widget_set_tooltip_text(widgets->user_set_spinbutton[spin],
            _("If set to 12 then then the user can download 12 times the file ratio before uploading. 0 means no credit."));
        a++;
        b++;
        spin++;
    }

    /* Fixme, Quota */
    if( use_quota )
    {


    }

    gtk_widget_show_all(widgets->main_window);
}
