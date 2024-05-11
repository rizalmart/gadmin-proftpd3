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
#include <unistd.h>
#include <sys/types.h>
#include "gettext.h"
#include "support.h"
#include "functions.h"
#include "create_main_window.h"
#include "gadmin_proftpd.h"
#include "widgets.h"
#include "commands.h"
#include "status_update.h"
#include "set_version.h"
#include "standard_conf.h"
#include "create_server_tab.h"
#include "create_server_settings.h"
#include "create_user_tab.h"
#include "create_user_settings.h"
#include "create_transfer_tab.h"
#include "create_disc_tab.h"
#include "create_file_tab.h"
#include "create_security_tab.h"
#include "create_conf_tab.h"
#include "populate_gadmin_proftpd.h"
#include "add_standard_users.h"
#include "show_info.h"

/* For switching between virtual or system users */
gchar *GP_PASSWD_BUF;
gchar *GP_SHADOW_BUF;
gchar *GP_GROUP_BUF;
gchar *GP_GSHADOW_BUF;

int MAX_READ_POPEN = 16384;
int activated = 0;
int use_tls = 0;
int use_ratio = 0;
int use_quota = 0;

/* Globals for adding and importing users */
long num_rows = 0;
int row_pos = 0;
gchar *homedir = NULL;

int info_window_exit_main = 0;

/* Used to stop iteration when adding directories */
int global_dir_error = 0;

char global_server_address[1024] = "";
char global_server_port[1024] = "";
char global_server_name[1024] = "";
char global_server_type[1024] = "";
char global_user_name[1024] = "";

/* Used to transform or remove deprecated options based on versions */
char global_version[1024] = "";


int main(int argc, char *argv[])
{
    FILE *fp;
    int i = 0, auto_security = 0;
    gchar *conf_path, *info = NULL, *utf8 = NULL;
    gchar *cmd, *openssl_conf, *settings_conf, *settings;

    /* Missing pam file in FC-3,4,5,6,7 proftpd rpm so its added if missing. */
    gchar *add_fedora;

#ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif

    //gtk_set_locale();
    gtk_init(&argc, &argv);

    wid *widgets = g_malloc(sizeof(wid));

    /* Initialize the conf question window */
    widgets->default_conf_question_window = NULL;

    /* Non root usage */
    if( ! getuid() == 0 )
    {
        /* For setting a different exit method in the info window */
        info_window_exit_main = 1;

        info = g_strdup_printf("You must be root to run: %s\nThis window will close in 10 seconds.\n", PACKAGE);
        show_info(info);
        g_free(info);

        /* Users can close the info window earlier then the timeout */
        for(i = 0; i < 10; i++)
        {
            while(gtk_events_pending())
                gtk_main_iteration();

            /* Set when close info window is clicked */
            if( info_window_exit_main == 2 )
                break;

            usleep(100000 * 10);
        }

        g_free(widgets);
        return 0;
    }

    /* Create the main window */
    create_main_window(widgets);

    /* Set ProFTPD Version */
    set_version(widgets);

    /* If proftpd has the mod_tls.c module we 
       create, show and use the TLS widgets */
    use_tls = using_module("mod_tls.c");

    /* If proftpd has the mod_ratio.c module we 
       create, show and use the RATIO widgets */
    use_ratio = using_module("mod_ratio.c");

    /* If proftpd has mod_quotatab.c and mod_quotatab_file.c
       modules we create, show and use the Quota widgets */
    if( using_module("mod_quotatab.c")
    &&  using_module("mod_quotatab_file.c") )
        use_quota = 1;

    /* Create the server tab */
    create_server_tab(widgets);

    /* Create the server settings */
    create_server_settings(widgets);

    /* Create the user tab */
    create_user_tab(widgets);

    /* Create the user settings */
    create_user_settings(widgets);

    /* Create the transfer tab */
    create_transfer_tab(widgets);

    /* Create the disc tab */
    create_disc_tab(widgets);

    /* Create the file tab */
    create_file_tab(widgets);

    /* Create the security tab */
    create_security_tab(widgets);

    /* Create the conf tab */
    create_conf_tab(widgets);

    /* Create the application configuration directory. */
    make_dir_chmod(GP_APPCONFDIR, "0700");

    /* Add default certificate settings if they are missing */
    openssl_conf = g_strdup_printf("%s/certs/gadmin-proftpd-openssl.conf", GP_APPCONFDIR);
    if( ! file_exists(openssl_conf) && use_tls )
    {
        info = g_strdup_printf("%s/certs", GP_APPCONFDIR);
        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[15]), info);
        g_free(info);

        info = g_strdup_printf("localhost");
        utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[16]), utf8);
        g_free(info);
        if(utf8 != NULL)
            g_free(utf8);

        info = g_strdup_printf("email@example.org");
        utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[17]), utf8);
        g_free(info);
        if(utf8 != NULL)
            g_free(utf8);

        info = g_strdup_printf(_("Country"));
        utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[18]), utf8);
        g_free(info);
        if(utf8 != NULL)
            g_free(utf8);

        info = g_strdup_printf(_("City"));
        utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[19]), utf8);
        g_free(info);
        if(utf8 != NULL)
            g_free(utf8);

        info = g_strdup_printf(_("Organization name"));
        utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[20]), utf8);
        g_free(info);
        if(utf8 != NULL)
            g_free(utf8);

        info = g_strdup_printf(_("Organization department"));
        utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[21]), utf8);
        g_free(info);
        if(utf8 != NULL)
            g_free(utf8);

        info = g_strdup_printf("EN");
        utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
        gtk_entry_set_text(GTK_ENTRY(widgets->server_set_entry[24]), utf8);
        g_free(info);
        if(utf8 != NULL)
            g_free(utf8);

        gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[11]), 1024);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->server_set_spinbutton[12]), 999999);
    }
    g_free(openssl_conf);


    conf_path = g_strdup_printf("%s", PROFTPD_CONF);

    /* Add the default gadmin-proftpd settings file if its missing */
    settings_conf = g_strdup_printf("%s/settings.conf", GP_APPCONFDIR);
    if( ! file_exists(settings_conf) )
    {
        if((fp = fopen(settings_conf, "w+")) == NULL)
        {
            info = g_strdup_printf(_("Error: Unable to create new settings file here:\n%s\n"), settings_conf);
            show_info(info);
            g_free(info);

            g_free(conf_path);
            g_free(settings_conf);
            return 0;
        }
        settings = g_strconcat("\n",
            "certificate_directory ", GP_APPCONFDIR, "/certs\n",
            "random_username_length 6\n",
            "random_password_length 6\n",
            "randomize_case lower\n",
            "useradd_homedir_path /var/ftp\n",
            "html_path ", HTML_STATISTICS, "\n",
            "welcome_name ", WELCOME_MESSAGE, "\n",
            "auth_type system_users\n", "\n",
            NULL);

        fputs(settings, fp);
        fclose(fp);
        g_free(settings);

        /* If proftpd.conf exists, switch to the new TLS settings */
        if( file_exists(conf_path) )
        {
            new_tls_module_conf();
        }
    }
    g_free(settings_conf);


    /* Use systems passwd files by default */
    GP_PASSWD_BUF  = g_strdup_printf("%s", GP_PASSWD);
    GP_SHADOW_BUF  = g_strdup_printf("%s", GP_SHADOW);
    GP_GROUP_BUF   = g_strdup_printf("%s", GP_GROUP);
    GP_GSHADOW_BUF = g_strdup_printf("%s", GP_GSHADOW);


    /* If theres no proftpd.conf, add one. */
    if( ! file_exists(conf_path) )
        add_standard_conf(widgets);
    else
        /* If proftpd.conf is bad, ask to add a new one.
           To support dynamically loaded modules we merge
           any ModulePath and LoadModule statements. */
    if( ! conf_ok(conf_path) )
        create_standard_conf_question(widgets);
    else
        /* The conf is ok, populate the entire gui */
        populate_gadmin_proftpd(widgets);

    g_free(conf_path);

    /* Make an automatic backup for this version of gadmin-proftpd */
    backup_configuration(widgets);


    /* This functions adds the required SERVER_USER AND SERVER_GROUP */
    add_standard_users(widgets);


    /* If theres no ftpusers one will be made containing all current usernames (security). */
    if((fp = fopen(GP_FTPUSERS, "r")) == NULL)
        auto_security = 1;
    else
        fclose(fp);

    if( auto_security )
    {
        cmd = g_strdup_printf("cat %s | cut -f1 -d\":\" > %s", GP_PASSWD, GP_FTPUSERS);
        if((fp = popen(cmd, "r")) == NULL)
        {
            info = g_strconcat(_("Couldnt create the ftpusers file here:\n"),
                GP_FTPUSERS,
                _("\n\nMake sure the directories exists then restart gadmin-proftpd.\n"),
                _("\nIf this security feature still fails then security will be good but not optimal.\n"),
                NULL);
            show_info(info);
            g_free(info);
        }
        else
        {
            info = g_strconcat(_("Couldnt find ftpusers here:\n"),
                GP_FTPUSERS,
                _("\n\nThe missing file was created and all users on the system was added to it.\n"),
                _("All current users are now banned from using the ftp server until they\n"),
                _("are unbanned. This in an auto security feature and its normal if this is\n"),
                _("the first time you start gadmin-proftpd.\n"),
                NULL);
            show_info(info);
            g_free(info);
        }
        g_free(cmd);
    }

    /* Fix for a missing ftp pam filein Fedora-3,4,5,6,7 proftpd rpm */
    if( file_exists("/etc/fedora-release")
    &&  file_exists("/etc/pam.d")
    &&  ! file_exists("/etc/pam.d/ftp") )
    {
        add_fedora = g_strconcat("#%PAM-1.0\n",
            "auth    required        pam_unix.so     nullok\n",
            "account required        pam_unix.so\n",
            "session required        pam_unix.so\n",
            NULL);

        if((fp = fopen("/etc/pam.d/ftp", "w+")) == NULL)
        {
            info = g_strconcat("Fedora's version of proftpd doesnt add /etc/pam.d/ftp\n",
                    "gadmin-proftpd has failed to add this file as well, this is a bug\n",
                    NULL);
            show_info(info);
            g_free(info);
        }
        else
        {
            fputs(add_fedora, fp);
            fclose(fp);

            info = g_strconcat("Fedora's version of proftpd doesnt add /etc/pam.d/ftp\n",
                                "Its been added now so that users can login\n",
                                NULL);
            show_info(info);
            g_free(info);
        }

        g_free(add_fedora);
    }

    /* Window close button (x) */
    g_signal_connect(G_OBJECT(widgets->main_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    g_timeout_add(1000, (GSourceFunc) status_update, widgets);

    gtk_main();

    /* Free virtual/system user paths */
    g_free(GP_PASSWD_BUF);
    g_free(GP_SHADOW_BUF);
    g_free(GP_GROUP_BUF);
    g_free(GP_GSHADOW_BUF);

    g_free(widgets);

    return 0;
}
