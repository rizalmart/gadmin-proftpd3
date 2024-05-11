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
#include "../config.h"
#include "support.h"
#include "gettext.h"
#include "widgets.h"



void show_help()
{
    gchar *help_text;
    GtkWidget *help_window, *vbox15, *scrolledwindow16;
    GtkWidget *help_textview, *close_help_button;
    GtkWidget *alignment19, *hbox52, *image19, *label109;

    help_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_name(help_window, "help_window");
    gtk_widget_set_size_request(help_window, 650, 350);
    gtk_window_set_title(GTK_WINDOW(help_window), _("GADMIN-PROFTPD Help"));
    gtk_window_set_position(GTK_WINDOW(help_window), GTK_WIN_POS_CENTER);

    vbox15 = gtk_vbox_new(FALSE, 0);
    gtk_widget_set_name(vbox15, "vbox15");
    gtk_widget_show(vbox15);
    gtk_container_add(GTK_CONTAINER(help_window), vbox15);

    scrolledwindow16 = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_name(scrolledwindow16, "scrolledwindow16");
    gtk_widget_show(scrolledwindow16);
    gtk_box_pack_start(GTK_BOX(vbox15), scrolledwindow16, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow16), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    help_textview = gtk_text_view_new();
    gtk_widget_set_name(help_textview, "help_textview");
    gtk_widget_show(help_textview);
    gtk_container_add(GTK_CONTAINER(scrolledwindow16), help_textview);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(help_textview), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(help_textview), FALSE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(help_textview), 30);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(help_textview), 10);

    help_text = g_strconcat(_("\n\nAdding users:\n\n"),
        _("When adding a user you can randomize its username and password or use your own.\n"),
        _("You can also select its login shell, group, comment and ftp home directory.\n"),
        _("Press add when you are satisfied with the users settings and permissions.\n\n"),
        _("Any other directories added must be located below the ftp home directory for this user.\n\n"),
        _("If you need to add more users with similar settings you can simply change a current users\n"),
        _("username, password and then press the add button.\n\n"),
        _("\nTypical setups:\n\n"),
        _("Users needs upload access to a http servers directory:\n\n"),
        _("Set the users home directory to the webserver's root directory IE:\n"),
        _("/var/www/html or /var/www/html/UserName for a private homepage.\n"),
        _("Press add when you are satisfied with the users settings and permissions.\n\n"),
        _("\nAdding anonymous access to a directory:\n\n"),
        _("Type anonymous in the username field.\n"),
        _("Press the password randomize button or type a password longer then 5 chars.\n"),
        _("Type anonymous as group and comment.\n"),
        _("Select /dev/null shell.\n"),
        _("Set require password to off.\n"),
        _("Type this users home directory IE: /var/ftp/anonymous.\n"),
        _("Press add to add this anonymous user.\n\n"),
        _("You can now login as anonymous with no password or get in directly with a web browser.\n\n"),
        _("If the selected shell is false or null the user can only login to this FTP server otherwise\n"),
        _("that user gets a private home directory as well as its FTP directory and can login via ssh\n"),
        _("etc if that server allows the user to do so.\n\n"),
        _("You can easily add more virtual servers to run on different interfaces and ports.\n"),
        _("The same users can have different directories and settings in different servers.\n"),
        _("When a user connects to one address and port it gets access according to that servers\n"),
        _("configuration for that user.\n"),
        _("If you want to turn a server off without deleting it you can set its port value to 0\n\n"),
        _("If this Proftpd server is behind a NAT router you should specify the routers\n"),
        _("external DNS name or IP-address as the \"NAT router\" option and then turn it on.\n\n"),
        _("If you want to add directories thats not under the users home directory you can do this:\n\n"),
        _("Linux (as of kernel 2.4.0):\n"),
        _("mount --bind /some/directory/to/share /var/ftp/make_this_directory_first\n\n"),
        _("BSD (as of 4.4BSD):\nmount_null /var/data /var/ftp/make_this_directory_first\n\n"),
        _("Solaris:\nmount -F lofs /var/data /var/ftp/make_this_directory_first\n\n"),
        _("Statistics:\n\n"),
        _("Pressing the generate welcome statistics button in the file tab will create\n"),
        _("welcome messages for all users in this FTP server.\n"),
        _("Pressing generate HTML statistics in the file tab creates top 10\n"),
        _("upload and download statistics as a webpage IE: /var/www/html/ftp.htm\n\n"),
        _("With crond and gprostats you can do this to have it automatically update the statistics:\n"),
        _("gprostats -html /path/to/output.html\ngprostats -w welcome.msg\n"),
        _("or both at the same time: gprostats -html /path/to/output.html -w welcome.msg\n"),
        _("Use -c /path/to/proftpd.conf -x /path/to/xferlog if you need to.\n\n"),
        _("If you have made customized welcome messages and dont want them overwritten by gprostats\n"),
        _("you can configure this by selecting a user and unchecking \"show user in statistics\" and\n"),
        _("then pressing the apply button, this will also exclude the user from the html output.\n\n"),
        _("If you copy or move a file directly to the ftpservers directory you may need to\n"),
        _("chmod it with 644 and chmod directories to 755 so that the directory and file can be accessed.\n\n"),
        _("This is one way of chmodding all files recursively in the directory /var/ftp:\n"),
        _("find /var/ftp -type f -exec chmod 644  -v {} \\;\n\n"),
        _("Upload directories should have a chmod of 777 and this can be used for /var/ftp/upload\n"),
        _("find /var/ftp/upload -type d -exec chmod 777  -v {} \\;\n\n\n"),
        _("For more detailed information about the server and its configuration directives visit:\n"),
        _("http://www.proftpd.org\n"),
        NULL);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(help_textview)), help_text, -1);

    if( help_text != NULL )
        g_free(help_text);

    close_help_button = gtk_button_new();
    gtk_widget_set_name(close_help_button, "close_help_button");
    gtk_widget_show(close_help_button);
    gtk_box_pack_start(GTK_BOX(vbox15), close_help_button, FALSE, FALSE, 0);

    alignment19 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_widget_set_name(alignment19, "alignment19");
    gtk_widget_show(alignment19);
    gtk_container_add(GTK_CONTAINER(close_help_button), alignment19);

    hbox52 = gtk_hbox_new(FALSE, 2);
    gtk_widget_set_name(hbox52, "hbox52");
    gtk_widget_show(hbox52);
    gtk_container_add(GTK_CONTAINER(alignment19), hbox52);

    image19 = gtk_image_new_from_stock("gtk-close", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_name(image19, "image19");
    gtk_widget_show(image19);
    gtk_box_pack_start(GTK_BOX(hbox52), image19, FALSE, FALSE, 0);

    label109 = gtk_label_new_with_mnemonic(_("Close"));
    gtk_widget_set_name(label109, "label109");
    gtk_widget_show(label109);
    gtk_box_pack_start(GTK_BOX(hbox52), label109, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label109), GTK_JUSTIFY_LEFT);

    g_signal_connect_swapped(close_help_button,
            "clicked", G_CALLBACK(gtk_widget_destroy), G_OBJECT(help_window));

    gtk_widget_show_all(help_window);
}
