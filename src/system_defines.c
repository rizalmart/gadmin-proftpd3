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



/* Several defines for different OSes.
 * Checking defaults to Linux macros.
 */

#include "../config.h"

#ifdef USE_LINUX
#define ADDUSER "useradd"
#define DELUSER "userdel"
#define ADDGROUP "groupadd"
#define USERSHOME "/home/"
#define DISC_USAGE "df -hP |grep /"
#elif defined USE_SUNOS
#define ADDUSER "useradd"
#define DELUSER "userdel"
#define ADDGROUP "groupadd"
#define USERSHOME "/opt/"
#define DISC_USAGE "df -hP |grep /"
#elif defined USE_FREEBSD
#define ADDUSER "pw useradd"
#define DELUSER "pw userdel"
#define ADDGROUP "pw groupadd"
#define USERSHOME "/usr/home/"
#define DISC_USAGE "df -hP |grep /"
#elif defined USE_OPENBSD
#define ADDUSER "useradd"
#define DELUSER "userdel"
#define ADDGROUP "groupadd"
#define USERSHOME "/usr/home/"
#define DISC_USAGE "df -hP |grep /"
#elif defined USE_NETBSD
#define ADDUSER "useradd"
#define DELUSER "userdel"
#define ADDGROUP "groupadd"
#define USERSHOME "/usr/home/"
#define DISC_USAGE "df -hP |grep /"
#elif defined USE_DARWIN
#define ADDUSER "useradd"
#define DELUSER "userdel"
#define ADDGROUP "groupadd"
#define USERSHOME "/Users/"
#define DISC_USAGE "df -hP |grep /"
#include "osx_functions.c"
#elif defined USE_AIX
#define ADDUSER "useradd"
#define DELUSER "userdel"
#define ADDGROUP "groupadd"
#define USERSHOME "/home/"
#define DISC_USAGE "df -hP |grep /"
#elif defined USE_HPUX
#define ADDUSER "useradd"
#define DELUSER "userdel"
#define ADDGROUP "groupadd"
#define USERSHOME "/home/"
#define DISC_USAGE "df -hP |grep /"
#else
  /* Default macros */
#define ADDUSER "useradd"
#define DELUSER "userdel"
#define ADDGROUP "groupadd"
#define USERSHOME "/home/"
#define DISC_USAGE "df -hP |grep /"
#endif
