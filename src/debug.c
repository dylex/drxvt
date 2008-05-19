/*--------------------------------*-C-*---------------------------------*
 * File:    debug.c
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2007        Jingmin Zhou <jimmyzhou@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *----------------------------------------------------------------------*/

#include "../config.h"
#include "rxvt.h"


uint32_t    g_dbg_mask = 0L;
uint32_t    g_dbg_level = DBG_ERROR;


static const char* dbg_level_name[DBG_LEVEL_MAX] = {
    "FATAL",
    "ERROR",
    "WARN",
    "INFO",
    "VERBOSE",
    "DEBUG",
};

/* !!! Sync the list with the above definition of dbgmask_t !!! */
static const char* dbg_mask_name[DBG_IDX_MAX] = {
    "COMMAND",
    "SCREEN",
    "PTYTTY",
    "INIT",
    "MAIN",
    "LOGGING",
    "MACROS",
    "MENUBAR",
    "TABBAR",
    "SCROLLBAR",
    "IMAGES",
    "PIXMAP",
    "TRANSPARENT",
    "ENCODING",
    "GKRELOT",
    "MEMORY",
    "SESSION",
    "STRING",
    "RESOURCE",
    "XFTACS",
    "MISC",
};

/* !!! Sync the list with the above definition of dbgmask_t !!! */
static dbgmask_t dbg_mask_map[DBG_IDX_MAX] = {
    DBG_COMMAND,
    DBG_SCREEN,
    DBG_PTYTTY,
    DBG_INIT,
    DBG_MAIN,
    DBG_LOGGING,
    DBG_MACROS,
    DBG_MENUBAR,
    DBG_TABBAR,
    DBG_SCROLLBAR,
    DBG_IMAGES,
    DBG_PIXMAP,
    DBG_TRANSPARENT,
    DBG_ENCODING,
    DBG_GKRELOT,
    DBG_MEMORY,
    DBG_SESSION,
    DBG_STRING,
    DBG_RESOURCE,
    DBG_XFTACS,
    DBG_MISC,
};



/*----------------------------------------------------------------------*/
/* EXTPROTO */
int
rxvt_msg (uint32_t level, uint32_t mask, const char* fmt, ...)
{

    /* print fatal/error message regardless of its category */
    if (level <= DBG_ERROR ||
	/* print message unless its category and level are defined */
	(g_dbg_level >= level && mask & g_dbg_mask))
    {
	int	len;
	va_list	ap;

	va_start (ap, fmt);
	len = vfprintf (stderr, fmt, ap);
	va_end (ap);
	
        /* add a newline if last character of fmt is not a newline */
        if( fmt[ STRLEN(fmt)-1 ] != '\n' )
	{
             fputc( '\n', stderr );
	     len++;
	}
        return (len);
    }
    return 0;
}


/* INTPROTO */
static int
rxvt_dbg_parse_masks (const char* maskstring)
{
    int		mask = 0;
    const char*	ptr;

    if ('0' == maskstring[0] &&
	('x' == maskstring[1] || 'X' == maskstring[1]))
    {
	for (ptr = maskstring+2; *ptr; ptr++)
	{
	    if (*ptr < '0' || *ptr > '9')
		return 0;   /* invalid hex number */
	    mask = (mask << 4) + (*ptr - '0');
	}
	if (ptr - maskstring > 32)
	    return 0;	    /* mask hex too long */
    }
    else
    {
	const char* pcoma;
	int	    len;

	ptr = pcoma = maskstring;

	do
	{
	    while (*pcoma && ',' != *pcoma)
		pcoma ++;

	    len = pcoma - ptr;
	    if (0 == len)
		return 0;	/* a single ',' without mask name */

	    if (len == sizeof ("COMMAND") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_COMMAND], ptr, len))
		mask |= DBG_COMMAND;
	    else if (len == sizeof ("SCREEN") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_SCREEN], ptr, len))
		mask |= DBG_SCREEN;
	    else if (len == sizeof ("PTYTTY") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_PTYTTY], ptr, len))
		mask |= DBG_PTYTTY;
	    else if (len == sizeof ("INIT") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_INIT], ptr, len))
		mask |= DBG_INIT;
	    else if (len == sizeof ("MAIN") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_MAIN], ptr, len))
		mask |= DBG_MAIN;
	    else if (len == sizeof ("LOGGING") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_LOGGING], ptr, len))
		mask |= DBG_LOGGING;
	    else if (len == sizeof ("MACROS") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_MACROS], ptr, len))
		mask |= DBG_MACROS;
	    else if (len == sizeof ("MENUBAR") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_MENUBAR], ptr, len))
		mask |= DBG_MENUBAR;
	    else if (len == sizeof ("TABBAR") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_TABBAR], ptr, len))
		mask |= DBG_TABBAR;
	    else if (len == sizeof ("SCROLLBAR") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_SCROLLBAR], ptr, len))
		mask |= DBG_SCROLLBAR;
	    else if (len == sizeof ("IMAGES") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_IMAGES], ptr, len))
		mask |= DBG_IMAGES;
	    else if (len == sizeof ("PIXMAP") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_PIXMAP], ptr, len))
		mask |= DBG_PIXMAP;
	    else if (len == sizeof ("TRANSPARENT") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_TRANSPARENT], ptr, len))
		mask |= DBG_TRANSPARENT;
	    else if (len == sizeof ("ENCODING") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_ENCODING], ptr, len))
		mask |= DBG_ENCODING;
	    else if (len == sizeof ("GKRELOT") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_GKRELOT], ptr, len))
		mask |= DBG_GKRELOT;
	    else if (len == sizeof ("MEMORY") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_MEMORY], ptr, len))
		mask |= DBG_MEMORY;
	    else if (len == sizeof ("SESSION") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_SESSION], ptr, len))
		mask |= DBG_SESSION;
	    else if (len == sizeof ("STRING") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_STRING], ptr, len))
		mask |= DBG_STRING;
	    else if (len == sizeof ("RESOURCE") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_RESOURCE], ptr, len))
		mask |= DBG_RESOURCE;
	    else if (len == sizeof ("XFTACS") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_XFTACS], ptr, len))
		mask |= DBG_XFTACS;
	    else if (len == sizeof ("MISC") -1 &&
		0 == STRNCASECMP (dbg_mask_name[DBG_IDX_MISC], ptr, len))
		mask |= DBG_MISC;
	    else if (len == sizeof ("ALL") -1 &&
		0 == STRNCASECMP ("ALL", ptr, len))
		mask |= 0xffffffff;
	    else
	    {
		fprintf (stderr, "Error: unknown debug mask name %s\n", ptr);
		return 0;	/* invalid mask name */
	    }

	    if (!*pcoma)
		break;

	    pcoma ++;
	    ptr = pcoma;
	}
	while (*ptr);
    }

    g_dbg_mask = mask;
    return 1;
}


/* INTPROTO */
static int
rxvt_dbg_parse_level (const char* levelstring)
{
    int	    level;

    if ((char) 0 == *levelstring)
	return 0;

    if ('0' <= *levelstring && *levelstring <= '9')
	level = atoi (levelstring);
    else
    {
	for (level = 0; level < DBG_LEVEL_MAX; level ++)
	    if (0 == STRCASECMP (dbg_level_name[level], levelstring))
		break;
    }
    if (level < 0 || level >= DBG_LEVEL_MAX)
	return 0;

    g_dbg_level = level;

    return 1;
}



/* INTPROTO */
static void
rxvt_dbg_usage (const char* argv)
{
    int	    i;

    fprintf (stderr, "Usage: %s -dm dbg_masks -dl dbg_level\n", argv);

    fprintf (stderr, "    Available debug masks values and their names are:\n");
    for (i = 0; i < DBG_IDX_MAX; i ++)
	fprintf (stderr, "        0x%08x : %s\n", dbg_mask_map[i], dbg_mask_name[i]);
	fprintf (stderr, "        0xffffffff : ALL\n");

    fprintf (stderr, "    Available debug levels and their names are:\n");
    for (i = 0; i < DBG_LEVEL_MAX; i ++)
	fprintf (stderr, "        %d : %s\n", i, dbg_level_name[i]);
}


/* EXTPROTO */
void
rxvt_parse_dbg_arguments (int argc, const char* const* argv)
{
    int	    i;

    for (i = 0; i < argc - 1; i ++)
    {
	if (0 == STRCASECMP("-dmask", argv[i]))
	{
	    if (!rxvt_dbg_parse_masks (argv[i+1]))
	    {
		rxvt_dbg_usage (argv[0]);
		exit (1);
	    }
	    i ++; /* skip arguments to -dm option */
	}

	if (0 == STRCASECMP("-dlevel", argv[i]))
	{
	    if (!rxvt_dbg_parse_level (argv[i+1]))
	    {
		rxvt_dbg_usage (argv[0]);
		exit (1);
	    }
	    i ++; /* skip arguments to -dl option */
	}
    }

#ifdef DEBUG
    fprintf (stderr, "Debug mask: 0x%08x, debug level: %d\n", g_dbg_mask, g_dbg_level);
#endif
}


/*----------------------- end-of-file (C source) -----------------------*/
