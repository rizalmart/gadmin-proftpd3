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


#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include "gettext.h"
#include "widgets.h"
#include "allocate.h"
#include "show_info.h"
#include "commands.h"
#include "generate_cert.h"
#include "reread_conf.h"


/* Support Explicit FTP */
void generate_cert(struct w *widgets)
{
    FILE *fp;
    gchar *cmd, *openssl_config_path, *config;
    gchar *openssl_script, *script_path, *passfile, *info;
    gdouble val = 0.0;
    int i = 0;

    G_CONST_RETURN gchar *hostname;
    G_CONST_RETURN gchar *email;
    G_CONST_RETURN gchar *bits;
    G_CONST_RETURN gchar *password;
    G_CONST_RETURN gchar *days;
    G_CONST_RETURN gchar *country;
    G_CONST_RETURN gchar *state;
    G_CONST_RETURN gchar *city;
    G_CONST_RETURN gchar *org;
    G_CONST_RETURN gchar *org_unit;
    G_CONST_RETURN gchar *cert_dir;

    cert_dir = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[15]));
    hostname = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[16]));
    email = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[17]));
    state = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[18]));
    city = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[19]));
    org  = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[20]));
    org_unit = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[21]));
    password = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[22]));
    country  = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_entry[24]));
    bits = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[11]));
    days = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[12]));

    /* Reset progress bar */
    val = 0.0;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->gen_cert_progressbar), val);
    while(gtk_events_pending())
        gtk_main_iteration();


    /* Show error if a value is missing */
    if( strlen(hostname) == 0 || strlen(email) == 0 || strlen(bits) == 0 || strlen(password) == 0
    ||  strlen(days) == 0 || strlen(country) == 0 || strlen(country) > 2 || strlen(state) == 0
    ||  strlen(city) == 0 || strlen(org) == 0 || strlen(org_unit) == 0 || strlen(cert_dir) < 7 )
    {
        info = g_strdup_printf(_("Every certificate field must be filled in.\n"));
        show_info(info);
        g_free(info);
        return;
    }

    /* The password field cant contain spaces */
    for(i = 0; password[i] != '\0'; i++)
    if( password[i] == ' ' || password[i] == '\n' || password[i] == '\r' || password[i] == '\t' )
    {
        info = g_strdup_printf(_("The password field cant contain any spaces or newline chars.\n"));
        show_info(info);
        g_free(info);
        return;
    }


    /* Create and secure the certificate directory */
    cmd = g_strdup_printf("mkdir -p %s && chmod 700 %s", cert_dir, cert_dir);
    if( ! run_command(cmd) )
    {
        info = g_strdup_printf(_("Can not create secure certificate directory with command:\n%s\n"), cmd);
        show_info(info);
        g_free(info);
        g_free(cmd);
        return;
    }
    g_free(cmd);


    val = 0.1; /* Can take some time depending on the selected number of bits */
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widgets->gen_cert_progressbar), _("Setting up environment"));
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->gen_cert_progressbar), val);
    while(gtk_events_pending())
        gtk_main_iteration();
    sleep(1);

    /* Create a new gadmin-proftpd-openssl.conf with selected values. */
    openssl_config_path = g_strdup_printf("%s/gadmin-proftpd-openssl.conf", cert_dir);
    if((fp = fopen(openssl_config_path, "w+")) == NULL)
    {
        info = g_strdup_printf(_("Error writing gadmin-proftpd-openssl.conf here: \n%s\n"), openssl_config_path);
        show_info(info);
        g_free(info);
        g_free(openssl_config_path);
        return;
    }
    fclose(fp);
    /* Secure the file */
    cmd = g_strdup_printf("chmod 600 %s", openssl_config_path);
    if( ! run_command(cmd) )
    {
        info = g_strdup_printf(_("Error: Could not chmod:\n%s\n"), openssl_config_path);
        show_info(info);
        g_free(info);
        g_free(cmd);
        g_free(openssl_config_path);
        return;
    }
    g_free(cmd);
    /* Write the file contents */
    if((fp = fopen(openssl_config_path, "w")) == NULL)
    {
        info = g_strdup_printf(_("Error writing gadmin-proftpd-openssl.conf here: \n%s\n"), openssl_config_path);
        show_info(info);
        g_free(info);
        g_free(openssl_config_path);
        return;
    }

    config = g_strconcat("\n",
        "dir = ", cert_dir, "\n",
        "[ ca ]\n",
        "default_ca = CA_default\n\n",
        "[ CA_default ]\n",
        "serial = $dir/serial\n",
        "database = $dir/index.txt\n",
        "new_certs_dir = $dir\n",
        "certificate = $dir/cacert.pem\n",
        "private_key = $dir/cakey.pem\n",
        "default_days = ", days, "\n",
        "default_md = md5\n",
        "preserve = no\n",
        "email_in_dn = no\n",
        "nameopt = default_ca\n",
        "certopt = default_ca\n",
        "policy = policy_match\n\n",
        "[ policy_match ]\n",
        "countryName = match\n",
        "stateOrProvinceName = match\n",
        "organizationName = match\n",
        "organizationalUnitName = optional\n",
        "commonName = supplied\n",
        "emailAddress = optional\n\n",
        "[ req ]\n",
        "default_bits = ", bits, "\n",
        "default_keyfile = key.pem\n",
        "default_md = md5\n",
        "string_mask = nombstr\n",
        "distinguished_name = req_distinguished_name\n",
        "req_extensions = v3_req\n\n",
        "[ req_distinguished_name ]\n",
        "0.organizationName = ", org, "\n",
        "organizationalUnitName = ", org_unit, "\n",
        "emailAddress = ", email, "\n",
        "emailAddress_max = 80\n",
        "localityName = ", city, "\n",
        "stateOrProvinceName = ", state, "\n",
        "countryName = ", country, "\n",
        "countryName_min = 2\n",
        "countryName_max = 2\n",
        "commonName = ", hostname, "\n",
        "commonName_max = 64\n\n",
        "0.organizationName_default = ", org, "\n",
        "emailAddress_default = ", email, "\n",
        "localityName_default = ", city, "\n",
        "stateOrProvinceName_default = ", state, "\n",
        "countryName_default = ", country, "\n",
        "commonName_default = ", hostname, "\n\n",
        "[ v3_ca ]\n",
        "basicConstraints = CA:TRUE\n",
        "subjectKeyIdentifier = hash\n",
        "authorityKeyIdentifier = keyid:always,issuer:always\n\n",
        "[ v3_req ]\n",
        "basicConstraints = CA:FALSE\n",
        "subjectKeyIdentifier = hash\n\n",
        NULL);
    fputs(config, fp);
    fclose(fp);


    /* Create the input and output password file. */
    passfile = g_strdup_printf("%s/passfile", cert_dir);
    if((fp = fopen(passfile, "w+")) == NULL)
    {
        info = g_strdup_printf(_("Error writing openssl password file here:\n%s\n"), passfile);
        show_info(info);
        g_free(info);
        g_free(passfile);
        g_free(openssl_config_path);
        return;
    }
    fclose(fp);

    if((fp = fopen(passfile, "w")) == NULL)
    {
        info = g_strdup_printf(_("Error writing openssl password file here:\n%s\n"), passfile);
        show_info(info);
        g_free(info);
        g_free(passfile);
        g_free(openssl_config_path);
        return;
    }
    fputs(password, fp);
    fputs("\n", fp);
    fputs(password, fp);
    fputs("\n", fp);
    fclose(fp);


    /* The last openssl command requires openssl conf to be in the same work dir. */
    if( chdir(cert_dir) != 0 )
    {
        info = g_strdup_printf(_("Error changing to certificate directory: \n%s\n"), cert_dir);
        show_info(info);
        g_free(info);
        return;
    }


    /* Create the first openssl script */
    script_path = g_strdup_printf("%s/create_certs.sh", cert_dir);
    if((fp = fopen(script_path, "w+")) == NULL)
    {
        info = g_strdup_printf(_("Error writing gadmin-proftpd-openssl.conf here: \n%s\n"), script_path);
        show_info(info);
        g_free(info);
        g_free(script_path);
        return;
    }
    openssl_script = g_strconcat("",
        "#!/bin/bash\n\n",
        "rm -f index*\n",
        "touch index.txt\n",
        "rm -f serial*\n",
        "echo '01' > serial\n",
        "rm -f *.pem\n",
        "openssl req -new -x509 -days ", days,
        " -extensions v3_ca -keyout cakey.pem -out cacert.pem -config ",
        openssl_config_path, " -batch -passout file:", passfile, "\n\n",
        NULL);
    fputs(openssl_script, fp);
    fclose(fp);
    g_free(openssl_script);

    /* Creating certificate authority */
    val = 0.3; /* Can take some time depending on the selected number of bits */
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widgets->gen_cert_progressbar),
                _("Creating certificate authority"));
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->gen_cert_progressbar), val);
    while(gtk_events_pending())
        gtk_main_iteration();
    sleep(1);

    /* Run the first openssl script */
    cmd = g_strdup_printf("chmod 755 %s && %s", script_path, script_path);
    if( ! run_command(cmd) )
    {
        info = g_strdup_printf(_("Error running openssl script 1:\n%s\n"), script_path);
        show_info(info);
        g_free(info);
        g_free(script_path);
        g_free(cmd);
        return;
    }
    g_free(cmd);

    /* Create the second openssl script */
    if((fp = fopen(script_path, "w+")) == NULL)
    {
        info = g_strdup_printf(_("Error writing gadmin-proftpd-openssl.conf here: \n%s\n"), script_path);
        show_info(info);
        g_free(info);
        g_free(script_path);
        return;
    }
    openssl_script = g_strconcat("",
        "#!/bin/bash\n\n", "openssl req -new -nodes -out req.pem -config ",
        openssl_config_path, " -batch -passout file:", passfile, "\n\n",
        NULL);
    fputs(openssl_script, fp);
    fclose(fp);
    g_free(openssl_script);

    /* Creating client certificate... */
    val = 0.6;
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widgets->gen_cert_progressbar), _("Creating client certificate"));
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->gen_cert_progressbar), val);
    while(gtk_events_pending())
        gtk_main_iteration();
    sleep(1);

    /* Run the second openssl script */
    cmd = g_strdup_printf("chmod 755 %s && %s", script_path, script_path);
    if( ! run_command(cmd) )
    {
        info = g_strdup_printf(_("Error running openssl script 2:\n%s\n"), script_path);
        show_info(info);
        g_free(info);
        g_free(script_path);
        g_free(cmd);
        return;
    }
    g_free(cmd);

    /* Create the third openssl script */
    if((fp = fopen(script_path, "w+")) == NULL)
    {
        info = g_strdup_printf(_("Error writing gadmin-proftpd-openssl.conf here: \n%s\n"), script_path);
        show_info(info);
        g_free(info);
        g_free(script_path);
        return;
    }
    openssl_script = g_strconcat("",
        "#!/bin/bash\n\n",
        "openssl ca -out cert.pem -config ", openssl_config_path,
        " -batch  -passin file:", passfile, " -infiles req.pem\n\n",
        "rm -f index*\n", "rm -f serial*\n",
        NULL);
    fputs(openssl_script, fp);
    fclose(fp);
    g_free(openssl_script);

    /* Signing client certificate... */
    val = 0.8;
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widgets->gen_cert_progressbar),
            _("Signing client certificate"));
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->gen_cert_progressbar), val);
    while(gtk_events_pending())
        gtk_main_iteration();
    sleep(1);

    /* Run the third openssl script */
    cmd = g_strdup_printf("chmod 755 %s && %s", script_path, script_path);
    if( ! run_command(cmd) )
    {
        info = g_strdup_printf(_("Error running openssl script 3:\n%s\n"), script_path);
        show_info(info);
        g_free(info);
        g_free(script_path);
        g_free(cmd);
        return;
    }
    g_free(cmd);

    /* Remove the script and passfile */
    unlink(script_path);
    unlink(passfile);

    g_free(script_path);
    g_free(passfile);


    val = 1.0;
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widgets->gen_cert_progressbar),
            "Certificate creation successful");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->gen_cert_progressbar), val);
    while(gtk_events_pending())
        gtk_main_iteration();

    info = g_strconcat("\n",
            _("The certificates have been created successfully.\n"),
            _("They will be used by new secure FTP logins.\n"), "\n",
            NULL);
    show_info(info);
    g_free(info);

    reread_conf(widgets);
}
