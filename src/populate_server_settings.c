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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "gettext.h"
#include "widgets.h"
#include "allocate.h"
#include "populate_server_settings.h"
#include "functions.h"
#include "get_option_pos.h"

extern gulong auth_changed_signal;

extern char global_server_address[1024];
extern char global_server_port[1024];
extern char global_server_type[1024];

extern int use_tls;
extern int use_ratio;
extern int use_quota;

extern gchar *GP_PASSWD_BUF;
extern gchar *GP_SHADOW_BUF;
extern gchar *GP_GROUP_BUF;
extern gchar *GP_GSHADOW_BUF;



void populate_server_settings(struct w *widgets)
{
    /* Lists the values for the selected server in the server tab */
    FILE *fp;
    int x = 0, val = 0, found = 0;
    int found_maskaddr = 0;
    long conf_size;
    char *old_buffer, *port_buffer, *new_buffer, *address_buffer, *openssl_conf;
    gchar *settings_conf, *path, *utf8 = NULL;

    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        return;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    old_buffer = allocate(conf_size + 1);
    new_buffer = allocate(conf_size + 1);
    address_buffer = allocate(8192 + 15);


    if( strstr(global_server_type, "Virtualhost") )
        sprintf(address_buffer, "<VirtualHost %s>", global_server_address);
    else
        sprintf(address_buffer, global_server_address);

    port_buffer = allocate(8192 + 3);

    sprintf(port_buffer, "Port %s", global_server_port);

    if( ! strstr((char *)global_server_type, "Virtualhost") )
        found = 1;

    if( conf_size > 1 )
    while(fgets(old_buffer, conf_size, fp) != NULL)
    {
        if( strlen(old_buffer) > 8000 )
        {
            fclose(fp);
            free(old_buffer);
            free(new_buffer);
            free(address_buffer);
            free(port_buffer);
            return;
        }

        /* We have all the values for the selected default server */
        if( cmplowercase(old_buffer, "<anonymous")
        &&  strstr((char *)global_server_type, "Default server") )
            break;

        /* We have a virtualhost with the same address as the selected server */
        if( strstr("Virtualhost", (char *)global_server_type)
        &&  ! found && strstr(old_buffer, address_buffer) )
        {
            /* Lets see if its the same port as the selected one */

            /* If this server has the same port its the correct server .. the end. */
            while(fgets(old_buffer, conf_size, fp) != NULL)
            {
                if( strlen(old_buffer) > 8000 )
                    continue;

                /* This will expect the servers port on the second line ! 
                 * else itll miss some vaules .. */
                if( cmplowercase(old_buffer, "port ")
                &&  strstr(old_buffer, port_buffer) )
                {
                    found = 1;
                    break;
                }

                if( cmplowercase(old_buffer, "</virtualhost>") )
                    break;
            }
        }

        /* Continue until we find the selected server */
        if( ! found )
            continue;

        /* Read and insert the selected servers values in the servers tab */

        /* If its the default server we stop listing the values when a vhost or anonymous is found */
        if( ! strstr("Virtualhost", (char *)global_server_type)
        && (cmplowercase(old_buffer, "<anonymous")
        ||  cmplowercase(old_buffer, "<virtualhost")) )
            break;

        /* In any case after listing the correct vhost and anon user we break */
        if( cmplowercase(old_buffer, "<anonymous") && found )
            break;

        /* In any case after listing the correct vhost we break */
        if( cmplowercase(old_buffer, "</virtualhost") && found )
            break;

        /* The name of the server */
        if( cmplowercase(old_buffer, "servername") )
        {
            for(x = 0; old_buffer[x] != '\0'; x++)
            {
                if( old_buffer[x] == '"' )
                    break;
            }
            x++;
            if( old_buffer[x] != '\0' )
                sprintf(new_buffer, &old_buffer[x]);
            for(x = 0; new_buffer[x] != '\0'; x++)
            {
                if( new_buffer[x] == '"' )
                {
                    new_buffer[x] = '\0';
                    break;
                }
            }

            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[0]), utf8);
        }

        if( cmplowercase(old_buffer, "serverident") )
        {
            for(x = 0; old_buffer[x] != '\0'; x++)
            {
                if( old_buffer[x] == '"' )
                    break;
            }
            x++;
            if( old_buffer[x] != '\0' )
                snprintf(new_buffer, 1000, "%s", &old_buffer[x]);

            for(x = 0; new_buffer[x] != '\0'; x++)
            {
                if( new_buffer[x] == '"' || new_buffer[x] == '\n' )
                {
                    new_buffer[x] = '\0';
                    break;
                }
            }

            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[1]), utf8);

            if( cmplowercase(old_buffer, "on") )
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[0]), 0);
            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[0]), 1);
        }

        if( cmplowercase(old_buffer, "masqueradeaddress") )
        {
            new_buffer[0] = '\0';
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( strlen(new_buffer) > 3 )
            {
                if( new_buffer[strlen(new_buffer)-1]=='\n'
                ||  new_buffer[strlen(new_buffer)-1]=='\r' )
                    new_buffer[strlen(new_buffer)-1]='\0';
                utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[2]), utf8);
            }

            if( strstr(old_buffer, "#") )
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[1]), 1);
            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[1]), 0);

            found_maskaddr = 1;
        }

        if( cmplowercase(old_buffer, "serveradmin") )
        {
            snprintf(new_buffer, 1000, "%s", &old_buffer[12]);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r' )
                new_buffer[strlen(new_buffer)-1]='\0';

            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[3]), utf8);
        }

        if( cmplowercase(old_buffer, "identlookups") )
        {
            if( cmplowercase(old_buffer, "identlookups on") )
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[2]), 0);

            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[2]), 1);
        }

        if( cmplowercase(old_buffer, "usereversedns") )
        {
            if( cmplowercase(old_buffer, "usereversedns on") )
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[3]), 0);
            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[3]), 1);
        }

        /* Off is local time */
        if( cmplowercase(old_buffer, "timesgmt") )
        {
            if( strstr(old_buffer, "on") )
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[4]), 0);
            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[4]), 1);
        }

        /* Dont match passiveports */
        if( cmplowercase(old_buffer, "port")
        &&  old_buffer[4] == ' '
        &&  ! strstr(old_buffer, "passiveports") )
        {
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r' )
                new_buffer[strlen(new_buffer)-1]='\0';

            if( chars_are_digits(new_buffer) )
            {
                val = atoi(new_buffer);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[0]), val);
            }
        }

        if( cmplowercase(old_buffer, "passiveports") )
        {
            new_buffer[0] = '\0';
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r' )
                new_buffer[strlen(new_buffer)-1]='\0';

            if( chars_are_digits(new_buffer) )
            {
                val = atoi(new_buffer);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[1]), val);
            }

            new_buffer[0] = '\0';
            sscanf(old_buffer, "%*s %*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r')
                new_buffer[strlen(new_buffer)-1]='\0';

            if( chars_are_digits(new_buffer) )
            {
                val = atoi(new_buffer);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[2]), val);
            }
        }

        /* Maximum simoultaneous connections */
        if( cmplowercase(old_buffer, "maxinstances") )
        {
            new_buffer[0] = '\0';
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r' )
                new_buffer[strlen(new_buffer)-1]='\0';

            if( chars_are_digits(new_buffer) )
            {
                val = atoi(new_buffer);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[3]), val);
            }

        }

        /* Max login attempts */
        if( cmplowercase(old_buffer, "maxloginattempts") )
        {
            new_buffer[0] = '\0';
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r' )
                new_buffer[strlen(new_buffer)-1]='\0';

            if( chars_are_digits(new_buffer) )
            {
                val = atoi(new_buffer);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[4]), val);
            }
        }

        /* Login timeout */
        if( cmplowercase(old_buffer, "timeoutlogin") )
        {
            new_buffer[0] = '\0';
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r' )
                new_buffer[strlen(new_buffer)-1]='\0';

            if( chars_are_digits(new_buffer) )
            {
                val = atoi(new_buffer);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[5]), val);
            }
        }

        /* Idle timeout */
        if( cmplowercase(old_buffer, "timeoutnotransfer") )
        {
            new_buffer[0] = '\0';
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r' )
                new_buffer[strlen(new_buffer)-1]='\0';

            if( chars_are_digits(new_buffer) )
            {
                val = atoi(new_buffer);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[6]), val);
            }
        }

        /* Upload speed */
        if( cmplowercase(old_buffer, "transferrate stor") )
        {
            new_buffer[0] = '\0';
            sscanf(old_buffer, "%*s %*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r' )
                new_buffer[strlen(new_buffer)-1]='\0';

            if( chars_are_digits(new_buffer) )
            {
                val = atoi(new_buffer);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[9]), val);
            }
        }

        /* Download speed */
        if( cmplowercase(old_buffer, "transferrate retr") )
        {
            new_buffer[0] = '\0';
            sscanf(old_buffer, "%*s %*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r' )
                new_buffer[strlen(new_buffer)-1]='\0';

            if( chars_are_digits(new_buffer) )
            {
                val = atoi(new_buffer);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[10]), val);
            }
        }

        /* Transfer mode */
        if( cmplowercase(old_buffer, "defaulttransfermode") )
        {
            if( strstr(old_buffer, "binary") )
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[5]), 0);
            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[5]), 1);
        }

        /* FXP transfers */
        if( cmplowercase(old_buffer, "allowforeignaddress") )
        {
            if( strstr(old_buffer, "on") )
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[6]), 0);
            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[6]), 1);
        }

        /* Server user */
        if( cmplowercase(old_buffer, "user") && old_buffer[4]==' ' )
        {
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r' )
                new_buffer[strlen(new_buffer)-1]='\0';

            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[9]), utf8);
        }

        /* Server group */
        if( cmplowercase(old_buffer, "group") && old_buffer[5]==' ' )
        {
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]== '\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r' )
                new_buffer[strlen(new_buffer)-1]= '\0';

            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[10]), utf8);
        }

        if( cmplowercase(old_buffer, "dirfakeuser") )
        {
            /* Show the fake username */
            sscanf(old_buffer, "%*s %*s %s", new_buffer);
            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[11]), utf8);

            /* Show fake user on - off */
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( strstr(new_buffer, "on") )
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[9]), 0);
            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[9]), 1);
        }

        if( cmplowercase(old_buffer, "dirfakegroup") )
        {
            /* Show the fake groupname */
            sscanf(old_buffer, "%*s %*s %s", new_buffer);
            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[12]), utf8);

            /* Show fake group on - off */
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( strstr(new_buffer, "on") )
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[10]), 0);
            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[10]), 1);
        }

        if( cmplowercase(old_buffer, "allowstorerestart") )
        {
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r')
                new_buffer[strlen(new_buffer)-1]='\0';

            if( strstr(old_buffer, "on") )
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[11]), 0);
            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[11]), 1);
        }

        if( cmplowercase(old_buffer, "allowretrieverestart") )
        {
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r')
                new_buffer[strlen(new_buffer)-1]='\0';

            if( strstr(old_buffer, "on") )
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[12]), 0);
            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[12]), 1);
        }

        if( cmplowercase(old_buffer, "deleteabortedstores") )
        {
            if( strstr(old_buffer, "on") )
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[13]), 0);
            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[13]), 1);
        }

        if( cmplowercase(old_buffer, "systemlog") && strlen(old_buffer) > 9 )
        {
            sprintf(new_buffer, &old_buffer[10]);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r')
                new_buffer[strlen(new_buffer)-1]='\0';

            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[8]), utf8);
        }


        if( use_tls )
        {
            if( cmplowercase(old_buffer, "tlsengine") )
            {
                if( cmplowercase(old_buffer, "on") )
                    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[14]), 0);
                else
                    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[14]), 1);
            }

            /* On(Both), Off, Data, Control, */
            if( cmplowercase(old_buffer, "tlsrequired") )
            {
                if( cmplowercase(old_buffer, "on") || cmplowercase(old_buffer, "auth+data") )
                    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[15]), 0);
                else
                if( cmplowercase(old_buffer, "off") )
                    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[15]), 1);
                else
                if( cmplowercase(old_buffer, "data") )
                    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[15]), 2);
                else
                if( cmplowercase(old_buffer, "ctrl") )
                    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[15]), 3);
            }

            if( cmplowercase(old_buffer, "tlsverifyclient") )
            {
                if( cmplowercase(old_buffer, "on") )
                    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[16]), 0);
                else
                    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[16]), 1);
            }

            if( cmplowercase(old_buffer, "tlsprotocol") )
            {
                sprintf(new_buffer, "%s", &old_buffer[12]);
                if( new_buffer[strlen(new_buffer)-1]=='\n'
                ||  new_buffer[strlen(new_buffer)-1]=='\r')
                    new_buffer[strlen(new_buffer)-1]='\0';

                utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[13]), utf8);
            }
            if( cmplowercase(old_buffer, "tlslog") )
            {
                sprintf(new_buffer, &old_buffer[7]);
                if( new_buffer[strlen(new_buffer)-1]=='\n'
                ||  new_buffer[strlen(new_buffer)-1]=='\r')
                    new_buffer[strlen(new_buffer)-1]='\0';

                utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[14]), utf8);
            }
        }


        if( use_ratio )
        {               /* Ratio on or off ... SaveRatios "ave" */
            if( cmplowercase(old_buffer, "ratios ") && ! cmplowercase(old_buffer, "ave") )
            {
                if( cmplowercase(old_buffer, "on") )
                    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[17]), 0);
                else
                    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[17]), 1);
            }

            /* Save Ratios on or off */
            if( cmplowercase(old_buffer, "saveratios ") )
            {
                if( cmplowercase(old_buffer, "on") )
                    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[18]), 0);
                else
                    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[18]), 1);
            }
        }               /* If use_ratio end */


        /* FIX QUOTA */
        if( use_quota )
        {


        }

    }
    fclose(fp);
    free(old_buffer);
    free(new_buffer);
    free(address_buffer);
    free(port_buffer);


    /* Set MasqueradeAddress to off if not found */
    if( ! found_maskaddr )
        gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[1]), 1);



    /* Add settings.conf values to the server settings. */
    settings_conf = g_strdup_printf("%s/settings.conf", GP_APPCONFDIR);
    if((fp = fopen(settings_conf, "r")) == NULL)
    {
        g_free(settings_conf);
        return;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    old_buffer = allocate(conf_size + 1);
    new_buffer = allocate(conf_size + 1);

    if( conf_size > 1 )
    while(fgets(old_buffer, conf_size, fp) != NULL)
    {
        if( cmplowercase(old_buffer, "certificate_directory") && use_tls )
        {
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r')
                new_buffer[strlen(new_buffer)-1]='\0';

            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[15]), utf8);
        }

        if( cmplowercase(old_buffer, "random_username_length") )
        {
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r')
                new_buffer[strlen(new_buffer)-1]='\0';

            if( chars_are_digits(new_buffer) )
            {
                val = atoi(new_buffer);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[7]), val);
            }
        }
        if( cmplowercase(old_buffer, "random_password_length") )
        {
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r')
                new_buffer[strlen(new_buffer)-1]='\0';

            if( chars_are_digits(new_buffer) )
            {
                val = atoi(new_buffer);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[8]), val);
            }
        }
        if( cmplowercase(old_buffer, "randomize_case") )
        {
            if( cmplowercase(old_buffer, "upper") )
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[7]), 0);
            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[7]), 1);
        }
        if( cmplowercase(old_buffer, "html_path") )
        {
            sscanf(old_buffer, "%*s %s", new_buffer);
            if( new_buffer[strlen(new_buffer)-1]=='\n'
            ||  new_buffer[strlen(new_buffer)-1]=='\r')
                new_buffer[strlen(new_buffer)-1]='\0';

            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[6]), utf8);
        }


        /* Authentication type */

        /* Remove this transitional cmplowercase "virtual_users" in version 0.5.2 or so */
        if( cmplowercase(old_buffer, "auth_type")
        ||  cmplowercase(old_buffer, "virtual_users") )
        {
            /* Free paths first */
            g_free(GP_PASSWD_BUF);
            g_free(GP_SHADOW_BUF);
            g_free(GP_GROUP_BUF);
            g_free(GP_GSHADOW_BUF);

            /* Block the combobox changed signal so it doesnt emit */
            g_signal_handler_block(GTK_WIDGET(widgets->server_set_combo[8]), auth_changed_signal);

            /* System users */
            if( cmplowercase(old_buffer, "system_users" ) )
            {
                /* System users is selected. */
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[8]), 0);

                GP_PASSWD_BUF  = g_strdup_printf("%s", GP_PASSWD);
                GP_SHADOW_BUF  = g_strdup_printf("%s", GP_SHADOW);
                GP_GROUP_BUF   = g_strdup_printf("%s", GP_GROUP);
                GP_GSHADOW_BUF = g_strdup_printf("%s", GP_GSHADOW);
            }


            /* Virtual users.   Remove in version 0.5.2 or later */
            if( cmplowercase(old_buffer, "virtual_users on") )
            {
                /* Virtual users is selected. */
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[8]), 1);

                path = g_strdup_printf("%s/users", GP_APPCONFDIR);
                make_dir_chmod(path, "0700");
                g_free(path);

                GP_PASSWD_BUF  = g_strdup_printf("%s/users/proftpd.passwd", GP_APPCONFDIR);
                GP_GROUP_BUF   = g_strdup_printf("%s/users/proftpd.group", GP_APPCONFDIR);
                GP_SHADOW_BUF  = g_strdup_printf("%s/users/shadow_NOT_USED", GP_APPCONFDIR);
                GP_GSHADOW_BUF = g_strdup_printf("%s/users/gshadow_NOT_USED", GP_APPCONFDIR);
            }
            else /* Remove in version 0.5.2 or later */
            if( cmplowercase(old_buffer, "virtual_users off" ) )
            {
                /* Virtual users is not selected. */
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[8]), 0);

                GP_PASSWD_BUF  = g_strdup_printf("%s", GP_PASSWD);
                GP_SHADOW_BUF  = g_strdup_printf("%s", GP_SHADOW);
                GP_GROUP_BUF   = g_strdup_printf("%s", GP_GROUP);
                GP_GSHADOW_BUF = g_strdup_printf("%s", GP_GSHADOW);
            }
            else /* Virtual users. Keep this "virtual_users" until after version 0.5.2 */
            if( cmplowercase(old_buffer, "virtual_users" ) )
            {
                /* Virtual users is selected. */
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[8]), 1);

                path = g_strdup_printf("%s/users", GP_APPCONFDIR);
                make_dir_chmod(path, "0700");
                g_free(path);

                GP_PASSWD_BUF  = g_strdup_printf("%s/users/proftpd.passwd", GP_APPCONFDIR);
                GP_GROUP_BUF   = g_strdup_printf("%s/users/proftpd.group", GP_APPCONFDIR);
                GP_SHADOW_BUF  = g_strdup_printf("%s/users/shadow_NOT_USED", GP_APPCONFDIR);
                GP_GSHADOW_BUF = g_strdup_printf("%s/users/gshadow_NOT_USED", GP_APPCONFDIR);
            }


            /* MySQL Users */
            if( cmplowercase(old_buffer, "mysql_users" ) )
            {
                /* MySQL users is selected. */
                gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[8]), 2);

                /* Standard gadmin MySQL DB settings. Not used. */
                GP_PASSWD_BUF  = g_strdup_printf("usertable_NOT_USED");   /* Passwd  */
                GP_GROUP_BUF   = g_strdup_printf("grouptable_NOT_USED");  /* Group   */
                GP_SHADOW_BUF  = g_strdup_printf("gadmintools_NOT_USED"); /* DB Name */
                GP_GSHADOW_BUF = g_strdup_printf("%s/users/gshadow_NOT_USED", GP_APPCONFDIR);
            }


            /* FIX: LDAP Users */
            if( cmplowercase(old_buffer, "ldap_users" ) )
            {
                /* LDAP users is selected. */
//            gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->server_set_combo[8]), 3);

                /* Standard gadmin DB settings */
/*
                GP_PASSWD_BUF  = g_strdup_printf("gadmin_users");
                GP_GROUP_BUF   = g_strdup_printf("gadmin_groups");
                GP_SHADOW_BUF  = g_strdup_printf("gadmintools");
                GP_GSHADOW_BUF = g_strdup_printf("%s/users/gshadow_NOT_USED", GP_APPCONFDIR);
*/
            }

            /* UnBlock the combobox changed signal so it emits again */
            g_signal_handler_unblock(GTK_WIDGET(widgets->server_set_combo[8]), auth_changed_signal);

        }
    }
    fclose(fp);
    free(old_buffer);
    free(new_buffer);
    g_free(settings_conf);


    /* If we have the TLS module then list the values for it from gadmin-proftpd-openssl.conf. */
    /* These are generated by gadmin-proftpd so theres no need to use cmplowercase */
    if( use_tls )
    {
        openssl_conf = g_strdup_printf("%s/certs/gadmin-proftpd-openssl.conf", GP_APPCONFDIR);
        if((fp = fopen(openssl_conf, "r")) == NULL)
        {
            g_free(openssl_conf);
            return;
        }
        fseek(fp, 0, SEEK_END);
        conf_size = ftell(fp);
        rewind(fp);

        old_buffer = allocate(conf_size + 1);
        new_buffer = allocate(conf_size + 1);

        if( conf_size > 1 )
        while(fgets(old_buffer, conf_size, fp) != NULL)
        {
            /* Server address */
            if( strstr(old_buffer, "commonName = ") )
            {
                for(x = 0; old_buffer[x] != '\0'; x++)
                {
                    if( x >= 2 && old_buffer[x-2]=='=' )
                    {
                        snprintf(new_buffer, old_buffer[x] + x, old_buffer + x);
                        new_buffer[strlen(new_buffer)-1]='\0';
                        utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[16]), utf8);
                        break;
                    }
                }
            }
            /* Email address */
            if( strstr(old_buffer, "emailAddress = ") )
            {
                for(x = 0; old_buffer[x] != '\0'; x++)
                {
                    if( x >= 2 && old_buffer[x-2]=='=' )
                    {
                        snprintf(new_buffer, old_buffer[x] + x, old_buffer + x);
                        new_buffer[strlen(new_buffer)-1]='\0';
                        utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[17]), utf8);
                        break;
                    }
                }
            }
            /* State or province */
            if( strstr(old_buffer, "stateOrProvinceName = ") )
            {
                for(x = 0; old_buffer[x] != '\0'; x++)
                {
                    if( x >= 2 && old_buffer[x-2]=='=' )
                    {
                        snprintf(new_buffer, old_buffer[x] + x, old_buffer + x);
                        new_buffer[strlen(new_buffer)-1] = '\0';
                        utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[18]), utf8);
                        break;
                    }
                }
            }
            /* City or town */
            if( strstr(old_buffer, "localityName = ") )
            {
                for(x = 0; old_buffer[x] != '\0'; x++)
                {
                    if(x >= 2 && old_buffer[x-2]=='=')
                    {
                        snprintf(new_buffer, old_buffer[x] + x, old_buffer + x);
                        new_buffer[strlen(new_buffer)-1]='\0';
                        utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[19]), utf8);
                        break;
                    }
                }
            }
            /* Organization */
            if( strstr(old_buffer, "organizationName = ") )
            {
                for(x = 0; old_buffer[x] != '\0'; x++)
                {
                    if( x >= 2 && old_buffer[x-2]=='=' )
                    {
                        snprintf(new_buffer, old_buffer[x] + x, old_buffer + x);
                        new_buffer[strlen(new_buffer)-1]='\0';
                        utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[20]), utf8);
                        break;
                    }
                }
            }
            /* Organizational unit */
            if( strstr(old_buffer, "organizationalUnitName = ") )
            {
                for(x = 0; old_buffer[x] != '\0'; x++)
                {
                    if( x >= 2 && old_buffer[x-2]=='=' )
                    {
                        snprintf(new_buffer, old_buffer[x] + x, old_buffer + x);
                        new_buffer[strlen(new_buffer)-1]='\0';
                        utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[21]), utf8);
                        break;
                    }
                }
            }
            /* Default certificate bits */
            if( strstr(old_buffer, "default_bits = ") )
            {
                for(x = 0; old_buffer[x] != '\0'; x++)
                {
                    if( x >= 2 && old_buffer[x-2]=='=' )
                    {
                        snprintf(new_buffer, old_buffer[x] + x, old_buffer + x);
                        new_buffer[strlen(new_buffer) - 1] = '\0';
                        if(chars_are_digits(new_buffer))
                        {
                            val = atoi(new_buffer);
                            gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[11]), val);
                            break;
                        }
                    }
                }
            }
            /* Default days valid */
            if( strstr(old_buffer, "default_days = ") )
            {
                for(x = 0; old_buffer[x] != '\0'; x++)
                {
                    if( x >= 2 && old_buffer[x-2]=='=' )
                    {
                        snprintf(new_buffer, old_buffer[x] + x, old_buffer + x);
                        new_buffer[strlen(new_buffer)-1]='\0';
                        if(chars_are_digits(new_buffer))
                        {
                            val = atoi(new_buffer);
                            gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[12]), val);
                            break;
                        }
                    }
                }
            }
            /* Country code */
            if( strstr(old_buffer, "countryName = ") )
            {
                for(x = 0; old_buffer[x] != '\0'; x++)
                {
                    if(x >= 2 && old_buffer[x-2]=='=')
                    {
                        snprintf(new_buffer, old_buffer[x] + x, old_buffer + x);
                        new_buffer[strlen(new_buffer)-1]='\0';
                        utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[24]), utf8);
                        break;
                    }
                }
            }
        }
        fclose(fp);
        g_free(openssl_conf);
        free(new_buffer);
        free(old_buffer);
    }

    if( utf8 != NULL )
        g_free(utf8);
}
