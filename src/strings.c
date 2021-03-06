/*--------------------------------*-C-*---------------------------------*
 * File:    strings.c
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1997-2001   Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2003-2004   Marc Lehmann <pcg@goof.com>
 * Copyright (c) 2004-2005   Jingmin Zhou <jimmyzhou@users.sourceforge.net>
 * Copyright (c) 2005-2006   Gautam Iyer <gi1242@users.sourceforge.net>
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


#ifdef HAVE_WCHAR_H
# if !defined (OS_FREEBSD) || _FreeBSD_version >= 500000

/* EXTPROTO */
wchar_t*
rxvt_mbstowcs (const char* str)
{
    wchar_t*	r;
    int		len = STRLEN (str);

    r = (wchar_t *)rxvt_malloc ((len + 1) * sizeof (wchar_t));

    if (mbstowcs (r, str, len + 1) == (size_t)-1)
	*r = 0;

    return r;
}


/* EXTPROTO */
char*
rxvt_wcstoutf8 (const wchar_t* str)
{
    char*   r;
    char*   p;
    int	    len;
    
    len = wcslen (str);

    r = (char *)rxvt_malloc (len * 4 + 1);
    p = r;

    while (len--) {
	unicode_t w = *str++ & UNICODE_MASK;

	if      (w < 0x000080)
	    *p++ = w;
	else if (w < 0x000800)	{
	    *p++ = 0xc0 | ( w >> 6);
	    *p++ = 0x80 | ( w & 0x3f);
	}
	else if (w < 0x010000)	{
	    *p++ = 0xe0 | ( w >> 12 );
	    *p++ = 0x80 | ((w >> 6) & 0x3f);
	    *p++ = 0x80 | ( w & 0x3f);
	}
	else if (w < 0x110000)	{
	    *p++ = 0xf0 | ( w >> 18);
	    *p++ = 0x80 | ((w >> 12) & 0x3f);
	    *p++ = 0x80 | ((w >> 6) & 0x3f);
	    *p++ = 0x80 | ( w & 0x3f);
	}
	else
	    *p++ = '?';
    }

    *p = 0;

    return r;
}

# endif /* !defined (OS_FREEBSD) ||... */
#endif	/* HAVE_WCHAR_H */

/*----------------------- end-of-file (C source) -----------------------*/
