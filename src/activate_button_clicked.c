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
#include "gettext.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "widgets.h"
#include "show_info.h"
#include "activate_button_clicked.h"
#include "commands.h"

extern int use_tls;
extern int activated;



void activate_button_clicked(struct w *widgets)
{
    gchar *info, *start, *test, *cert_path;
    G_CONST_RETURN gchar *cert_dir;
    gint active_index = 0;

    /* Remove the shutdown file if it exists */
    unlink(GP_SHUTMSG);

    if( activated )
        return;

    /* If certs are required but have not been generated we inform the user */
    if( use_tls )
    {
        /* Check if secure FTP is enabled in the combo. If so, require certificates before start. */
        active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[15]));
        if( active_index != 1 ) /* '1' is Off. (Proftpd wants the file even though the module is not "on"...) */
        {
            cert_dir = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[15]));
            cert_path = g_strdup_printf("%s/cert.pem", cert_dir);
            if( ! file_exists(cert_path) )
            {
                info = g_strdup_printf(_("You need to generate certificates before you can start the server.\n"));
                show_info(info);
                g_free(info);
                g_free(cert_path);
                return;
            }
            g_free(cert_path);
        }
    }

    /* Start the server */
    start = g_strdup_printf("%s -c %s", PROFTPD_BINARY, PROFTPD_CONF);
    if( ! run_command(start) )
    {
        printf("Starting proftpd failed using this command: %s\n", start);
        test = g_strdup_printf("%s -c %s 2>&1", PROFTPD_BINARY, PROFTPD_CONF);
        run_command_show_err(test);
        g_free(test);

        /* No sense in starting a malfunctioning server at system boot */
        init_stop(widgets);
    }
    else
        init_start(widgets);

    g_free(start);
}
