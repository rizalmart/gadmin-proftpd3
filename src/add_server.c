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
#include <string.h>
#include <stdlib.h>
#include "widgets.h"
#include "gettext.h"
#include "allocate.h"
#include "add_server.h"
#include "show_info.h"
#include "populate_servers.h"
#include "populate_conf_tab.h"



void add_server(struct w *widgets)
{
    FILE *fp;
    gchar *vhost_config;

    if((fp = fopen(PROFTPD_CONF, "a")) == NULL)
    {
        printf("Cant write to: %s\n", PROFTPD_CONF);
        return;
    }

    /* The vhost server configuration to be added */

    vhost_config = g_strconcat("",
        "\n<VirtualHost localhost>\n",
        "Port 65535\n",
        "ServerName \"localhost\"\n",
        "ServerIdent on \"Vhost server\"\n",
        "PassivePorts 49152 65534\n",
        "#MasqueradeAddress None\n",
        "ServerAdmin Admin@example.org\n",
        "Umask 022\n",
        "TimesGMT off\n",
        "MaxLoginAttempts 3\n",
        "TimeoutLogin 300\n",
        "TimeoutNoTransfer 120\n",
        "TimeoutIdle 120\n",
        "User ", SERVER_USER, "\n",
        "Group ", SERVER_GROUP, "\n",
        "DirFakeUser on ", SERVER_USER, "\n",
        "DirFakeGroup on ", SERVER_GROUP, "\n",
        "DefaultTransferMode binary\n",
        "AllowForeignAddress off\n",
        "DeleteAbortedStores off\n",
        "AllowRetrieveRestart on\n",
        "AllowStoreRestart on\n",
        "TransferRate RETR 142\n",
        "TransferRate STOR 142\n",
        "TransferRate STOU 142\n",
        "TransferRate APPE 142\n",
        "RequireValidShell off\n",
        "<IfModule mod_tls.c>\n",
        "TLSEngine off\n",
        "TLSRequired off\n",
        "TLSVerifyClient off\n",
        "TLSLog ", GP_VARDIR, "/log/proftpd_tls.log\n",
        "TLSRSACertificateFile ", GP_APPCONFDIR, "/certs/cert.pem\n",
        "TLSRSACertificateKeyFile ", GP_APPCONFDIR, "/certs/key.pem\n",
        "TLSCACertificateFile ", GP_APPCONFDIR, "/certs/cacert.pem\n",
        "TLSRenegotiate required off\n",
        "</IfModule>\n\n",
        "<IfModule mod_ratio.c>\n",
        "Ratios off\n",
        "SaveRatios off\n",
        "RatioFile \"/restricted/proftpd_ratios\"\n",
        "RatioTempFile \"/restricted/proftpd_ratios_temp\"\n",
        "CwdRatioMsg \"Please upload first!\"\n",
        "FileRatioErrMsg \"FileRatio limit exceeded, upload something first...\"\n",
        "ByteRatioErrMsg \"ByteRatio limit exceeded, upload something first...\"\n",
        "LeechRatioMsg \"Your ratio is unlimited.\"\n",
        "</IfModule>\n\n", "<Limit LOGIN>\n", "  DenyAll\n", "</Limit>\n", "</VirtualHost>\n\n", NULL);

    fputs(vhost_config, fp);
    fclose(fp);
    g_free(vhost_config);

    /* Repopulate the affected parts of the gui */
    populate_servers(widgets);

    populate_conf_tab(widgets);

    /* Dont update the server, itll be updated when a user is added */
}
