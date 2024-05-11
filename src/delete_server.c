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
#include "show_info.h"
#include "allocate.h"
#include "delete_server.h"
#include "populate_servers.h"
#include "select_first_server.h"
#include "populate_server_settings.h"
#include "populate_users.h"
#include "select_first_user.h"
#include "populate_user_settings.h"
#include "populate_conf_tab.h"
#include "reread_conf.h"

extern char global_server_address[1024];
extern char global_server_port[1024];
extern char global_server_type[1024];

extern int activated;



void delete_server(struct w *widgets)
{
    FILE *fp;
    char *old_buffer, *config, *temp_vhost, *address_buffer, *port_buffer;
    int found_server = 0, deleted_server = 0;
    long conf_size;

    address_buffer = allocate(8192 + 15);
    port_buffer = allocate(8192 + 3);

    if( strstr((char *)global_server_type, "Virtualhost") )
    {
        /* Added \n so it wont match another similar server */
        sprintf(address_buffer, "<VirtualHost %s>\n", global_server_address);
    }
    else
    {
        free(address_buffer);
        free(port_buffer);
        return;
    }

    sprintf(port_buffer, "Port %s\n", global_server_port);

    /* Standard server selected, delete the selected user in this (first) server */
    if( ! strstr((char *)global_server_type, "Virtualhost") )
        found_server = 1;

    if( strlen(address_buffer) == 0 )
    {
        free(address_buffer);
        free(port_buffer);
        return;
    }

    if( strlen(address_buffer) == 0 )
    {
        free(address_buffer);
        free(port_buffer);
        return;
    }

    if( strlen(port_buffer) == 0 )
    {
        free(address_buffer);
        free(port_buffer);
        return;
    }


    /* Delete the configuration for the deleted user in the right server */
    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        free(address_buffer);
        free(port_buffer);
        return;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    config = allocate(conf_size);
    old_buffer = allocate(conf_size);
    temp_vhost = allocate(conf_size);

    if( conf_size > 1 )
    while(fgets(old_buffer, conf_size, fp) != NULL)
    {
        /* Is this the correct server..name to delete */
        if( strstr("Virtualhost", (char *)global_server_type)
        &&  ! found_server && ! deleted_server
        &&  strcmp(old_buffer, address_buffer) == 0 )
        {
            strcpy(temp_vhost, old_buffer);
            while(fgets(old_buffer, conf_size, fp) != NULL)
            {
                if( strlen(old_buffer) > 8000 )
                    continue;

                strcat(temp_vhost, old_buffer);

                if( strstr(old_buffer, "Port ")
                &&  strcmp(old_buffer, port_buffer) == 0 )
                {
                    found_server   = 1;
                    deleted_server = 1;

                    /* Scroll past this vhost */
                    while(fgets(old_buffer, conf_size, fp) != NULL)
                    {
                        if( strlen(old_buffer) > 8000 )
                            continue;

                        strcat(temp_vhost, old_buffer);

                        if( strstr(old_buffer, "</VirtualHost>") )
                            break;
                    }
                }
                if( strstr(old_buffer, "</VirtualHost>") )
                    break;
            }

            if( ! found_server )
                strcat(config, temp_vhost);
            else
                found_server = 0;
        }
        else                /* Get everything thats not a vhost */
            strcat(config, old_buffer);
    }
    free(old_buffer);
    free(address_buffer);
    free(port_buffer);
    free(temp_vhost);
    fclose(fp);

    if( ! deleted_server )
    {
        free(config);
        return;
    }

    /* Write the new config without the deleted vhost */
    if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
    {
        printf("Error Writing configuration here: %s\n", PROFTPD_CONF);
        free(config);
        return;
    }
    fputs(config, fp);
    fclose(fp);
    free(config);


    populate_servers(widgets);
    select_first_server(widgets);
    populate_server_settings(widgets);

    populate_users(widgets);
    select_first_user(widgets);
    populate_user_settings(widgets);

    populate_conf_tab(widgets);

    /* Update the server */
    reread_conf(widgets);
}
