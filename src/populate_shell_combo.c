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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "widgets.h"
#include "gettext.h"
#include "allocate.h"
#include "commands.h"
#include "show_info.h"
#include "populate_shell_combo.h"

gboolean item_shell_exists(GtkComboBox *combo_box, const gchar *text) {
    GtkTreeIter iter;
    GtkListStore *store;
    
    store = GTK_LIST_STORE(gtk_combo_box_get_model(combo_box));

    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)) {
        do {
            gchar *entry_text;
            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 0, &entry_text, -1);
            if (g_strcmp0(entry_text, text) == 0) {
                g_free(entry_text);
                return TRUE; // Item exists
            }
            g_free(entry_text);
        } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter));
    }

    return FALSE; // Item does not exist
}




void populate_shell_combo(GtkWidget *shell_combo)
{
    FILE *fp;
    long conf_size;
    int shell_found = 0;
    char *old_buffy, *new_buffy;
    gchar *utf8 = NULL;
    gchar *combo_text = NULL;
    gchar *null_shell, *info;

    null_shell = g_strdup_printf("/dev/null");
    
   if( file_exists(null_shell) )
   {
        /* Append /dev/null as the first shell and set is as the active shell */
        combo_text = g_strdup_printf("/dev/null");
        utf8 = g_locale_to_utf8(combo_text, strlen(combo_text), NULL, NULL, NULL);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX(shell_combo), utf8);
        gtk_combo_box_set_active(GTK_COMBO_BOX(shell_combo), 0);
        g_free(combo_text);
        shell_found = 1;
    }
    g_free(null_shell);
	
	//return;

    if((fp = fopen(GP_SHELLS, "r")) == NULL)
    {
        info = g_strdup_printf(_("Cant find the shells file here:\n%s\n"), GP_SHELLS);
        show_info(info);
        g_free(info);
        return;
    }

    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    old_buffy = allocate(conf_size + 1024);
    new_buffy = allocate(conf_size + 1024);

    if( conf_size > 1 )
    while(fgets(old_buffy, conf_size, fp) != NULL)
    {

        if( strstr(old_buffy, "/dev/null") && ! strstr(old_buffy, "#") )
            shell_found = 1;

        /* Append the /bin/false or /usr/local/bin/false or /usr/bin/false etc shell */
        if( strstr(old_buffy, "false") && ! strstr(old_buffy, "#") )
        {
            shell_found = 1;
            strcpy(new_buffy, old_buffy);
            utf8 = g_locale_to_utf8(new_buffy, strlen(new_buffy) - 1, NULL, NULL, NULL);
            
            if(item_shell_exists(shell_combo,utf8)==FALSE){
				gtk_combo_box_text_append_text(GTK_COMBO_BOX(shell_combo), utf8);
				break;
			}
            
        }

        /* Append the /sbin/nologin or /usr/sbin/nologin or /usr/local/sbin/nologin etc shell */
        if( strstr(old_buffy, "nologin") && ! strstr(old_buffy, "#") )
        {
            shell_found = 1;
            strcpy(new_buffy, old_buffy);
            utf8 = g_locale_to_utf8(new_buffy, strlen(new_buffy) - 1, NULL, NULL, NULL);
            if(item_shell_exists(shell_combo,utf8)==FALSE){
				gtk_combo_box_text_append_text(GTK_COMBO_BOX(shell_combo), utf8);
				break;
			}
        }

    }

    rewind(fp);

    /* No false or nologin shell found */
    if( ! shell_found )
    {
        info = g_strdup_printf(_("Warning: could not find a false or no login shell here:\n%s\nAppending the other shells."), GP_SHELLS);
        show_info(info);
        g_free(info);
    }


    /* Append the rest of the shells */
    if( conf_size > 1 )
    while(fgets(old_buffy, conf_size, fp) != NULL)
    {
        /* Append every shell but the /bin/false shell */
        if( ! strstr(old_buffy, "/dev/null") && ! strstr(old_buffy, "nologin")
        &&  ! strstr(old_buffy, "false") && ! strstr(old_buffy, "#") && strlen(old_buffy) > 7)
        {
            strcpy(new_buffy, old_buffy);
            utf8 = g_locale_to_utf8(new_buffy, strlen(new_buffy)-1, NULL, NULL, NULL);
            if(item_shell_exists(shell_combo,utf8)==FALSE){
				gtk_combo_box_text_append_text(GTK_COMBO_BOX(shell_combo), utf8);
			}
        }
    }
    fclose(fp);

    free(old_buffy);
    free(new_buffy);

    if( utf8 != NULL )
        g_free(utf8);
}
