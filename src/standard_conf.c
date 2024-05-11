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
#include <stdlib.h>
#include <string.h>
#include "allocate.h"
#include "widgets.h"
#include "functions.h"
#include "commands.h"
#include "standard_conf.h"
#include "populate_gadmin_proftpd.h"
#include "file_chooser.h"
#include "show_info.h"

/* Temporary for option deprecated options */
extern char global_version[1024];


void new_tls_module_conf()
{
    FILE *fp;
    long file_size = 0;
    char *line, *new_conf;
    gchar *info, *module_conf;

    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        info = g_strdup_printf("Error adding new TLS module configuration.\n");
        show_info(info);
        g_free(info);
        return;
    }

    module_conf = g_strconcat("\n",
        "<IfModule mod_tls.c>\n",
        "TLSEngine off\n",
        "TLSRequired auth+data\n",
        "TLSVerifyClient off\n",
        "TLSProtocol SSLv23\n",
        "TLSLog ", GP_VARDIR, "/log/proftpd_tls.log\n",
        "TLSRSACertificateFile ", GP_APPCONFDIR, "/certs/cert.pem\n",
        "TLSRSACertificateKeyFile ", GP_APPCONFDIR, "/certs/key.pem\n",
        "TLSCACertificateFile ", GP_APPCONFDIR, "/certs/cacert.pem\n",
        "TLSRenegotiate required off\n",
        "TLSOptions AllowClientRenegotiation\n",
        "</IfModule>\n",
        NULL);

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);
    new_conf = allocate(file_size + 16384);

    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        if( commented(line) )
            continue;

        if( cmplowercase(line, "ifmodule")
        &&  cmplowercase(line, "mod_tls.c")
        &&  strstr(line, "<")
        &&  strstr(line, ">") )
        {
            while(fgets(line, file_size, fp) != NULL)
            if(cmplowercase(line, "ifmodule") && strstr(line, "<") && strstr(line, ">"))
            {
                strcat(new_conf, module_conf);
                break;
            }
        }
        else
            strcat(new_conf, line);
    }
    g_free(module_conf);
    free(line);
    fclose(fp);

    /* Write the new configuration */
    if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
    {
        info = g_strdup_printf("Error writing new TLS module configuration.\n");
        show_info(info);
        g_free(info);

        g_free(new_conf);
        return;
    }
    fputs(new_conf, fp);
    fclose(fp);
    free(new_conf);

    info = g_strconcat("Successfully added new TLS module configuration.\n\n",
        "GAdmin-PRoFTPd now supports explicit/total FTP encryption.\n\n",
        "New signed certificates needs to be generated.\n",
        "Press the generate button and then turn on secure communications\n\n",
        "Clients that supports explicit FTP encryption can be used.\n",
        NULL);
    show_info(info);
    g_free(info);
}



int conf_ok(gchar * file_path)
{
    FILE *fp;
    long conf_size;
    char *line;
    int retval = 0;
    int good_conf = 0;

    if((fp = fopen(file_path, "r")) == NULL)
    {
        return retval;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    line = allocate(conf_size + 1);

    if( conf_size > 1 )
    while(fgets(line, conf_size, fp) != NULL)
    {
        /* Dont allow excessively long lines */
        if( strlen(line) > 500 )
        {
            fclose(fp);
            free(line);
            return 0;
        }

        if( commented(line) )
            continue;

        /* Finding these options or contexts first
           means that the configuration is not ok */
        if( cmplowercase(line, "<anonymous") )
            break;
        if( cmplowercase(line, "<virtual") )
            break;
        if( cmplowercase(line, "<ifmodule") )
            break;
        /* We dont support the DefaultRoot directive. */
        if( cmplowercase(line, "defaultroot ") )
            break;
        /* We dont support the UserAlias directive. */
        if( cmplowercase(line, "useralias ") )
            break;

        /* Very good for security so it must exist */
        if( cmplowercase(line, "<limit login>") )
            good_conf++;

        /* We handle "standalone". Refuse to use inetd. */
        if( cmplowercase(line, "servertype standalone") )
            good_conf++;

        if( cmplowercase(line, "transferrate retr") )
            good_conf++;

        if( cmplowercase(line, "transferrate stor") )
            good_conf++;

        if( cmplowercase(line, "transferrate stou") )
            good_conf++;

        if( cmplowercase(line, "transferrate appe") )
            good_conf++;

        if( cmplowercase(line, "passiveports ") )
            good_conf++;

        if( cmplowercase(line, "defaulttransfermode ") )
            good_conf++;
    }
    fclose(fp);
    free(line);

    if( good_conf >= 7 )
        retval = 1;

    return retval;
}


/* Automated configuration backups for new versions */
void backup_configuration(struct w *widgets)
{
    gchar *cmd, *info;

    if( ! file_exists(PROFTPD_CONF_BACKUP)
    &&  file_exists(PROFTPD_CONF) )
    {
        cmd = g_strdup_printf("cp %s %s", PROFTPD_CONF, PROFTPD_CONF_BACKUP);
        if( ! run_command(cmd) )
        {
            info = g_strdup_printf(_("Error: Could not backup the current configuration:\n%s\n"), PROFTPD_CONF);
            show_info(info);
            g_free(info);
        }
        else
        {
            info = g_strdup_printf(_("The configuration was backed up here for safety:\n%s\n"), PROFTPD_CONF_BACKUP);
            show_info(info);
            g_free(info);
        }
        g_free(cmd);
    }
}


/* Backup button in the configuration tab backs up the conf with an added "-gadmin-backup" tag to it */
/* Manual backups are not automatically overwritten then. */
void force_configuration_backup(struct w *widgets)
{
    gchar *cmd, *info;

    if( file_exists(PROFTPD_CONF) )
    {
        cmd = g_strdup_printf("cp %s %s-gadmin-backup", PROFTPD_CONF, PROFTPD_CONF_BACKUP);
        if( ! run_command(cmd) )
        {
            info = g_strdup_printf(_("Error: Could not backup the current configuration:\n%s\n"), PROFTPD_CONF);
            show_info(info);
            g_free(info);
        }
        else
        {
            info = g_strdup_printf(_("The configuration was backed up here for safety:\n%s%s\n"),
                                                                PROFTPD_CONF_BACKUP, "-admin-backup");
            show_info(info);
            g_free(info);
        }
        g_free(cmd);
    }
}


/* Restore and use another proftpd.conf file */
void restore_configuration(struct w *widgets)
{
    gchar *info = NULL, *cmd = NULL, *path = NULL;

    path = get_dialog_path_selection("FILE", GP_APPCONFDIR, "None");

    /* conf_ok() also checks that the configuration lines have valid length */

    if( path != NULL && strlen(path) > 8 && conf_ok(path) )
    {
        cmd = g_strdup_printf("cp %s %s", path, PROFTPD_CONF);
        if( ! run_command(cmd) )
        {
            info = g_strdup_printf(_("The configuration file could not be restored.\n"));
            show_info(info);
            g_free(info);
        }
        g_free(cmd);
        g_free(path);
        path = NULL;
    }
    else
    {
        info = g_strdup_printf(_("The configuration file could not be restored.\n"));
        show_info(info);
        g_free(info);
    }

    if( path != NULL )
        g_free(path);

    populate_gadmin_proftpd(widgets);
}


/* Adds a default/standard configuration and also adds any ModulePath and
   LoadModule statements from the old conf if any, for included DSO's. */
void add_standard_conf(struct w *widgets)
{
    FILE *fp;
    char *includes = NULL;
    gchar *info, *config;

    /* First backup/copy the old configuration */
    backup_configuration(widgets);

    /* Get ModulePath and LoadModule statements from included configurations. */
    includes = get_dynamic_module_statements();

    if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
    {
        info = g_strdup_printf(_("Cant write a new proftpd.conf here:\n%s\n"), PROFTPD_CONF);
        show_info(info);
        g_free(info);
        if( includes != NULL )
            free(includes);
        return;
    }

    /* Add any Include lines we found at the top of the conf. */
    if( includes != NULL )
    {
        fputs(includes, fp);
        free(includes);
        fputs("\n", fp);
    }

    /* Note: inetd functions would limit the servers usability */
    config = g_strconcat("ServerType standalone\n",
        "DefaultServer on\n",
        "Umask 022\n",
        "ServerName \"0.0.0.0\"\n",
        "ServerIdent on \"My FTP Server\"\n",
        "ServerAdmin email@example.org\n",
        "IdentLookups off\n",
        "UseReverseDNS off\n",
        "Port 21\n",
        "PassivePorts 49152 65534\n",
        "#MasqueradeAddress None\n",
        "TimesGMT off\n",
        "MaxInstances 30\n",
        "MaxLoginAttempts 3\n",
        "TimeoutLogin 300\n",
        "TimeoutNoTransfer 120\n",
        "TimeoutIdle 120\n",
        "DisplayLogin ", WELCOME_MESSAGE, "\n",
        "DisplayChdir .message\n"
        "User ", SERVER_USER, "\n",
        "Group ", SERVER_GROUP, "\n",
        "DirFakeUser off ", SERVER_USER, "\n",
        "DirFakeGroup off ", SERVER_GROUP, "\n",
        "DefaultTransferMode binary\n",
        "AllowForeignAddress off\n",
        "AllowRetrieveRestart on\n",
        "AllowStoreRestart on\n",
        "DeleteAbortedStores off\n",
        "TransferRate RETR 220\n",
        "TransferRate STOR 250\n",
        "TransferRate STOU 250\n",
        "TransferRate APPE 250\n",
        "SystemLog ", GP_VARDIR, "/log/secure\n",
        "RequireValidShell off\n\n",
#ifdef CAN_BE_ANNOYING_FOR_USERS
        "<IfModule mod_delay.c>\n" "DelayEngine on\n" "</IfModule>\n\n"
#endif
        "<IfModule mod_tls.c>\n",
        "TLSEngine off\n",
        "TLSRequired off\n",
        "TLSVerifyClient off\n",
        "TLSProtocol SSLv23\n",
        "TLSLog ", GP_VARDIR, "/log/proftpd_tls.log\n",
        "TLSRSACertificateFile ", GP_APPCONFDIR, "/certs/cert.pem\n",
        "TLSRSACertificateKeyFile ", GP_APPCONFDIR, "/certs/key.pem\n",
        "TLSCACertificateFile ", GP_APPCONFDIR, "/certs/cacert.pem\n",
        "TLSRenegotiate required off\n",
        "TLSOptions AllowClientRenegotiation\n",
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
        "</IfModule>\n\n",
        "<Limit LOGIN>\n", "  DenyALL\n", "</Limit>\n\n",
        NULL);

    fputs(config, fp);
    fclose(fp);
    g_free(config);

    if( widgets->default_conf_question_window != NULL )
    {
        gtk_widget_destroy(widgets->default_conf_question_window);
        widgets->default_conf_question_window = NULL;
    }
    populate_gadmin_proftpd(widgets);
}


void dont_add_standard_conf(struct w *widgets)
{
    if( widgets->default_conf_question_window != NULL )
    {
        gtk_widget_destroy(widgets->default_conf_question_window);
        widgets->default_conf_question_window = NULL;
    }

    populate_gadmin_proftpd(widgets);
}


void create_standard_conf_question(struct w *widgets)
{
    FILE *fp;
    long file_size = 0;
    int found_conf_question = 0;
    char *conf, *line;
    gchar *cmd, *info, *utf8, *settings_conf;
    GtkWidget *vbox18, *label182;
    GtkWidget *scrolledwindow;
    GtkWidget *default_question_textview;
    GtkWidget *hbuttonbox11;
    GtkWidget *yes_default_question_button;
    GtkWidget *alignment44, *hbox98;
    GtkWidget *image44, *label184;
    GtkWidget *no_default_question_button;
    GtkWidget *alignment45, *hbox99;
    GtkWidget *image45, *label185;

    widgets->default_conf_question_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(widgets->default_conf_question_window, -1, -1);
    gtk_window_set_title(GTK_WINDOW(widgets->default_conf_question_window), _("GADMIN-PROFTPD Question"));
    gtk_window_set_position(GTK_WINDOW(widgets->default_conf_question_window), GTK_WIN_POS_CENTER);

    vbox18 = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(widgets->default_conf_question_window), vbox18);

    label182 = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(vbox18), label182, FALSE, FALSE, 0);
    gtk_widget_set_size_request(label182, -1, 20);
    gtk_label_set_justify(GTK_LABEL(label182), GTK_JUSTIFY_LEFT);

    scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox18), scrolledwindow, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_NEVER);

    default_question_textview = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(scrolledwindow), default_question_textview);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(default_question_textview), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(default_question_textview), FALSE);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(default_question_textview), 3);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(default_question_textview), 30);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(default_question_textview), 30);

    info = g_strconcat("\n\n",
        _("GADMIN-PROFTPD could not find the proftpd configuration\n"),
        _("or you are using a basic configuration that doesnt have\n"),
        _("all the features that this program requires.\n\n\n"),
        _("Do you want to overwrite your current proftpd configuration\n"),
        _("with a suitable standard configuration for gadmin-proftpd ?\n\n"),
        _("\t\t\t(If you dont know then press yes)\n"),
        NULL);
    utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(default_question_textview)), utf8, -1);
    if( info != NULL )
        g_free(info);
    if( utf8 != NULL )
        g_free(utf8);

    hbuttonbox11 = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(vbox18), hbuttonbox11, FALSE, FALSE, 0);
    gtk_widget_set_size_request(hbuttonbox11, -1, 40);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox11), GTK_BUTTONBOX_SPREAD);

    yes_default_question_button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(hbuttonbox11), yes_default_question_button);
    //GTK_WIDGET_SET_FLAGS(yes_default_question_button, GTK_CAN_DEFAULT);
    gtk_widget_set_can_default(yes_default_question_button, TRUE);

    alignment44 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(yes_default_question_button), alignment44);

    hbox98 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(alignment44), hbox98);

    image44 = gtk_image_new_from_stock("gtk-yes", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(hbox98), image44, FALSE, FALSE, 0);

    label184 = gtk_label_new_with_mnemonic(_("Yes"));
    gtk_box_pack_start(GTK_BOX(hbox98), label184, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label184), GTK_JUSTIFY_LEFT);

    no_default_question_button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(hbuttonbox11), no_default_question_button);
    //GTK_WIDGET_SET_FLAGS(no_default_question_button, GTK_CAN_DEFAULT);
    gtk_widget_set_can_default(no_default_question_button, TRUE);

    alignment45 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(no_default_question_button), alignment45);

    hbox99 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(alignment45), hbox99);

    image45 = gtk_image_new_from_stock("gtk-no", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(hbox99), image45, FALSE, FALSE, 0);

    label185 = gtk_label_new_with_mnemonic(_("No"));
    gtk_box_pack_start(GTK_BOX(hbox99), label185, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label185), GTK_JUSTIFY_LEFT);


    g_signal_connect_swapped(G_OBJECT(yes_default_question_button), "clicked", G_CALLBACK(add_standard_conf), widgets);

    g_signal_connect_swapped(G_OBJECT(no_default_question_button), "clicked", G_CALLBACK(dont_add_standard_conf), widgets);

    gtk_widget_show_all(widgets->default_conf_question_window);


    /* Create gadmin-proftpds settings directory if it doesnt exist. */
    if( ! file_exists(GP_APPCONFDIR) )
    {
        cmd = g_strdup_printf("mkdir -p %s", GP_APPCONFDIR);
        if( ! run_command(cmd) )
        {
            info = g_strdup_printf(_("Error: cant create the settings directory here:\n%s\n"), GP_APPCONFDIR);
            show_info(info);
            g_free(info);
            g_free(cmd);
            return;
        }
        g_free(cmd);
    }

    settings_conf = g_strdup_printf("%s/settings.conf", GP_APPCONFDIR);

    /* The settings file doesnt exist, create and write the setting to it */
    if( ! file_exists(settings_conf) )
    {
        if((fp = fopen(settings_conf, "w+")) == NULL)
        {
            info = g_strdup_printf(_("Error: cant write the settings file here:\n%s\n"), settings_conf);
            show_info(info);
            g_free(info);
            return;
        }
        fputs("show_conf_question: false\n", fp);
        fclose(fp);
        g_free(settings_conf);
        return;
    }

    /* Gather old values and re/write 'show_conf_question: false' to gadmin-proftpds settings */
    if((fp = fopen(settings_conf, "r")) == NULL)
    {
        info = g_strdup_printf(_("Error: cant write the settings file here:\n%s\n"), settings_conf);
        show_info(info);
        g_free(info);
        g_free(settings_conf);
        return;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);
    conf = allocate(file_size + 1024);

    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        if( strlen(line) > 5 )
        {
            if( strstr(line, "show_conf_question:") )
            {
                found_conf_question = 1;
                strcat(conf, "show_conf_question: false\n");
                break;
            }
            else
                strcat(conf, line);
        }
    }
    fclose(fp);
    free(line);

    /* If theres no 'show_conf_question: false' line, add one */
    if( ! found_conf_question )
        strcat(conf, "show_conf_question: false\n");

    /* Write the altered conf */
    if((fp = fopen(settings_conf, "w+")) == NULL)
    {
        info = g_strdup_printf(_("Error: cant write the settings file here:\n%s\n"), settings_conf);
        show_info(info);
        g_free(info);
        free(conf);
        return;
    }
    fputs(conf, fp);
    fclose(fp);
    free(conf);
    g_free(settings_conf);
}
