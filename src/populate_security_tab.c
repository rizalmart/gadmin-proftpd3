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
#include "populate_security_tab.h"
#include "show_info.h"

#define MAX_READ_POPEN 16384


void populate_security(struct w *widgets)
{
    /* Lists security entries in the security treeview, denials first */
    FILE *fp;
    long file_size;
    char *buffy;
    GtkTreeIter iter;
    GtkTreePath *path;
    gboolean edit = 0;
    int found = 0;
    gchar *utf8 = NULL;
    gchar *info = NULL;

    /* Default to the configured security log */
    gchar *secure_log;
    secure_log = g_strdup_printf("%s", SECURE_LOG);

    /* Use the server settings security log if its set */
    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        /* Do nothing */
    }
    else
    {
        fseek(fp, 0, SEEK_END);
        file_size = ftell(fp);
        rewind(fp);
        buffy = allocate(file_size + 1024);

        if( file_size > 1 )
        while(fgets(buffy, file_size, fp) != NULL)
        {
            if( ! strstr(buffy, "SystemLog ") || strstr(buffy, "#") )
                continue;

            if( strlen(buffy) < 12 )
                continue;

            /* We have a valid security logfile (doesnt have to exist yet) */
            g_free(secure_log);
            secure_log = g_strdup_printf("%s", &buffy[10]);
            if(secure_log[strlen(secure_log)-1]=='\n')
                secure_log[strlen(secure_log)-1]='\0';
            break;
        }
        free(buffy);
    }


    gtk_list_store_clear(widgets->security_store);


    if((fp = fopen(secure_log, "r")) == NULL)
    {
        info = g_strdup_printf(_("Cant open the security log here:\n%s\n"), secure_log);
        show_info(info);
        g_free(info);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    buffy = allocate(file_size + 1024);

    if( file_size > 1 )
    while(fgets(buffy, file_size, fp) != NULL)
    {
        if( ! strstr(buffy, "proftpd") )
            continue;


        if( strstr(buffy, "Login failed") )
        {
            found = 1;
            gtk_list_store_append(GTK_LIST_STORE(widgets->security_store), &iter);
            utf8 = g_locale_to_utf8(buffy, strlen(buffy) - 1, NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->security_store), &iter, 0, utf8, -1);
        }
        if( strstr(buffy, "refused") )
        {
            found = 1;
            gtk_list_store_append(GTK_LIST_STORE(widgets->security_store), &iter);
            utf8 = g_locale_to_utf8(buffy, strlen(buffy) - 1, NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->security_store), &iter, 0, utf8, -1);
        }
        if( strstr(buffy, "no such user") )
        {
            found = 1;
            gtk_list_store_append(GTK_LIST_STORE(widgets->security_store), &iter);
            utf8 = g_locale_to_utf8(buffy, strlen(buffy) - 1, NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->security_store), &iter, 0, utf8, -1);
        }
        if( strstr(buffy, "Maximum login attempts") )
        {
            found = 1;
            gtk_list_store_append(GTK_LIST_STORE(widgets->security_store), &iter);
            utf8 = g_locale_to_utf8(buffy, strlen(buffy) - 1, NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->security_store), &iter, 0, utf8, -1);
        }

        if( strstr(buffy, "SECURITY VIOLATION") )
        {
            found = 1;
            gtk_list_store_append(GTK_LIST_STORE(widgets->security_store), &iter);
            utf8 = g_locale_to_utf8(buffy, strlen(buffy) - 1, NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->security_store), &iter, 0, utf8, -1);
        }
    }

    rewind(fp);
    memset(buffy, 0, file_size);

    if( file_size > 1 )
    while(fgets(buffy, file_size, fp) != NULL)
    {
        if( ! strstr(buffy, "proftpd") )
            continue;

        if( strstr(buffy, "Login successful") )
        {
            found = 1;
            gtk_list_store_append(GTK_LIST_STORE(widgets->security_store), &iter);
            utf8 = g_locale_to_utf8(buffy, strlen(buffy) - 1, NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->security_store), &iter, 0, utf8, -1);
        }
    }

    fclose(fp);
    free(buffy);
    g_free(secure_log);

    if( utf8 != NULL )
        g_free(utf8);

    /* Cant set treepath if there arent any rows, then itll crash */
    if( ! found )
        return;

    path = gtk_tree_path_new_first();
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(widgets->security_treeview), path, NULL, edit);
    gtk_tree_path_free(path);
}
