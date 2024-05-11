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
#include "add_user.h"
#include "functions.h"
#include "dir_treeview_funcs.h"
#include "system_defines.h"
#include "populate_users.h"
#include "populate_user_settings.h"
#include "populate_conf_tab.h"
#include "select_first_user.h"
#include "reread_conf.h"

#include "mysql_functions.h"
#include "apply_user.h"

#ifdef USE_DARWIN
#include "osx_functions.h"
#endif

extern char global_server_address[1024];
extern char global_server_port[1024];
extern char global_server_type[1024];

extern int use_ratio;
extern int use_quota;

extern long num_rows;
extern int row_pos;

/* Used globally with dir_treeview_funcs.c */
char *user_profile;

/* Set in dir_treeview_funcs.c */
extern gchar *homedir;

/* Declared in gadmin-proftpd.c and set in treeview_dir_funcs */
extern int global_dir_error;

/* Temporary, for option deprecations in proftpd */
extern char global_version[1024];

extern gchar *GP_PASSWD_BUF;
extern gchar *GP_GROUP_BUF;



/* Get the highest virtual user number +1 */
long get_virtual_hnumber()
{
    FILE *fp;
    char *line, *tmp;
    gchar *info;
    long file_size = 0;
    long count = 0, tmpnumber = 0, hnumber = 0;

    if((fp = fopen(GP_PASSWD_BUF, "r")) == NULL)
    {
        info = g_strdup_printf(_("Failed reading file: %s\n"), GP_PASSWD_BUF);
        show_info(info);
        g_free(info);

        return 1001;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);

    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        if( strlen(line) < 3 )
            continue;

        /* Get value from 3:rd or 4:th section */
        tmp = strtok(line, ":");

        count = 0;
        tmpnumber = 0;
        while(tmp != NULL)
        {
            count++;

            if(chars_are_digits(tmp) && (count == 3 || count == 4))
            {
                tmpnumber = atoi(tmp);
                if(tmpnumber > hnumber)
                    hnumber = tmpnumber + 1;
            }
            tmp = strtok(NULL, ":");
        }
    }
    fclose(fp);

    if(hnumber < 1000)
        hnumber = 1000;

    return hnumber + 1;
}


void add_user(struct w *widgets)
{
    /* Adds a new user to the selected server */
    FILE *fp;
    long hnumber = 0, file_size = 0;
    long profile_size = 0;
    char *line, *new_buffer;
    char *address_buffer = NULL, *port_buffer = NULL;
    const char *encrypted_pass;
    int length = 0, limit_access = 0, user_added = 0;
    int found_server = 0, standard_server = 0;
    char **connect_args;
    char **user_args;
    int cmd_num = 0;
    gchar *utf8 = NULL;
    gchar *info, *cmd, *restricted_dir = NULL;
    gint active_index = 0;

    G_CONST_RETURN gchar *username;
    G_CONST_RETURN gchar *password;
    G_CONST_RETURN gchar *group;
    G_CONST_RETURN gchar *comment;
    G_CONST_RETURN gchar *shell;
    G_CONST_RETURN gchar *login_from;
    G_CONST_RETURN gchar *max_logins;
    G_CONST_RETURN gchar *br = NULL, *brc = NULL, *fr = NULL, *frc = NULL;

    username = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_entry[0]));
    password = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_entry[1]));
    group = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_entry[2]));
    comment = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_entry[3]));
    /* Shell is a gtk_combo_box_entry_new_text */
    
    //shell = gtk_entry_get_text(GTK_ENTRY(gtk_container_get_children(GTK_BIN(widgets->user_set_combo[0]))));

    GtkTreeIter iter;
    GtkListStore *store;
    
    store = GTK_LIST_STORE(gtk_combo_box_get_model(widgets->user_set_combo[0]));
    gtk_combo_box_get_active_iter(widgets->user_set_combo[0], &iter);
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 0, &shell, -1);



    /* If the shell is false it will add the users directory as /dev/null and the specified false shell.
     * If the shell is real it will add the users directory as /USERSHOME/username and the specified real shell. 
     * The ftp directory will be located where specified in the application.
     * This is done so that no user gets sshd/etc access unless the admin wants to.
     */

    login_from = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_entry[5]));
    max_logins = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_spinbutton[0]));

    /* Ratios */
    if( use_ratio )
    {
        br  = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_spinbutton[1]));
        brc = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_spinbutton[2]));
        fr  = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_spinbutton[3]));
        frc = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_spinbutton[4]));
    }

    /* Username must be atleast 1 char long */
    length = strlen(username);
    if( length == 0 )
    {
        info = g_strdup_printf(_("You must specify a username.\n"));
        show_info(info);
        g_free(info);
        return;
    }
    /* Usernames can not begin with a number */
    if( username[0] == '0' || username[0] == '1' || username[0] == '2' || username[0] == '3' || username[0] == '4'
    ||  username[0] == '5' || username[0] == '6' || username[0] == '7' || username[0] == '8' || username[0] == '9')
    {
        info = g_strdup_printf(_("Failed adding user: %s\nA user name can not begin with a number.\n"), username);
        show_info(info);
        g_free(info);
        return;
    }
    /* Username can not be root */
    if( username[0] == 'r' && username[1] == 'o' && username[2] == 'o' && username[3] == 't' && strlen(username) == 4 )
    {
        info = g_strdup_printf(_("Failed adding user: %s\nThe user root can not be added for security reasons.\n"), username);
        show_info(info);
        g_free(info);
        return;
    }

    if( strstr((char *)username, "<") || strstr((char *)username, ">") )
    {
        info = g_strdup_printf(
        _("Failed adding user: %s\nchars \"<\" and \">\" arent allowed.\n"), username);
        show_info(info);
        g_free(info);
        return;
    }

    /* Password must be atleast 6 chars long */
    length = strlen(password);
    if( length < 6 )
    {
        info = g_strdup_printf(
        _("Failed adding user: %s\nA minimum password length of 6 chars is required.\n"), username);
        show_info(info);
        g_free(info);
        return;
    }

    /* A groupname is required */
    length = strlen(group);
    if( length == 0 )
    {
        info = g_strdup_printf(_("Failed adding user: %s\nNo group specified.\n"), username);
        show_info(info);
        g_free(info);
        return;
    }

    /* Shell must be atlest 3 chars long */
    length = strlen(shell);
    if( length < 3 )
    {
        info = g_strdup_printf(_("Failed adding user: %s\nNo shell specified.\n"), username);
        show_info(info);
        g_free(info);
        return;
    }

    /* A comment is required */
    length = strlen(comment);
    if( length == 0 )
    {
		comment=g_strdup_printf("-");
        //info = g_strdup_printf(_("A comment is required to add a user.\n"));
        //show_info(info);
        //g_free(info);
        //return;
    }

    /* A home directory is required */
    num_rows = 0;
    gtk_tree_model_foreach(GTK_TREE_MODEL(widgets->directory_store),
                    (GtkTreeModelForeachFunc) num_rows_func, widgets);
    if( num_rows < 1 )
    {
        info = g_strdup_printf(_("Missing ftp home directory. Scroll down and add one first.\n"));
        show_info(info);
        g_free(info);
        return;
    }

    /* Set the users home dir globally */
    row_pos = 0; /* 0 = Only get the home directory */
    gtk_tree_model_foreach(GTK_TREE_MODEL(widgets->directory_store), (GtkTreeModelForeachFunc) dirs_foreach, widgets);

    /* Bad directory name or no directory at all */
    if( global_dir_error )
    {
        /* Info is shown in the foreach function */
        global_dir_error = 0;

        if( homedir != NULL )
            g_free(homedir);

        return;
    }


    /* Auth types */
    active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[8]));
    if( active_index == 0 )
    {
        /* System users */

        /* The selected shell is false, add the group and user with a false shell and home /dev/null */
        if(strstr(shell, "nologin") || strstr(shell, "false") || strstr(shell, "dev/null"))
        {
            /* All supported systems but darwin */
#ifndef USE_DARWIN

            /* Add the group if it doesnt exist */
            if( ! group_exists(group) )
            {
                cmd = g_strdup_printf("%s '%s'", ADDGROUP, group);
                if( ! run_command(cmd) )
                {
                    info = g_strdup_printf(_("Error adding group: %s\n"), group);
                    show_info(info);
                    g_free(info);
                }
                g_free(cmd);
            }

            /* Add the user to this group if it doesnt exist */
            if( ! user_exists(username) )
            {
                cmd = g_strdup_printf("%s '%s' -g '%s' -d /dev/null -c '%s' -s %s", ADDUSER, username, group, comment, shell);
                if( ! run_command(cmd) )
                {
                    info = g_strdup_printf(_("Failed adding user: %s\n"), username);
                    show_info(info);
                    g_free(info);
                }
                else
                    user_added = 1;

                g_free(cmd);
            }

#elif USE_DARWIN
            /* Add the false user using darwins niutil commands (Darwin is out of sync) */
            if( ! niutil_user_exists(username) )
            {
                if( ! niutil_useradd(username, shell) )
                {
                    info = g_strdup_printf(_("Failed adding user: %s\n"), username);
                    show_info(info);
                    g_free(info);
                }
                else
                    user_added = 1;
            }
#endif
        }
        else
        {
            /* The selected shell is not false add a real user account */
#ifndef USE_DARWIN

            /* Add the group if it doesnt exist */
            if( ! group_exists(group) )
            {
                cmd = g_strdup_printf("%s '%s'", ADDGROUP, group);
                if( ! run_command(cmd) )
                {
                    info = g_strdup_printf(_("Failed adding group: %s\n"), group);
                    show_info(info);
                    g_free(info);
                }
                g_free(cmd);
            }

            /* Add the user to this group if it doesnt exist */
            if( ! user_exists(username) )
            {
                /* Add the user with a real shell to /USERSHOME/ UserName (was: -m -s) */
                cmd = g_strdup_printf("%s '%s' -g '%s' -d '%s%s' -c '%s' -s %s", ADDUSER, username, group, USERSHOME, username, comment, shell);
                if( ! run_command(cmd) )
                {
                    info = g_strdup_printf(_("Failed adding user: %s\n"), username);
                    show_info(info);
                    g_free(info);
                }
                else
                    user_added = 1;

                g_free(cmd);
            }

#elif USE_DARWIN
            /* Add a real darwin user using the niutil commands (Darwin is out of sync) */
            if( ! niutil_user_exists(username) )
            {
                if( ! niutil_useradd(username, shell) )
                {
                    info = g_strdup_printf(_("Failed adding user: %s\n"), username);
                    show_info(info);
                    g_free(info);
                }
                else
                    user_added = 1;
            }
#endif
        }


        /* Dont add anything if we couldnt add the system user */
#ifndef USE_DARWIN
        if( ! user_exists(username) )
#elif USE_DARWIN
        if( ! niutil_user_exists(username) )
#endif
        {
            info = g_strdup_printf(
            _("The system user was not added because uppercase\nor language specific letters are not allowed.\n"));
            show_info(info);
            g_free(info);

            return;
        }

        /* Password the user if it didnt exist before */
        if( user_added )
        {
#ifndef USE_DARWIN
            password_user(username, password);
#elif USE_DARWIN
            niutil_password_user(username, password);
#endif
        }
        else
        {
            info = g_strdup_printf(
            _("The system user \"%s\" already exists.\nThe user was added to this server but the password was not changed.\n"), username);
            show_info(info);
            g_free(info);
        }
    }
    else
    if( active_index == 1 )
    {
        /* Virtual users */

        if( ! strstr(GP_PASSWD_BUF, "/users/") )
        {
            printf("add_user.c: Virtual user path must contain: .../users/...\n");
            return;
        }

        if( ! file_exists(GP_PASSWD_BUF) )
        {
            cmd = g_strdup_printf("touch %s", GP_PASSWD_BUF);
            if(!run_command(cmd))
                printf("Error running command: %s\n", cmd);
            g_free(cmd);
        }

        if( ! file_exists(GP_GROUP_BUF) )
        {
            cmd = g_strdup_printf("touch %s", GP_GROUP_BUF);
            if(!run_command(cmd))
                printf("Error running command: %s\n", cmd);
            g_free(cmd);
        }

        /* Chown /etc/gadmin-proftpd so that the server user/group has access */
        cmd = g_strdup_printf("chown %s:%s %s", SERVER_USER, SERVER_GROUP, GP_APPCONFDIR);
        if( ! run_command(cmd) )
            printf("Error running command: %s\n", cmd);
        g_free(cmd);

        /* Chown /etc/gadmin-proftpd and /etc/gadmin-proftpd/users directories */
        cmd = g_strdup_printf("chown %s:%s %s/users", SERVER_USER, SERVER_GROUP, GP_APPCONFDIR);
        if( ! run_command(cmd) )
            printf("Error running command: %s\n", cmd);
        g_free(cmd);

        /* Append a user line to proftpd.passwd
           name:$1$5GQxXfZ3$4NcgFKj7M9UxbEGZEdAH8.:1002:1002::/home/name:shell */
        encrypted_pass = encrypt_password(password);
        if( encrypted_pass == NULL )
        {
            printf("Crypt error\n");
            return;
        }

        hnumber = get_virtual_hnumber();

        info = g_strdup_printf("%s:%s:%ld:%ld:%s:%s:%s\n",
            username, encrypted_pass, hnumber, hnumber, comment, "/dev/null", shell);

        if((fp = fopen(GP_PASSWD_BUF, "a+")) == NULL)
        {
            info = g_strdup_printf(_("Failed adding user: %s\n"), username);
            show_info(info);
            g_free(info);
            return;
        }
        fputs(info, fp);
        fclose(fp);
        g_free(info);

        /* Append to proftpd.group / group:x:1001 */
        info = g_strdup_printf("%s:x:%ld\n", group, hnumber);

        if((fp = fopen(GP_GROUP_BUF, "a+")) == NULL)
        {
            info = g_strdup_printf(_("Failed adding group: %s\n"), username);
            show_info(info);
            g_free(info);
            return;
        }
        fputs(info, fp);
        fclose(fp);
        g_free(info);
    }
    else
    if( active_index == 2 )
    {
        /* MySQL users */
        encrypted_pass = encrypt_password(password);
        if( encrypted_pass == NULL )
        {
            printf("Crypt error\n");
            return;
        }
        /* The user data to add or change */
        user_args = malloc((9) * sizeof(char *));
        memset(user_args, '\0', sizeof(user_args));
        user_args[0]=(char *)username;
        user_args[1]=(char *)encrypted_pass;
        user_args[2]="1000"; /* UID Standard. Autoincremented */
        user_args[3]="1000"; /* GID Standard. Autoincremented */
        user_args[4]=(char *)homedir;
        user_args[5]=(char *)shell;
        user_args[6]=(char *)comment;
        user_args[7]=(char *)group;
        user_args[8]=NULL;

        connect_args = mysql_get_connect_args();

        /* Add or change a MySQL user according to user_args
           This also adds the group if missing with
           the same gid as the uid. */
        cmd_num = 2;
        if( ! mysql_connect(connect_args, widgets, cmd_num, user_args) )
            printf("Error: Could not add MySQL user.\n");

        free(user_args);
        mysql_free_connect_args(connect_args);
    }
    else
    {
        /* LDAP users */
        printf("add_user.c: Selection out of range.\n");
        return;
    }


    /* The user account has been added */

    /* Dont add duplicate profiles for database users. */
    if( user_exists_in_selected_server(widgets, (gchar *)username) )
    {
        return;
    }


    /* Add a new user profile... */


    /* Setup the users profile and create its directories */
    /* (Global) Statics + entries + (number of rows * dirlen + APPE STOR STOU etc) */
    profile_size = 16384 + 1400 + (num_rows * 16384);
    /* Allocate the user profile */
    user_profile = allocate(profile_size + 1);

    /* If ratios is used it needs a restricted toplevel directory and
       a block all .ftpaccess file in this toplevel directory */
    if( use_ratio )
    {
        /* Make the restricted directory under each users chroot directory */
        restricted_dir = g_strdup_printf("%s/%s", homedir, "restricted");
        make_dir_chmod((gchar *) restricted_dir, "0777");   /* Must be like this */

        /* Add a block all .ftpaccess file to this directory */
        cmd = g_strdup_printf("echo \"DenyAll\n\" > '%s/.ftpaccess'", restricted_dir);
        if( ! run_command(cmd) )
        {
            printf("Error creating .ftpaccess file here: %s/.ftpaccess\n", restricted_dir);
            /* Fixme, popup */
        }
        g_free(cmd);

        /* Add the ratio files */
        cmd = g_strdup_printf("touch '%s/proftpd_ratios' '%s/proftpd_ratios_temp'", restricted_dir, restricted_dir);
        if( ! run_command(cmd) )
        {
            printf("Error creating the ratio files here: %s\n", restricted_dir);
            /* Fixme, popup */
        }
        g_free(cmd);

        /* Chmod a+rw on the ratio files (must be a+rw) */
        cmd = g_strdup_printf("chmod a+rw '%s/proftpd_ratios' '%s/proftpd_ratios_temp'", restricted_dir, restricted_dir);
        if( ! run_command(cmd) )
        {
            printf("Error chmodding the ratio files here: %s\n", restricted_dir);
            /* Fixme, popup */
        }
        g_free(cmd);

        /* Chown ratio files to SERVERUSER:SERVERGROUP */
        cmd = g_strdup_printf("chown %s:%s '%s/proftpd_ratios' '%s/proftpd_ratios_temp'", SERVER_USER, SERVER_GROUP, restricted_dir, restricted_dir);
        if( ! run_command(cmd) )
        {
            printf("Error chmodding the ratio files here: %s\n", restricted_dir);
            /* Fixme, popup */
        }
        g_free(cmd);

        g_free(restricted_dir);
    }

    /* The users configuration profile */
    strcpy(user_profile, "\n<Anonymous ");
    strcat(user_profile, homedir);
    strcat(user_profile, ">\n");
    strcat(user_profile, "User ");
    strcat(user_profile, username);
    strcat(user_profile, "\nGroup ");
    strcat(user_profile, group);
    strcat(user_profile, "\n");

    /* Require password */
    if( ! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[1])) )
        strcat(user_profile, "AnonRequirePassword off\n");
    else
        strcat(user_profile, "AnonRequirePassword on\n");

    /* Maximum logins */
    strcat(user_profile, "MaxClients ");
    strcat(user_profile, max_logins);
    strcat(user_profile, " \"");
    strcat(user_profile, "The server is full, hosting %m users");
    strcat(user_profile, "\"\n");
    strcat(user_profile, "DisplayLogin welcome.msg\n");
    strcat(user_profile, "DisplayChdir .msg\n");

    /* Ratio Module has been found (reversed br/fr order in the gui) */
    if( use_ratio )
    {
        /* Ratios have been turned on in global settings */
        active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[17]));
        if( active_index == 0 )
        {
            strcat(user_profile, "UserRatio ");
            strcat(user_profile, username);
            strcat(user_profile, " ");
            strcat(user_profile, fr);
            strcat(user_profile, " ");
            strcat(user_profile, frc);
            strcat(user_profile, " ");
            strcat(user_profile, br);
            strcat(user_profile, " ");
            strcat(user_profile, brc);
            strcat(user_profile, "\n");
        }
    }

    /* Logins from, if nothing is specified we allow everything */
    if( login_from == NULL || strlen(login_from) < 3 )
    {
        strcat(user_profile, "<Limit LOGIN>\n");
        strcat(user_profile, "Allow from all\nDeny from all\n");
        strcat(user_profile, "</Limit>\n");
    }
    else
    {
        strcat(user_profile, "<Limit LOGIN>\n");
        strcat(user_profile, "Allow from ");
        strcat(user_profile, login_from);
        strcat(user_profile, "\nDeny from all\n");
        strcat(user_profile, "</Limit>\n");
    }


    /* Append limits to the user profile */
    append_limit_cmds(); /* Also adds AllowOverwrite on/off */


    row_pos = 1; /* Append and create the rest of the directories */
    gtk_tree_model_foreach(GTK_TREE_MODEL(widgets->directory_store),
                           (GtkTreeModelForeachFunc) dirs_foreach, widgets);

    strcat(user_profile, "</Anonymous>\n\n");

    if( homedir != NULL )
        g_free(homedir);

    /* Add the new user settings and AllowUser to the correct server */
    found_server = 0;

    /* Standard server selected, start adding users directly */
    if( standard_server )
        found_server = 1;

    /* Add AllowUser UserName to the selected server */
    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        free(user_profile);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);
    new_buffer = allocate(file_size + 8192);
    address_buffer = allocate(1024);
    port_buffer = allocate(1024);

    if( strstr((char *)global_server_type, "Virtualhost") )
        sprintf(address_buffer, "<VirtualHost %s>\n", global_server_address);
    else
    {
        standard_server = 1;
        found_server = 1;
    }
    sprintf(port_buffer, "Port %s\n", global_server_port);


    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        strcat(new_buffer, line);

        if( ! standard_server && ! found_server && strcmp(line, address_buffer) == 0 )
        {
            /* Lets see if this is the selected server */
            while(fgets(line, file_size, fp) != NULL)
            {
                strcat(new_buffer, line);

                /* This will expect the servers port on the second line !
                 * else itll miss some vaules .. */
                if( strstr(line, "Port") && strcmp(line, port_buffer) == 0 )
                {
                    found_server = 1;
                    break;
                }

                if( strstr(line, "</Virtualhost>") )
                    break;
            }
        }

        /* Continue until we find the selected server */
        if( ! found_server )
            continue;

        /* Add AllowUser Username .. to this server only */
        if( strstr(line, "<Limit LOGIN") && found_server && ! limit_access )
        {
            strcat(new_buffer, "  AllowUser ");
            strcat(new_buffer, username);
            strcat(new_buffer, "\n");
            limit_access = 1;   /* just incase so we just change the first occurance */

            /* Add the user after </Limit> */
            while(fgets(line, file_size, fp) != NULL)
            {
                strcat(new_buffer, line);
                if( strstr(line, "</Limit") && limit_access == 1 )
                {
                    /* Only add it once */
                    limit_access = 2;
                    strcat(new_buffer, user_profile);
                }
            }
        }

        /* Add the new user settings if we have another user */
        if( strstr(line, "</Anonymous") && limit_access == 1 )
        {
            /* Only add it once */
            limit_access = 2;
            strcat(new_buffer, user_profile);
        }
    }
    fclose(fp);
    free(line);
    free(address_buffer);
    free(port_buffer);
    free(user_profile);


    /* Write the new configuration if the user profile was added.
     * Since the user could have already existed we use limit_access */
    if( limit_access )
    {
        if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
        {
            info = g_strdup_printf(
            _("Could not write the new user configuration to:\n%s\nRun gadmin-proftpd as root\n"), PROFTPD_CONF);
            show_info(info);
            g_free(info);
            free(new_buffer);
            return;
        }
        else
        {
            fputs(new_buffer, fp);
            fclose(fp);
        }
    }
    free(new_buffer);

    fix_newlines_in_conf();

    /* Update the user list and the user settings */
    populate_users(widgets);
    select_first_user(widgets);

    populate_user_settings(widgets);

    populate_conf_tab(widgets);

    /* Update the server */
    reread_conf(widgets);

    if( utf8 != NULL )
        g_free(utf8);
}
