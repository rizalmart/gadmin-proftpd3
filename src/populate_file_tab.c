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
#include "support.h"
#include "gettext.h"
#include <gtk/gtk.h>
#include "support.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "allocate.h"
#include "widgets.h"
#include "populate_file_tab.h"
#include "show_info.h"



void populate_files(struct w *widgets)
{
    /* Lists files in the file treeview */
    FILE *fp;
    GtkTreeIter iter;
    GtkTreePath *path;
    gboolean edit = 0;
    gchar *utf8 = NULL;
    char *old_buffer, *new_buffer;
    long file_size;
    int begin = 0, end = 0, i = 0, count = 0, found = 0;
    int max_line_len = 1024, max_username_len = 40;

    gtk_list_store_clear(widgets->file_store);

    if((fp = fopen(XFER_LOG, "r")) == NULL)
    {
        /* A popup here is just annoying */
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    old_buffer = allocate(file_size);

    /* Min allocate + 1024 */
    new_buffer = allocate(max_line_len + 1024);

    while(fgets(old_buffer, file_size, fp) != NULL)
    {
        if( strlen(old_buffer) < 10 )
            continue;

        /* Add a row */
        gtk_list_store_append(GTK_LIST_STORE(widgets->file_store), &iter);


/* ------------- Username ----------- */
        count = 0;
        begin = 0;
        end = 0;
        for(i = strlen(old_buffer) - 1; old_buffer[i] != '\0'; i--)
        {
            if( old_buffer[i] == ' ' && old_buffer[i + 1] != ' ' )
                count++;

            if( count == 5 )
            {
                begin = i;
                break;
            }
        }

        for(i = begin + 1; old_buffer[i] != '\0'; i++)
        {
            if( old_buffer[i] == ' ' && old_buffer[i + 1] != ' ' )
            {
                end = i;
                break;
            }
        }

        memset(new_buffer, 0, max_line_len + 1024);

        if( end - begin > max_username_len )
        {
            while(end - begin > max_username_len)
                end--;
            strncat(new_buffer, &old_buffer[begin], end);
        }
        else
            strncat(new_buffer, &old_buffer[begin], end);

        if( new_buffer[strlen(new_buffer) - 1] != '\0' )
            new_buffer[strlen(new_buffer) - 1] = '\0';

        utf8 = g_locale_to_utf8(new_buffer, end - begin, NULL, NULL, NULL);
        gtk_list_store_set(GTK_LIST_STORE(widgets->file_store), &iter, 0, utf8, -1);
/* ------------- Username End ----------- */


/* ------------- Action (upload/download/delete) ----------------- */
        count = 0;
        begin = 0;
        end = 0;
        for(i = strlen(old_buffer) - 1; old_buffer[i] != '\0'; i--)
        {
            if( old_buffer[i] == ' ' && old_buffer[i + 1] != ' ' )
                count++;

            if( count == 7 )
            {
                begin = i + 1;
                break;
            }
        }

        snprintf(new_buffer, 3, "%s", &old_buffer[begin]);

        if( new_buffer[strlen(new_buffer) - 1] != '\0' )
            new_buffer[strlen(new_buffer) - 1] = '\0';

        if( strcmp(new_buffer, "i") == 0 )
            sprintf(new_buffer, "%s", _("uploaded"));
        else
        if( strcmp(new_buffer, "o") == 0 )
            sprintf(new_buffer, "%s", _("downloaded"));
        else
        if( strcmp(new_buffer, "d") == 0 )
            sprintf(new_buffer, "%s", _("deleted"));
        else
            sprintf(new_buffer, "%s", _("split filename ?"));

        utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
        gtk_list_store_set(GTK_LIST_STORE(widgets->file_store), &iter, 1, utf8, -1);
/* ------------- Action End ------------- */


/* ------------- Bytes ----------- */
        count = 0;
        for(i = 0; old_buffer[i] != '\0'; i++)
        {
            if( old_buffer[i] == ' ' && old_buffer[i + 1] != ' ' )
            {
                count++;
                begin = i;
            }
            if( count == 7 )
                break;
        }

        count = 0;
        for(i = begin + 1; old_buffer[i] != '\0'; i++)
        {
            if( old_buffer[i] == ' ' && old_buffer[i + 1] != ' ' )
            {
                end = i;
                break;
            }
        }

        memset(new_buffer, 0, max_line_len + 1024);

        if( end - begin > max_line_len )
        {
            while(end - begin > max_line_len)
                end--;
            strncat(new_buffer, &old_buffer[begin], end);
        }
        else
            strncat(new_buffer, &old_buffer[begin], end);

        if( new_buffer[strlen(new_buffer) - 1] != '\0' )
            new_buffer[strlen(new_buffer) - 1] = '\0';

        utf8 = g_locale_to_utf8(new_buffer, end - begin, NULL, NULL, NULL);
        gtk_list_store_set(GTK_LIST_STORE(widgets->file_store), &iter, 2, utf8, -1);
/* ------------- Bytes End ----------- */


/* ------------- FILES ------------ */
        /* If we find a / then heres where the filepath begins 8 spaces from the start */

        /* Theres no problem with split filenames because a file 
           called "this is my file" is logged as "this_is_my_file" */
        for(i = 0; old_buffer[i] != '\0'; i++)
        {
            if( old_buffer[i] == '/' )
            {
                begin = i;
                break;
            }
        }
        /* The file end is 9 spaces from the end */
        count = 0;
        for(i = strlen(old_buffer) - 1; old_buffer[i] != '\0'; i--)
        {
            if( old_buffer[i] == ' ' && old_buffer[i + 1] != ' ' )
                count++;

            if( count == 9 )
            {
                end = i;
                break;
            }
        }

        memset(new_buffer, 0, max_line_len + 1024);

        if( end - begin > max_line_len )
        {
            while(end - begin > max_line_len)
                end--;
            strncat(new_buffer, &old_buffer[begin], end);
        }
        else
            strncat(new_buffer, &old_buffer[begin], end);

        if( new_buffer[strlen(new_buffer) - 1] != '\0' )
            new_buffer[strlen(new_buffer) - 1] = '\0';

        utf8 = g_locale_to_utf8(new_buffer, end - begin, NULL, NULL, NULL);
        gtk_list_store_set(GTK_LIST_STORE(widgets->file_store), &iter, 3, utf8, -1);
/* ------------- FILES END ------------ */


/* ------------- Host ----------- */
        count = 0;
        for(i = 0; old_buffer[i] != '\0'; i++)
        {
            if( old_buffer[i] == ' ' && old_buffer[i + 1] != ' ' )
            {
                count++;
                begin = i;
            }
            if( count == 6 )
                break;
        }

        count = 0;
        for(i = begin + 1; old_buffer[i] != '\0'; i++)
        {
            if( old_buffer[i] == ' ' && old_buffer[i + 1] != ' ' )
            {
                end = i;
                break;
            }
        }

        memset(new_buffer, 0, max_line_len + 1024);

        if( end - begin > max_line_len )
        {
            while(end - begin > max_line_len)
                end--;
            strncat(new_buffer, &old_buffer[begin], end);
        }
        else
            strncat(new_buffer, &old_buffer[begin], end);

        if( new_buffer[strlen(new_buffer) - 1] != '\0' )
            new_buffer[strlen(new_buffer) - 1] = '\0';

        utf8 = g_locale_to_utf8(new_buffer, end - begin, NULL, NULL, NULL);
        gtk_list_store_set(GTK_LIST_STORE(widgets->file_store), &iter, 4, utf8, -1);
/* ------------- Host End ----------- */



/* ------------- Start Date ----------- */
        count = 0;
        for(i = 0; old_buffer[i] != '\0'; i++)
        {
            /* Day can be " 2" or "20" etc */
            if( old_buffer[i] == ' ' && old_buffer[i + 1] != ' ' )
            {
                count++;
                end = i;
            }
            if( count == 5 )
                break;
        }

        if( end <= max_line_len )
        {
            utf8 = g_locale_to_utf8(old_buffer, end, NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->file_store), &iter, 5, utf8, -1);
        }
/* ------------- Start Date End ----------- */

        found = 1;
    }
    fclose(fp);
    free(old_buffer);
    free(new_buffer);


    if( utf8 != NULL )
        g_free(utf8);

    /* Cant set treepath if there arent any files/rows, then itll crash */
    if( ! found )
        return;

    path = gtk_tree_path_new_first();
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(widgets->file_treeview), path, NULL, edit);
    gtk_tree_path_free(path);
}
