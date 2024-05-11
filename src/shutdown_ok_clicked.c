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
#include <stdio.h>
#include "widgets.h"
#include "shutdown_ok_clicked.h"



void shutdown_ok_clicked(struct w *widgets)
{
    /* Shutdown the server (create the shutmsg file) */
    FILE *fp;
    gchar *shutdown;
    G_CONST_RETURN gchar *new_logins_dc;
    G_CONST_RETURN gchar *existing_users_dc;
    G_CONST_RETURN gchar *real_shutdown;
    G_CONST_RETURN gchar *shut_msg;

    new_logins_dc = gtk_entry_get_text(GTK_ENTRY(widgets->new_acc_disabled_entry));
    existing_users_dc = gtk_entry_get_text(GTK_ENTRY(widgets->existing_users_dc_entry));
    real_shutdown = gtk_entry_get_text(GTK_ENTRY(widgets->real_shutdown_entry));
    shut_msg = gtk_entry_get_text(GTK_ENTRY(widgets->shutdown_msg_entry));

    shutdown = g_strdup_printf("ftpshut -l %s -d %s %s \"%s\"",
                            new_logins_dc, existing_users_dc, real_shutdown, shut_msg);

    fp = popen(shutdown, "r");
    pclose(fp);

    g_free(shutdown);
    gtk_widget_destroy(widgets->shutdown_window);
}
