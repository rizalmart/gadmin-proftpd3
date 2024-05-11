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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "get_option_pos.h"



int get_option_pos(char *line, int optnum)
{
    int z = 0;
    int num = 1;
    long i = 0;

    if( optnum == 0 )
    {
        for(i = 0; line[i] != '\0'; i++)
            if( line[i] != ' ' && line[i] != '\t' )
                break;

        return i;
    }

    if( optnum == 1 )
    {
        for(i = 0; line[i] != '\0'; i++)
            if( line[i] != ' ' && line[i] != '\t' )
                break;

        for(i = i; line[i] != '\0'; i++)
            if( line[i] == ' ' && line[i + 1] != ' ' )
                break;
        i++;
        return i;
    }


    /* Scroll past the first option declaration
       to the first option value */
    if( line != NULL && strlen(line) > 0 )
    {
        for(i = 0; line[i] != '\0'; i++)
        {
            if( line[i] != ' ' )
                break;
        }
        for(i = i; line[i] != '\0'; i++)
        {
            if( line[i - 1] == ' ' && line[i] != ' ' )
                break;
        }
    }

    i++;

    for(z = 0; z <= optnum; z++)
    {
        if( num == optnum )
            break;

        /* Scroll to the beginning of the option value */
        if( line != NULL && strlen(line) > 0 )
        {
            for(i = i; line[i] != '\0'; i++)
            {
                if( line[i - 1] == ' ' && line[i] != ' ' )
                {
                    num++;
                    break;
                }
            }
        }
    }

    return i;
}
