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
#include "gtk/gtk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gettext.h"
#include "widgets.h"
#include "allocate.h"
#include "show_info.h"
//#include "populate_conf_tab.h"
#include "apply_server_settings.h"
#include "functions.h"
#include "mysql_functions.h"


/* Block or allow pam logins for system users 
   1 to add restriction, 0 to remove the restriction. */
void restrict_pam_conf(int add_del)
{
    FILE *fp;
    long file_size = 0;
    char *line, *new_conf;
    gchar *pam_module_off;

    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);
    new_conf = allocate(file_size + 1024);

    pam_module_off = g_strconcat("\n",
            "<IfModule mod_auth_pam.c>\n",
            "    AuthPAM off\n",
            "</IfModule>\n\n",
            NULL);

    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        /* Remove the pam restriction */
        if( line != NULL
        &&  cmplowercase(line, "<ifmodule mod_auth_pam.c>")
        &&  add_del == 0 )
        {
            while(fgets(line, file_size, fp) != NULL)
                if( line != NULL && cmplowercase(line, "</ifmodule>") )
                    break;
        }
        else /* Add pam restriction before the tls module */
        if( line != NULL
        &&  cmplowercase(line, "<ifmodule mod_tls.c>")
        &&  add_del == 1 )
        {
            strcat(new_conf, pam_module_off);
            strcat(new_conf, line);
        }
        else
            strcat(new_conf, line);
    }
    fclose(fp);
    free(line);
    g_free(pam_module_off);

    /* Write the new conf */
    if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
    {
        free(new_conf);
        return;
    }
    fputs(new_conf, fp);
    fclose(fp);
    free(new_conf);
}


/* Add or remove AuthUserFile and AuthGroupFile directives.
   1 to add, 0 to remove the directives. */
void virtual_user_directives(int add_del)
{
    FILE *fp;
    long file_size = 0;
    char *line, *new_conf;
    gchar *directives;
    gchar *auth_user_file, *auth_group_file;

    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);
    new_conf = allocate(file_size + 1024);

    auth_user_file = g_strdup_printf("%s/users/proftpd.passwd", GP_APPCONFDIR);
    auth_group_file = g_strdup_printf("%s/users/proftpd.group", GP_APPCONFDIR);

    directives = g_strconcat("\n",
        "AuthUserFile ", auth_user_file, "\n",
        "AuthGroupFile ", auth_group_file, "\n\n",
        NULL);

    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        /* Remove the directives */
        if( add_del == 0 && (cmplowercase(line, "authuserfile ") || cmplowercase(line, "authgroupfile ")) )
        {
            continue;
        }
        else /* Add the directives before the tls module. FIX: May not exist. */
        if( add_del == 1 && strstr(line, "<IfModule mod_tls.c>") )
        {
            strcat(new_conf, directives);
            strcat(new_conf, line);
        }
        else
            strcat(new_conf, line);
    }
    fclose(fp);
    free(line);
    g_free(directives);

    g_free(auth_user_file);
    g_free(auth_group_file);

    /* Write the new conf */
    if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
    {
        free(new_conf);
        return;
    }
    fputs(new_conf, fp);
    fclose(fp);
    free(new_conf);
}


/* Add or remove MySQL directives.
   1 to add, 0 to remove the directives. */
void mysql_user_directives(int add_del, struct w *widgets)
{
    /* Add directives is called from mysql_window in mysql_functions.c */
    FILE *fp;
    long file_size = 0;
    char *line, *new_conf;
    const gchar *server, *port, *user, *pass, *database, *usertable, *grouptable;
    gchar *directives = NULL;

    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);
    new_conf = allocate(file_size + 2048);

    if( add_del == 1 )
    {
        server = gtk_entry_get_text(GTK_ENTRY(widgets->remote_server_entry));
        port = gtk_entry_get_text(GTK_ENTRY(widgets->remote_port_entry));
        user = gtk_entry_get_text(GTK_ENTRY(widgets->remote_user_entry));
        pass = gtk_entry_get_text(GTK_ENTRY(widgets->remote_password_entry));
        database = gtk_entry_get_text(GTK_ENTRY(widgets->remote_database_entry));
        usertable = gtk_entry_get_text(GTK_ENTRY(widgets->remote_usertable_entry));
        grouptable = gtk_entry_get_text(GTK_ENTRY(widgets->remote_grouptable_entry));

        directives = g_strconcat("\n",
            "<IfModule mod_sql.c>\n",
            "    AuthOrder mod_sql.c\n",
            "    SQLAuthTypes Crypt Backend\n",
            "    SQLAuthenticate users\n",
            "    SQLConnectInfo ", database, "@", server, ":", port, " ", user, " ", pass, "\n",
            "    SQLUserInfo ", usertable, " username password uid gid homedir shell\n",
            "    SQLGroupInfo ", grouptable, " groupname gid members\n",
            "    <IfModule mod_quotatab_sql.c>\n",
            "        SQLNamedQuery get-quota-limit SELECT \"* FROM quotalimits WHERE name = '%{0}' AND quota_type = '%{1}'\"\n",
            "        SQLNamedQuery get-quota-tally SELECT \"* FROM quotatallies WHERE name = '%{0}' AND quota_type = '%{1}'\"\n",
            "        SQLNamedQuery update-quota-tally UPDATE \"bytes_in_used = bytes_in_used + %{0}, bytes_out_used = bytes_out_used + %{1}, bytes_xfer_used = bytes_xfer_used + %{2}, files_in_used = files_in_used + %{3}, files_out_used = files_out_used + %{4}, files_xfer_used = files_xfer_used + %{5} WHERE name = '%{6}' AND quota_type = '%{7}'\" quotatallies\n",
            "        SQLNamedQuery insert-quota-tally INSERT \"%{0}, %{1}, %{2}, %{3}, %{4}, %{5}, %{6}, %{7}\" quotatallies\n",
            "        QuotaLock /var/lock/ftpd.quotatab.lock\n",
            "        QuotaLimitTable sql:/get-quota-limit\n",
            "        QuotaTallyTable sql:/get-quota-tally/update-quota-tally/insert-quota-tally\n",
            "    </IfModule>\n",
            "</IfModule>\n\n",
        NULL);
    }


    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        /* Remove the mysql directives and both closing ifmodule lines. */
        if( add_del == 0 && cmplowercase(line, "<ifmodule mod_sql.c>") )
        {
            while(fgets(line, file_size, fp) != NULL)
                if( cmplowercase(line, "</ifmodule>") )
                    break;
            while(fgets(line, file_size, fp) != NULL)
                if( cmplowercase(line, "</ifmodule>") )
                    break;
                else
                    break;
        }
        else /* Add directives before the tls module. */
        if( add_del == 1 && cmplowercase(line, "<ifmodule mod_tls.c>") )
        {
            strcat(new_conf, directives);
            strcat(new_conf, line);
        }
        else
            strcat(new_conf, line);
    }
    fclose(fp);
    free(line);
    if( directives != NULL )
      g_free(directives);

    /* Write the new conf */
    if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
    {
        free(new_conf);
        return;
    }
    fputs(new_conf, fp);
    fclose(fp);
    free(new_conf);
}


void user_auth_changed(GtkComboBox *combobutton, struct w *widgets)
{
    gchar *info;
    gint active_index = 0;

    active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[8]));
    if( active_index == 0 )
    {
        /* System users */
        info = g_strconcat("\n",
            _("\t\tSystem user management activated.\n\n"),
        NULL);
        show_info(info);
        g_free(info);

        /* Remove PAM Authentication restriction. */
        restrict_pam_conf(0);

        /* Remove AuthUserFile and AuthGroupFile statements. */
        virtual_user_directives(0);

        /* Remove MySQL directives */
        mysql_user_directives(0, widgets);

        // FIX: ldap_user_directives(0);

          apply_server_settings(widgets);
    }
    else
    if( active_index == 1 )
    {
        /* Virtual users */
        info = g_strconcat("\n",
            _("\t\tVirtual user management activated.\n\n"),
            _("Any system users you have created will no longer be able to log in.\n"),
            _("If you also want to let system users login then remove the newly\n"),
            _("added AuthPAM off module section in the configuration tab.\n\n"),
            NULL);
        show_info(info);
        g_free(info);

        /* Remove PAM Authentication restriction. */
        restrict_pam_conf(0);
        /* Remove AuthUserFile and AuthGroupFile statements. */
        virtual_user_directives(0);
        /* Remove MySQL directives */
        mysql_user_directives(0, widgets);

        // FIX: ldap_user_directives(0);

        /* Restrict PAM authentications. */
        restrict_pam_conf(1);
        /* Add virtual AuthUserFile and AuthGroupFile statements . */
        virtual_user_directives(1);

        apply_server_settings(widgets);
    }
    else
    if( active_index == 2 )
    {
        /* MySQL users */
        info = g_strconcat("\n",
            _("\t\tMySQL user management activated.\n\n"),

            _("Keep this window open until your MySQL setup is working.\n\n"),

            _("Any system users you have created will no longer be able to log in.\n"),
            _("If you also want to let system users login then remove the newly\n"),
            _("added AuthPAM off module section in the configuration tab.\n\n"),

            _("These lines must exist and be uncommented in the configuration tab:\n"),
            _("LoadModule mod_sql.c\n"),
            _("LoadModule mod_sql_passwd.c\n"),
            _("LoadModule mod_sql_mysql.c\n\n"),

            _("Redhat/Fedora needs: \"yum install proftpd-mysql\"\n"),
            _("Debian/Ubuntu needs: \"apt-get install proftpd-mod-mysql\"\n\n"),

            _("You need to know or set the MySQL server password. Set a new password like this:\n"),
            _("mysqladmin -p password TYPE-NEW-PASSWORD-HERE --user=root --host=127.0.0.1 --port=3306\n\n"),

            _("--- Remote databases must allow remote access: ---\n\n"),

            _("Login and grant access on the remote MySQL server:\n"),
            _("mysql -p (Press enter and type password).\n"),
            _("GRANT ALL ON *.* to root@'*.*' IDENTIFIED BY 'TYPE-ROOT-PASSWORD-HERE';\n"),
            _("(This grants access for root to the database from anywhere).\n\n"),

            _("Set the remote MySQL server to listen to all network interfaces:\n"),
            _("Edit: Fedora: /etc/my.cnf or Debian: /etc/mysql/my.cnf\n"),
            _("Change: \"bind-address = 127.0.0.1\" to \"bind-address = 0.0.0.0\"\n"),
            _("(This means listen for connections on all network interfaces).\n\n"),

            _("Restart the MySQL server when done. Press apply in the MySQL setup window.\n\n"),

            _("If it fails the first time then switch to system users\n"),
            _("and back to MySQL users to try again.\n\n"),

            _("Extra information on setting up MySQL in a more secure way:\n"),
            _("GRANT ALL ON *.* can be set to GRANT ALL ON DB_NAME.* ...\n"),
            _("The root user can be set to any other user that has the required privileges:\n"),
            _("GRANT ALL PRIVILEGES ON DB_NAME.* TO DB_USER@'localhost' IDENTIFIED BY 'DB_USER_PASSWORD';\n\n"),
        NULL);
        show_info(info);
        g_free(info);

        /* Remove PAM Authentication restriction. */
        restrict_pam_conf(0);
        /* Remove AuthUserFile and AuthGroupFile statements. */
        virtual_user_directives(0);

        /* Restrict PAM authentications. */
        restrict_pam_conf(1);

        /* Show a window with mysql settings. Located in mysql_functions.c */
        show_mysql_window(widgets);
        /* This also runs apply_server_settings() and
           removes the old mysql directives first. */

        // FIX: ldap_user_directives(0);
    }
    else
    if( active_index == 3 )
    {
        /* LDAP users */

        info = g_strconcat("\n",
            _("\t\tLDAP user management activated.\n\n"),
            _("Any system users you have created will no longer be able to log in.\n"),
            _("If you also want to let system users login then remove the newly\n"),
            _("added AuthPAM off module section in the configuration tab.\n\n"),
            NULL);
        show_info(info);
        g_free(info);

        /* Remove PAM Authentication restriction. */
        restrict_pam_conf(0);
        /* Remove AuthUserFile and AuthGroupFile statements. */
        virtual_user_directives(0);

        /* Restrict PAM authentications. */
        restrict_pam_conf(1);

        /* Remove MySQL directives. */
        mysql_user_directives(0, widgets);

        // FIX: show_ldap_window(widgets);
    }
    else
    {
        printf("create_server_settings.c: User auth changed - out of range.\n");
    }
}
