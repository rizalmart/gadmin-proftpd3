/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */

/* always defined to indicate that i18n is enabled */
#define ENABLE_NLS 1

/* Path to the ftpwho binary */
#define FTPWHO_BINARY "ftpwho"

/* needed for gettext... */
#define GETTEXT_PACKAGE "gadmin-proftpd"

/* application config dir */
#define GP_APPCONFDIR "/etc/gadmin-proftpd"

/* path to ftpusers */
#define GP_FTPUSERS "/etc/ftpusers"

/* path to group */
#define GP_GROUP "/etc/group"

/* path to gshadow */
#define GP_GSHADOW "/etc/gshadow"

/* path to passwd */
#define GP_PASSWD "/etc/passwd"

/* path to shadow */
#define GP_SHADOW "/etc/shadow"

/* path to shells */
#define GP_SHELLS "/etc/shells"

/* path to shutmsg */
#define GP_SHUTMSG "/etc/shutmsg"

/* var dir */
#define GP_VARDIR "/usr/var"

/* Define to 1 if you have the 'bind_textdomain_codeset' function. */
#define HAVE_BIND_TEXTDOMAIN_CODESET 1

/* Define to 1 if you have the Mac OS X function CFLocaleCopyCurrent in the
   CoreFoundation framework. */
/* #undef HAVE_CFLOCALECOPYCURRENT */

/* Define to 1 if you have the Mac OS X function CFPreferencesCopyAppValue in
   the CoreFoundation framework. */
/* #undef HAVE_CFPREFERENCESCOPYAPPVALUE */

/* Define for linking with -lcrypt */
#define HAVE_CRYPT 1

/* Define to 1 if you have the 'dcgettext' function. */
#define HAVE_DCGETTEXT 1

/* Define if the GNU gettext() function is already present or preinstalled. */
#define HAVE_GETTEXT 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define if your <locale.h> file defines LC_MESSAGES. */
#define HAVE_LC_MESSAGES 1

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* path to generated html statistics */
#define HTML_STATISTICS "/var/www/html/ftp.htm"

/* The minimum accepted password length. */
#define MIN_PASS_LEN 6

/* Name of package */
#define PACKAGE "gadmin-proftpd"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Path to the proc filesystem */
#define PROC_PATH "/proc"

/* Path to the proftpd binary */
#define PROFTPD_BINARY "proftpd"

/* path to proftpd.conf */
#define PROFTPD_CONF "/etc/proftpd.conf"

/* Path to backup proftpd.conf */
#define PROFTPD_CONF_BACKUP "/etc/gadmin-proftpd/proftpd.conf.old.gadmin-proftpd-3.0.0"

/* path to the systems security log */
#define SECURE_LOG "/var/log/secure"

/* The server runs as this group */
#define SERVER_GROUP "nobody"

/* The server runs as this user */
#define SERVER_USER "nobody"

/* Define to 1 if all of the C89 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#define STDC_HEADERS 1

/* Command for making the server start at boot */
#define SYSINIT_START_CMD "echo"

/* Command for not making the server start at boot */
#define SYSINIT_STOP_CMD "echo"

/* using aix macros... */
/* #undef USE_AIX */

/* using darwin macros... */
/* #undef USE_DARWIN */

/* using freebsd macros... */
/* #undef USE_FREEBSD */

/* using hpux macros... */
/* #undef USE_HPUX */

/* using Linux macros... */
#define USE_LINUX 1

/* using netbsd macros... */
/* #undef USE_NETBSD */

/* using openbsd macros... */
/* #undef USE_OPENBSD */

/* using sunos macros... */
/* #undef USE_SUNOS */

/* Version number of package */
#define VERSION "3.0.0"

/* Name of generated welcome messages */
#define WELCOME_MESSAGE "welcome.msg"

/* path to proftpds xferlog */
#define XFER_LOG "/var/log/xferlog"
