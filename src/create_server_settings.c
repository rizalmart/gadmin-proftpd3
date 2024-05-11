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
#include "gtk/gtk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gettext.h"
#include "widgets.h"
#include "allocate.h"
#include "show_info.h"
#include "make_settings_entries.h"
#include "make_settings_combos.h"
#include "make_settings_spinbuttons.h"
#include "make_settings_progressbars.h"
#include "make_settings_checkbuttons.h"
#include "make_settings_labels.h"
#include "make_settings_hseparators.h"
#include "generate_cert.h"
#include "create_server_settings.h"
#include "populate_conf_tab.h"
#include "functions.h"

#include "user_auth_directives.h"
#include "mysql_functions.h"


extern int use_tls;
extern int use_ratio;

gulong auth_changed_signal;


void create_server_settings(struct w *widgets)
{
    //GtkTooltips *tooltips;
    GtkWidget *gen_cert_button;
    gchar *utf8 = NULL;
    gdouble progress_val = 1.0;
    gchar *combo_text;

    /* Counters for the widgets and positions in the table */
    int a = 0;
    int b = 1;
    int ent = 0;
    int spin = 0;
    int comb = 0;

    /* Max lengths and input */
    int entry_size = 100;
    int combo_size = 100;

    //tooltips = gtk_tooltips_new();


    /* The server address */
    widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Server address:"), 0, 1, a, b, entry_size);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
    //    _("ftp.mydomain.org, 192.168.0.100 or 0.0.0.0 which means listen to all network interfaces"), NULL);
    gtk_widget_set_tooltip_text(widgets->server_set_entry[ent], _("ftp.mydomain.org, 192.168.0.100 or 0.0.0.0 which means listen to all network interfaces"));       
    a++;
    b++;
    ent++;

    /* Server name / Alternative server ident Label+Entry+Combo */
    widgets->server_set_entry[ent] = make_entry_with_label(GTK_TABLE(widgets->srv_set_table),
        _("Server name:"), 0, 1, a, b, entry_size);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
    //    _("My FTP server or set it to off to have the server use its own name and version"), NULL);

    gtk_widget_set_tooltip_text(widgets->server_set_entry[ent], _("My FTP server or set it to off to have the server use its own name and version"));       

    widgets->server_set_combo[comb] = make_combo(GTK_TABLE(widgets->srv_set_table), 1, 2, a, b, combo_size);
    combo_text = g_strdup_printf(_("On"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Off"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    ent++;
    comb++;

    /* Port */
    widgets->server_set_spinbutton[spin] = make_short_spinbutton_with_label(GTK_TABLE(widgets->srv_set_table),
        _("Server port:"), 0, 1, a, b, 50);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_spinbutton[spin],
    //    _("A port the server listens on. 21 is the standard port. 0 means that the server is deactivated"), NULL);

    gtk_widget_set_tooltip_text(widgets->server_set_spinbutton[spin], _("A port the server listens on. 21 is the standard port. 0 means that the server is deactivated"));       
        
    a++;
    b++;
    spin++;

    /* Nat router Label+Entry+Combo */
    widgets->server_set_entry[ent] = make_entry_with_label(GTK_TABLE(widgets->srv_set_table),
            _("NAT router:"), 0, 1, a, b, entry_size);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
    //        _("The external IP address or DNS name of the NAT router."), NULL);
	
	gtk_widget_set_tooltip_text(widgets->server_set_entry[ent], _("The external IP address or DNS name of the NAT router."));       

    widgets->server_set_combo[comb] = make_combo(GTK_TABLE(widgets->srv_set_table), 1, 2, a, b, combo_size);
    combo_text = g_strdup_printf(_("On"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Off"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    ent++;
    comb++;

    /* Email */
    widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Admin email:"), 0, 1, a, b, 200);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
    //        _("The administrators email address"), NULL);
    gtk_widget_set_tooltip_text(widgets->server_set_entry[ent], _("The administrators email address"));       
        
    a++;
    b++;
    ent++;

    /* Identity lookups */
    widgets->server_set_combo[comb] = make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Identity lookups:"), 0, 1, a, b, 50);
    combo_text = g_strdup_printf(_("On"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Off"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    comb++;

    /* Reverse lookups */
    widgets->server_set_combo[comb] = make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Reverse lookups:"), 0, 1, a, b, 50);
    combo_text = g_strdup_printf(_("On"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Off"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    comb++;

    /* Time standard */
    widgets->server_set_combo[comb] = make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Time standard:"), 0, 1, a, b, 50);
            
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_combo[comb],
    //        _("GMT or local time"), NULL);

    gtk_widget_set_tooltip_text(widgets->server_set_combo[comb],_("GMT or local time"));       
                  
    combo_text = g_strdup_printf(_("GMT"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Local time"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    comb++;

    /* Passive ports */
    widgets->server_set_spinbutton[spin] = make_padded_spinbutton_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Passive port range:"), 0, 1, a, b, 50);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_spinbutton[spin],
    //        _("From this port"), NULL);
    
    gtk_widget_set_tooltip_text(widgets->server_set_spinbutton[spin],_("From this port"));        
            
    spin++;
    widgets->server_set_spinbutton[spin] = make_spinbutton(GTK_TABLE(widgets->srv_set_table), 0, 1, a, b, 50);
    
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_spinbutton[spin], _("To this port"), NULL);
    gtk_widget_set_tooltip_text(widgets->server_set_spinbutton[spin], _("To this port"));        

    a++;
    b++;
    spin++;

    /* Max connections */
    widgets->server_set_spinbutton[spin] = make_short_spinbutton_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Max connections:"), 0, 1, a, b, 50);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_spinbutton[spin],
    //        _("The maximum simoultaneous connections for this server"), NULL);
    gtk_widget_set_tooltip_text(widgets->server_set_spinbutton[spin],
            _("The maximum simoultaneous connections for this server"));         
    a++;
    b++;
    spin++;

    /* Maximum logins */
    widgets->server_set_spinbutton[spin] =
        make_short_spinbutton_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Max login attempts:"), 0, 1, a, b, 50);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_spinbutton[spin],
    //        _("The maximum number of logins attempts a user can fail before disconnected"), NULL);
    gtk_widget_set_tooltip_text(widgets->server_set_spinbutton[spin],
            _("The maximum number of logins attempts a user can fail before disconnected"));             
    a++;
    b++;
    spin++;

    /* Login timeout */
    widgets->server_set_spinbutton[spin] = make_short_spinbutton_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Login timeout:"), 0, 1, a, b, 50);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_spinbutton[spin],
    //        _("The maximum time a user can spend authenticating"), NULL);
    gtk_widget_set_tooltip_text(widgets->server_set_spinbutton[spin],
            _("The maximum time a user can spend authenticating"));              
    a++;
    b++;
    spin++;

    /* Idle timeout */
    widgets->server_set_spinbutton[spin] = make_short_spinbutton_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Idle timeout:"), 0, 1, a, b, 50);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_spinbutton[spin],
    //        _("The time it takes before an idle user is thrown out in seconds"), NULL);
    gtk_widget_set_tooltip_text(widgets->server_set_spinbutton[spin],
            _("The time it takes before an idle user is thrown out in seconds"));             
    a++;
    b++;
    spin++;

    /* Transfer mode */
    widgets->server_set_combo[comb] = make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Transfer mode:"), 0, 1, a, b, 50);
    combo_text = g_strdup_printf(_("Binary"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Ascii"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    comb++;

    /* FXP transfers */
    widgets->server_set_combo[comb] = make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
            _("FXP transfers:"), 0, 1, a, b, 50);
    combo_text = g_strdup_printf(_("On"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Off"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    comb++;



// The following should be added to a settings window....

    /* Randomized username length */
    widgets->server_set_spinbutton[spin] = make_short_spinbutton_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Username length:"), 0, 1, a, b, 50);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_spinbutton[spin],
    //        _("The length of the randomized usernames"), NULL);

    gtk_widget_set_tooltip_text(widgets->server_set_spinbutton[spin],
            _("The length of the randomized usernames"));              
            
    a++;
    b++;
    spin++;

    /* Randomized password length */
    widgets->server_set_spinbutton[spin] = make_short_spinbutton_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Password length:"), 0, 1, a, b, 50);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_spinbutton[spin],
    //        _("The length of the randomized passwords"), NULL);
 
    gtk_widget_set_tooltip_text(widgets->server_set_spinbutton[spin],
            _("The length of the randomized passwords"));            
            
    a++;
    b++;
    spin++;

    /* Randomize case */
    widgets->server_set_combo[comb] = make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Randomized letters:"), 0, 1, a, b, 50);
    combo_text = g_strdup_printf(_("Upper"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Lower"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    comb++;

    /* Leave these for now */
    a++;
    b++;
    ent++;

    a++;
    b++;
    ent++;


    /* Authentication type */
    widgets->server_set_combo[comb] = make_combo_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Authentication type:"), 0, 1, a, b, 50);
    combo_text = g_strdup_printf(_("System users"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Virtual users"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("MySQL database"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
/*
    combo_text = g_strdup_printf(_("LDAP database"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
*/

    /* Auth combo changed signal */
    auth_changed_signal = g_signal_connect(GTK_COMBO_BOX(widgets->server_set_combo[comb]),
                "changed", G_CALLBACK(user_auth_changed), widgets);


    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    comb++;


    /* Statistics file */
    widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
            _("HTML statistics:"), 0, 1, a, b, 200);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
    //       _("Generated HTML statistics file, IE: /var/www/ftp.htm"), NULL);

    gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
            _("Generated HTML statistics file, IE: /var/www/ftp.htm"));                
            
    a++;
    b++;
    ent++;


    ent++;  // Let this be for now.


    /* The systems security logfile */
    widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Security log:"), 0, 1, a, b, 200);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
    //        _("IE: /var/log/secure"), NULL);

    gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
            _("IE: /var/log/secure"));               
            
    a++;
    b++;
    ent++;

    /* The server runs as this user */
    widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Server user:"), 0, 1, a, b, 200);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
    //        _("The server drops its privileges and runs as this user, nobody is often used"), NULL);

    gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
            _("The server drops its privileges and runs as this user, nobody is often used"));              
            
    a++;
    b++;
    ent++;

    /* The server runs as this group */
    widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Server group:"), 0, 1, a, b, 200);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
    //        _("The server drops its privileges and runs as this group, nobody is often used"), NULL);

    gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
            _("The server drops its privileges and runs as this group, nobody is often used"));  
            
    a++;
    b++;
    ent++;

    /* This fake user owns all files Label+Entry+Combo */
    widgets->server_set_entry[ent] = make_entry_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Fake file user:"), 0, 1, a, b, entry_size);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
    //    _("It will look like this fake user owns all files.\nIt does not have to be an existing user"), NULL);
    gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
        _("It will look like this fake user owns all files.\nIt does not have to be an existing user"));  


    widgets->server_set_combo[comb] = make_combo(GTK_TABLE(widgets->srv_set_table), 1, 2, a, b, combo_size);
    combo_text = g_strdup_printf(_("On"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Off"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    ent++;
    comb++;

    /* This fake group owns all files Label+Entry+Combo */
    widgets->server_set_entry[ent] = make_entry_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Fake file group:"), 0, 1, a, b, entry_size);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
    //    _("It will look like this fake group owns all files.\nIt does not have to be an existing group"), NULL);
    gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
		_("It will look like this fake group owns all files.\nIt does not have to be an existing group"));  

    widgets->server_set_combo[comb] = make_combo(GTK_TABLE(widgets->srv_set_table), 1, 2, a, b, combo_size);
    combo_text = g_strdup_printf(_("On"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Off"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    ent++;
    comb++;

    /* Upload speed */
    widgets->server_set_spinbutton[spin] = make_short_spinbutton_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Upload speed:"), 0, 1, a, b, 50);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_spinbutton[spin],
    //        _("Total upload speed shared by all clients in KiloBytes/sec"), NULL);
    gtk_widget_set_tooltip_text(widgets->server_set_spinbutton[spin],
            _("Total upload speed shared by all clients in KiloBytes/sec"));             
    a++;
    b++;
    spin++;

    /* Download speed */
    widgets->server_set_spinbutton[spin] = make_short_spinbutton_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Download speed:"), 0, 1, a, b, 50);
    //gtk_tooltips_set_tip(tooltips, widgets->server_set_spinbutton[spin],
    //        _("Total download speed shared by all clients in KiloBytes/sec"), NULL);
    gtk_widget_set_tooltip_text(widgets->server_set_spinbutton[spin],
            _("Total download speed shared by all clients in KiloBytes/sec"));              
    a++;
    b++;
    spin++;

    /* Resume broken uploads */
    widgets->server_set_combo[comb] = make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Resume uploads:"), 0, 1, a, b, 50);
    combo_text = g_strdup_printf(_("On"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Off"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    comb++;

    /* Resume broken downloads */
    widgets->server_set_combo[comb] = make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Resume downloads:"), 0, 1, a, b, 50);
    combo_text = g_strdup_printf(_("On"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Off"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    comb++;

    /* Remove aborted uploads */
    widgets->server_set_combo[comb] = make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
            _("Remove aborted uploads:"), 0, 1, a, b, 50);
    combo_text = g_strdup_printf(_("On"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    combo_text = g_strdup_printf(_("Off"));
    utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
    g_free(combo_text);
    g_free(utf8);
    a++;
    b++;
    comb++;


    /* TLS Module settings begins here */
    if( use_tls )
    {
        /* A hseparator */
        make_3columns_hseparator(GTK_TABLE(widgets->srv_set_table), 0, 1, a, b);
        a++;
        b++;

        /* Secure communications label */
        make_3columns_label(GTK_TABLE(widgets->srv_set_table),
                       _("Secure communications:"), 0, 1, a, b);
        a++;
        b++;

        /* Use secure communication */
        widgets->server_set_combo[comb] =
            make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
                            _("Use secure communication:"), 0, 1, a, b, 50);
        combo_text = g_strdup_printf(_("On"));
        utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
        combo_text = g_strdup_printf(_("Off"));
        utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
        g_free(combo_text);
        g_free(utf8);
        a++;
        b++;
        comb++;

        /* Require encryption on channels */
        widgets->server_set_combo[comb] = make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
                _("Require for channels:"), 0, 1, a, b, 50);
        combo_text = g_strdup_printf(_("Both"));
        utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
        combo_text = g_strdup_printf(_("Off"));
        utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
        combo_text = g_strdup_printf(_("Data"));
        utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
        combo_text = g_strdup_printf(_("Control"));
        utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
        g_free(combo_text);
        g_free(utf8);
        a++;
        b++;
        comb++;

        /* Verify clients */
        widgets->server_set_combo[comb] = make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
                _("Verify clients:"), 0, 1, a, b, 50);
        combo_text = g_strdup_printf(_("On"));
        utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
        combo_text = g_strdup_printf(_("Off"));
        utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
        g_free(combo_text);
        g_free(utf8);
        a++;
        b++;
        comb++;

        /* Use these protocols */
        widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
                _("Use these protocols:"), 0, 1, a, b, 200);
        //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
        //        _("IE: SSLv23 or TLSv1"), NULL);
		gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],_("IE: SSLv23 or TLSv1"));                  
        a++;
        b++;
        ent++;

        /* The TLS log */
        widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table), 
                _("TLS log:"), 0, 1, a, b, 200);
        //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
        //        _("IE: /var/log/gadmin-proftpd_tls.log"), NULL);

		gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
                  _("IE: /var/log/gadmin-proftpd_tls.log"));  
                
        a++;
        b++;
        ent++;

        /* A hseparator */
        make_3columns_hseparator(GTK_TABLE(widgets->srv_set_table), 0, 1, a, b);
        a++;
        b++;

        /* The signed certificate settings */
        make_3columns_label(GTK_TABLE(widgets->srv_set_table),
                _(" Signed certificate settings: "), 0, 1, a, b);
        a++;
        b++;

        /* The certificate directory */
        widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
                _("Certificate directory:"), 0, 1, a, b, 200);
        //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
        //      _("IE: /etc/gadmin-proftpd/certs"), NULL);

		gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
                _("IE: /etc/gadmin-proftpd/certs"));                  
                
        a++;
        b++;
        ent++;


        /* This servers DNS name or IP-Address */
        widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
                _("Server address:"), 0, 1, a, b, 200);
        //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
        //        _("This servers DNS name or IP address"), NULL);

		gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
                _("This servers DNS name or IP address"));                 
                
        a++;
        b++;
        ent++;

        /* Email address */
        widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
                _("Email address:"), 0, 1, a, b, 200);
        //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
        //        _("The administrators email address"), NULL);


		gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
                  _("The administrators email address"));                  
                
        a++;
        b++;
        ent++;

        /* Country */
        widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
                _("Country:"), 0, 1, a, b, 200);
        //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
        //        _("A country"), NULL);

		gtk_widget_set_tooltip_text( widgets->server_set_entry[ent],
                _("A country"));                  
                
                
        a++;
        b++;
        ent++;

        /* City or town */
        widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
                _("City or town:"), 0, 1, a, b, 200);
        //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
        //        _("A city or a town"), NULL);

		gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
                _("A city or a town"));                  
                
        a++;
        b++;
        ent++;

        /* Organization */
        widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
                _("Organization:"), 0, 1, a, b, 200);
        //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
        //        _("Some organization"), NULL);

		gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
                _("Some organization"));                 
                
        a++;
        b++;
        ent++;

        /* Organizational unit */
        widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
                _("Organizational unit:"), 0, 1, a, b, 200);
        
        //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
        //        _("Accounting"), NULL);
                
		gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
                _("Accounting"));                 
                
        a++;
        b++;
        ent++;

        /* Password */
        widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
                _("Password:"), 0, 1, a, b, 200);
        //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
        //        _("A password"), NULL);

		gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
                _("A password"));         
        
        a++;
        b++;
        ent++;

        /* Was challenge password, let it be for now */
        a++;
        b++;
        ent++;

        /* Encryption bits (Min value = 384) */
        widgets->server_set_spinbutton[spin] =
            make_short_spinbutton_with_label(GTK_TABLE(widgets->srv_set_table),
                    _("Encryption bits:"), 0, 1, a, b, 50);
        //gtk_tooltips_set_tip(tooltips, widgets->server_set_spinbutton[spin],
        //        _("Number of bits in the generated certificate"), NULL);
                

		gtk_widget_set_tooltip_text(widgets->server_set_spinbutton[spin],
                  _("Number of bits in the generated certificate"));                   
                
        gtk_spin_button_set_range(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[spin]), 384, 999999);
        a++;
        b++;
        spin++;

        /* Number of days the certificate is valid */
        widgets->server_set_spinbutton[spin] = make_short_spinbutton_with_label(GTK_TABLE(widgets->srv_set_table),
                _("Days valid:"), 0, 1, a, b, 50);
        
        //gtk_tooltips_set_tip(tooltips, widgets->server_set_spinbutton[spin],
        //        _("Number of days the certificate is valid"), NULL);
 
 		gtk_widget_set_tooltip_text(widgets->server_set_spinbutton[spin],
                _("Number of days the certificate is valid")) ;                 
                
        a++;
        b++;
        spin++;

        /* Countrycode */
        widgets->server_set_entry[ent] = make_long_entry_with_label(GTK_TABLE(widgets->srv_set_table),
                _("Country code:"), 0, 1, a, b, 200);
        //gtk_tooltips_set_tip(tooltips, widgets->server_set_entry[ent],
        //       _("The country code, IE: SV, DK, EN, US, etc"), NULL);

		gtk_widget_set_tooltip_text(widgets->server_set_entry[ent],
                _("The country code, IE: SV, DK, EN, US, etc"));                  
                
        a++;
        b++;
        ent++;

        /* A progress bar */
        widgets->gen_cert_progressbar =
            make_progressbar_with_label(GTK_TABLE(widgets->srv_set_table),
                    _("Generate new certificates:"), 0, 1, a, b, 210);
        gtk_widget_set_sensitive(widgets->gen_cert_progressbar, FALSE);

        /* The progress bar looks nicer if its blue from start */
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->gen_cert_progressbar), progress_val);

        /* An apply button for the progress bar (make_button is in make_settings_progressbars.c) */
        gen_cert_button = make_button(GTK_TABLE(widgets->srv_set_table), 0, 1, a, b, 200);
        a++;
        b++;

        g_signal_connect_swapped(G_OBJECT(gen_cert_button),
                "clicked", G_CALLBACK(generate_cert), widgets);
    }




    /* RATIO Module settings begins here */
    if( use_ratio )
    {
        /* A hseparator */
        make_3columns_hseparator(GTK_TABLE(widgets->srv_set_table), 0, 1, a, b);
        a++;
        b++;

        /* Ratio label */
        make_3columns_label(GTK_TABLE(widgets->srv_set_table),
                _("Ratios for uploads and downloads:"), 0, 1, a, b);
        a++;
        b++;

        /* Use Ratios */
        widgets->server_set_combo[comb] = make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
                    _("Use ratios:"), 0, 1, a, b, 50);
        combo_text = g_strdup_printf(_("On"));
        utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
        combo_text = g_strdup_printf(_("Off"));
        utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
        g_free(combo_text);
        g_free(utf8);
        a++;
        b++;
        comb++;

        /* Save Ratios */
        widgets->server_set_combo[comb] = make_short_combo_with_label(GTK_TABLE(widgets->srv_set_table),
                _("Save ratios:"), 0, 1, a, b, 50);
        combo_text = g_strdup_printf(_("On"));
        utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
        combo_text = g_strdup_printf(_("Off"));
        utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX(widgets->server_set_combo[comb]), utf8);
        g_free(combo_text);
        g_free(utf8);
        a++;
        b++;
        comb++;
    }

    gtk_widget_show_all(widgets->main_window);
}
