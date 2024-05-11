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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "widgets.h"
#include "gettext.h"
#include "commands.h"
#include "show_info.h"
#include "allocate.h"
#include "deactivate_button_clicked.h"
#include "commands.h"

extern int activated;



void deactivate_button_clicked(struct w *widgets)
{
    gchar *cmd, *info;

    init_stop(widgets);

    if( ! activated )
        return;

    /* Issues with users in CLOSE_WAIT states so we kill -15 */
    cmd = g_strdup_printf("killall -15 %s", PROFTPD_BINARY);
    if( ! run_command(cmd) )
    {
        info = g_strdup_printf("Failed to deactivate the server with command:\n%s\n", cmd);
        show_info(info);
        g_free(info);
    }
    g_free(cmd);
}
