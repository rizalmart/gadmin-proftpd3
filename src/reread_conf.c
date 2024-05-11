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
#include <unistd.h>
#include <string.h>
#include "widgets.h"
#include "gettext.h"
#include "commands.h"

extern int activated;


void reread_conf(struct w *widgets)
{
    /* Reread with a HUP and show any errors */
    gchar *cmd, *test;

    if( ! activated )
        return;

    cmd = g_strdup_printf("killall -HUP %s", PROFTPD_BINARY);
    if( ! run_command(cmd) )
    {
        printf("Restarting proftpd failed\n");
        test = g_strdup_printf("killall -HUP %s 2>&1", PROFTPD_BINARY);
        run_command_show_err(test);
        g_free(test);
    }
    g_free(cmd);
}
