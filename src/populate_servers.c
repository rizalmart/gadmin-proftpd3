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
#include "support.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "allocate.h"
#include "widgets.h"
#include "populate_servers.h"
#include "show_info.h"
#include "functions.h"
#include "get_option_pos.h"

extern char global_server_port[1024];
extern char global_server_name[1024];
extern char global_server_type[1024];



void populate_servers(struct w *widgets)
{
    /* Lists servers in the serverlist */
    FILE *fp;
    GtkTreeIter iter;
    GtkTreePath *path;
    char *line, *new_buffer;
    long file_size;
    gboolean edit = 0;
    int i = 0, found = 0;
    gchar *utf8 = NULL;

    gtk_list_store_clear(widgets->server_store);

    /* Populate the serverlist */
    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        /* Dont show annoying popup */
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size);
    new_buffer = allocate(file_size);

    /* Add a row */
    gtk_list_store_append(GTK_LIST_STORE(widgets->server_store), &iter);

    /* Always append this for the default server */
    utf8 = g_locale_to_utf8("All interfaces", 14, NULL, NULL, NULL);
    gtk_list_store_set(GTK_LIST_STORE(widgets->server_store), &iter, 0, utf8, -1);


    /* List the default server first */
    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        if( commented(line) )
            continue;

        if( cmplowercase(line, "<virtualhost ")
        ||  cmplowercase(line, "<anonymous ") )
            break;

        new_buffer[0] = '\0';

        if( cmplowercase(line, "port ")
        &&  ! cmplowercase(line, "passiveports ")
        &&  ! cmplowercase(line, "standard") )
        {
            // Fix: use opt_pos = get_option_pos(line, 1);

            /* Insert the ipaddress or hostname */
            sprintf(new_buffer, "%s", &line[5]);
            new_buffer[strlen(new_buffer) - 1] = '\0';

            /* Insert the port */
            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->server_store), &iter, 1, utf8, -1);

            /* Insert the type */
            utf8 = g_locale_to_utf8("Default server", 14, NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->server_store), &iter, 3, utf8, -1);
        }
        if( strstr(line, "ServerName ")
        ||  strstr(line, "ServerIdent on ") )
        {
            utf8 = g_locale_to_utf8("Default server", 14, NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->server_store), &iter, 3, utf8, -1);

            if( strstr(line, "ServerIdent on ") )
            {
                for(i = 16; line[i] != '\0'; i++)
                    if( line[i] != '"'&& line[i] != ' '
                    &&  line[i] != '\n' && line[i] != '\t' )
                        break;

                sprintf(new_buffer, "%s", &line[i]);
                new_buffer[strlen(new_buffer) - 2] = '\0';
                utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                gtk_list_store_set(GTK_LIST_STORE(widgets->server_store), &iter, 2, utf8, -1);
            }
            if( strstr(line, "ServerName ") )
            {
                for(i = 12; line[i] != '\0'; i++)
                    if(line[i] != '"' && line[i] != ' ' && line[i] != '\n' && line[i] != '\t')
                        break;

                sprintf(new_buffer, "%s", &line[i]);
                new_buffer[strlen(new_buffer) - 2] = '\0';
                utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                gtk_list_store_set(GTK_LIST_STORE(widgets->server_store), &iter, 2, utf8, -1);
            }
        }
    }
    /* End of listing the default server */



    /* List the VirtualHosts */

    found = 0;
    rewind(fp);

    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        if( commented(line) )
            continue;

        if( strstr(line, "<VirtualHost ") )
        {
            /* Add a row */
            gtk_list_store_append(GTK_LIST_STORE(widgets->server_store), &iter);

            /* Insert the ipaddress or hostname */
            sprintf(new_buffer, "%s", &line[13]);
            new_buffer[strlen(new_buffer) - 2] = '\0';

            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->server_store), &iter, 0, utf8, -1);

            utf8 = g_locale_to_utf8("Virtualhost", 11, NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->server_store), &iter, 3, utf8, -1);
            found = 1;
        }

        if( ! found )
            continue;

        if( strstr(line, "Port ")
        &&  ! strstr(line, "PassivePorts ")
        &&  ! strstr(line, "standard ") )
        {
            /* Insert the ipaddress or hostname */
            sprintf(new_buffer, "%s", &line[5]);
            new_buffer[strlen(new_buffer) - 1] = '\0';

            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->server_store), &iter, 1, utf8, -1);
        }

        if( strstr(line, "ServerName ") )
        {
            for(i = 12; line[i] != '\0'; i++)
                if( line[i] != '"' && line[i] != ' ' && line[i] != '\n' && line[i] != '\t' )
                    break;

            /* Insert the ipaddress or hostname */
            sprintf(new_buffer, "%s", &line[i]);
            new_buffer[strlen(new_buffer) - 2] = '\0';
            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->server_store), &iter, 2, utf8, -1);
        }

        if( strstr(line, "ServerIdent on ") )
        {
            for(i = 16; line[i] != '\0'; i++)
                if( line[i] != '"' && line[i] != ' ' && line[i] != '\n' && line[i] != '\t' )
                    break;

            /* Insert the ipaddress or hostname */
            sprintf(new_buffer, "%s", &line[i]);
            new_buffer[strlen(new_buffer) - 2] = '\0';
            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->server_store), &iter, 2, utf8, -1);
        }
    }
    fclose(fp);
    free(line);
    free(new_buffer);
    if( utf8 != NULL )
        g_free(utf8);


    /* We always append a line for the default server first.. so this wont crash */
    path = gtk_tree_path_new_first();
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(widgets->server_treeview), path, NULL, edit);
    gtk_tree_path_free(path);
}
