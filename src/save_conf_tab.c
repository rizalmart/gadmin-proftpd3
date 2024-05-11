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
#include "save_conf_tab.h"
#include "populate_servers.h"
#include "populate_server_settings.h"
#include "populate_users.h"
#include "populate_user_settings.h"
#include "reread_conf.h"



void save_conf_tab(struct w *widgets)
{
    /* Save the configuration in the conf tab to PROFTPD_CONF. */
    FILE *fp;
    GtkTextBuffer *conf_textbuffer;
    GtkTextIter start, end;
    gchar *conf = NULL;
    gchar *utf8 = NULL;

    conf_textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->conf_textview));
    gtk_text_buffer_get_bounds(conf_textbuffer, &start, &end);

    conf = gtk_text_iter_get_text(&start, &end);

    if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
    {
        return;
    }
    fputs(conf, fp);
    fclose(fp);

    /* Update the server tabs */
    populate_servers(widgets);
    populate_server_settings(widgets);

    /* Update the user tabs */
    populate_users(widgets);
    populate_user_settings(widgets);

    /* Dont update the conf tab */

    if( conf != NULL )
        g_free(conf);
    if( utf8 != NULL )
        g_free(utf8);

    reread_conf(widgets);
}
