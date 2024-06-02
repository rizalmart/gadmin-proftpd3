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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "widgets.h"
#include "gettext.h"
#include "reread_conf.h"
#include "show_info.h"
#include "apply_server_settings.h"
#include "allocate.h"
#include "populate_servers.h"
#include "populate_server_settings.h"
#include "select_first_server.h"
#include "populate_conf_tab.h"


extern char global_server_address[1024];
extern char global_server_port[1024];
extern char global_server_type[1024];

extern int activated;

extern int use_tls;
extern int use_ratio;
extern int use_quota;



void apply_server_settings(struct w *widgets)
{
    /* Change the selected servers configuration. */
    FILE *fp;
    long conf_size;
    int address_match = 0, port_match = 0;
    int configuration_changed = 0;
    gint active_index;
    gchar *address_line, *port_line, *info;
    gchar *auth_type, *settings, *settings_conf;

    char *change_srv_ident;
    char *old_buffer, *config, *temp_server, *conf_line, *address_buffer, *port_buffer;
    gchar *old_server_ident;

    G_CONST_RETURN gchar *server_name;
    G_CONST_RETURN gchar *server_port;
    G_CONST_RETURN gchar *admin_email;
    G_CONST_RETURN gchar *passive_port_1;
    G_CONST_RETURN gchar *passive_port_2;
    G_CONST_RETURN gchar *masquerade_address;
    G_CONST_RETURN gchar *max_connect;
    G_CONST_RETURN gchar *idle_timeout; /* TimeoutIdle and TimeoutNoTransfer */
    G_CONST_RETURN gchar *server_ident;
    G_CONST_RETURN gchar *server_user;
    G_CONST_RETURN gchar *server_group;
    G_CONST_RETURN gchar *fake_username;
    G_CONST_RETURN gchar *fake_groupname;
    G_CONST_RETURN gchar *max_login_attempts;
    G_CONST_RETURN gchar *login_timeout;
    G_CONST_RETURN gchar *upload_bandwidth;
    G_CONST_RETURN gchar *download_bandwidth;
    G_CONST_RETURN gchar *systemlog;
    G_CONST_RETURN gchar *username_length;
    G_CONST_RETURN gchar *password_length;
    G_CONST_RETURN gchar *html_path;

    /* TLS Entries */
    G_CONST_RETURN gchar *tls_protocols = NULL;
    G_CONST_RETURN gchar *tls_logfile = NULL;
    G_CONST_RETURN gchar *tls_certdir = NULL;

    /* Get settings from the entries */
    server_name = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[0]));
    server_ident = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[1]));
    masquerade_address = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[2]));
    admin_email = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[3]));

    html_path = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[6]));

    systemlog = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[8]));
    server_user = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[9]));
    server_group = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[10]));
    fake_username = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[11]));
    fake_groupname = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[12]));

    /* Get values from the spinbuttons */
    server_port = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[0]));
    passive_port_1 = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[1]));
    passive_port_2 = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[2]));

    max_connect = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[3]));
    max_login_attempts = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[4]));
    login_timeout = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[5]));
    idle_timeout = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[6]));

    username_length = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[7]));
    password_length = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[8]));
    upload_bandwidth = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[9]));
    download_bandwidth = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[10]));

    /* Save the old server ident so we know if we should repopulate the server treeview */
    old_server_ident = g_strdup_printf("%s", server_ident);


    if( use_tls )
    {
        /* Get values from the TLS entries */
        tls_protocols = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[13]));
        tls_logfile = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[14]));
        tls_certdir = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[15]));
    }


    /* First save the settings */
    settings_conf = g_strdup_printf("%s/settings.conf", GP_APPCONFDIR);
    if((fp = fopen(settings_conf, "w+")) == NULL)
    {
        g_free(settings_conf);
        // Show popup
        return;
    }

    /* Get Authentication type */
    active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[8]));
    if( active_index == 0 )
        auth_type = g_strdup_printf("system_users");
    else
    if( active_index == 1 )
        auth_type = g_strdup_printf("virtual_users");
    else
    if( active_index == 2 )
        auth_type = g_strdup_printf("mysql_users");
    else /* Remove in version 0.5.2 or later. */
        auth_type = g_strdup_printf("system_users");


    settings = g_strconcat("\n",
        "certificate_directory ", tls_certdir, "\n",
        "random_username_length ", username_length, "\n",
        "random_password_length ", password_length, "\n",
        "randomize_case lower\n",
        "useradd_homedir_path /var/ftp\n",
        "html_path ", html_path, "\n",
        "welcome_name welcome.msg\n",
        "auth_type ", auth_type, "\n\n",
        NULL);
    fputs(settings, fp);
    fclose(fp);

    g_free(settings);
    g_free(settings_conf);



    address_buffer = allocate(8192 + 15);
    port_buffer = allocate(8192 + 3);

    if( strstr((char *)global_server_type, "Virtualhost") )
        sprintf(address_buffer, "<VirtualHost %s>\n", global_server_address);

    sprintf(port_buffer, "Port %s\n", global_server_port);


// use global_server_address
    if( strlen(address_buffer) == 0 && strstr((char *)global_server_type, "Virtualhost") )
    {
        info = g_strdup_printf(_("Refusing to change a server without a specified name.\n"));
        show_info(info);
        g_free(info);
        free(address_buffer);
        free(port_buffer);
        g_free(old_server_ident);
        return;
    }

    // use < 8 or global_server_port
    if( strlen(port_buffer) == 0 )
    {
        info = g_strdup_printf(_("Refusing to change a server without a specified port.\n"));
        show_info(info);
        g_free(info);
        free(address_buffer);
        free(port_buffer);
        g_free(old_server_ident);
        return;
    }


    /* Change the configuration for the selected server */
    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        info = g_strdup_printf(_("Cant open proftpd.conf here: \n%s\n"), PROFTPD_CONF);
        show_info(info);
        g_free(info);
        free(address_buffer);
        free(port_buffer);
        g_free(old_server_ident);
        return;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);


    config = allocate(conf_size + 16384);
    old_buffer = allocate(conf_size);
    temp_server = allocate(conf_size);
    conf_line = allocate(8192);
    change_srv_ident = allocate(8192);



/* ------------- Change the standard server ----------- */

    if( strstr("Default server", (char *)global_server_type) )
    {
        configuration_changed = 1;

        if( conf_size > 1 )
        while(fgets(old_buffer, conf_size, fp) != NULL)
        {
            if( strlen(old_buffer) > 4000 )   // 8000...
            {
                info = g_strdup_printf(_("A line with over 8000 chars is not valid in: %s\n"), PROFTPD_CONF);
                show_info(info);
                g_free(info);
                free(config);
                free(old_buffer);
                free(temp_server);
                free(conf_line);
                free(change_srv_ident);
                free(address_buffer);
                free(port_buffer);
                g_free(old_server_ident);

                fclose(fp);
                return;
            }


            /* Change matching server directives otherwise just gather them */
            if( strstr(old_buffer, "<VirtualHost") || strstr(old_buffer, "<Anonymous") )
            {
                strcat(config, old_buffer);
                /* When <Anonymous or <VirtualHost is found we collect all other lines to the end */
                while(fgets(old_buffer, conf_size, fp) != NULL)
                    strcat(config, old_buffer);
            }
            else
            if( strstr(old_buffer, "ServerName") )
            {
                sprintf(conf_line, "ServerName \"%s\"\n", server_name);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "ServerIdent") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[0]));
                if( active_index == 0 )
                    sprintf(conf_line, "ServerIdent on \"%s\"\n", server_ident);
                else
                    sprintf(conf_line, "ServerIdent off \"%s\"\n", server_ident);

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "MasqueradeAddress") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[1]));
                if( active_index == 0 && strlen(masquerade_address) > 4 )
                    sprintf(conf_line, "MasqueradeAddress %s\n", masquerade_address);
                else if(strlen(masquerade_address) > 4)
                    sprintf(conf_line, "#MasqueradeAddress %s\n", masquerade_address);
                else
                    sprintf(conf_line, "#MasqueradeAddress None\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "ServerAdmin") )
            {
                sprintf(conf_line, "ServerAdmin %s\n", admin_email);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "Port") && old_buffer[4] == ' ' )
            {
                sprintf(conf_line, "Port %s\n", server_port);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "PassivePorts") )
            {
                sprintf(conf_line, "PassivePorts %s %s\n", passive_port_1, passive_port_2);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "MaxInstances") )
            {
                sprintf(conf_line, "MaxInstances %s\n", max_connect);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TimeoutNoTransfer") )
            {
                sprintf(conf_line, "TimeoutNoTransfer %s\n", idle_timeout);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TimeoutIdle") )
            {
                sprintf(conf_line, "TimeoutIdle %s\n", idle_timeout);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "AllowStoreRestart") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[11]));

                if( active_index == 0 )
                    sprintf(conf_line, "AllowStoreRestart on\n");
                else
                    sprintf(conf_line, "AllowStoreRestart off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "AllowRetrieveRestart") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[12]));
                if( active_index == 0 )
                    sprintf(conf_line, "AllowRetrieveRestart on\n");
                else
                    sprintf(conf_line, "AllowRetrieveRestart off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TransferRate RETR") )
            {
                sprintf(conf_line, "TransferRate RETR %s\n", download_bandwidth);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TransferRate STOR") )
            {
                sprintf(conf_line, "TransferRate STOR %s\n", upload_bandwidth);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TransferRate STOU") )
            {
                sprintf(conf_line, "TransferRate STOU %s\n", upload_bandwidth);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TransferRate APPE") )
            {
                sprintf(conf_line, "TransferRate APPE %s\n", upload_bandwidth);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "IdentLookups") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[2]));
                if( active_index == 0 )
                    sprintf(conf_line, "IdentLookups on\n");
                else
                    sprintf(conf_line, "#IdentLookups off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "UseReverseDNS") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[3]));
                if( active_index == 0 )
                    sprintf(conf_line, "UseReverseDNS on\n");
                else
                    sprintf(conf_line, "UseReverseDNS off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "DefaultTransferMode") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[5]));
                if( active_index == 0 )
                    sprintf(conf_line, "DefaultTransferMode binary\n");
                else
                    sprintf(conf_line, "DefaultTransferMode ascii\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "User") && old_buffer[4] == ' ' )
            {
                sprintf(conf_line, "User %s\n", server_user);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "Group") && old_buffer[5] == ' ' )
            {
                sprintf(conf_line, "Group %s\n", server_group);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "DirFakeUser") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[9]));
                if( active_index == 0 )
                    sprintf(conf_line, "DirFakeUser on %s\n", fake_username);
                else
                    sprintf(conf_line, "DirFakeUser off %s\n", fake_username);

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "DirFakeGroup") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[10]));
                if( active_index == 0 )
                    sprintf(conf_line, "DirFakeGroup on %s\n", fake_groupname);
                else
                    sprintf(conf_line, "DirFakeGroup off %s\n", fake_groupname);

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "AllowForeignAddress") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[6]));
                if( active_index == 0 )
                    sprintf(conf_line, "AllowForeignAddress on\n");
                else
                    sprintf(conf_line, "AllowForeignAddress off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TimesGMT") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[4]));
                if( active_index == 0 )
                    sprintf(conf_line, "TimesGMT on\n");
                else
                    sprintf(conf_line, "TimesGMT off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "MaxLoginAttempts") )
            {
                sprintf(conf_line, "MaxLoginAttempts %s\n", max_login_attempts);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TimeoutLogin") )
            {
                sprintf(conf_line, "TimeoutLogin %s\n", login_timeout);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "DeleteAbortedStores") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[13]));
                if( active_index == 0 )
                    sprintf(conf_line, "DeleteAbortedStores on\n");
                else
                    sprintf(conf_line, "DeleteAbortedStores off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "SystemLog") )
            {
                sprintf(conf_line, "SystemLog %s\n", systemlog);
                strcat(config, conf_line);
            }
            else
                /* If using TLS */
            if( use_tls && strstr(old_buffer, "TLSEngine") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[14]));
                if( active_index == 0 )
                    sprintf(conf_line, "TLSEngine on\n");
                else
                    sprintf(conf_line, "TLSEngine off\n");

                strcat(config, conf_line);
            }
            else
            if( use_tls && strstr(old_buffer, "TLSRequired") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[15]));
                if( active_index == 0 )
//            sprintf(conf_line, "TLSRequired on\n"); It cant be more "on" then auth+data.
                    sprintf(conf_line, "TLSRequired auth+data\n");
                else if(active_index == 1)
                    sprintf(conf_line, "TLSRequired off\n");
                else if(active_index == 2)
                    sprintf(conf_line, "TLSRequired data\n");
                else if(active_index == 3)
                    sprintf(conf_line, "TLSRequired ctrl\n");

                strcat(config, conf_line);
            }
            else
            if( use_tls && strstr(old_buffer, "TLSVerifyClient") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[16]));
                if( active_index == 0 )
                    sprintf(conf_line, "TLSVerifyClient on\n");
                else
                    sprintf(conf_line, "TLSVerifyClient off\n");

                strcat(config, conf_line);
            }
            else
            if( use_tls && strstr(old_buffer, "TLSProtocol") )
            {
                sprintf(conf_line, "TLSProtocol %s\n", tls_protocols);
                strcat(config, conf_line);
            }
            else
            if( use_tls && strstr(old_buffer, "TLSLog") )
            {
                sprintf(conf_line, "TLSLog %s\n", tls_logfile);
                strcat(config, conf_line);
            }
            else
                /* If using Ratios */
            if( use_ratio && strstr(old_buffer, "Ratios ") && !strstr(old_buffer, "ave") ) /* Not SaveRatios */
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[17]));
                if( active_index == 0 )
                    sprintf(conf_line, "Ratios on\n");
                else
                    sprintf(conf_line, "Ratios off\n");

                strcat(config, conf_line);
            }
            else
            if( use_ratio && strstr(old_buffer, "SaveRatios ") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[18]));
                if( active_index == 0 )
                    sprintf(conf_line, "SaveRatios on\n");
                else
                    sprintf(conf_line, "SaveRatios off\n");

                strcat(config, conf_line);
            }
            else
                strcat(config, old_buffer); /* Gather all other values */
        }
        free(old_buffer);
        free(address_buffer);
        free(port_buffer);
        free(temp_server);
        free(conf_line);
        free(change_srv_ident);
        fclose(fp);

        /* Write the new config if any changes where made */
        if( ! configuration_changed )
        {
            g_free(old_server_ident);
            free(config);
            return;
        }

        /* Write the new conf */
        if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
        {
            g_free(old_server_ident);
            free(config);
            return;
        }
        fputs(config, fp);
        fclose(fp);
        free(config);

        /* The server address, ident or port hasnt changed, dont populate server tab */
        if( strcmp(global_server_address, server_name) == 0
        &&  strcmp(old_server_ident, server_ident) == 0
        &&  strcmp(global_server_port, server_port) == 0 )
        {
        }
        else
        {
            /* Populate the server treeview */
            populate_servers(widgets);
            /* Select the first server and set global server values */
            select_first_server(widgets);
        }

        g_free(old_server_ident);
        /* Populate the server settings */
        populate_server_settings(widgets);
        populate_conf_tab(widgets);
        /* Update the server */
        reread_conf(widgets);

        return;
    }
/* ----------------- End of default server changes ------------------ */




/* ------------ Change a virtualhost --------------- */
    if( strstr("Virtualhost", (char *)global_server_type) )
    {
        /* Setup the address and port lines to identify it with */
        address_line = g_strdup_printf("<VirtualHost %s>\n", global_server_address);
        port_line = g_strdup_printf("Port %s\n", global_server_port);

        /* Look for these address and port lines, break if they are found */
        if( conf_size > 1 )
        while(fgets(old_buffer, conf_size, fp) != NULL)
        {
            if( strcmp(old_buffer, address_line) == 0 )
            {
                address_match = 1;

                /* Save the old server name line */
                snprintf(temp_server, 1024, "%s", old_buffer);

                /* Go to the next line where we expect Port to be */
                while(fgets(old_buffer, conf_size, fp) != NULL)
                    break;

                if( address_match && strcmp(old_buffer, port_line) == 0 )
                {
                    port_match = 1;

                    /* Put the new conf settings */
                    g_free(address_line);
                    g_free(port_line);
                    address_line = g_strdup_printf("<VirtualHost %s>\n", server_name);
                    port_line = g_strdup_printf("Port %s\n", server_port);
                    strcat(config, address_line);
                    strcat(config, port_line);
                    break;
                }
                else        /* This was not the correct server */
                {
                    address_match = 0;
                    port_match = 0;
                    /* Put back the old server and port lines */
                    strcat(config, temp_server);
                    strcat(config, old_buffer);
                }
            }
            else
                /* Put back all other lines */
                strcat(config, old_buffer);
        }


        if( address_line != NULL )
            g_free(address_line);
        if( port_line != NULL )
            g_free(port_line);


        /* Could not find the server */
        if( ! address_match || ! port_match )
        {
            // Show info
            printf("The virtualhost was not found, no changes where made.\n");

            fclose(fp);
            free(change_srv_ident);
            g_free(old_server_ident);
            free(config);
            return;
        }


        /* Change the rest of the values in this vhost */
        if( conf_size > 1 )
        while(fgets(old_buffer, conf_size, fp) != NULL)
        {
            if( strlen(old_buffer) > 4000 )
                continue;

            /* The end of what we want to change has been reached, 
             * add this end line and everything else */
            if( strstr(old_buffer, "</VirtualHost") || strstr(old_buffer, "<Anonymous") )
            {
                /* This server will dissapear if it has no ending */

                configuration_changed = 1;

                /* Add the end of this server */
                strcat(config, old_buffer);

                /* At the end of the VirtualHost, copy everything else  */
                while(fgets(old_buffer, conf_size, fp) != NULL)
                    strcat(config, old_buffer);
            }
            else
                /* We have the server name and port in the conf already */
            if( strstr(old_buffer, "ServerName") )
            {
                sprintf(conf_line, "ServerName \"%s\"\n", server_name);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "ServerIdent") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[0]));
                if( active_index == 0 )
                    sprintf(conf_line, "ServerIdent on \"%s\"\n", server_ident);
                else
                    sprintf(conf_line, "ServerIdent off \"%s\"\n", server_ident);

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "MasqueradeAddress") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[1]));
                if( active_index == 0 && strlen(masquerade_address) > 4 )
                    sprintf(conf_line, "MasqueradeAddress %s\n", masquerade_address);
                else if(strlen(masquerade_address) > 4)
                    sprintf(conf_line, "#MasqueradeAddress %s\n", masquerade_address);
                else
                    sprintf(conf_line, "#MasqueradeAddress None\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "ServerAdmin") )
            {
                sprintf(conf_line, "ServerAdmin %s\n", admin_email);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "PassivePorts") )
            {
                sprintf(conf_line, "PassivePorts %s %s\n", passive_port_1, passive_port_2);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "MaxInstances") )
            {
                sprintf(conf_line, "MaxInstances %s\n", max_connect);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TimeoutNoTransfer") )
            {
                sprintf(conf_line, "TimeoutNoTransfer %s\n", idle_timeout);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TimeoutIdle") )
            {
                sprintf(conf_line, "TimeoutIdle %s\n", idle_timeout);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "AllowStoreRestart") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[11]));

                if( active_index == 0 )
                    sprintf(conf_line, "AllowStoreRestart on\n");
                else
                    sprintf(conf_line, "AllowStoreRestart off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "AllowRetrieveRestart") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[12]));
                if( active_index == 0 )
                    sprintf(conf_line, "AllowRetrieveRestart on\n");
                else
                    sprintf(conf_line, "AllowRetrieveRestart off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TransferRate RETR") )
            {
                sprintf(conf_line, "TransferRate RETR %s\n", download_bandwidth);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TransferRate STOR") )
            {
                sprintf(conf_line, "TransferRate STOR %s\n", upload_bandwidth);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TransferRate STOU") )
            {
                sprintf(conf_line, "TransferRate STOU %s\n", upload_bandwidth);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TransferRate APPE") )
            {
                sprintf(conf_line, "TransferRate APPE %s\n", upload_bandwidth);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "IdentLookups") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[2]));
                if( active_index == 0 )
                    sprintf(conf_line, "IdentLookups on\n");
                else
                    sprintf(conf_line, "#IdentLookups off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "UseReverseDNS") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[3]));
                if( active_index == 0 )
                    sprintf(conf_line, "UseReverseDNS on\n");
                else
                    sprintf(conf_line, "UseReverseDNS off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "DefaultTransferMode") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[5]));
                if( active_index == 0 )
                    sprintf(conf_line, "DefaultTransferMode binary\n");
                else
                    sprintf(conf_line, "DefaultTransferMode ascii\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "User") && old_buffer[4] == ' ' )
            {
                sprintf(conf_line, "User %s\n", server_user);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "Group") && old_buffer[5] == ' ' )
            {
                sprintf(conf_line, "Group %s\n", server_group);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "DirFakeUser") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[9]));
                if( active_index == 0 )
                    sprintf(conf_line, "DirFakeUser on %s\n", fake_username);
                else
                    sprintf(conf_line, "DirFakeUser off %s\n", fake_username);

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "DirFakeGroup") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[10]));
                if( active_index == 0 )
                    sprintf(conf_line, "DirFakeGroup on %s\n", fake_groupname);
                else
                    sprintf(conf_line, "DirFakeGroup off %s\n", fake_groupname);

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "AllowForeignAddress") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[6]));
                if( active_index == 0 )
                    sprintf(conf_line, "AllowForeignAddress on\n");
                else
                    sprintf(conf_line, "AllowForeignAddress off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TimesGMT") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[4]));
                if( active_index == 0 )
                    sprintf(conf_line, "TimesGMT on\n");
                else
                    sprintf(conf_line, "TimesGMT off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "MaxLoginAttempts") )
            {
                sprintf(conf_line, "MaxLoginAttempts %s\n", max_login_attempts);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "TimeoutLogin") )
            {
                sprintf(conf_line, "TimeoutLogin %s\n", login_timeout);
                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "DeleteAbortedStores") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[13]));
                if( active_index == 0 )
                    sprintf(conf_line, "DeleteAbortedStores on\n");
                else
                    sprintf(conf_line, "DeleteAbortedStores off\n");

                strcat(config, conf_line);
            }
            else
            if( strstr(old_buffer, "SystemLog") )
            {
                sprintf(conf_line, "SystemLog %s\n", systemlog);
                strcat(config, conf_line);
            }
            else
                /* If using TLS */
            if( use_tls && strstr(old_buffer, "TLSEngine") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[14]));
                if( active_index == 0 )
                    sprintf(conf_line, "TLSEngine on\n");
                else
                    sprintf(conf_line, "TLSEngine off\n");

                strcat(config, conf_line);
            }
            else
            if( use_tls && strstr(old_buffer, "TLSRequired") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[15]));
                if( active_index == 0 )
//            sprintf(conf_line, "TLSRequired on\n");
                    sprintf(conf_line, "TLSRequired auth+data\n");
                else
                if( active_index == 1 )
                    sprintf(conf_line, "TLSRequired data\n");
                else
                if( active_index == 2 )
                    sprintf(conf_line, "TLSRequired ctrl\n");
                else
                if( active_index == 3 )
                    sprintf(conf_line, "TLSRequired off\n");

                strcat(config, conf_line);
            }
            else
            if( use_tls && strstr(old_buffer, "TLSVerifyClient") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[16]));
                if( active_index == 0 )
                    sprintf(conf_line, "TLSVerifyClient on\n");
                else
                    sprintf(conf_line, "TLSVerifyClient off\n");

                strcat(config, conf_line);
            }
            else
            if( use_tls && strstr(old_buffer, "TLSProtocol") )
            {
                /* Remove TLSprotocol from any vhost as its not allowed in this context. */
            }
            else
            if( use_tls && strstr(old_buffer, "TLSLog") )
            {
                sprintf(conf_line, "TLSLog %s\n", tls_logfile);
                strcat(config, conf_line);
            }
            else
                /* If using Ratios */
            if( use_ratio && strstr(old_buffer, "Ratios ") && !strstr(old_buffer, "ave")) /* Not SaveRatios */
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[17]));
                if( active_index == 0 )
                    sprintf(conf_line, "Ratios on\n");
                else
                    sprintf(conf_line, "Ratios off\n");

                strcat(config, conf_line);
            }
            else
            if( use_ratio && strstr(old_buffer, "SaveRatios ") )
            {
                active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[18]));
                if( active_index == 0 )
                    sprintf(conf_line, "SaveRatios on\n");
                else
                    sprintf(conf_line, "SaveRatios off\n");

                strcat(config, conf_line);
            }
            else
                strcat(config, old_buffer); /* Gather all other values */
        }
        free(old_buffer);
        free(address_buffer);
        free(temp_server);
        free(port_buffer);
        free(change_srv_ident);
        free(conf_line);
        fclose(fp);


        /* Write the new config if any changes where made */
        if( ! configuration_changed )
        {
            g_free(old_server_ident);
            free(config);
            return;
        }

        if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
        {
            g_free(old_server_ident);
            free(config);
            return;
        }
        fputs(config, fp);
        fclose(fp);
        free(config);

        /* The server address, ident or port hasnt changed, dont populate_servers */
        if( strcmp(global_server_address, server_name) == 0
        &&  strcmp(old_server_ident, server_ident) == 0
        &&  strcmp(global_server_port, server_port) == 0 )
        {
        }
        else
        {
            /* Populate the server treeview */
            populate_servers(widgets);
            /* Select the first server and set global server values */
            select_first_server(widgets);
        }
        g_free(old_server_ident);
        /* Populate the server settings */
        populate_server_settings(widgets);
        populate_conf_tab(widgets);
        /* Update the server */
        reread_conf(widgets);

    } /* Vhost end */
}
