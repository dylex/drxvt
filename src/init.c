/*--------------------------------*-C-*---------------------------------*
 * File:    init.c
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1992       John Bovey <jdb@ukc.ac.uk>
 * Copyright (c) 1994       Robert Nation <nation@rocket.sanders.lockheed.com>
 * Copyright (c) 1998-2001  Geoff Wing <gcw@pobox.com>
 * Copyright (c) 1999       D J Hawkey Jr <hawkeyd@visi.com>
 * Copyright (c) 2003       marcus at #fluxbox on freenode.net
 * Copyright (c) 2004       Mr. Dobalina <losermcloser@users.sourceforge.net>
 * Copyright (c) 2003-2004  Marc Lehmann <pcg@goof.com>
 * Copyright (c) 2004-2006  Jingmin Zhou <jimmyzhou@users.sourceforge.net>
 * Copyright (c) 2005-2006  Gautam Iyer <gi1242@users.sourceforge.net>
 * Copyright (C) 2008		 Jehan Hysseo <hysseo@users.sourceforge.net>
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

#ifdef XFT_SUPPORT
# include "xftacs.h"
#endif


/* #define XTERM_REVERSE_VIDEO 1 */


/*--------------------------------------------------------------------*
 *         BEGIN `INTERNAL' ROUTINE PROTOTYPES                        *
 *--------------------------------------------------------------------*/
#if defined(OS_SVR4) && !defined(_POSIX_VERSION)
int    rxvt_getdtablesize     ();
#endif
int    rxvt_xerror_handler    (const Display*, const XErrorEvent*);
void   rxvt_init_colors       (rxvt_t*);
void   rxvt_init_win_size     (rxvt_t*);
void   rxvt_color_aliases     (rxvt_t*, int);
void   rxvt_get_ourmods       (rxvt_t*);
int    rxvt_run_child         (rxvt_t*, int, const char**);
void   rxvt_get_ttymode       (ttymode_t*, int);
/*--------------------------------------------------------------------*
 *         END   `INTERNAL' ROUTINE PROTOTYPES                        *
 *--------------------------------------------------------------------*/



const char *const def_colorName[] = {
    COLOR_FOREGROUND,
    COLOR_BACKGROUND,
/* low-intensity colors */
    "black",		/* 0: black	    (#000000) */
#ifndef NO_BRIGHTCOLOR
    "red3",	    /* 1: red		    (#CD0000) */
    "green3",		/* 2: green	    (#00CD00) */
    "yellow3",		/* 3: yellow		(#CDCD00) */
    "blue3",		/* 4: blue		(#0000CD) */
    "magenta3",		/* 5: magenta		(#CD00CD) */
    "cyan3",		/* 6: cyan		(#00CDCD) */
# ifdef XTERM_COLORS
    "grey90",		/* 7: white	    (#E5E5E5) */
# else
    "antiquewhite",	/* 7: white	    (#FAEBD7) */
# endif
/* high-intensity colors */
# ifdef XTERM_COLORS
    "grey30",		/* 8: bright black	(#4D4D4D) */
# else
    "grey25",		/* 8: bright black	(#404040) */
# endif
#endif		    /* NO_BRIGHTCOLOR */
    "red",	    /* 1/9: bright red	    (#FF0000) */
    "green",		/* 2/10: bright green   (#00FF00) */
    "yellow",		/* 3/11: bright yellow  (#FFFF00) */
    "blue",	    /* 4/12: bright blue    (#0000FF) */
    "magenta",		/* 5/13: bright magenta (#FF00FF) */
    "cyan",	    /* 6/14: bright cyan    (#00FFFF) */
    "white",		/* 7/15: bright white   (#FFFFFF) */
#ifdef TTY_256COLOR
# ifdef XTERM_COLORS
    /* use the same color cube as xterm. 16-231 is a more or less uniform
     * rgb ramp, and 231-255 is a greyscale ramp */
    "rgb:00/00/00",	/* default 16-255 color table	    */
    "rgb:00/00/5f",	/* consists of 6x6x6 colour cub	    */
    "rgb:00/00/87",	/* and a 24 greyscale ramp w/o	    */
    "rgb:00/00/af",	/* black or white		    */
    "rgb:00/00/d7",
    "rgb:00/00/ff",
    "rgb:00/5f/00",
    "rgb:00/5f/5f",
    "rgb:00/5f/87",
    "rgb:00/5f/af",
    "rgb:00/5f/d7",
    "rgb:00/5f/ff",
    "rgb:00/87/00",
    "rgb:00/87/5f",
    "rgb:00/87/87",
    "rgb:00/87/af",
    "rgb:00/87/d7",
    "rgb:00/87/ff",
    "rgb:00/af/00",
    "rgb:00/af/5f",
    "rgb:00/af/87",
    "rgb:00/af/af",
    "rgb:00/af/d7",
    "rgb:00/af/ff",
    "rgb:00/d7/00",
    "rgb:00/d7/5f",
    "rgb:00/d7/87",
    "rgb:00/d7/af",
    "rgb:00/d7/d7",
    "rgb:00/d7/ff",
    "rgb:00/ff/00",
    "rgb:00/ff/5f",
    "rgb:00/ff/87",
    "rgb:00/ff/af",
    "rgb:00/ff/d7",
    "rgb:00/ff/ff",
    "rgb:5f/00/00",
    "rgb:5f/00/5f",
    "rgb:5f/00/87",
    "rgb:5f/00/af",
    "rgb:5f/00/d7",
    "rgb:5f/00/ff",
    "rgb:5f/5f/00",
    "rgb:5f/5f/5f",
    "rgb:5f/5f/87",
    "rgb:5f/5f/af",
    "rgb:5f/5f/d7",
    "rgb:5f/5f/ff",
    "rgb:5f/87/00",
    "rgb:5f/87/5f",
    "rgb:5f/87/87",
    "rgb:5f/87/af",
    "rgb:5f/87/d7",
    "rgb:5f/87/ff",
    "rgb:5f/af/00",
    "rgb:5f/af/5f",
    "rgb:5f/af/87",
    "rgb:5f/af/af",
    "rgb:5f/af/d7",
    "rgb:5f/af/ff",
    "rgb:5f/d7/00",
    "rgb:5f/d7/5f",
    "rgb:5f/d7/87",
    "rgb:5f/d7/af",
    "rgb:5f/d7/d7",
    "rgb:5f/d7/ff",
    "rgb:5f/ff/00",
    "rgb:5f/ff/5f",
    "rgb:5f/ff/87",
    "rgb:5f/ff/af",
    "rgb:5f/ff/d7",
    "rgb:5f/ff/ff",
    "rgb:87/00/00",
    "rgb:87/00/5f",
    "rgb:87/00/87",
    "rgb:87/00/af",
    "rgb:87/00/d7",
    "rgb:87/00/ff",
    "rgb:87/5f/00",
    "rgb:87/5f/5f",
    "rgb:87/5f/87",
    "rgb:87/5f/af",
    "rgb:87/5f/d7",
    "rgb:87/5f/ff",
    "rgb:87/87/00",
    "rgb:87/87/5f",
    "rgb:87/87/87",
    "rgb:87/87/af",
    "rgb:87/87/d7",
    "rgb:87/87/ff",
    "rgb:87/af/00",
    "rgb:87/af/5f",
    "rgb:87/af/87",
    "rgb:87/af/af",
    "rgb:87/af/d7",
    "rgb:87/af/ff",
    "rgb:87/d7/00",
    "rgb:87/d7/5f",
    "rgb:87/d7/87",
    "rgb:87/d7/af",
    "rgb:87/d7/d7",
    "rgb:87/d7/ff",
    "rgb:87/ff/00",
    "rgb:87/ff/5f",
    "rgb:87/ff/87",
    "rgb:87/ff/af",
    "rgb:87/ff/d7",
    "rgb:87/ff/ff",
    "rgb:af/00/00",
    "rgb:af/00/5f",
    "rgb:af/00/87",
    "rgb:af/00/af",
    "rgb:af/00/d7",
    "rgb:af/00/ff",
    "rgb:af/5f/00",
    "rgb:af/5f/5f",
    "rgb:af/5f/87",
    "rgb:af/5f/af",
    "rgb:af/5f/d7",
    "rgb:af/5f/ff",
    "rgb:af/87/00",
    "rgb:af/87/5f",
    "rgb:af/87/87",
    "rgb:af/87/af",
    "rgb:af/87/d7",
    "rgb:af/87/ff",
    "rgb:af/af/00",
    "rgb:af/af/5f",
    "rgb:af/af/87",
    "rgb:af/af/af",
    "rgb:af/af/d7",
    "rgb:af/af/ff",
    "rgb:af/d7/00",
    "rgb:af/d7/5f",
    "rgb:af/d7/87",
    "rgb:af/d7/af",
    "rgb:af/d7/d7",
    "rgb:af/d7/ff",
    "rgb:af/ff/00",
    "rgb:af/ff/5f",
    "rgb:af/ff/87",
    "rgb:af/ff/af",
    "rgb:af/ff/d7",
    "rgb:af/ff/ff",
    "rgb:d7/00/00",
    "rgb:d7/00/5f",
    "rgb:d7/00/87",
    "rgb:d7/00/af",
    "rgb:d7/00/d7",
    "rgb:d7/00/ff",
    "rgb:d7/5f/00",
    "rgb:d7/5f/5f",
    "rgb:d7/5f/87",
    "rgb:d7/5f/af",
    "rgb:d7/5f/d7",
    "rgb:d7/5f/ff",
    "rgb:d7/87/00",
    "rgb:d7/87/5f",
    "rgb:d7/87/87",
    "rgb:d7/87/af",
    "rgb:d7/87/d7",
    "rgb:d7/87/ff",
    "rgb:d7/af/00",
    "rgb:d7/af/5f",
    "rgb:d7/af/87",
    "rgb:d7/af/af",
    "rgb:d7/af/d7",
    "rgb:d7/af/ff",
    "rgb:d7/d7/00",
    "rgb:d7/d7/5f",
    "rgb:d7/d7/87",
    "rgb:d7/d7/af",
    "rgb:d7/d7/d7",
    "rgb:d7/d7/ff",
    "rgb:d7/ff/00",
    "rgb:d7/ff/5f",
    "rgb:d7/ff/87",
    "rgb:d7/ff/af",
    "rgb:d7/ff/d7",
    "rgb:d7/ff/ff",
    "rgb:ff/00/00",
    "rgb:ff/00/5f",
    "rgb:ff/00/87",
    "rgb:ff/00/af",
    "rgb:ff/00/d7",
    "rgb:ff/00/ff",
    "rgb:ff/5f/00",
    "rgb:ff/5f/5f",
    "rgb:ff/5f/87",
    "rgb:ff/5f/af",
    "rgb:ff/5f/d7",
    "rgb:ff/5f/ff",
    "rgb:ff/87/00",
    "rgb:ff/87/5f",
    "rgb:ff/87/87",
    "rgb:ff/87/af",
    "rgb:ff/87/d7",
    "rgb:ff/87/ff",
    "rgb:ff/af/00",
    "rgb:ff/af/5f",
    "rgb:ff/af/87",
    "rgb:ff/af/af",
    "rgb:ff/af/d7",
    "rgb:ff/af/ff",
    "rgb:ff/d7/00",
    "rgb:ff/d7/5f",
    "rgb:ff/d7/87",
    "rgb:ff/d7/af",
    "rgb:ff/d7/d7",
    "rgb:ff/d7/ff",
    "rgb:ff/ff/00",
    "rgb:ff/ff/5f",
    "rgb:ff/ff/87",
    "rgb:ff/ff/af",
    "rgb:ff/ff/d7",
    "rgb:ff/ff/ff",
# else	/* !XTERM_COLORS */
    "rgbi:0/0/0",	/* default 16-255 color table	    */
    "rgbi:0/0/.2",	/* consists of 6x6x6 colour cubes   */
    "rgbi:0/0/.4",	/* and a 24 greyscale ramp w/o	    */
    "rgbi:0/0/.6",	/* black or white		    */
    "rgbi:0/0/.8",
    "rgbi:0/0/1",
    "rgbi:0/.2/0",
    "rgbi:0/.2/.2",
    "rgbi:0/.2/.4",
    "rgbi:0/.2/.6",
    "rgbi:0/.2/.8",
    "rgbi:0/.2/1",
    "rgbi:0/.4/0",
    "rgbi:0/.4/.2",
    "rgbi:0/.4/.4",
    "rgbi:0/.4/.6",
    "rgbi:0/.4/.8",
    "rgbi:0/.4/1",
    "rgbi:0/.6/0",
    "rgbi:0/.6/.2",
    "rgbi:0/.6/.4",
    "rgbi:0/.6/.6",
    "rgbi:0/.6/.8",
    "rgbi:0/.6/1",
    "rgbi:0/.8/0",
    "rgbi:0/.8/.2",
    "rgbi:0/.8/.4",
    "rgbi:0/.8/.6",
    "rgbi:0/.8/.8",
    "rgbi:0/.8/1",
    "rgbi:0/1/0",
    "rgbi:0/1/.2",
    "rgbi:0/1/.4",
    "rgbi:0/1/.6",
    "rgbi:0/1/.8",
    "rgbi:0/1/1",
    "rgbi:.2/0/0",
    "rgbi:.2/0/.2",
    "rgbi:.2/0/.4",
    "rgbi:.2/0/.6",
    "rgbi:.2/0/.8",
    "rgbi:.2/0/1",
    "rgbi:.2/.2/0",
    "rgbi:.2/.2/.2",
    "rgbi:.2/.2/.4",
    "rgbi:.2/.2/.6",
    "rgbi:.2/.2/.8",
    "rgbi:.2/.2/1",
    "rgbi:.2/.4/0",
    "rgbi:.2/.4/.2",
    "rgbi:.2/.4/.4",
    "rgbi:.2/.4/.6",
    "rgbi:.2/.4/.8",
    "rgbi:.2/.4/1",
    "rgbi:.2/.6/0",
    "rgbi:.2/.6/.2",
    "rgbi:.2/.6/.4",
    "rgbi:.2/.6/.6",
    "rgbi:.2/.6/.8",
    "rgbi:.2/.6/1",
    "rgbi:.2/.8/0",
    "rgbi:.2/.8/.2",
    "rgbi:.2/.8/.4",
    "rgbi:.2/.8/.6",
    "rgbi:.2/.8/.8",
    "rgbi:.2/.8/1",
    "rgbi:.2/1/0",
    "rgbi:.2/1/.2",
    "rgbi:.2/1/.4",
    "rgbi:.2/1/.6",
    "rgbi:.2/1/.8",
    "rgbi:.2/1/1",
    "rgbi:.4/0/0",
    "rgbi:.4/0/.2",
    "rgbi:.4/0/.4",
    "rgbi:.4/0/.6",
    "rgbi:.4/0/.8",
    "rgbi:.4/0/1",
    "rgbi:.4/.2/0",
    "rgbi:.4/.2/.2",
    "rgbi:.4/.2/.4",
    "rgbi:.4/.2/.6",
    "rgbi:.4/.2/.8",
    "rgbi:.4/.2/1",
    "rgbi:.4/.4/0",
    "rgbi:.4/.4/.2",
    "rgbi:.4/.4/.4",
    "rgbi:.4/.4/.6",
    "rgbi:.4/.4/.8",
    "rgbi:.4/.4/1",
    "rgbi:.4/.6/0",
    "rgbi:.4/.6/.2",
    "rgbi:.4/.6/.4",
    "rgbi:.4/.6/.6",
    "rgbi:.4/.6/.8",
    "rgbi:.4/.6/1",
    "rgbi:.4/.8/0",
    "rgbi:.4/.8/.2",
    "rgbi:.4/.8/.4",
    "rgbi:.4/.8/.6",
    "rgbi:.4/.8/.8",
    "rgbi:.4/.8/1",
    "rgbi:.4/1/0",
    "rgbi:.4/1/.2",
    "rgbi:.4/1/.4",
    "rgbi:.4/1/.6",
    "rgbi:.4/1/.8",
    "rgbi:.4/1/1",
    "rgbi:.6/0/0",
    "rgbi:.6/0/.2",
    "rgbi:.6/0/.4",
    "rgbi:.6/0/.6",
    "rgbi:.6/0/.8",
    "rgbi:.6/0/1",
    "rgbi:.6/.2/0",
    "rgbi:.6/.2/.2",
    "rgbi:.6/.2/.4",
    "rgbi:.6/.2/.6",
    "rgbi:.6/.2/.8",
    "rgbi:.6/.2/1",
    "rgbi:.6/.4/0",
    "rgbi:.6/.4/.2",
    "rgbi:.6/.4/.4",
    "rgbi:.6/.4/.6",
    "rgbi:.6/.4/.8",
    "rgbi:.6/.4/1",
    "rgbi:.6/.6/0",
    "rgbi:.6/.6/.2",
    "rgbi:.6/.6/.4",
    "rgbi:.6/.6/.6",
    "rgbi:.6/.6/.8",
    "rgbi:.6/.6/1",
    "rgbi:.6/.8/0",
    "rgbi:.6/.8/.2",
    "rgbi:.6/.8/.4",
    "rgbi:.6/.8/.6",
    "rgbi:.6/.8/.8",
    "rgbi:.6/.8/1",
    "rgbi:.6/1/0",
    "rgbi:.6/1/.2",
    "rgbi:.6/1/.4",
    "rgbi:.6/1/.6",
    "rgbi:.6/1/.8",
    "rgbi:.6/1/1",
    "rgbi:.8/0/0",
    "rgbi:.8/0/.2",
    "rgbi:.8/0/.4",
    "rgbi:.8/0/.6",
    "rgbi:.8/0/.8",
    "rgbi:.8/0/1",
    "rgbi:.8/.2/0",
    "rgbi:.8/.2/.2",
    "rgbi:.8/.2/.4",
    "rgbi:.8/.2/.6",
    "rgbi:.8/.2/.8",
    "rgbi:.8/.2/1",
    "rgbi:.8/.4/0",
    "rgbi:.8/.4/.2",
    "rgbi:.8/.4/.4",
    "rgbi:.8/.4/.6",
    "rgbi:.8/.4/.8",
    "rgbi:.8/.4/1",
    "rgbi:.8/.6/0",
    "rgbi:.8/.6/.2",
    "rgbi:.8/.6/.4",
    "rgbi:.8/.6/.6",
    "rgbi:.8/.6/.8",
    "rgbi:.8/.6/1",
    "rgbi:.8/.8/0",
    "rgbi:.8/.8/.2",
    "rgbi:.8/.8/.4",
    "rgbi:.8/.8/.6",
    "rgbi:.8/.8/.8",
    "rgbi:.8/.8/1",
    "rgbi:.8/1/0",
    "rgbi:.8/1/.2",
    "rgbi:.8/1/.4",
    "rgbi:.8/1/.6",
    "rgbi:.8/1/.8",
    "rgbi:.8/1/1",
    "rgbi:1/0/0",
    "rgbi:1/0/.2",
    "rgbi:1/0/.4",
    "rgbi:1/0/.6",
    "rgbi:1/0/.8",
    "rgbi:1/0/1",
    "rgbi:1/.2/0",
    "rgbi:1/.2/.2",
    "rgbi:1/.2/.4",
    "rgbi:1/.2/.6",
    "rgbi:1/.2/.8",
    "rgbi:1/.2/1",
    "rgbi:1/.4/0",
    "rgbi:1/.4/.2",
    "rgbi:1/.4/.4",
    "rgbi:1/.4/.6",
    "rgbi:1/.4/.8",
    "rgbi:1/.4/1",
    "rgbi:1/.6/0",
    "rgbi:1/.6/.2",
    "rgbi:1/.6/.4",
    "rgbi:1/.6/.6",
    "rgbi:1/.6/.8",
    "rgbi:1/.6/1",
    "rgbi:1/.8/0",
    "rgbi:1/.8/.2",
    "rgbi:1/.8/.4",
    "rgbi:1/.8/.6",
    "rgbi:1/.8/.8",
    "rgbi:1/.8/1",
    "rgbi:1/1/0",
    "rgbi:1/1/.2",
    "rgbi:1/1/.4",
    "rgbi:1/1/.6",
    "rgbi:1/1/.8",
    "rgbi:1/1/1",
# endif	/* XTERM_COLORS */
    "rgb:08/08/08", /* xterm, rxvt, mrxvt use the same greyscale ramp */
    "rgb:12/12/12",
    "rgb:1c/1c/1c",
    "rgb:26/26/26",
    "rgb:30/30/30",
    "rgb:3a/3a/3a",
    "rgb:44/44/44",
    "rgb:4e/4e/4e",
    "rgb:58/58/58",
    "rgb:62/62/62",
    "rgb:6c/6c/6c",
    "rgb:76/76/76",
    "rgb:80/80/80",
    "rgb:8a/8a/8a",
    "rgb:94/94/94",
    "rgb:9e/9e/9e",
    "rgb:a8/a8/a8",
    "rgb:b2/b2/b2",
    "rgb:bc/bc/bc",
    "rgb:c6/c6/c6",
    "rgb:d0/d0/d0",
    "rgb:da/da/da",
    "rgb:e4/e4/e4",
    "rgb:ee/ee/ee",
#endif	/* TTY_256COLOR */
#ifndef NO_CURSORCOLOR
    COLOR_CURSOR_BACKGROUND,
    COLOR_CURSOR_FOREGROUND,
#endif		    /* ! NO_CURSORCOLOR */
    NULL,	    /* Color_pointer		    */
    NULL,	    /* Color_border		    */
    NULL,	    /* Color_ufbg		    */
#ifndef NO_BOLD_UNDERLINE_REVERSE
    NULL,	    /* Color_BD			    */
    NULL,	    /* Color_UL			    */
    NULL,	    /* Color_RV			    */
#endif		    /* ! NO_BOLD_UNDERLINE_REVERSE  */
#ifdef OPTION_HC
    NULL,	    /* Color_HL			    */
#endif
#ifdef KEEP_SCROLLCOLOR
    COLOR_SCROLLBAR,
    COLOR_SCROLLTROUGH,
#endif		    /* KEEP_SCROLLCOLOR */
#ifdef TINTING_SUPPORT
# ifdef HAVE_LIBXRENDER
    "rgb:00/00/00",
# else
    "rgb:ff/ff/ff",
# endif
#endif
};


/*
** MUST sync with rxvt.h:enum XA_XXXX
*/
const char *const xa_names[NUM_XA] = {
    "COMPOUND_TEXT",
    "UTF8_STRING",
    "TEXT",
    "MULTIPLE",	
    "TARGETS",	
    "TIMESTAMP",
    "VT_SELECTION",
    "INCR",
    "WM_DELETE_WINDOW",
    "_NET_WM_DESKTOP",
    "_WIN_WORKSPACE",
    "_NET_WM_NAME",
    "_NET_WM_ICON_NAME",
    "WM_CLIENT_LEADER",
    "_NET_WM_WINDOW_OPACITY",
#ifndef NO_FRILLS
    "_NET_WM_PID",
#endif
#ifdef HAVE_X11_SM_SMLIB_H
    "SM_CLIENT_ID",
#endif
#ifdef USE_XIM
    "WM_LOCALE_NAME",
#endif
#ifdef TRANSPARENT
    "_XROOTPMAP_ID",
    "_XSETROOT_ID",
#endif
#ifdef OFFIX_DND
    "DndProtocol",
    "DndSelection",
#endif
    "CLIPBOARD"
};

/* substitute system functions */
#if defined(OS_SVR4) && ! defined(_POSIX_VERSION)
/* INTPROTO */
int
rxvt_getdtablesize(void)
{
    struct rlimit   rlim;

    getrlimit(RLIMIT_NOFILE, &rlim);
    return rlim.rlim_cur;
}
#endif


/* EXTPROTO */
/* This initialization function sets all variable in the mrxvt structure
 * to their "neutral" value.
 * It is run by rxvt_init and returns 0
 * (no failure possible unless the memory allocation for mrxvt structure has failed
 * in rxvt_init).
 */
int
rxvt_init_vars(rxvt_t *r)
{
    register int	i;
    struct rxvt_hidden*	h;


    MEMSET(r, 0, sizeof(rxvt_t));

    h = r->h = (struct rxvt_hidden *)rxvt_calloc(1, sizeof(struct rxvt_hidden));

	 SET_NULL (r->vts);

    SET_NULL(r->Xdisplay);
#ifdef USE_XIM
    SET_NULL(r->TermWin.fontset);
#endif
    SET_NULL(r->TermWin.font);
#ifdef MULTICHAR_SET
    SET_NULL(r->TermWin.mfont);
#endif
#ifndef NO_BOLDFONT
    SET_NULL(r->TermWin.bfont);
#endif

#ifdef XFT_SUPPORT
    SET_NULL(r->TermWin.xftpattern);
    SET_NULL(r->TermWin.xftfont);
    SET_NULL(r->TermWin.xftpfont);
    SET_NULL(r->TermWin.xftPfont);
# ifndef NO_BOLDFONT
    SET_NULL(r->TermWin.xftbfont);
# endif	/* NO_BOLDFONT */
# ifdef MULTICHAR_SET
#  ifdef HAVE_ICONV_H
    r->TermWin.xfticonv = (iconv_t) -1;
#  endif
    SET_NULL(r->TermWin.xftmpattern);
    SET_NULL(r->TermWin.xftmfont);
# endif	/* MULTICHAR_SET */
#endif	/* XFT_SUPPORT */

    UNSET_ATOM(h->xa[XA_COMPOUND_TEXT]);
    UNSET_ATOM(h->xa[XA_MULTIPLE]);
    UNSET_ATOM(h->xa[XA_TARGETS]);
    UNSET_ATOM(h->xa[XA_TEXT]);
    UNSET_ATOM(h->xa[XA_TIMESTAMP]);
    UNSET_ATOM(h->xa[XA_VT_SELECTION]);
    UNSET_ATOM(h->xa[XA_INCR]);
    h->locale = NULL;

# ifdef HAVE_MENUBAR
    SET_NULL(h->BuildMenu);
    SET_NULL(h->ActiveMenu);
    SET_NULL(h->popupMenu[0]);
    SET_NULL(h->popupMenu[1]);
    SET_NULL(h->popupMenu[2]);
    h->showingMenu = 0;

    /* Set the current menubar to empty defaults */
    SET_NULL(h->MenuBar.head);
    SET_NULL(h->MenuBar.tail);
    SET_NULL(h->MenuBar.title);
# endif

# ifdef USE_XIM
    SET_NULL(h->Input_Context);
# endif
    /* SET_NULL(h->inbuf_start); */
    SET_NULL(h->buffer);

# ifdef TRANSPARENT
    h->am_pixmap_trans = 0;
    h->am_transparent  = 0;
    UNSET_PIXMAP(h->rootPixmap);
    h->bgRefreshInterval = DEFAULT_BG_REFRESH_INTERVAL;
    h->lastCNotify.tv_sec = 0;	    /* No BG update pending */
# endif

    /* Initialize timeouts to 0 */
    for( i=NUM_TIMEOUTS; i--;)
        h->timeout[i].tv_sec = 0;


    /* Back to undocumented code :) */
    h->MEvent.time = CurrentTime;
    h->MEvent.button = AnyButton;
    r->Options[0] = DEFAULT_OPTIONS;
    r->Options[1] = DEFAULT_OPTIONS2;
    r->Options[2] = DEFAULT_OPTIONS3;
    r->Options[3] = DEFAULT_OPTIONS4;
    h->want_clip_refresh = 0;
    /*
     * We only want to set want_resize when we call XResizeWindow. In that
     * case if XResizeWindow fails, we know that we called it, and can run
     * our internal resize routines anyway (e.g. put the tabbar in place)
     */
    h->want_resize = 0;
    h->ttygid = -1;
    r->Xfd = -1;
#ifdef USE_FIFO
    r->fifo_fd = -1;
#endif
    r->ndead_childs = 0;

    r->nAsyncChilds = 0;

    /* default values */
#ifdef NO_FRILLS
    r->TermWin.int_bwidth = DEFAULT_INTERNALBORDERWIDTH;
    r->TermWin.ext_bwidth = DEFAULT_EXTERNALBORDERWIDTH;
#else
    r->TermWin.int_bwidth = (INTERNALBORDERWIDTH >= 0 && INTERNALBORDERWIDTH <= MAX_INTERNALBORDERWIDTH) ? INTERNALBORDERWIDTH : DEFAULT_INTERNALBORDERWIDTH;
    r->TermWin.ext_bwidth = (EXTERNALBORDERWIDTH >= 0 && EXTERNALBORDERWIDTH <= MAX_EXTERNALBORDERWIDTH) ? EXTERNALBORDERWIDTH : DEFAULT_EXTERNALBORDERWIDTH;
#endif

#ifndef NO_LINESPACE
    r->TermWin.lineSpace = (LINESPACE >= 0 && LINESPACE <= MAX_LINESPACE) ? LINESPACE : DEFAULT_LINESPACE;
#endif

#ifdef CURSOR_BLINK
    r->h->blinkInterval = DEFAULT_BLINK_TIME;
#endif
#ifdef POINTER_BLANK
    r->h->pointerBlankDelay = DEFAULT_BLANKDELAY;
#endif

    /* Initialize selection data */
#ifndef NO_NEW_SELECTION
    r->selection_style = NEW_SELECT;
#else
    r->selection_style = OLD_SELECT;
#endif
    r->selection.vt = -1;
    r->selection.op = SELECTION_CLEAR;
    r->selection.screen = PRIMARY;
    r->selection.clicks = 0;
    SET_NULL(r->selection.text);
    r->selection.len = 0;
    r->selection.beg.row = 0;
    r->selection.beg.col = 0;
    r->selection.end.row = 0;
    r->selection.end.col = 0;

#ifndef NO_BRIGHTCOLOR
    h->colorfgbg = DEFAULT_RSTYLE;
#endif
#ifdef GREEK_SUPPORT
    h->ks_greekmodeswith = GREEK_KEYBOARD_MODESWITCH;
#endif
    h->refresh_type = SLOW_REFRESH;
    UNSET_REGION(h->refreshRegion);	    /* Will be created when needed */
    h->prev_nrow = h->prev_ncol = 0;

    r->encoding_method = ENC_NOENC;
    h->multichar_decode = rxvt_decode_dummy;

    h->oldcursor.row = h->oldcursor.col = -1;
    h->last_bot = h->last_state = -1;

#ifdef HAVE_X11_SM_SMLIB_H
    SET_NULL(r->TermWin.sm_conn);
    SET_NULL(r->TermWin.ice_conn);
    r->TermWin.ice_fd = -1;
    SET_NULL(r->TermWin.sm_client_id);
#endif

#ifdef USE_FIFO
    {
	char fifo_name[FILENAME_MAX];

	sprintf( fifo_name, "/tmp/.mrxvt-%d", getpid() );
	r->fbuf_ptr = r->fifo_buf;
	r->fifo_name = STRDUP( fifo_name );
    }
#endif/*USE_FIFO*/

    r->tabClicked = -1; /* No tab has been clicked by user */

    h->allowedxerror = 0;
    h->xerror_return = Success;
    return 0;
}


/* EXTPROTO */
void
rxvt_init_secondary(rxvt_t *r)
{
    int		i, num_fds;
#ifdef TTY_GID_SUPPORT
    struct group   *gr = getgrnam("tty");

    if (gr)	    /* change group ownership of tty to "tty" */
    {
	r->h->ttygid = gr->gr_gid;
    }
    else
#endif		    /* TTY_GID_SUPPORT */
    {
	r->h->ttygid = getgid();
    }

    rxvt_set_default_locale (r);

    /* get number of available file descriptors */
#if defined(_POSIX_VERSION) || ! defined(OS_SVR4)
    num_fds = (int)sysconf(_SC_OPEN_MAX);
#else
    num_fds = rxvt_getdtablesize();
#endif

    /*
    ** Close all unused file descriptors
    ** We don't want them, we don't need them.
    */
    if ((i = open("/dev/null", O_RDONLY)) < 0)
    {
	/* TODO: BOO HISS */
	dup2(STDERR_FILENO, STDIN_FILENO);
    }
    else if (i > STDIN_FILENO)
    {
	dup2(i, STDIN_FILENO);
	close(i);
    }
    dup2(STDERR_FILENO, STDOUT_FILENO);
    for (i = STDERR_FILENO + 1; i < num_fds; i++)
    {
    /* #ifdef __sgi */
#ifdef OS_IRIX
	/* Alex Coventry says we need 4 & 7 too */
	if (i == 4 || i == 7)
	    continue;
#endif
	close(i);
    }

    /* Now set the correct num_fds */
    r->num_fds = STDERR_FILENO + 1;
#ifdef OS_IRIX
    r->num_fds = 7 + 1;
#endif
}


/* INTPROTO */
int
rxvt_xerror_handler(const Display *display __attribute__((unused)), const XErrorEvent *event)
{
    rxvt_t*	r = rxvt_get_r();
    char    error_msg[1024];

    XGetErrorText (r->Xdisplay, event->error_code, error_msg, 1023);
    r->h->xerror_return = event->error_code;

    if( !r->h->allowedxerror )
    {
	rxvt_msg (DBG_ERROR, DBG_INIT, "%s", error_msg);

#ifdef DEBUG_X
	abort();
#endif
    }

    return 0;	    /* ignored anyway */
}


#ifdef TEXT_SHADOW
/* INTPROTO */
void
rxvt_init_shadow_mode (rxvt_t* r, const char* shadow_mode)
{
    if (
	  !shadow_mode || !STRCASECMP ("botright", shadow_mode)
	  || !STRCASECMP ("default", shadow_mode))
    {
	r->TermWin.shadow_mode = SHADOW_BOTRIGHT;
    }
    else if (!STRCASECMP ("botleft", shadow_mode))
    {
	r->TermWin.shadow_mode = SHADOW_BOTLEFT;
    }
    else if (!STRCASECMP ("topright", shadow_mode))
    {
	r->TermWin.shadow_mode = SHADOW_TOPRIGHT;
    }
    else if (!STRCASECMP ("topleft", shadow_mode))
    {
	r->TermWin.shadow_mode = SHADOW_TOPLEFT;
    }
    else if (!STRCASECMP ("top", shadow_mode))
    {
	r->TermWin.shadow_mode = SHADOW_TOP;
    }
    else if (!STRCASECMP ("bottom", shadow_mode))
    {
	r->TermWin.shadow_mode = SHADOW_BOTTOM;
    }
    else if (!STRCASECMP ("left", shadow_mode))
    {
	r->TermWin.shadow_mode = SHADOW_LEFT;
    }
    else if (!STRCASECMP ("right", shadow_mode))
    {
	r->TermWin.shadow_mode = SHADOW_RIGHT;
    }
    else if (!STRCASECMP ("none", shadow_mode))
    {
	r->TermWin.shadow_mode = SHADOW_NONE;
    }
    else	/* no match == default */
    {
	r->TermWin.shadow_mode = SHADOW_NONE;
    }
}
#endif


/*----------------------------------------------------------------------*/
/* EXTPROTO */
void
rxvt_set_jumpscroll( rxvt_t *r )
{
    if( r->h->rs[Rs_refreshLimit] )
    {
	r->h->refresh_limit = atol( r->h->rs[Rs_refreshLimit] );
	if( r->h->refresh_limit < 0 )
	    r->h->refresh_limit = 0;
    }
    else
	r->h->refresh_limit = DEFAULT_REFRESH_LIMIT;

    if( r->h->rs[Rs_skipPages] )
    {
	r->h->skip_pages = atol( r->h->rs[Rs_skipPages] );
	if( r->h->skip_pages <= 0 )
	    r->h->skip_pages = 1;
    }
    else
	r->h->skip_pages = DEFAULT_SKIP_PAGES;
}


/* EXTPROTO */
const char**
rxvt_init_resources(rxvt_t* r, int argc, const char *const *argv)
{
    register int    i, r_argc;
    const char**    cmd_argv;
    const char**    r_argv;
    const char**    rs;


    /*
     * Look for -exec option.  Find => split and make cmd_argv[] of command args
     */
    for( r_argc = 0; r_argc < argc; r_argc++ )
	if( !STRCMP(argv[r_argc], "-e") || !STRCMP(argv[r_argc], "-exec") )
	    break;

    r_argv = (const char**) rxvt_malloc( sizeof(char*) * (r_argc + 1) );

    for( i = 0; i < r_argc; i++ )
	r_argv[i] = (const char*) argv[i];
    SET_NULL(r_argv[i]);

    if (r_argc == argc)
	SET_NULL(cmd_argv);
    else
    {
	cmd_argv = (const char **)rxvt_malloc(sizeof(char*) * (argc - r_argc));

	for (i = 0; i < argc - r_argc - 1; i++)
	    cmd_argv[i] = (const char *)argv[i + r_argc + 1];
	SET_NULL(cmd_argv[i]);
    }

    /* clear all resources */
    rs = r->h->rs;
    for (i = 0; i < NUM_RESOURCES;)
	SET_NULL(rs[i++]);

    rs[Rs_name] = rxvt_r_basename( argv[0] );

    /*
     * Open display, get options/resources and create the window
     */
    rxvt_get_options( r, r_argc, r_argv );
    rxvt_free( r_argv ); /* XXX memory leak? */

#ifdef LOCAL_X_IS_UNIX
    /*
     * 2008-04-29 gi1242: Force UNIX sockets for security (Gentoo Bug #219750)
     */
    if( IS_NULL( rs[Rs_display_name] ) )
	rs[Rs_display_name] = getenv( "DISPLAY" );

    if( rs[Rs_display_name] && rs[Rs_display_name][0] == ':' )
    {
	char *val;
	int l = 5 + STRLEN(rs[Rs_display_name]);
	if (l <= 0 || l > 1024) /* possible integer overflow */
	    l = 1024;

	val = rxvt_malloc(l);
	STRCPY( val, "unix");
	STRNCAT( val, rs[Rs_display_name], l-5);

	rs[Rs_display_name] = val;
    }
#endif /* LOCAL_X_IS_UNIX */

    rxvt_msg( DBG_INFO, DBG_INIT, "Open X display %s\n", rs[Rs_display_name] );
    r->Xdisplay = XOpenDisplay( rs[Rs_display_name] );
    if( IS_NULL(r->Xdisplay) )
    {
	rxvt_msg( DBG_ERROR, DBG_INIT, "Error opening display %s\n",
		rs[Rs_display_name] );
	exit( EXIT_FAILURE );
    }


#ifdef DEBUG_X
    /*
     * Makes life a lot simpler when handling X events, as they are not cached,
     * but processed immediately.
     */
    XSynchronize( r->Xdisplay, True );
#endif

    /*
     * Always set XErrorHandler to our own error handler because sometimes
     * errors are legal! Our error handler will abort when errors are not
     * allowed.
     */
    XSetErrorHandler( (XErrorHandler) rxvt_xerror_handler );

    /* Initialize all atoms after establishing connection to X */
    for (i = 0; i < NUM_XA; i++)
	r->h->xa[i] = XInternAtom( r->Xdisplay, xa_names[i], False );

    rxvt_extract_resources( r, r->Xdisplay, rs[Rs_name] );

    /*
     * set any defaults not already set
     */
    if( cmd_argv && cmd_argv[0] )
    {
	if( !rs[Rs_title] )
	    rs[Rs_title] = rxvt_r_basename( cmd_argv[0] );
	if( !rs[Rs_iconName] )
	    rs[Rs_iconName] = rs[Rs_title];
    }
    else
    {
	if( !rs[Rs_title] )
	    rs[Rs_title] = rs[Rs_name];
	if( !rs[Rs_iconName] )
	    rs[Rs_iconName] = rs[Rs_name];
    }

    if( rs[Rs_maxTabWidth] )
    {
	register int	tmp = atoi( rs[ Rs_maxTabWidth]);
	r->TermWin.maxTabWidth = ( tmp >=1 && tmp <= MAX_DISPLAY_TAB_TXT ) ?
					tmp : MAX_DISPLAY_TAB_TXT;
    }
    else
	/*
	 * If we're using Xft, then we will probably also use a PFont. So we
	 * should set this to the maximum possible.
	 */
	r->TermWin.maxTabWidth =
#ifdef XFT_SUPPORT
	    ISSET_OPTION(r, Opt_xft) ? MAX_DISPLAY_TAB_TXT :
#endif
	    DEFAULT_DISPLAY_TAB_TXT;


    if( rs[Rs_minVisibleTabs] )
    {
	register int	n = atoi( rs[Rs_minVisibleTabs] );
	r->TermWin.minVisibleTabs = (n >= 2) ?
	    n : DEFAULT_MIN_VISIBLE_TABS;
    }
    else r->TermWin.minVisibleTabs = DEFAULT_MIN_VISIBLE_TABS;

#ifndef NO_FRILLS
    if (rs[Rs_int_bwidth])
    {
	register int	tmp = atoi( rs[Rs_int_bwidth] );
	r->TermWin.int_bwidth =( tmp >= 0 && tmp <= MAX_INTERNALBORDERWIDTH ) ?
					tmp : DEFAULT_INTERNALBORDERWIDTH;
    }

    if (rs[Rs_ext_bwidth])
    {
	register int	tmp = atoi( rs[Rs_ext_bwidth] );
	r->TermWin.ext_bwidth = (tmp >= 0 && tmp <= MAX_EXTERNALBORDERWIDTH) ?
					tmp : DEFAULT_EXTERNALBORDERWIDTH;
    }
#endif

#ifndef NO_LINESPACE
    if (rs[Rs_lineSpace])
    {
	register int	tmp = atoi( rs[Rs_lineSpace] );
	r->TermWin.lineSpace = (tmp >= 0 && tmp <= MAX_LINESPACE) ?
					tmp : DEFAULT_LINESPACE;
    }
#endif

#ifdef POINTER_BLANK
    if (rs[Rs_pointerBlankDelay])
    {
	register int	tmp = atoi( rs[Rs_pointerBlankDelay] );
	r->h->pointerBlankDelay = (tmp >= 0 && tmp <= MAX_BLANKDELAY) ?
					tmp : DEFAULT_BLANKDELAY;
    }
#endif

    /* Handle opacity of translucent window */
    if (rs[Rs_opacity])	
    {
	register int	tmp = atoi( rs[Rs_opacity] );
	r->TermWin.opacity = (tmp >= 0 && tmp <= 100) ? 100 - tmp : 0;

#ifdef TRANSPARENT
	if (
	      IS_ATOM(r->h->xa[XA_NET_WM_WINDOW_OPACITY]) &&
	      ISSET_OPTION(r, Opt_transparent)
	   )
	{
	    /* Override pseudo-transparent */
	    UNSET_OPTION(r, Opt_transparent);
	}
#endif
    }
    if (rs[Rs_opacityDegree])	
    {
	register int	tmp = atoi (rs[Rs_opacityDegree]);
	r->TermWin.opacity_degree = (tmp > 0 && tmp <= 100) ? tmp : 1;
    }

#ifdef TINTING_SUPPORT
    if (rs[Rs_shade])
    {
	register int	shade;
	shade = atoi( rs[Rs_shade] );
	if (shade < 0 || shade > 100)
	    shade = 100;
	r->TermWin.shade = 100 - shade;
    }
#endif

    rxvt_set_jumpscroll(r);

#ifdef TRANSPARENT
    if (rs[Rs_bgRefreshInterval])
    {
	register unsigned long interval = atol( rs[Rs_bgRefreshInterval] );

	if( interval > 1000 ) interval = 1000;
	r->h->bgRefreshInterval = interval * 1000L; /* convert to micro-sec */
    }
#endif

    if (rs[Rs_fade])
    {
	register int	fade;
	fade = atoi( rs[Rs_fade] );

	/*
	 * Fade levels of 0 will make the text completely black, so let's ignore
	 * it.
	 */
	if( fade <= 0 || fade > 100 )
	    fade = 100;
	r->TermWin.fade = 100 - fade;
    }
    /* else r->TermWin.fade is 0 */

#ifdef CURSOR_BLINK
    if (rs[Rs_cursorBlinkInterval])
    {
	register long	tmp = atol( rs[Rs_cursorBlinkInterval] );
	r->h->blinkInterval = (tmp >= MIN_BLINK_TIME && tmp <= MAX_BLINK_TIME) ? tmp : DEFAULT_BLINK_TIME;
    }
    /* convert msec to usec */
    r->h->blinkInterval *= 1000;
#endif

#ifdef PRINTPIPE
    if (!rs[Rs_print_pipe])
	rs[Rs_print_pipe] = PRINTPIPE;
#endif
    if (!rs[Rs_cutchars])
	rs[Rs_cutchars] = CUTCHARS;

#ifdef ACS_ASCII
    if( !rs[Rs_acs_chars] )
	rs[Rs_acs_chars] = ACS_CHARS;
    if( (i = STRLEN(rs[Rs_acs_chars])) < 0x20 )
    {
	char *val = rxvt_realloc( (void*) rs[Rs_acs_chars], 0x20 );
	for( ; i < 0x20; )
	    val[i++] = ' ';
	rs[Rs_acs_chars] = val;
    }
#endif

#ifndef NO_BACKSPACE_KEY
    if( !rs[Rs_backspace_key] )
# ifdef DEFAULT_BACKSPACE
	r->h->key_backspace = DEFAULT_BACKSPACE;
# else
	r->h->key_backspace = "DEC";	/* can toggle between \010 or \177 */
# endif
    else
    {
	char*	val = STRDUP(rs[Rs_backspace_key]);
	rxvt_str_trim( val );
	rxvt_str_escaped( val );
	r->h->key_backspace = val;
    }
#endif
#ifndef NO_DELETE_KEY
    if( !rs[Rs_delete_key] )
# ifdef DEFAULT_DELETE
	r->h->key_delete = DEFAULT_DELETE;
# else
	r->h->key_delete = "\033[3~";
# endif
    else
    {
	char *val = STRDUP( rs[Rs_delete_key] );
	rxvt_str_trim( val );
	rxvt_str_escaped( val );
	r->h->key_delete = val;
    }
#endif
    if( rs[Rs_answerbackstring] )
    {
	rxvt_str_trim( (char*) rs[Rs_answerbackstring] );
	rxvt_str_escaped( (char*) rs[Rs_answerbackstring] );
    }

    if( rs[Rs_selectstyle] )
    {
	if( STRNCASECMP( rs[Rs_selectstyle], "oldword", 7 ) == 0 )
	    r->selection_style = OLD_WORD_SELECT;
#ifndef NO_OLD_SELECTION
	else if( STRNCASECMP( rs[Rs_selectstyle], "old", 3 ) == 0 )
	    r->selection_style = OLD_SELECT;
#endif
    }


    /* Set default X11 fonts */
    rxvt_set_default_font_x11( r );
#ifdef XFT_SUPPORT
    if( rs[Rs_xftsz] )
    {
	int	sz = atoi( rs[Rs_xftsz] );
	r->TermWin.xftsize = (sz >= MIN_XFT_FONT_SIZE) ? sz : MIN_XFT_FONT_SIZE;
    }
    else /* default xft font size */
	r->TermWin.xftsize = DEFAULT_XFT_FONT_SIZE;

    if (rs[Rs_xftpsz])
    {
	int	sz = atoi (rs[Rs_xftpsz]);
	r->TermWin.xftpsize  =  (sz >= MIN_XFT_FONT_SIZE) ?
					sz : MIN_XFT_FONT_SIZE;
    }
    else /* default xft Pfont size */
	r->TermWin.xftpsize = DEFAULT_XFT_PFONT_SIZE;

# ifdef MULTICHAR_SET
    if (rs[Rs_xftmsz])
    {
	int	sz = (int) atof (rs[Rs_xftmsz]);
	r->TermWin.xftmsize = (sz >= MIN_XFT_FONT_SIZE) ?
					sz : MIN_XFT_FONT_SIZE;
    }
    else /* default xft font size */
	r->TermWin.xftmsize = DEFAULT_XFT_FONT_SIZE;
# endif	/* MULTICHAR_SET */

    /* Set default Freetype fonts */
    rxvt_set_default_font_xft (r);
#endif	/* XFT_SUPPORT */


#ifdef TEXT_SHADOW
    rxvt_init_shadow_mode (r, rs[Rs_textShadowMode]);
#endif

#ifdef XTERM_REVERSE_VIDEO
    /* this is how xterm implements reverseVideo */
    if (ISSET_OPTION(r, Opt_reverseVideo))
    {
	if (!rs[Rs_color + Color_fg])
	    rs[Rs_color + Color_fg] = def_colorName[Color_bg];
	if (!rs[Rs_color + Color_bg])
	    rs[Rs_color + Color_bg] = def_colorName[Color_fg];

	for (i = 0; i < MAX_PROFILES; i++)
	{
	    int	    vtfg = Rs_foreground + i;
	    int	    vtbg = Rs_background + i;

	    char*   fg = (char*) rs[vtfg];
	    char*   bg = (char*) rs[vtbg];

	    /* foreground color of i terminal */
	    if (ISSET_VTFG(r, i))
		rs[vtfg] = ISSET_VTBG(r, i) ? bg :
				def_colorName[Color_bg];
	    /* background color of i terminal */
	    if (ISSET_VTBG(r, i))
		rs[vtbg] = ISSET_VTFG(r, i) ? fg :
			    def_colorName[Color_fg];
	}
    }
#endif

    for (i = 0; i < NRS_COLORS; i++)
	if (!rs[Rs_color + i])
	    rs[Rs_color + i] = def_colorName[i];

#ifndef XTERM_REVERSE_VIDEO
    /* this is how we implement reverseVideo */
    if (ISSET_OPTION(r, Opt_reverseVideo))
    {
	if (!rs[Rs_color + Color_fg])
	    rs[Rs_color + Color_fg] = def_colorName[Color_fg];
	if (!rs[Rs_color + Color_bg])
	    rs[Rs_color + Color_bg] = def_colorName[Color_bg];

	SWAP_IT(rs[Rs_color + Color_fg], rs[Rs_color + Color_bg], const char *);

	for (i = 0; i < MAX_PROFILES; i++)
	{
	    int	    vtfg = Rs_foreground + i;
	    int	    vtbg = Rs_background + i;
	    if (!rs[vtfg])
		rs[vtfg] = def_colorName[Color_fg];
	    if (!rs[vtbg])
		rs[vtbg] = def_colorName[Color_bg];

	    SWAP_IT(rs[vtfg], rs[vtbg], const char*);
	}
    }
#endif

    /* convenient aliases for setting fg/bg to colors */
    rxvt_color_aliases(r, Color_fg);
    rxvt_color_aliases(r, Color_bg);
#ifndef NO_CURSORCOLOR
    rxvt_color_aliases(r, Color_cursor);
    rxvt_color_aliases(r, Color_cursor2);
#endif		    /* NO_CURSORCOLOR */
    rxvt_color_aliases(r, Color_pointer);
    rxvt_color_aliases(r, Color_border);
#ifndef NO_BOLD_UNDERLINE_REVERSE
    rxvt_color_aliases(r, Color_BD);
    rxvt_color_aliases(r, Color_UL);
    rxvt_color_aliases(r, Color_RV);
#endif		    /* ! NO_BOLD_UNDERLINE_REVERSE */

    /*
     * On startup, use autohideTabbar to override hideTabbar. Thus on startup,
     * using autohideTabbar will only display the tabbar if there are multiple
     * tabs. The user can hide / show the tabbar using a macro at will.
     */
    if(ISSET_OPTION(r, Opt2_autohideTabbar))
	SET_OPTION(r, Opt2_hideTabbar);

    /* Cleanup the macro list */
    rxvt_cleanup_macros( r );


    /*
     * Profile settings.
     */
    for( i=0; i < MAX_PROFILES; i++ )
    {
	/* Set saveLines */
	if( r->h->rs[Rs_saveLines + i] )
	{
	    int tmp = atoi( r->h->rs[Rs_saveLines + i] );

	    r->profile[i].saveLines = ( tmp >= 0 && tmp <= MAX_SAVELINES ) ?
		    tmp : DEFAULT_SAVELINES;
	}
	else
	    r->profile[i].saveLines = ( i > 0 ) ? r->profile[0].saveLines :
							    DEFAULT_SAVELINES;

	/* Set holdOption */
	if( r->h->rs[Rs_holdExit + i] )
	{
	    const char *s = r->h->rs[Rs_holdExit + i];

	    /* Backward compatibility hack */
	    if(
		 !STRCASECMP( s, "true" )	    ||
		 !STRCASECMP( s, "yes" )	    ||
		 !STRCASECMP( s, "on" )
	      )
		r->profile[i].holdOption = HOLD_ALWAYSBIT;
	    else
		r->profile[i].holdOption = strtoul( s, NULL, 0 );
	}
	else
	    r->profile[i].holdOption = (i > 0) ? r->profile[0].holdOption :
					    (HOLD_STATUSBIT|HOLD_NORMALBIT);
    } /* for(i) */

    if( !r->h->rs[Rs_holdExitTtl] )
	r->h->rs[Rs_holdExitTtl] = "(Done) %t";

    if( !r->h->rs[Rs_holdExitTxt] )
	r->h->rs[Rs_holdExitTxt] = "\n\n\r\e[31m"
				   "Process exited %N with status %S. "
				   "Press any key to close tab.\e[0m";

#ifdef OS_LINUX
    if( !r->h->rs[Rs_cwd] )
	r->h->rs[Rs_cwd] = ".";
#endif

#ifndef NO_BEEP
    if( r->h->rs[Rs_vBellDuration] )
	r->TermWin.vBellDuration =
	    1000000ul * strtoul( r->h->rs[Rs_vBellDuration], NULL, 0 );
    else
	r->TermWin.vBellDuration = 0;
#endif


    return cmd_argv;
}

/*----------------------------------------------------------------------*/
/* EXTPROTO */
void
rxvt_init_env(rxvt_t *r)
{
    int		    i;
    unsigned int    u;
    char*	    val;


#ifdef DISPLAY_IS_IP
    /* Fixup display_name for export over pty to any interested
    ** terminal clients via "ESC[7n" (e.g. shells).  Note we use
    ** the pure IP number (for the first non-loopback interface)
    ** that we get from rxvt_network_display(). This is more
    ** "name-resolution-portable", if you will, and probably allows
    ** for faster x-client startup if your name server is beyond
    ** a slow link or overloaded at client startup. Of course that
    ** only helps the shell's child processes, not us.
    **
    ** Giving out the display_name also affords a potential
    ** security hole
    */
    val = rxvt_network_display(r->h->rs[Rs_display_name]);
    r->h->rs[Rs_display_name] = (const char *)val;
    if (IS_NULL(val))
#endif		    /* DISPLAY_IS_IP */
    val = XDisplayString(r->Xdisplay);
    if (IS_NULL(r->h->rs[Rs_display_name]))
	r->h->rs[Rs_display_name] = val;    /* use broken `:0' value */

    i = STRLEN(val) + 9;
    if (i <= 0 || i > 1024) /* possible integer overflow */
	i = 1024;
    r->h->env_display = rxvt_malloc(i * sizeof(char));
    STRCPY (r->h->env_display, "DISPLAY=");
    STRNCAT (r->h->env_display, val, i-9);
    r->h->env_display[i-1] = (char) 0;

    /* avoiding the math library:
     * i = (int)(ceil(log10((unsigned int)r->TermWin.parent))) */
    for (i = 0, u = (unsigned int)r->TermWin.parent; u; u /= 10, i++)
	;
    MAX_IT(i, 1);
    r->h->env_windowid = rxvt_malloc((i + 10) * sizeof(char));

    sprintf(r->h->env_windowid, "WINDOWID=%u",
	(unsigned int)r->TermWin.parent);

    /*
    ** add entries to the environment:
    ** @ DISPLAY:   in case we started with -display
    ** @ WINDOWID:  X window id number of the window
    ** @ COLORTERM: terminal sub-name and also indicates its color
    ** @ TERM:	    terminal name
    ** @ TERMINFO:  path to terminfo directory
    */
#ifdef HAVE_PUTENV
    putenv(r->h->env_display);
    putenv(r->h->env_windowid);

# ifdef RXVT_TERMINFO
    putenv("TERMINFO=" RXVT_TERMINFO);
# endif
    if (XDEPTH <= 2)
	putenv("COLORTERM=" COLORTERMENV "-mono");
    else
	putenv("COLORTERM=" COLORTERMENVFULL);
    if (NOT_NULL(r->h->rs[Rs_term_name]))
    {
	int	l = 6 + STRLEN(r->h->rs[Rs_term_name]);
	if (l <= 0 || l > 1024)	/* possible integer overflow */
	    l = 1024;
	r->h->env_term = rxvt_malloc(l * sizeof(char));
	STRCPY (r->h->env_term, "TERM=");
	STRNCAT (r->h->env_term, r->h->rs[Rs_term_name], l-6);
	r->h->env_term[l-1] = (char) 0;
	putenv(r->h->env_term);
    }
    else
	putenv("TERM=" TERMENV);
#endif	/* HAVE_PUTENV */

#ifdef HAVE_UNSETENV
    /* avoid passing old settings and confusing term size */
    unsetenv("LINES");
    unsetenv("COLUMNS");
    unsetenv("TERMCAP");    /* terminfo should be okay */
#endif		    /* HAVE_UNSETENV */

    /*
    ** allocate environment variable for MRXVT_TABTITLE, we will
    ** use it in rxvt_create_termwin later for each tab terminal
    */
    r->h->env_tabtitle = rxvt_malloc(sizeof(TABTITLEENV) + MAX_TAB_TXT + 1);
}

/*----------------------------------------------------------------------*/
/*
 * This is more or less stolen straight from XFree86 xterm.
 * This should support all European type languages.
 */
/* EXTPROTO */
void
rxvt_init_xlocale(rxvt_t *r)
{
#ifdef USE_XIM
    if (IS_NULL(r->h->locale))
	rxvt_msg (DBG_ERROR, DBG_INIT, "Setting locale failed.");
    else
    {
	XChangeProperty(r->Xdisplay, r->TermWin.parent,
	    r->h->xa[XA_WM_LOCALE_NAME], XA_STRING, 8, PropModeReplace,
	    (unsigned char *)r->h->locale, STRLEN(r->h->locale));

	if (XSupportsLocale() != True)
	{
	    rxvt_msg (DBG_ERROR, DBG_INIT, "The locale is not supported by Xlib");
	    return;
	}
	rxvt_IM_set_fontset (r, 0);

	/* see if we can connect yet */
	rxvt_IM_init_callback (r->Xdisplay, NULL, NULL);

	/* To avoid Segmentation Fault in C locale: Solaris only? */
	if (STRCMP(r->h->locale, "C"))
	    XRegisterIMInstantiateCallback(r->Xdisplay, NULL, NULL,
		NULL, rxvt_IM_init_callback, NULL);
    }
#endif
}

/*----------------------------------------------------------------------*/

/* EXTPROTO */
#ifdef USE_FIFO
void
rxvt_init_fifo( rxvt_t *r )
{
    unlink( r->fifo_name );
    mkfifo( r->fifo_name, 0600 );

    /*
     * Create the fifo in read write mode. If not, when no clients have the
     * fifo open, select() will claim our fifo has data pending and return.
     */
    r->fifo_fd = open( r->fifo_name, O_RDONLY|O_NDELAY );
    if( r->fifo_fd == -1 )
	UNSET_OPTION( r, Opt_useFifo );
    else
	MAX_IT( r->num_fds, r->fifo_fd + 1);

    /* Reset the fifo buffer */
    r->fbuf_ptr = r->fifo_buf;
}
#endif

/* EXTPROTO */
void
rxvt_init_command(rxvt_t* r)
{
    /*
     * Initialize the command connection. This should be called after the X
     * server connection is established.
     */
    struct sigaction	act;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "%s()\n", __func__));


    /*
     * Enable delete window protocol so that if the top-level window of the
     * terminal is destroyed by the Session Manager, we can receive a
     * ClientMessage event and do something gracefully.
     */
    XSetWMProtocols (r->Xdisplay, r->TermWin.parent,
	    &(r->h->xa[XA_WMDELETEWINDOW]), 1);

#ifdef META8_OPTION
    r->h->meta_char = (ISSET_OPTION(r, Opt_meta8) ? 0x80 : C0_ESC);
#endif
    rxvt_get_ourmods(r);

#ifdef GREEK_SUPPORT
    greek_init();
#endif

    r->Xfd = XConnectionNumber(r->Xdisplay);
#ifdef USE_FIFO
    if( ISSET_OPTION( r, Opt_useFifo ) )
	rxvt_init_fifo( r );
    else
	r->fifo_fd = -1;
#endif

#ifdef CURSOR_BLINK
    if (ISSET_OPTION(r, Opt_cursorBlink))
	(void)gettimeofday(&r->h->lastcursorchange, NULL);
#endif

    /*
     * Gracefully exit on term signals.
     */
    act.sa_handler  = rxvt_Exit_signal;
    act.sa_flags    = 0;
    sigemptyset (&act.sa_mask);

#ifndef OS_SVR4
    sigaction( SIGINT , &act, NULL);
#endif
    sigaction( SIGQUIT, &act, NULL);
    sigaction( SIGTERM, &act, NULL);

    /*
     * 2006-04-28 gi1242: Ignore HUP signals. We sometimes receive this if bash
     * is killed. Chances are that we don't have to exit ...
     */
    act.sa_handler  = SIG_IGN;
    sigaction( SIGHUP, &act, NULL);

#ifdef PRINTPIPE
    /*
     * 2006-04-28 gi1242: If there is an error opening the printer command, then
     * we'll get SIGPIPE. If not handled, mrxvt will exit.
     * 
     * There's nothing we really need to do on broken pipes, so just ignore
     * SIGPIPE for now.
     */
    sigaction( SIGPIPE, &act, NULL);
#endif

    act.sa_handler  = rxvt_Child_signal;
    sigaction (SIGCHLD, &act, NULL);
}


/* EXTPROTO */
void
rxvt_fade_color( rxvt_t* r, const XColor *xcol,
	unsigned long	*pix_return,
# ifdef XFT_SUPPORT
	XftColor	*xft_return
# else
	void		*xft_return
# endif
	)
{
    if( r->TermWin.fade )
    {
	XColor	faded_xcol;

	faded_xcol.red   = (xcol->red	/ 100) * r->TermWin.fade;
	faded_xcol.green = (xcol->green	/ 100) * r->TermWin.fade;
	faded_xcol.blue  = (xcol->blue	/ 100) * r->TermWin.fade;

	rxvt_alloc_color( r, &faded_xcol, "Faded" );

	*pix_return = faded_xcol.pixel;
# ifdef XFT_SUPPORT
	if( NOT_NULL( xft_return ) )
	    rxvt_alloc_xft_color( r, &faded_xcol, xft_return );

# endif
    }
}


#define setChanged( a, b )  \
    if( (a) != (b) ) {(a) = (b); changed = 1;}
#define setChangedXft( a, b ) \
    if( (a).pixel != (b).pixel ) { (a) = (b); changed = 1;}
/*
 * Sets r->pixColors[Color_fg] / etc to the correct color (depending on the ufbg
 * color, off focus fading and weather we have focus or not).
 */
/* EXTPROTO */
int
rxvt_set_fgbg_colors( rxvt_t *r, int page )
{
    int changed = 0;

    rxvt_dbgmsg(( DBG_DEBUG, DBG_INIT, "%s(r, page=%d)"
		": fgbg_tabnum=%d, globalTabNum=%d\n", __func__, page,
		r->fgbg_tabnum, PVTS(r, page)->globalTabNum));

    if(
	 r->fgbg_tabnum == PVTS(r, page)->globalTabNum &&
	 (
	   r->TermWin.fade			||
	   !ISSET_PIXCOLOR( r->h, Color_ufbg )	||
	   (
	     /*
	      * If we dont have fading, but have ufbg, then make sure that
	      * Color_bg points to the correct color.
	      */
	     r->TermWin.focus						?
		( r->pixColors[Color_bg] == PVTS(r, page)->p_bg )	:
		( r->pixColors[Color_bg] == r->pixColors[Color_ufbg] )
	   )
	 )
      )
	return 0;   /* No change */

    setChanged( r->pixColorsFocus[Color_fg], PVTS( r, page)->p_fg );
#ifdef XFT_SUPPORT
    if( ISSET_OPTION( r, Opt_xft ) )
	setChangedXft( r->xftColorsFocus[Color_fg], PVTS(r, page)->p_xftfg );
#endif

    if( r->TermWin.fade )
    {
	/* Ignore ufbg, and use faded colors */
	setChanged( r->pixColorsFocus[Color_bg], PVTS(r, page)->p_bg );

	setChanged( r->pixColorsUnfocus[Color_fg], PVTS(r, page)->p_fgfade );
	setChanged( r->pixColorsUnfocus[Color_bg], PVTS(r, page)->p_bgfade );

#ifdef XFT_SUPPORT
	if( ISSET_OPTION( r, Opt_xft ) )
	{
	    setChangedXft( r->xftColorsFocus[Color_bg],
		    PVTS(r, page)->p_xftbg );

	    setChangedXft( r->xftColorsUnfocus[Color_fg],
		    PVTS(r, page)->p_xftfgfade );
	    setChangedXft( r->xftColorsUnfocus[Color_bg],
		    PVTS(r, page)->p_xftbgfade );
	}
#endif
    }

    else if( ISSET_PIXCOLOR( r->h, Color_ufbg ) && !r->TermWin.focus )
    {
	/* No fading. But use Color_ufbg */
	setChanged( r->pixColorsFocus[Color_bg],
		r->pixColorsFocus[Color_ufbg] );
#ifdef XFT_SUPPORT
	if( ISSET_OPTION( r, Opt_xft ) )
	    setChangedXft( r->xftColorsFocus[Color_bg],
		    r->xftColorsFocus[Color_ufbg] );
#endif
    }

    else
    {
	/* Use fgbg from profile */
	setChanged( r->pixColorsFocus[Color_bg],
		PVTS(r, page)->p_bg );
#ifdef XFT_SUPPORT
	if( ISSET_OPTION( r, Opt_xft ) )
	    setChangedXft( r->xftColorsFocus[Color_bg],
		    PVTS(r, page)->p_xftbg );
#endif
    }

    r->fgbg_tabnum = PVTS( r, page )->globalTabNum;

    rxvt_dbgmsg(( DBG_DEBUG, DBG_INIT, "%s(r, page=%d) returning %d\n",
	    __func__, page, changed ));
    return changed; /* Changed */
}
#undef setChanged
#undef setChangedXft


void
rxvt_copy_color( rxvt_t *r, int dst_index, int src_index )
{
    r->pixColorsFocus[ dst_index ] = r->pixColorsFocus[ src_index ];

    if( r->TermWin.fade )
	r->pixColorsUnfocus[ dst_index ] = r->pixColorsUnfocus[ src_index ];

#ifdef XFT_SUPPORT
    if( ISSET_OPTION( r, Opt_xft ) )
    {
	r->xftColorsFocus[ dst_index ] = r->xftColorsFocus[ src_index ];

	if( r->TermWin.fade )
	    r->xftColorsUnfocus[ dst_index ] = r->xftColorsUnfocus[ src_index ];
    }
#endif

    SET_PIXCOLOR( r->h, dst_index );
}


void
rxvt_set_color( rxvt_t *r, int cIndex, const XColor *xcol )
{
    /* xcol must contain an ALLOCATED color */
    r->pixColorsFocus[cIndex] = xcol->pixel;
#ifdef XFT_SUPPORT
    if( ISSET_OPTION( r, Opt_xft ) )
	rxvt_alloc_xft_color( r, xcol, &r->xftColorsFocus[cIndex] );
#endif

    if( r->TermWin.fade )
    {
	if( cIndex == Color_pointer )
	{
	    /* Don't fade these colors */
	    r->pixColorsUnfocus[cIndex] = r->pixColorsFocus[cIndex];
#ifdef XFT_SUPPORT
	    if( ISSET_OPTION( r, Opt_xft ) )
		r->xftColorsUnfocus[cIndex] = r->xftColorsFocus[cIndex];
#endif
	}

	else
	    rxvt_fade_color( r, xcol, &r->pixColorsUnfocus[cIndex],
#ifdef XFT_SUPPORT
		ISSET_OPTION(r, Opt_xft) ? &r->xftColorsUnfocus[cIndex] : NULL
#else
		NULL
#endif
		);
    }

    SET_PIXCOLOR( r->h, cIndex );
}


/*
 * XXX 2006-05-24 gi1242: Should allocate Xft colors only if Opt_xft is set. In
 * that case we should avoid allocating XColors, since we can always access them
 * through xftcolor.pixel.
 */
/* INTPROTO */
void
rxvt_init_colors( rxvt_t *r )
{
    register int    i;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "%s()\n", __func__));

    /* Initialize fg/bg colors for each profile */
    for (i = 0; i < MAX_PROFILES; i++)
    {
	XColor	    xcol;
	int	    vtfg = Rs_foreground + i;
	int	    vtbg = Rs_background + i;

	if( !ISSET_VTFG( r, i ) )
	    r->h->rs[vtfg] =  ISSET_VTFG( r, 0 ) ?
		    r->h->rs[Rs_foreground] : def_colorName[ Color_fg ];
	if( !ISSET_VTBG( r, i ) )
	    r->h->rs[vtbg] = ISSET_VTBG( r, 0 ) ?
		    r->h->rs[Rs_background] : def_colorName[ Color_bg ];

	/* foreground color of i terminal */
	if( rxvt_parse_alloc_color(r, &xcol, r->h->rs[vtfg]) )
	{
	    VTFG(r, i) = xcol.pixel;

#ifdef XFT_SUPPORT
	    rxvt_alloc_xft_color( r, &xcol, &(VTXFTFG(r, i)) );
	    rxvt_fade_color( r, &xcol, &VTFG_FADE(r, i), &VTXFTFG_FADE(r, i) );
#else
	    rxvt_fade_color( r, &xcol, &VTFG_FADE(r, i), NULL );
#endif /* XFT_SUPPORT */
	}

	else
	{
	    rxvt_msg (DBG_ERROR, DBG_INIT,  "Could not alloc foreground color of profile %d",
		    i );
	    if( i == 0 )
		/* Need default fg/bg */
		exit( EXIT_FAILURE );

	    /* Use foreground from profie 0 */
	    VTFG( r, i ) = VTFG( r, 0 );
	    VTFG_FADE( r, i ) = VTFG_FADE( r, 0 );

#ifdef XFT_SUPPORT
	    VTXFTFG( r, i ) = VTXFTFG( r, 0 );
	    VTXFTFG_FADE( r, i ) = VTXFTFG_FADE( r, 0 );
#endif
	}

	/* background color of i terminal */
	if( rxvt_parse_alloc_color(r, &xcol, r->h->rs[vtbg]) )
	{
	    VTBG(r, i) = xcol.pixel;

#ifdef XFT_SUPPORT
	    rxvt_alloc_xft_color( r, &xcol, &(VTXFTBG(r, i)) );
	    rxvt_fade_color( r, &xcol, &VTBG_FADE(r, i), &VTXFTBG_FADE(r, i) );
#else
	    rxvt_fade_color( r, &xcol, &VTBG_FADE(r, i), NULL );
#endif /* XFT_SUPPORT */
	}

	else
	{
	    rxvt_msg (DBG_ERROR, DBG_INIT,  "Could not alloc background color of profile %d",
		    i );
	    if( i == 0 )
		/* Need default fg/bg */
		exit( EXIT_FAILURE );

	    /* Use background from profie 0 */
	    VTBG( r, i ) = VTBG( r, 0 );
	    VTBG_FADE( r, i ) = VTBG_FADE( r, 0 );

#ifdef XFT_SUPPORT
	    VTXFTBG( r, i ) = VTXFTBG( r, 0 );
	    VTXFTBG_FADE( r, i ) = VTXFTBG_FADE( r, 0 );
#endif
	}
    }

    /* Set foreground / background colors */
    r->pixColorsFocus[ Color_fg ] = VTFG( r, 0 );
    r->pixColorsFocus[ Color_bg ] = VTBG( r, 0 );

    if( r->TermWin.fade )
    {
	r->pixColorsUnfocus[ Color_fg ] = VTFG_FADE( r, 0 );
	r->pixColorsUnfocus[ Color_bg ] = VTBG_FADE( r, 0 );
    }

#ifdef XFT_SUPPORT
    if( ISSET_OPTION( r, Opt_xft ) )
    {
	r->xftColorsFocus[ Color_fg ] = VTXFTFG( r, 0 );
	r->xftColorsFocus[ Color_bg ] = VTXFTBG( r, 0 );

	if( r->TermWin.fade )
	{
	    r->xftColorsUnfocus[ Color_fg ] = VTXFTFG( r, 0 );
	    r->xftColorsUnfocus[ Color_bg ] = VTXFTBG( r, 0 );
	}
    }
#endif

    r->fgbg_tabnum = -1;    /* fg/bg corresponds to profile 0, not any
			       particular tab during initialization. */


    /*
     * Allocate generic colors.
     */
    for (i = minCOLOR; i < (XDEPTH <= 2 ? 2 : NRS_COLORS); i++)
    {
	XColor		xcol;

	if( IS_NULL(r->h->rs[Rs_color + i]) )
	    continue;

	if( !rxvt_parse_alloc_color(r, &xcol, r->h->rs[Rs_color + i]) )
	{
	    if( r->h->rs[Rs_color+i] != def_colorName[i] )
	    {
		rxvt_msg (DBG_ERROR, DBG_INIT,  "Could not allocate color '%s'\n",
			r->h->rs[Rs_color + i] );

		/* Try again with default color */
		r->h->rs[Rs_color + i] = def_colorName[i];
		i--;
		continue;
	    }

	    /*
	     * Unable to alloc even the default color. Fall back to fg/bg.
	     */
	    switch( i )
	    {
#ifndef NO_CURSORCOLOR
		case Color_cursor2:
#endif /* !NO_CURSORCOLOR */
		case Color_pointer:
		    rxvt_copy_color( r, i, Color_fg );
		    break;

		default:
		    rxvt_copy_color( r, i, Color_bg );
		    break;
	    }
	}

	else
	{
	    /*
	     * Succeeded allocating the color. Store it in pixColors.
	     */
	    rxvt_set_color( r, i, &xcol );
	}
    } /* for(i) */


    /*
     * Allocate colors which are essential if they have not been allocated.
     */
    if( XDEPTH <= 2 || !ISSET_PIXCOLOR( r->h, Color_pointer ) )
    {
	/*
	 * NOTE: Fading should be disabled for low depths. And the pointer color
	 * should not be faded.
	 */
	r->pixColorsFocus[Color_pointer]    = VTFG(r, 0);
	if( r->TermWin.fade )
	    r->pixColorsUnfocus[Color_pointer]  = VTFG(r, 0);
#ifdef XFT_SUPPORT
	if( ISSET_OPTION( r, Opt_xft ) )
	{
	    r->xftColorsFocus[Color_pointer]	= VTXFTFG(r, 0);
	    if( r->TermWin.fade )
		r->xftColorsUnfocus[Color_pointer]  = VTXFTFG(r, 0);
	}
#endif
	SET_PIXCOLOR( r->h, Color_pointer );
    }

    if( XDEPTH <= 2 || !ISSET_PIXCOLOR( r->h, Color_border ) )
	rxvt_copy_color( r, Color_border, Color_fg );


    /*
     * get scrollBar/menuBar shadow colors
     *
     * The calculations of topShadow/bottomShadow values are adapted from the
     * fvwm window manager.
     */
#ifdef KEEP_SCROLLCOLOR
    if (XDEPTH <= 2)	    /* Monochrome */
    {
	rxvt_copy_color( r, Color_scroll,	Color_fg );
	rxvt_copy_color( r, Color_topShadow,	Color_bg );
	rxvt_copy_color( r, Color_bottomShadow, Color_bg );
    }

    else
    {
	XColor		xcol[3];
	/*
	 * xcol[0] == white
	 * xcol[1] == top shadow
	 * xcol[2] == bot shadow
	 */

	xcol[1].pixel = r->pixColorsFocus[Color_scroll];
# ifdef PREFER_24BIT
	xcol[0].red = xcol[0].green = xcol[0].blue = 0xffffu;
	rxvt_alloc_color( r, &(xcol[0]), "White" );
	XQueryColors(r->Xdisplay, XCMAP, &(xcol[1]), 1);
# else
	xcol[0].pixel = WhitePixel(r->Xdisplay, XSCREEN);
	XQueryColors(r->Xdisplay, XCMAP, xcol, 2);
# endif

	/* bottomShadowColor */
	xcol[2].red	= xcol[1].red	/ 2;
	xcol[2].green	= xcol[1].green / 2;
	xcol[2].blue	= xcol[1].blue	/ 2;
	if( !rxvt_alloc_color( r, &(xcol[2]), "Color_bottomShadow" ) )
	    rxvt_copy_color( r, Color_bottomShadow, Color_Black );

	else
	    rxvt_set_color( r, Color_bottomShadow, &xcol[2] );


	/* topShadowColor */
	xcol[1].red	= max((xcol[0].red   / 5), xcol[1].red	);
	xcol[1].green	= max((xcol[0].green / 5), xcol[1].green);
	xcol[1].blue	= max((xcol[0].blue  / 5), xcol[1].blue	);
	xcol[1].red	= min(xcol[0].red,   (xcol[1].red   * 7) / 5);
	xcol[1].green	= min(xcol[0].green, (xcol[1].green * 7) / 5);
	xcol[1].blue	= min(xcol[0].blue,  (xcol[1].blue  * 7) / 5);

	if( !rxvt_alloc_color(r, &(xcol[1]), "Color_topShadow") )
	    rxvt_copy_color( r, Color_topShadow, Color_White );
	else
	    rxvt_set_color( r, Color_topShadow, &xcol[1] );

    }
#endif	/* KEEP_SCROLLCOLOR */


#ifdef TEXT_SHADOW
    if (r->h->rs[Rs_textShadow])
    {
	XColor	xcol;
	if( rxvt_parse_alloc_color( r, &xcol, r->h->rs[Rs_textShadow] ) )
	{
	    r->TermWin.shadow = xcol.pixel;
# ifdef XFT_SUPPORT
	    rxvt_alloc_xft_color( r, &xcol, &(r->TermWin.xftshadow));
# endif
	}
	else
	{
	    r->TermWin.shadow = r->pixColorsFocus[Color_Black];
# ifdef XFT_SUPPORT
	    r->TermWin.xftshadow = r->xftColorsFocus[Color_Black];
# endif
	}
    }
#endif
}


/*----------------------------------------------------------------------*/
/* color aliases, fg/bg bright-bold */
/* INTPROTO */
void
rxvt_color_aliases( rxvt_t *r, int idx )
{
    if (r->h->rs[Rs_color + idx] && isdigit((int) *(r->h->rs[Rs_color + idx])))
    {
	int	    i = atoi(r->h->rs[Rs_color + idx]);

	if (i >= 8 && i <= 15)		/* bright colors */
	{
	    i -= 8;
#ifndef NO_BRIGHTCOLOR
	    r->h->rs[Rs_color + idx] = r->h->rs[Rs_color + minBrightCOLOR + i];
	    return;
#endif
	}
	if (i >= 0 && i <= 7)	/* normal colors */
	    r->h->rs[Rs_color + idx] = r->h->rs[Rs_color + minCOLOR +i];
    }
}


/* INTPROTO */
void
rxvt_init_win_size( rxvt_t *r )
{
    int		    flags = 0;	/* must initialize to 0!!! */
    short	    recalc_x = 0, recalc_y = 0,
		    recalc_width = 1, recalc_height = 1;
    int		    x, y;
    unsigned int    w, h;


    r->szHint.flags = PMinSize | PResizeInc | PBaseSize | PWinGravity;
    r->szHint.win_gravity = NorthWestGravity;

    /* Set default terminal columns and rows */
    r->TermWin.ncol = 80;
    r->TermWin.nrow = 24;
    r->szHint.x = 0;
    r->szHint.y = 0;

#ifdef TRANSPARENT
    refreshRootBGVars( r );
#endif

    /* Get geometry in x, y, w, h */
    if (r->h->rs[Rs_geometry])
	flags = XParseGeometry(r->h->rs[Rs_geometry], &x, &y, &w, &h);

    /* Calculate the terminal increment width and height */
#ifndef NO_FRILLS
    if( ISSET_OPTION(r, Opt2_smoothResize))
    {
	r->szHint.width_inc = 1;
	r->szHint.height_inc = 1;
    }
    else
#endif
    {
	r->szHint.width_inc = r->TermWin.fwidth;
	r->szHint.height_inc = r->TermWin.fheight;
    }

    /* Calculate the base width and height */
    r->szHint.base_width = 2 * r->TermWin.int_bwidth;
    r->szHint.base_height = 2 * r->TermWin.int_bwidth;
#ifdef HAVE_SCROLLBARS
    if (ISSET_OPTION(r, Opt_scrollBar))
	r->szHint.base_width += rxvt_scrollbar_rwidth (r);
#endif
#ifdef HAVE_MENUBAR
    if (ISSET_OPTION(r, Opt_showMenu))
	r->szHint.base_height += rxvt_menubar_rheight (r);
#endif
    if (NOTSET_OPTION(r, Opt2_hideTabbar))
	r->szHint.base_height += rxvt_tabbar_rheight (r);

    /* Set the terminal minimal width and height */
    r->szHint.min_width = r->szHint.base_width + r->TermWin.fwidth;
    r->szHint.min_height = r->szHint.base_height + r->TermWin.fheight;

    /* Parse Geometry */
    if (flags & WidthValue)
    {
	r->TermWin.ncol = BOUND_POSITIVE_INT16(w);
#ifndef NO_FRILLS
	if( ISSET_OPTION(r, Opt2_smoothResize) )
	{
	    /* For smoothResize, w as a pixel width (if large enough) */
	    if(r->TermWin.ncol > r->szHint.base_width + r->TermWin.fwidth)
	    {
		r->szHint.width = r->TermWin.ncol;
		r->TermWin.ncol = (r->TermWin.ncol - r->szHint.base_width) /
							r->TermWin.fwidth;
		recalc_width = 0;
	    }
	    else r->TermWin.ncol = 1;
	}
#endif
	r->szHint.flags |= USSize;
    }
    if (flags & HeightValue)
    {
	r->TermWin.nrow = BOUND_POSITIVE_INT16(h);
#ifndef NO_FRILLS
	if(ISSET_OPTION(r, Opt2_smoothResize))
	{
	    /* For smoothResize, w as a pixel height (if large enough) */
	    if(r->TermWin.nrow > r->szHint.base_height + r->TermWin.fheight)
	    {
		r->szHint.height = r->TermWin.nrow;
		r->TermWin.nrow =
		    (r->TermWin.nrow - r->szHint.base_height) / r->TermWin.fheight;
		recalc_height = 0;
	    }
	    else r->TermWin.nrow = 1;
	}
#endif
	r->szHint.flags |= USSize;
    }
    if (flags & XValue)
    {
	r->szHint.x = x;
	r->szHint.flags |= USPosition;
	if (flags & XNegative)
	{
	    recalc_x = 1;
	    r->szHint.win_gravity = NorthEastGravity;
	}
    }
    if (flags & YValue)
    {
	r->szHint.y = y;
	r->szHint.flags |= USPosition;
	if (flags & YNegative)
	{
	    recalc_y = 1;
	    if (r->szHint.win_gravity == NorthEastGravity)
		r->szHint.win_gravity = SouthEastGravity;
	    else
		r->szHint.win_gravity = SouthWestGravity;
	}
    }

    /* Set the terminal width and height */
    if( recalc_width)
	r->szHint.width = r->szHint.base_width + Width2Pixel (r->TermWin.ncol);
    if( recalc_height)
	r->szHint.height = r->szHint.base_height + Height2Pixel (r->TermWin.nrow);

    /* Recalculate the starting position */
    if (recalc_x)
	r->szHint.x += (DisplayWidth(r->Xdisplay, XSCREEN)
	    - r->szHint.width - 2 * r->TermWin.ext_bwidth);
    if (recalc_y)
	r->szHint.y += (DisplayHeight(r->Xdisplay, XSCREEN)
	    - r->szHint.height - 2 * r->TermWin.ext_bwidth);

    /* Set the terminal window starting position */
    r->h->window_vt_x = (ISSET_OPTION(r, Opt_scrollBar_right)) ? 
	    0 : r->szHint.base_width - 2*r->TermWin.int_bwidth;
    r->h->window_vt_y = r->szHint.base_height - 2*r->TermWin.int_bwidth;
    if (ISSET_OPTION(r, Opt2_bottomTabbar) && NOTSET_OPTION(r, Opt2_hideTabbar))
	r->h->window_vt_y -= rxvt_tabbar_rheight (r);
}


/*----------------------------------------------------------------------*/
/*
 * Probe the modifier keymap to get the Meta (Alt) and Num_Lock settings
 * Use resource ``modifier'' to override the Meta modifier
 */
/* INTPROTO */
void
rxvt_get_ourmods( rxvt_t *r )
{
    int			i, j, k;
    int			requestedmeta, realmeta, realalt;
    const char*		cm;
    const char*		rsmod;
    XModifierKeymap*	map;
    KeyCode*		kc;
    const unsigned int	modmasks[] =
    {
	Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
    };


    requestedmeta = realmeta = realalt = 0;

    rsmod = r->h->rs[Rs_modifier];
    if (rsmod &&
	STRCASECMP(rsmod, "mod1") >= 0 &&
	STRCASECMP(rsmod, "mod5") <= 0)
	requestedmeta = rsmod[3] - '0';

    map = XGetModifierMapping(r->Xdisplay);
    kc = map->modifiermap;
    for (i = 1; i < 6; i++)
    {
	k = (i + 2) * map->max_keypermod;   /* skip shift/lock/control */
	for (j = map->max_keypermod; j--; k++)
	{
	    if (kc[k] == 0)
		break;

	    switch (XKeycodeToKeysym(r->Xdisplay, kc[k], 0))
	    {
		case XK_Num_Lock:
		    r->h->ModNumLockMask = modmasks[i - 1];
		    /* FALLTHROUGH */
		default:
		    continue;	/* for(;;) */
		case XK_Meta_L:
		case XK_Meta_R:
		    cm = "meta";
		    realmeta = i;
		    break;
		case XK_Alt_L:
		case XK_Alt_R:
		    cm = "alt";
		    realalt = i;
		    break;
		case XK_Super_L:
		case XK_Super_R:
		    cm = "super";
		    break;
		case XK_Hyper_L:
		case XK_Hyper_R:
		    cm = "hyper";
		    break;
	    }

	    if (rsmod && STRNCASECMP(rsmod, cm, STRLEN(cm)) == 0)
		requestedmeta = i;
	}
    }
    XFreeModifiermap(map);

    i = (requestedmeta ? requestedmeta : (
	    realmeta ? realmeta : (
	    realalt ? realalt : 0)));

    if (i)
	r->h->ModMetaMask = modmasks[i - 1];
}



/* EXTPROTO */
char**
rxvt_string_to_argv( const char *string, int *argc )
{
    int		    i = 0;
    char**	    pret;
    const char*	    pcur;
#ifdef INTERNAL_ARGV_SPLIT
    const char*	    pbeg;
#endif

    *argc = 0;
    if( IS_NULL(string) || *string == '\0' )
    {
	*argc = 0;
	return NULL;
    }

#define MAX_ARGV    (1024)
    /* Up to 64 argv.
     *
     * 2006-02-23 gi1242: Use calloc instead of malloc. Thus when freeing pret,
     * we can safely free all elements till we encounter a NULL pointer.
     */
    pret = (char**) rxvt_calloc (MAX_ARGV, sizeof (char*));

    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "fetch command argv for the tab\n"));
#ifdef INTERNAL_ARGV_SPLIT
    /*{{{*/
    /*
     * 2006-02-23 gi1242: Remember to leave space for a NULL terminated pointer
     * at the end
     */
    pbeg = pcur = string;
    for (i = 0; i < MAX_ARGV-2; i ++)
    {
	int	dq = 0;	/* double quote */
	int	sq = 0;	/* single quote */
	/* set default argument to NULL */
	SET_NULL(pret[i]);

	/* skip any spaces and non-printable */
	while (*pcur && 
	    (isspace ((int) *pcur) || !isprint ((int) *pcur)))
	    pcur ++;
	/* stop if reach end of string */
	if (!*pcur)
	    break;

	/* beginning of the token */
	if (isalnum ((int) *pcur) || ispunct ((int) *pcur))
	{
	    if ('\"' == *pcur)
	    {
		/* beginning of double quote */
		dq = 1;	pbeg = pcur + 1; pcur ++;
	    }
	    else if ('\'' == *pcur)
	    {
		/* beginning of single quote */
		sq = 1;	pbeg = pcur + 1; pcur ++;
	    }
	    else /* normal characters */
		pbeg = pcur;
	}
#ifdef DEBUG
	else	/* shouldn't happen */
	    assert (0);
#endif

	/* move forward one character */
	pcur ++;

	/* now fetch the new token */
	while(
		*pcur &&			/* not end of string */
		(
		  (dq && *pcur != '\"') ||	/* not end of double quote */
		  (sq && *pcur != '\'') ||	/* not end of single quote */
		  (!dq && !sq && !isspace ((int) *pcur))
		)
	     )
	{
	    pcur ++;
	}

	if (!*pcur &&	/* end of string */
	    (dq || sq))	/* no match of quote is found */
	    goto NotMatch;

	if (!*pcur)	/* end of string */
	{
	    pret[i] = STRDUP (pbeg);
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "   argv[%d] = %s\n", i, pret[i]));
	    break;	/* ready to return */
	}

	if (
	      (dq && *pcur == '\"')		    /* end of double quote */
	      || (sq && *pcur == '\'')		    /* end of single quote */
	      || (!dq && !sq && isspace ((int) *pcur))	    /* space */
	   )
	{
	    int	    len = sizeof (char) * (pcur - pbeg) + 1;

	    assert (len > 0);	/* possible integer overflow? */
	    pret[i] = (char*) rxvt_malloc (len * sizeof(char));
	    MEMCPY (pret[i], pbeg, len-1);
	    pret[i][len-1] = (char) 0;
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "   argv[%d] = %s\n", i, pret[i]));

	    /* forward to next character */
	    pcur ++;

	    /* fetch next token */
	    continue;
	}

	/* shouldn't get here */
	assert (0);
    }
    /*}}}*/
#else /* !INTERNAL_ARGV_SPLIT */

    /*
     * Pass strings beginning with "!" but not "!!" to /bin/sh -c
     */
    if( *string == '!' && *(++string) != '!' )
    {
	/* Pass command to the shell for word splitting */
	pret[i++] = STRDUP( "/bin/sh" );
	pret[i++] = STRDUP( "-c" );
	pret[i++] = STRDUP( string );
    }

    else
    {
	/*
	 * Split command into words at spaces. White spaces can be quoted with
	 * a backslash. However we don't processes chains of "\\" which don't
	 * end in a space or tab. That is "\\a" expands to "\\a" however "\\ "
	 * expands to "\ ", and "\ " expands to a " " which does not split
	 * words.
	 */
	pcur = string;
	for( ; i < MAX_ARGV && *pcur; i++ )
	{
	    const int	max_argv_len = 1024;
	    char	argval[max_argv_len];
	    int		j = 0;

	    /* Skip leading spaces */
	    while( *pcur == ' ' || *pcur == '\t' ) pcur++;

	    while(
		    *pcur && *pcur != ' ' && *pcur != '\t' &&
		    j < max_argv_len - 1
		 )
	    {
		if( *pcur == '\\' )
		{
		    const char *s = pcur + 1;

		    /* Count backslashes */
		    while( *s == '\\' )
			s++;

		    if( *s == ' ' || *s == '\t' )
		    {
			int nbslashs = s - pcur;
			int nbytes = min( nbslashs / 2, max_argv_len - j - 1);

			/* Halve # backslashes */
			MEMSET( &argval[j], '\\', nbytes );
			pcur	=  s;
			j	+= nbytes;

			if( nbslashs % 2 == 0 )
			    break;  /* Split word here */
		    }

		    else
		    {
			/* Copy backslashes over verbatim */
			int nbytes = min( s - pcur, max_argv_len -j -1 );

			MEMCPY( &argval[j], pcur, nbytes );
			j	+= nbytes;
			pcur	=  s;
		    }
		} /* if( pcur = '\\' ) */

		argval[j++] = *pcur++;
	    } /* while( *pcur ... ) */

	    if( j )
	    {
		argval[j] = '\0';
		pret[i] = STRDUP( argval );
	    }
	    else
		break;
	}
    } /* else [ if( *string != '!' ) ] */
#endif /* !INTERNAL_ARGV_SPLIT */

#undef MAX_ARGV
    /* set the end of argv */
    if (pret[i])
    {
	*argc = i+1;
	SET_NULL(pret[i+1]);
    }
    else if (i)	    /* non-empty argv */
    {
	*argc = i;
    }
    else	    /* empty argv */
    {
#if 0
	/* 2006-02-23 gi1242: Also need to free the char* pointers in pret? */
	rxvt_free (pret);
	return NULL;
#endif
	goto NotMatch;
    }

    /* 2006-02-23  gi1242: Can now reduce the size of pret. */
    pret = (char **) rxvt_realloc( pret, (*argc + 1) * sizeof(char*) );
    return pret;

NotMatch:
    *argc = 0;
    {
	char **s;
	for( s = pret; NOT_NULL(*s); s++) rxvt_free(*s);
    }

    rxvt_free (pret);
    return NULL;
}


/*
 * Sets up the fg/bg correctly in pixColorsFocus/unfocus, the fg/bg of
 * TermWin.gc, and the fg/bg of the window (if active).
 */
/* EXTPROTO */
int
rxvt_set_vt_colors( rxvt_t *r, int page )
{
    int		    changed = 0;
    int		    useFocusColors;
    unsigned long   *pix_colors;

    rxvt_dbgmsg(( DBG_DEBUG, DBG_INIT, "%s(r, page=%d). ", __func__, page ));

    useFocusColors = ( r->TermWin.focus || !r->TermWin.fade );
    pix_colors = (useFocusColors ? r->pixColorsFocus : r->pixColorsUnfocus);
    if( r->pixColors != pix_colors )
    {
	changed		= 1;
	r->pixColors	= pix_colors;
    }

#ifdef XFT_SUPPORT
    if( ISSET_OPTION( r, Opt_xft ) )
	r->xftColors = useFocusColors ? r->xftColorsFocus : r->xftColorsUnfocus;
#endif

    if( rxvt_set_fgbg_colors( r, page ) )
	changed = 1;

    if( changed )
    {
	/*
	 * Set foreground/background color for GC. This is necessary. Since all
	 * VTs share the same GC, if we do not set the color here, color from
	 * other VTs will be used to draw the following text till there is a
	 * color change.
	 */
	XSetForeground( r->Xdisplay, r->TermWin.gc, r->pixColors[Color_fg] );
	XSetBackground( r->Xdisplay, r->TermWin.gc, r->pixColors[Color_bg] );

	if( IS_WIN( PVTS(r, page)->vt ) )
# ifdef TRANSPARENT
	    if (NOTSET_OPTION(r, Opt_transparent))
# endif	/* TRANSPARENT */
#ifdef BACKGROUND_IMAGE
		if (NOT_PIXMAP(PVTS(r, page)->pixmap))
#endif	/* BACKGROUND_IMAGE */
		    XSetWindowBackground(r->Xdisplay, PVTS(r, page)->vt,
			r->pixColors[Color_bg]);
    }

    rxvt_dbgmsg(( DBG_DEBUG, DBG_INIT, "Returning %d\n", changed ));
    return changed;
}


/* INTPROTO */
termenv_t
rxvt_get_termenv( const char *env )
{
    if (IS_NULL(env))
	return (TERMENV_XTERM);
    else if (0 == STRCASECMP (env, "xterm"))
	return (TERMENV_XTERM);
    else if (0 == STRCASECMP (env, "rxvt"))
	return (TERMENV_RXVT);
    else if (0 == STRCASECMP (env, "vt102"))
	return (TERMENV_VT102);
    else if (0 == STRCASECMP (env, "vt100"))
	return (TERMENV_VT100);
    else if (0 == STRCASECMP (env, "ansi"))
	return (TERMENV_ANSI);
    else if (0 == STRCASECMP (env, "dumb"))
	return (TERMENV_DUMB);
    else
	return (TERMENV_XTERM);
}


/* INTPROTO */
/*
 * rxvt_init_vts (r, page, profile) will initialize a new page (term_t),
 * accessible at r->vts[page] and set with 'profile'.
 * It returns 1 on success, 0 on failure.
 * Cases of failure can be:
 * - The (vts) array's size could not be increased (allocation problem);
 * - The term_t vts[page] could not be allocated;
 * - 'page' is not valid: it must be between 0 and the last page + 1.
 *   If it is less than the last page + 1, then all pages >= page are moved up.
 */
int
rxvt_init_vts( rxvt_t *r, int page, int profile )
{
#ifdef TTY_GID_SUPPORT
    struct group*   gr = getgrnam( "tty" );
#endif
    if (page < 0 || page > LTAB(r) + 1)
	return 0;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "rxvt_init_vts (r, %d)\n", page));
    LTAB(r)++;
    {
	/*
	 * I increase vts's size and place the new page at the right place in it.
	 */
	term_t** temp_vts;
	term_t* temp_vts_page;

	if ((temp_vts_page = rxvt_malloc (sizeof (term_t))) == NULL)
	{
	    LTAB(r)--;
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "\tThe terminal allocation failed. Returning. Last tab is %d.\n", LTAB(r)));
	    return 0;
	}
	rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "\tA new terminal has been allocated.\n"));

	if ((temp_vts = rxvt_realloc (r->vts, (LTAB (r) + 1) * sizeof (term_t*))) == NULL)
	{
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "\tThe terminal array's reallocation to (%d * sizeof (term_t*)) failed.\n", LTAB(r) + 1));
	    LTAB(r)--;
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "\tLast tab is now %d.\n", LTAB(r)));
	    rxvt_free (temp_vts_page);
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "\tPreviously allocated terminal freed. Returning.\n"));
	    return 0;
	}

	r->vts = temp_vts;
	rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "\tThe terminal array has been reallocated to (%d * sizeof (term_t*)). The last tab is now %d.\n", LTAB(r) + 1, LTAB(r)));

	if (page != LTAB (r))
	    MEMMOVE (r->vts[page + 1], r->vts[page], (LTAB(r) - page) * sizeof (term_t*));

	r->vts[page] = temp_vts_page;
    }

    PVTS(r, page)->vts_idx = page;
    MEMSET( r->vts[page], 0, sizeof(term_t)); //r->vterm[0] ) );

    /* Set the profile number */
    PVTS(r, page)->profileNum	= profile;

    /* Save the "static" number of this tab */
    PVTS(r, page)->globalTabNum	= r->ntabs++;

#ifdef TTY_GID_SUPPORT
    /* change group ownership of tty to "tty" */
    if (gr)
    {
	PVTS(r, page)->ttymode = S_IRUSR | S_IWUSR | S_IWGRP;
    }
    else
#endif	    /* TTY_GID_SUPPORT */
    {
	PVTS(r, page)->ttymode = S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH;
    }

    /* Initialize term_t (vts) structure */
    PVTS( r, page )->saveLines = r->profile[profile].saveLines;

    /* will be set in rxvt_create_termwin */
    UNSET_WIN(PVTS(r, page)->vt);

#ifdef XFT_SUPPORT
    SET_NULL(PVTS(r, page)->xftvt);
#endif
    SET_NULL(PVTS(r, page)->tab_title);

    /*
     * Set the tab title format, and window title format. getProfileOption
     * returns a static string, so duplicate it here
     */
    {
	const char *stf = getProfileOption( r, profile, Rs_titleFormat );
	PVTS(r, page)->title_format = NOT_NULL(stf) ? STRDUP(stf) : NULL;
    }

    {
	const char *wtf = getProfileOption( r, profile, Rs_winTitleFormat );
	PVTS(r, page)->winTitleFormat = NOT_NULL(wtf) ? STRDUP(wtf) : NULL;
    }

#ifdef BACKGROUND_IMAGE
    UNSET_PIXMAP(PVTS(r, page)->pixmap);
    UNSET_PIXMAP(PVTS(r, page)->bg.pixmap);
    PVTS(r, page)->bg.x = PVTS(r, page)->bg.y = 50;
#endif
    PVTS(r, page)->cmd_pid = -1;
    PVTS(r, page)->cmd_fd = PVTS(r, page)->tty_fd = -1;
#ifdef UTMP_SUPPORT
    PVTS(r, page)->next_utmp_action = SAVE;
#endif
#ifndef NO_SETOWNER_TTYDEV
    PVTS(r, page)->next_tty_action = SAVE;
#endif

    PVTS(r, page)->holdOption = r->profile[profile].holdOption;

    PVTS(r, page)->status = 0;
    PVTS(r, page)->hold = 0;	    /* clear hold flag */
    PVTS(r, page)->dead = 0;	    /* clear dead flag */
    PVTS(r, page)->highlight = 0;   /* clear highlight flag */

    /* Get term_env type */
    PVTS(r, page)->termenv = rxvt_get_termenv (
	    r->h->rs[Rs_term_name] ? r->h->rs[Rs_term_name] : TERMENV);

    /* Initialize PrivateModes and SavedModes */
    PVTS(r, page)->PrivateModes = PVTS(r, page)->SavedModes =
	PrivMode_Default;
    if (ISSET_OPTION(r, Opt_scrollTtyOutputInhibit))
	SET_PMODE(r, page, PrivMode_TtyOutputInh);
    if (ISSET_OPTION(r, Opt_scrollTtyKeypress))
	SET_PMODE(r, page, PrivMode_Keypress);
    if( r->h->skip_pages > 1 /* jump scroll is unset */ )
	SET_PMODE(r, page, PrivMode_smoothScroll);
#ifndef NO_BACKSPACE_KEY
    if (STRCMP(r->h->key_backspace, "DEC") == 0)
	SET_PMODE(r, page, PrivMode_HaveBackSpace);
#endif
#ifdef HAVE_SCROLLBARS
    if (rxvt_scrollbar_visible(r))
    {
	SET_PMODE(r, page, PrivMode_scrollBar);
	SET_SMODE(r, page, PrivMode_scrollBar);
    }
#endif
#ifdef HAVE_MENUBAR
    if (rxvt_menubar_visible(r))
    {
	SET_PMODE(r, page, PrivMode_menuBar);
	SET_SMODE(r, page, PrivMode_menuBar);
    }
#endif

    /* Now set VT fg/bg color */
    PVTS(r, page)->p_fg = VTFG(r, profile);
    PVTS(r, page)->p_bg = VTBG(r, profile);

    if( r->TermWin.fade )
    {
	PVTS(r, page)->p_fgfade = VTFG_FADE(r, profile);
	PVTS(r, page)->p_bgfade = VTBG_FADE(r, profile);
    }

#ifdef XFT_SUPPORT
    if( ISSET_OPTION( r, Opt_xft ) )
    {
	PVTS(r, page)->p_xftfg = VTXFTFG(r, profile);
	PVTS(r, page)->p_xftbg = VTXFTBG(r, profile);

	if( r->TermWin.fade )
	{
	    PVTS(r, page)->p_xftfgfade = VTXFTFG_FADE(r, profile);
	    PVTS(r, page)->p_xftbgfade = VTXFTBG_FADE(r, profile);
	}
    }
#endif

    /* Initialize input buffer */
    PVTS(r, page)->outbuf_start	= PVTS(r, page)->outbuf_end
	= PVTS(r, page)->outbuf_base;

    /* Initialize write out buffer */
    SET_NULL(PVTS(r, page)->inbuf_base);
    SET_NULL(PVTS(r, page)->inbuf_start);
    SET_NULL(PVTS(r, page)->inbuf_end);
    PVTS(r, page)->inbuf_room = 0;

    /* Set screen structure initialization flag */
    PVTS(r, page)->init_screen = 0;

    /* Request a refresh */
    PVTS(r, page)->want_refresh = 1;
    return 1;
}


/*----------------------------------------------------------------------*/
/* rxvt_destroy_termwin() - destroy a terminal window */
/* EXTPROTO */
void
rxvt_destroy_termwin( rxvt_t *r, int page )
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "rxvt_destroy_termwin (r, %d)\n", page));
    assert (page <= LTAB(r));
    assert (PVTS(r, page)->tab_title);

    rxvt_free (PVTS(r, page)->tab_title);
    SET_NULL(PVTS(r, page)->tab_title);

    rxvt_free( PVTS(r, page)->title_format );
    SET_NULL( PVTS(r, page)->title_format );

#ifdef XFT_SUPPORT
    if (ISSET_OPTION(r, Opt_xft))
    {
	if (PVTS(r, page)->xftvt)
	    XftDrawDestroy (PVTS(r, page)->xftvt);
	SET_NULL(PVTS(r, page)->xftvt);
    }
#endif
    assert (IS_WIN(PVTS(r, page)->vt));
    XDestroyWindow (r->Xdisplay, PVTS(r, page)->vt);
    UNSET_WIN(PVTS(r, page)->vt);

#ifdef BACKGROUND_IMAGE
    if (IS_PIXMAP(PVTS(r, page)->pixmap))
    {
	XFreePixmap (r->Xdisplay, PVTS(r, page)->pixmap);
	UNSET_PIXMAP(PVTS(r, page)->pixmap);
    }
    if (IS_PIXMAP(PVTS(r, page)->bg.pixmap))
    {
	XFreePixmap (r->Xdisplay, PVTS(r, page)->bg.pixmap);
	UNSET_PIXMAP(PVTS(r, page)->bg.pixmap);
    }
#endif

    rxvt_free (PVTS(r, page));
    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "\tThe terminal %d has been successfully freed.\n", page));
}



/* rxvt_create_termwin() - create a terminal window */
/* EXTPROTO */
int
rxvt_create_termwin( rxvt_t *r, int page, int profile,
	const char TAINTED *title )
{
    long	    vt_emask;

    if (!rxvt_init_vts (r, page, profile))
	return 0;

    /*
     * Set the tab title
     */
    if (IS_NULL(title))
	title = DEFAULT_TAB_TITLE;
    PVTS(r, page)->tab_title = (char UNTAINTED *) STRNDUP( title, MAX_TAB_TXT );

#ifdef HAVE_PUTENV
    /* Set environment variable of tab title */
    sprintf (r->h->env_tabtitle, TABTITLEENV "%s", PVTS(r, page)->tab_title);
    putenv (r->h->env_tabtitle);
#endif

    PVTS(r, page)->tab_width = rxvt_tab_width (r, PVTS(r, page)->tab_title);

    /*
     * Now switch fg/bg colors before creating VT because this will use the
     * fg/bg colors
     */
    rxvt_set_vt_colors( r, page );

    /* create the terminal window */
    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "Create VT %d (%dx%d+%dx%d) fg=%06lx, bg=%06lx\n", page, r->h->window_vt_x, r->h->window_vt_y, VT_WIDTH(r), VT_HEIGHT(r), r->pixColors[Color_fg], r->pixColors[Color_bg]));

    PVTS(r, page)->vt = XCreateSimpleWindow (r->Xdisplay, r->TermWin.parent,
	    r->h->window_vt_x, r->h->window_vt_y,
	    VT_WIDTH(r), VT_HEIGHT(r),
	    0,
	    r->pixColors[Color_fg],
	    r->pixColors[Color_bg]);
    assert (IS_WIN(PVTS(r, page)->vt));
#ifdef XFT_SUPPORT
    if (ISSET_OPTION(r, Opt_xft))
    {
	PVTS(r, page)->xftvt = XftDrawCreate (r->Xdisplay,
		PVTS(r, page)->vt, XVISUAL, XCMAP);
	assert (NOT_NULL(PVTS(r, page)->xftvt));
    }
#endif


    /* define cursor for the terminal window */
    rxvt_pointer_unblank(r, page);

    /* define event mask fo the terminal window */
    vt_emask = (ExposureMask | ButtonPressMask | ButtonReleaseMask
	    | PropertyChangeMask);
#ifdef POINTER_BLANK
    if (ISSET_OPTION(r, Opt_pointerBlank))
	vt_emask |= PointerMotionMask;
    else
#endif
	vt_emask |= (Button1MotionMask | Button3MotionMask);
    XSelectInput(r->Xdisplay, PVTS(r, page)->vt, vt_emask);

#ifdef TRANSPARENT
    /* Set transparent background */
    if (ISSET_OPTION(r, Opt_transparent))
    {
	XSetWindowBackgroundPixmap (r->Xdisplay, PVTS(r, page)->vt,
		ParentRelative);
    }
#endif

    /*
     * Load the background image for terminal window when not transparent
     */
#ifdef BACKGROUND_IMAGE
# ifdef TRANSPARENT
    if( NOTSET_OPTION(r,  Opt_transparent) )
# endif
    {
	const char *pf = getProfileOption( r, profile, Rs_backgroundPixmap );
	if (NOT_NULL(pf))
	{
	    /* Load pixmap for each individual tab */
	    const char *p = pf;

	    if (NOT_NULL(p = STRCHR(p, ';')))
	    {
		p++;
		rxvt_scale_pixmap(r, page, p);
	    }
	    rxvt_load_bg_pixmap(r, page, pf);
	    /* rxvt_scr_touch(r, page, True); */
	}
    } /* if( NOTSET_OPTION(r,  Opt_transparent) ) */
#endif

    XMapWindow (r->Xdisplay, PVTS(r, page)->vt);
    return 1;
}


/*
 * Return the value of an option with profile number "profile". This function
 * should only be called for profile options.
 *
 * The string returned is one of r->h->rs[], so should not be freed.
 */
/* EXTPROTO */
const char *
getProfileOption( rxvt_t *r, int profile, int resource )
{
    assert( profile >= 0 || profile < MAX_PROFILES );

    /*
     * Profile 0 is default, so if the profile option is unset, fall back to
     * profile 0.
     */
    return NOT_NULL(r->h->rs[resource + profile]) ?
	r->h->rs[resource + profile] : r->h->rs[resource];
}

/* INTPROTO */
void
rxvt_set_borderless( rxvt_t *r )
{
    Atom	prop;
    CARD32	hints;	    /* KDE/GNOME hints */
    MWMHints	mwmhints;   /* Motif hints */

    hints = (CARD32) 0;
    mwmhints.flags = MWM_HINTS_DECORATIONS;
    mwmhints.decorations = 0;

    /* Motif compatible WM */
    prop = XInternAtom (r->Xdisplay, "_MOTIF_WM_HINTS", True);
    if (IS_ATOM(prop))
	XChangeProperty (r->Xdisplay, r->TermWin.parent, prop, prop,
	    32, PropModeReplace, (unsigned char*) &mwmhints,
	    PROP_MWM_HINTS_ELEMENTS);

    /* GNOME compatible WM */
    prop = XInternAtom (r->Xdisplay, "_WIN_HINTS", True);
    if (IS_ATOM(prop))
	XChangeProperty (r->Xdisplay, r->TermWin.parent, prop, prop,
	    32, PropModeReplace, (unsigned char*) &hints, 1);

    /* KDE compatible WM */
    prop = XInternAtom (r->Xdisplay, "KWM_WIN_DECORATION", True);
    if (IS_ATOM(prop))
	XChangeProperty (r->Xdisplay, r->TermWin.parent, prop, prop,
	    32, PropModeReplace, (unsigned char*) &hints, 1);
}

/*
 * Send a message to an EWMH compatible window manager.
 */
/* EXTPROTO */
Status
ewmh_message( Display *dpy, Window root_win, Window client_win,
	Atom msgAtom, long d0, long d1, long d2, long d3, long d4)
{

    XEvent event;

    if (NOT_ATOM(msgAtom))
	return 1;

    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.message_type = msgAtom;
    event.xclient.window = client_win;
    event.xclient.format = 32;

    event.xclient.data.l[0] = d0;
    event.xclient.data.l[1] = d1;
    event.xclient.data.l[2] = d2;
    event.xclient.data.l[3] = d3;
    event.xclient.data.l[4] = d4;

    return XSendEvent( dpy, root_win, False,
	    SubstructureRedirectMask | SubstructureNotifyMask,
	    &event);
}


/* INTPROTO */
void
rxvt_set_desktop( rxvt_t* r, CARD32 desktop )
{
    /* GNOME compatible WM */
    if (desktop >= 0 && desktop <= 64 &&
	IS_ATOM(r->h->xa[XA_WIN_WORKSPACE]))
	XChangeProperty(r->Xdisplay, r->TermWin.parent,
	    r->h->xa[XA_WIN_WORKSPACE], XA_CARDINAL, 32,
	    PropModeReplace, (unsigned char*) &desktop, 1L);

    /* WindowMaker/FreeDesktop.org compatible WM */
    if (desktop >= 0 && desktop <= 64 &&
	IS_ATOM(r->h->xa[XA_NET_WM_DESKTOP]))
	XChangeProperty(r->Xdisplay, r->TermWin.parent, 
	    r->h->xa[XA_NET_WM_DESKTOP], XA_CARDINAL, 32,
	    PropModeReplace, (unsigned char*) &desktop, 1L);
}


/* EXTPROTO */
CARD32
rxvt_get_desktop( rxvt_t* r )
{
    Atom    ret_type;
    int	    format;
    unsigned long   nitems;
    unsigned long   bytes_after;
    unsigned char   *prop;
    CARD32  desktop;

    if (NOT_ATOM(r->h->xa[XA_NET_WM_DESKTOP]))
	return 0;

    if(
	  XGetWindowProperty( r->Xdisplay, r->TermWin.parent,
	      r->h->xa[XA_NET_WM_DESKTOP], 0L, LONG_MAX, False, XA_CARDINAL,
	      &ret_type, &format, &nitems, &bytes_after, &prop)
	    != Success
      )
	return 0;

    if( ret_type == XA_CARDINAL && format == 32 )
    {
	desktop = ((CARD32*) prop)[0];
	if (desktop < 0 || desktop > 64)
	    desktop = 0;
    }
    else
	desktop = 0;

    XFree( prop);

    rxvt_dbgmsg(( DBG_DEBUG, DBG_INIT, "Desktop: %lu\n", desktop ));
    return desktop;
}


/*----------------------------------------------------------------------*/
/* rxvt_create_show_windows() - Open and map the window */
/* EXTPROTO */
void
rxvt_create_show_windows( rxvt_t *r, int argc, const char *const *argv )
{
    XClassHint		    class_hint;
    XWMHints		    wm_hint;
    XTextProperty	    win_prop;
    XTextProperty	    icon_prop;
    XGCValues		    gcvalue;
    unsigned long	    gcmask;
#ifndef NO_FRILLS
    CARD32		    pid = (CARD32) getpid ();
#endif
#ifdef TRANSPARENT
    register int	    i;
#endif
#ifdef POINTER_BLANK
    static const XColor	    blackcolour = { 0, 0, 0, 0, 0, 0 };
#endif

    Window		    parent;	/* WinID to use for parent window */

#ifdef PREFER_24BIT
    XSetWindowAttributes    attributes;
    XWindowAttributes	    gattr;


    XCMAP = DefaultColormap(r->Xdisplay, XSCREEN);
    XVISUAL = DefaultVisual(r->Xdisplay, XSCREEN);

    if (ISSET_OPTION(r, Opt_transparent))
    {
	XGetWindowAttributes(r->Xdisplay,
	    RootWindow(r->Xdisplay, XSCREEN), &gattr);
	XDEPTH = gattr.depth;
    }
    else
    {
	XDEPTH = DefaultDepth(r->Xdisplay, XSCREEN);
	/*
	 * If depth is not 24, look for a 24bit visual.
	 */
	if (XDEPTH != 24)
	{
	    XVisualInfo	 vinfo;

	    if (XMatchVisualInfo(r->Xdisplay, XSCREEN, 24, TrueColor, &vinfo))
	    {
		XDEPTH = 24;
		XVISUAL = vinfo.visual;
		XCMAP = XCreateColormap(r->Xdisplay,
			    RootWindow(r->Xdisplay, XSCREEN),
			    XVISUAL, AllocNone);
	    }
	}
    }
#endif


    /* grab colors before netscape does */
    rxvt_init_colors (r);

    /*
     * Initialize fonts.
     * . Always load X11 fonts since pointer_blank uses it
     * . Load XFT font after X11 fonts. If succeeds, XFT font will
     * update font width/height and be used by default
     *
     * 03/09/2006 gi1242: TODO Don't load the X11 font unless absolutely
     * necessary. It will speed up startup (minimaly), and reduce resource
     * usage (minimaly).
     */
#ifdef XFT_SUPPORT
    if (ISSET_OPTION(r, Opt_xft))
    {
	if (!rxvt_init_font_xft (r))
	{
	    rxvt_msg (DBG_INFO, DBG_INIT,
		    "Failed to load FreeType font, fallback to X11 font\n");
	    /* disable xft */
	    UNSET_OPTION(r, Opt_xft);
	}
	else
	    xftInitACS (r->Xdisplay, XROOT, XDEPTH);
    }
#endif
    /* init fallback X11 font */
    rxvt_init_font_x11( r );


    /*
     * must initialize scrollbar before initialize window size and
     * create windows.
     */
#ifdef HAVE_SCROLLBARS
    rxvt_scrollbar_init (r);
#endif
    rxvt_init_win_size (r);

    /*
     * Use window specified by -into option as the parent window.
     */
    if( r->h->rs[Rs_container_window] )
    {
	XWindowAttributes   attrs;

	r->h->allowedxerror = 1;    /* Enable Xerror reporting */
	r->h->xerror_return = Success;

        parent = strtoul( r->h->rs[Rs_container_window], NULL, 0 );

	XGetWindowAttributes( r->Xdisplay, parent, &attrs );

	/* Check if we have valid attributes */
	if( r->h->xerror_return != Success || attrs.class == InputOnly )
	{
	    rxvt_msg (DBG_ERROR, DBG_INIT,  "Unable to embed into Win 0x%lx", parent );
	    parent = XROOT;
	}

	r->h->allowedxerror = 0;    /* Disable Xerror reporting */
    }
    else
        parent = XROOT;
    
    /*
     * parent window - reverse video so we can see placement errors sub-window
     * placement & size in rxvt_resize_subwindows()
     */

#ifdef PREFER_24BIT
    attributes.background_pixel = r->pixColorsFocus[Color_bg];
    attributes.border_pixel = r->pixColorsFocus[Color_border];
    attributes.colormap = XCMAP;
    r->TermWin.parent = XCreateWindow(r->Xdisplay, parent,
		    r->szHint.x, r->szHint.y,
		    r->szHint.width, r->szHint.height,
		    r->TermWin.ext_bwidth,
		    XDEPTH, InputOutput,
		    XVISUAL,
		    CWBackPixel | CWBorderPixel
		    | CWColormap, &attributes);
#else
    r->TermWin.parent = XCreateSimpleWindow(r->Xdisplay, parent,
			r->szHint.x, r->szHint.y,
			r->szHint.width,
			r->szHint.height,
			r->TermWin.ext_bwidth,
			r->pixColorsFocus[Color_border],
			r->pixColorsFocus[Color_bg]);
#endif

#ifdef XFT_SUPPORT
    if (ISSET_OPTION(r, Opt_xft))
    {
	/* create XFT draw, test only */
	XftDraw*	xftdraw = XftDrawCreate( r->Xdisplay,
						 r->TermWin.parent, XVISUAL,
						 XCMAP);
	if (xftdraw)
	{
	    XftDrawDestroy (xftdraw);
	    /* some cleanup work if successfully create xft window */
# ifdef POINTER_BLANK
	    /*
	     * 2006-01-21 gi1242: I'm not sure why pointer blank is disabled
	     * with xft. It works fine for me, so I re-enabled it.
	     */
#  if 0
	    /* disable pointer blank */
	    UNSET_OPTION(r, Opt_pointerBlank);
#  endif
# endif
	}
    }
#endif


# ifdef HAVE_X11_SM_SMLIB_H
    if (ISSET_OPTION(r, Opt2_enableSessionMgt))
	rxvt_session_init (r);
# endif


    /*
     * Now set window properties, like title, icon name and hints
     */
    /* window title name */
    win_prop.value = (unsigned char*) r->h->rs[Rs_title];
    win_prop.nitems = STRLEN (win_prop.value);
    win_prop.encoding = XA_STRING;
    win_prop.format = 8; 
    /* icon name */
    icon_prop.value = (unsigned char*) r->h->rs[Rs_iconName];
    icon_prop.nitems = STRLEN (icon_prop.value);
    icon_prop.encoding = XA_STRING;
    icon_prop.format = 8; 
    /* window manager hints */
    wm_hint.flags = (InputHint | StateHint | WindowGroupHint);
    wm_hint.input = True;
    wm_hint.initial_state = ISSET_OPTION(r, Opt_iconic) ? IconicState
			    : NormalState;
    wm_hint.window_group = r->TermWin.parent;
    /* window icon hint */
#ifdef HAVE_LIBXPM
    if( r->h->rs[Rs_appIcon] )
    {
	Pixmap appIcon, appIconMask;

        XpmReadFileToPixmap( r->Xdisplay, r->TermWin.parent,
		(char*) r->h->rs[Rs_appIcon], &appIcon, &appIconMask, 0);

	if( appIcon != None &&  appIconMask != None ) {
	    wm_hint.icon_pixmap = appIcon;
	    wm_hint.icon_mask = appIconMask;
	    wm_hint.flags |= IconPixmapHint | IconMaskHint;
	}
    }
#endif /* HAVE_LIBXPM */
    /* class hints */
    class_hint.res_name = (char*) r->h->rs[Rs_name];
    class_hint.res_class = (char*) APL_CLASS;
    XSetWMProperties (r->Xdisplay, r->TermWin.parent,
	&win_prop, &icon_prop, (char**)argv, argc,
	&r->szHint, &wm_hint, &class_hint);

#if 0 /* If the pixmap's are free'ed, then the WM will not display them. */
    if( wm_hint.flags & IconPixmapHint )
    {
	XFreePixmap( r->Xdisplay, wm_hint.icon_pixmap );
	XFreePixmap( r->Xdisplay, wm_hint.icon_mask );
    }
#endif

    /* set terminal title */
    rxvt_set_term_title (r, win_prop.value);
    /* set icon title */
    rxvt_set_icon_name (r, icon_prop.value);
    /* command line */
    XSetCommand (r->Xdisplay, r->TermWin.parent, (char**) argv, argc);

    /* override redirect */
    if (ISSET_OPTION(r, Opt2_overrideRedirect))
    {
	XSetWindowAttributes	attrib;
	attrib.override_redirect = True;
	XChangeWindowAttributes(r->Xdisplay, r->TermWin.parent,
	    CWOverrideRedirect, &attrib);
    }

#ifndef NO_FRILLS
    XChangeProperty (r->Xdisplay, r->TermWin.parent,
	r->h->xa[XA_NET_WM_PID], XA_CARDINAL, 32,
	PropModeReplace, (unsigned char*) &pid, 1);
#endif

    if (ISSET_OPTION(r, Opt2_borderLess))
    {
	rxvt_set_borderless (r);
    }
    if (r->h->rs[Rs_desktop])
    {
	CARD32	desktop = (CARD32) atoi (r->h->rs[Rs_desktop]);
	rxvt_set_desktop (r, desktop);
    }

    /*
     * set WM_CLIENT_LEADER property so that session management proxy can handle
     * us even session management is not enabled.
     */
    if (IS_ATOM(r->h->xa[XA_WM_CLIENT_LEADER]))
	XChangeProperty( r->Xdisplay, r->TermWin.parent,
	    r->h->xa[XA_WM_CLIENT_LEADER], XA_WINDOW, 32,
	    PropModeReplace, (unsigned char*) &(r->TermWin.parent), 1L );

# ifdef HAVE_X11_SM_SMLIB_H
    if (NOT_NULL(r->TermWin.sm_conn) &&
	NOT_NULL(r->TermWin.sm_client_id) &&
	STRCMP (r->TermWin.sm_client_id, "")
       )
    {
	if (IS_ATOM(r->h->xa[XA_SM_CLIENT_ID]))
	    XChangeProperty(r->Xdisplay, r->TermWin.parent,
		r->h->xa[XA_SM_CLIENT_ID], XA_STRING, 8,
		PropModeReplace,
		(unsigned char*) r->TermWin.sm_client_id, 
		STRLEN(r->TermWin.sm_client_id));
    }
# endif	/* HAVE_X11_SM_SMLIB_H */


#ifdef TRANSPARENT
    r->TermWin.parenttree[0] = r->TermWin.parent;
    for (i = 1; i < PARENT_NUMBER; i ++)
	UNSET_WIN(r->TermWin.parenttree[i]);

    /*
     * XXX 2006-01-02 gi1242: This is inefficient. If window is pseudo
     * transparent, then the background pixmap will be reset later to something
     * else.
     */
#if 0
    if (ISSET_OPTION(r, Opt_transparent))
    {
	XSetWindowBackgroundPixmap (r->Xdisplay, r->TermWin.parent,
	    ParentRelative);
    }
#endif
#endif	/* TRANSPARENT */


    XSelectInput(r->Xdisplay, r->TermWin.parent,
	    (KeyPressMask
#if defined(MOUSE_WHEEL) && defined(MOUSE_SLIP_WHEELING)
	    | KeyReleaseMask
#endif
	    | FocusChangeMask
#ifdef MONITOR_ENTER_LEAVE
	    | EnterWindowMask | LeaveWindowMask
#endif
	    | VisibilityChangeMask
	    | StructureNotifyMask));


    /*
    ** vt cursor: Black-on-White is standard, but this is more
    ** popular
    */
    r->term_pointer = XCreateFontCursor(r->Xdisplay, XC_xterm);
    /* scrollbar/menubar/tabbar window pointer */
    r->h->bar_pointer = XCreateFontCursor(r->Xdisplay, XC_left_ptr);

#ifdef POINTER_BLANK
    if (NOTSET_OPTION(r, Opt_pointerBlank))
	UNSET_CURSOR(r->h->blank_pointer);
    else
	r->h->blank_pointer = XCreateGlyphCursor(r->Xdisplay,
	    r->TermWin.font->fid, r->TermWin.font->fid, ' ', ' ',
	    (XColor*) &blackcolour, (XColor*) &blackcolour);
#endif


    /* graphics context for the vt window */
#ifdef XFT_SUPPORT
    if (NOTSET_OPTION(r, Opt_xft))
#endif
    gcvalue.font = r->TermWin.font->fid;
    gcvalue.foreground = r->pixColorsFocus[Color_fg];
    gcvalue.background = r->pixColorsFocus[Color_bg];
    gcvalue.graphics_exposures = 1;
    gcmask = GCForeground | GCBackground | GCGraphicsExposures;
#ifdef XFT_SUPPORT
    if (NOTSET_OPTION(r, Opt_xft))
#endif
    gcmask |= GCFont;
    r->TermWin.gc = XCreateGC(r->Xdisplay, r->TermWin.parent,
		    gcmask, &gcvalue);

#ifdef HAVE_SCROLLBARS
    rxvt_scrollbar_create (r);
    if (ISSET_OPTION(r, Opt_scrollBar))
    {
	rxvt_scrollbar_show (r);
    }
#endif
#ifdef HAVE_MENUBAR
    if (r->h->rs[Rs_menu] && STRCASECMP( r->h->rs[Rs_menu], "none"))
    {
	/*
	 * Only load menubar if arg of -menu option is not none
	 */
	rxvt_menubar_load_file (r, (unsigned char*) r->h->rs[Rs_menu]);
    }
    else rxvt_menubar_load_file( r, (unsigned char*) "default.menu");

    rxvt_menubar_create (r);
    if (ISSET_OPTION(r, Opt_showMenu))
	rxvt_menubar_show (r);

    /*
     * 2006-05-28 gi1242: If popup menu 1 is not defined, set it to an empty
     * menu (so that the tab list will be popped up on control clicks and right
     * clicks on the tabbar).
     */
    if (IS_NULL(r->h->popupMenu[0]))
    {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "Setting popup menu 1 to a tab list\n"));
	r->h->popupMenu[0] = (menu_t *) rxvt_calloc( 1, sizeof(menu_t) );

	r->h->popupMenu[0]->len	    = sizeof( "Switch to tab" );
	r->h->popupMenu[0]->name    = (unsigned char*) STRDUP ("Switch to tab");
    }
# endif

    rxvt_tabbar_create (r);
    if (NOTSET_OPTION(r, Opt2_hideTabbar))
	rxvt_tabbar_show (r);

    XMapWindow (r->Xdisplay, r->TermWin.parent);

    /*
     * We have to wait till our window is mapped before we can set the maximized
     * or fullscreen options.
     */
    if( ISSET_OPTION(r, Opt2_maximized))
	ewmh_message( r->Xdisplay, XROOT, r->TermWin.parent,
	    XInternAtom( r->Xdisplay, "_NET_WM_STATE", True),
	    _NET_WM_STATE_ADD,
	    XInternAtom( r->Xdisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", True),
	    XInternAtom( r->Xdisplay, "_NET_WM_STATE_MAXIMIZED_VERT", True),
	    0, 0);
    else if (ISSET_OPTION (r, Opt2_fullscreen))
	ewmh_message( r->Xdisplay, XROOT, r->TermWin.parent,
	    XInternAtom( r->Xdisplay, "_NET_WM_STATE", True),
	    _NET_WM_STATE_ADD,
	    XInternAtom( r->Xdisplay, "_NET_WM_STATE_FULLSCREEN", True),
	    0, 0, 0);
}

/*----------------------------------------------------------------------*/
/*
 * Executes a command in the background, and returns immediately. Returns 1 on
 * success, 0 otherwise.
 */
int
rxvt_async_exec( rxvt_t *r, const char *cmd)
{
    int	    pid;
    int	    argc;
    char    **argv;

    if( r->nAsyncChilds >= MAX_CHILDS )
    {
	rxvt_msg (DBG_ERROR, DBG_INIT,  "Too many childs."
		" Increase MAX_CHILDS in src/feature.h" );
	return 0;
    }

    pid = fork();
    switch( pid )
    {
	case -1:
	    rxvt_msg (DBG_ERROR, DBG_INIT,  "Unable to fork" );
	    return 0;	/* Failure */
	    /* NOT REACHED */

	case 0:
	    /*
	     * Close all file descriptors, and reset signal masks to their
	     * default values before exec'ing the child process.
	     */
	    clean_sigmasks_and_fds( r, ATAB(r) );

	    argv = rxvt_string_to_argv( cmd, &argc );

	    execvp( argv[0], argv );

	    rxvt_msg (DBG_ERROR, DBG_INIT,  "Failed to exec %s", argv[0] );
	    exit(1);
	    /* NOT REACHED */

	default:
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_INIT, "Forked %s", cmd));
	    r->asyncChilds[ r->nAsyncChilds++ ] = pid;
	    return 1;
    }
}


/*
 * Run the command in a subprocess and return a file descriptor for the
 * master end of the pseudo-teletype pair with the command talking to
 * the slave.
 */
/* EXTPROTO */
int
rxvt_run_command(rxvt_t *r, int page, const char **argv)
{
    int		cfd, er;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "%s(%d, argv)", __func__, page));

    /* get master (pty) */
    if ((cfd = rxvt_get_pty(&(PVTS(r, page)->tty_fd),
	(char**) &(PVTS(r, page)->ttydev))) < 0)
    {
	rxvt_msg (DBG_ERROR, DBG_INIT, "can't open pseudo-tty");
	return -1;
    }
#ifdef FD_SETSIZE
    if (r->Xfd > FD_SETSIZE || cfd > FD_SETSIZE)
    {
	rxvt_msg (DBG_ERROR, DBG_INIT, "fd too high: %d max", FD_SETSIZE);
	return -1;
    }
#endif
    fcntl(cfd, F_SETFL, O_NDELAY);

    /* get slave (tty) */
    if (PVTS(r, page)->tty_fd < 0)
    {
#if !defined(NO_SETOWNER_TTYDEV) && !defined(OS_CYGWIN)
	rxvt_privileged_ttydev(r, page, SAVE);
#endif
	if ((PVTS(r, page)->tty_fd = rxvt_get_tty(PVTS(r, page)->ttydev)) < 0)
	{
	    close(cfd);
	    rxvt_msg (DBG_ERROR, DBG_INIT, "can't open slave tty %s", PVTS(r, page)->ttydev);
	    return -1;
	}
    }

    /* Get tty mode before fork */
#ifndef NO_BACKSPACE_KEY
    if (r->h->key_backspace[0] && !r->h->key_backspace[1])
	er = r->h->key_backspace[0];
    else if (STRCMP(r->h->key_backspace, "DEC") == 0)
	er = '\177';	/* the initial state anyway */
    else
#endif
	er = -1;
    rxvt_get_ttymode(&(PVTS(r, page)->tio), er);


    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "argv = 0x%x\n", (unsigned int) argv));
#ifndef __QNX__
    /*
     * Spin off the command interpreter
     */
    switch (PVTS(r, page)->cmd_pid = fork())
    {
	case -1:
	    rxvt_msg (DBG_ERROR, DBG_INIT, "can't fork");
	    close (cfd);
	    return -1;

	case 0:
	    /*
	     * To debug the child, follow these steps:
	     *
	     * 	    - enable sleep in the following
	     * 	    - launch gdb, set breakpoint before fork
	     * 	    - run the program, step over fork, then get child pid
	     * 	    - launch another gdb, attach to child process via pid
	     * 	    - in child's gdb, set breakpoint after sleep
	     * 	    - run 'continue' in child's gdb, debug child process
	     */
#if 0
	    sleep(10);
#endif


	    if(
		 rxvt_control_tty( PVTS(r, page)->tty_fd,
				    PVTS(r, page)->ttydev ) < 0
	      )
	    {
		rxvt_msg (DBG_ERROR, DBG_INIT, "Could not obtain control of tty");
	    }
	    else
	    {
		/*
		 * Reopen stdin, stdout and stderr over the tty file
		 * descriptor
		 */
		dup2( PVTS(r, page)->tty_fd, STDIN_FILENO  );
		dup2( PVTS(r, page)->tty_fd, STDOUT_FILENO );
		dup2( PVTS(r, page)->tty_fd, STDERR_FILENO );

		clean_sigmasks_and_fds( r, page );

		/*
		 * Spin off command interpreter.
		 */
		rxvt_run_child(r, page, argv);

		/*
		 * If we got here, then we failed to exec the child process.
		 */
		rxvt_msg (DBG_ERROR, DBG_INIT, "Could not execute %s.\n", (argv && argv[0]) ? argv[0] : "shell");
	    }

	    /* Something went wrong. Kill the child. */
	    if(
		 !(PVTS(r,page)->holdOption & HOLD_STATUSBIT) &&
		 !(PVTS(r,page)->holdOption & HOLD_ALWAYSBIT)
	      )
		/* If tab won't be held open, wait a little */
		sleep(5);
	    exit( EXIT_FAILURE );

	    /* NOT REACHED */

	default:
	    {
#if defined(HAVE_STRUCT_UTMP) && defined(HAVE_TTYSLOT)
		int	    fdstdin;

		fdstdin = dup(STDIN_FILENO);
		dup2(PVTS(r, page)->tty_fd, STDIN_FILENO);
#endif
#ifdef UTMP_SUPPORT
# ifdef UTEMPTER_SUPPORT
		/* utempter hack, it needs cmd_fd */
		PVTS(r, page)->cmd_fd = cfd;
# endif
		rxvt_privileged_utmp(r, page, SAVE);
# ifdef UTEMPTER_SUPPORT
		/* utempter hack, restore cmd_fd */
		PVTS(r, page)->cmd_fd = -1;
# endif
#endif
#if defined(HAVE_STRUCT_UTMP) && defined(HAVE_TTYSLOT)
		dup2(fdstdin, STDIN_FILENO);
		close(fdstdin);
#endif
	    }

	    /*
	     * keep STDERR_FILENO, PVTS(r, page)->cmd_fd, r->Xfd open
	     */
	    close(PVTS(r, page)->tty_fd);
	    PVTS(r, page)->tty_fd = -1;
	    break;
    }
#else		    /* __QNX__ uses qnxspawn() */
    fchmod(PVTS(r, page)->tty_fd, 0622);
    fcntl(PVTS(r, page)->tty_fd, F_SETFD, FD_CLOEXEC);
    fcntl(cfd, F_SETFD, FD_CLOEXEC);
    PVTS(r, page)->cmd_fd = cfd;

    if (rxvt_run_child(r, page, argv) == -1)
	/*exit(EXIT_FAILURE);*/
	return -1;
#endif

    return cfd;
}


/* ------------------------------------------------------------------------- *
 *			    CHILD PROCESS OPERATIONS			     *
 * ------------------------------------------------------------------------- */

/*
 * Reset signal masks to their default values, and close all open file
 * descriptors.
 */
/* EXTPROTO */
void
clean_sigmasks_and_fds( rxvt_t* r, int page )
{
#ifdef SIGTSTP
    struct sigaction	ignore;
#endif
    struct sigaction	deflt;
    register int	i;

    /* Close all file descriptors except STDXXX */
    for (i = STDERR_FILENO + 1; i < r->num_fds; i ++)
	close (i);
    if (PVTS(r, page)->tty_fd > 2)
    {
	close (PVTS(r, page)->tty_fd);
	PVTS(r, page)->tty_fd = -1;
    }

    /* reset signal handlers */
    deflt.sa_handler = SIG_DFL;
    deflt.sa_flags = 0;
    sigemptyset( &deflt.sa_mask );

    sigaction( SIGINT,	&deflt, NULL );
    sigaction( SIGQUIT, &deflt, NULL );
    sigaction( SIGTERM, &deflt, NULL );
    sigaction( SIGHUP,	&deflt, NULL );
    sigaction( SIGPIPE,	&deflt, NULL );
    sigaction( SIGCHLD, &deflt, NULL );

    /*
     * Mimick login's behavior by disabling the job control signals a shell that
     * wants them can turn them back on
     */
#ifdef SIGTSTP
    ignore.sa_handler	= SIG_IGN;
    ignore.sa_flags	= 0;
    sigemptyset( &ignore.sa_mask );

    sigaction( SIGTSTP, &ignore, NULL );
    sigaction( SIGTTIN, &ignore, NULL );
    sigaction( SIGTTOU, &ignore, NULL );
#endif	/* SIGTSTP */
}


/*
 * The only open file descriptor is the slave tty - so no error messages.
 * returns are fatal
 */
/* INTPROTO */
int
rxvt_run_child(rxvt_t* r, int page, const char **argv)
{
    char*		login;


    /* rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "argv = %x\n", argv)); */

    /* init terminal attributes */
    SET_TTYMODE( STDIN_FILENO, &(PVTS(r, page)->tio) );

    if (ISSET_OPTION(r, Opt_console))	/* be virtual console, fail
					 * silently */
    {
#ifdef TIOCCONS
	unsigned int	on = 1;
	ioctl(STDIN_FILENO, TIOCCONS, &on);
#elif defined (SRIOCSREDIR)
	int	    fd;
	fd = open( CONSOLE, O_WRONLY, 0 );
	if (fd >= 0)
	{
	    if( ioctl( fd, SRIOCSREDIR, NULL ) < 0 )
	    close( fd );
	}
#endif	/* SRIOCSREDIR */
    }

    /* set window size */
    rxvt_tt_winsize( STDIN_FILENO, r->TermWin.ncol, r->TermWin.nrow, 0 );

#ifndef __QNX__
    /* command interpreter path */
    if (NOT_NULL(argv))
    {
	int	    i;
	for (i = 0; argv[i]; i++)
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "argv [%d] = \"%s\"\n", i, argv[i]));
	execvp(argv[0], (char *const *)argv);
	/* no error message: STDERR is closed! */
    }
    else
    {
	const char   *argv0, *shell;

	if (IS_NULL(shell = getenv("SHELL")) || ((char) 0 == *shell) )
	{
# ifdef HAVE_GETPWUID
	    struct passwd* pwent = getpwuid( getuid () );

	    if (IS_NULL(pwent) ||
		IS_NULL(shell = pwent->pw_shell) ||
		(char) 0 == *shell
	      )
# endif	/* HAVE_GETPWUID */
		shell = "/bin/sh";
	}

	argv0 = (const char *) rxvt_r_basename( shell);
	if (ISSET_OPTION(r, Opt_loginShell))
	{
	    int	    l = STRLEN(argv0) + 2;
	    if (l <= 0 || l > 4096) /* possible integer overflow */
		l = 4096;
	    login = rxvt_malloc(l * sizeof(char));
    
	    login[0] = '-';
	    STRNCPY(&login[1], argv0, l-2);
	    login[l-1] = (char) 0;
	    argv0 = login;
	}
	execlp( shell, argv0, NULL );
	/* no error message: STDERR is closed! */
    }
#else		    /* __QNX__ uses qnxspawn() */
    {
	char	iov_a[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	char*	command = NULL, fullcommand[_MAX_PATH];
	char**	arg_v, *arg_a[2] = { NULL, NULL };

	if (argv != NULL)
	{
	    if (access(argv[0], X_OK) == -1)
	    {
		if (IS_NULL(STRCHR(argv[0], '/')))
		{
		    searchenv(argv[0], "PATH", fullcommand);
		    if (fullcommand[0] != '\0')
			command = fullcommand;
		}
		if (access(command, X_OK) == -1)
		    return -1;
	    }
	    else
		command = argv[0];
	    arg_v = argv;
	}
	else
	{
	    if (IS_NULL(command = getenv("SHELL")) || 
		(char) 0 == *command)
		command = "/bin/sh";

	    arg_a[0] = my_basename(command);
	    if (ISSET_OPTION(r, Opt_loginShell))
	    {
		int	l = STRLEN(arg_a[0]) + 2;
		if (l <= 0 || l > 4096)	/* possible integer overflow */
		    l = 4096;
		login = rxvt_malloc(l * sizeof(char));

		login[0] = '-';
		STRNCPY(&login[1], arg_a[0], l-2);
		login[l-1] = (char) 0;
		arg_a[0] = login;
	    }
	    arg_v = arg_a;
	}
	iov_a[0] = iov_a[1] = iov_a[2] = PVTS(r, page)->tty_fd;
	PVTS(r, page)->cmd_pid = qnx_spawn(0, 0, 0, -1, -1,
		    _SPAWN_SETSID | _SPAWN_TCSETPGRP,
		    command, arg_v, environ, iov_a, 0);
	if (login)
	    rxvt_free(login);
	close(PVTS(r, page)->tty_fd);
	PVTS(r, page)->tty_fd = -1;
	return PVTS(r, page)->cmd_fd;
    }
#endif
    return -1;
}

/* ------------------------------------------------------------------------- *
 *			    GET TTY CURRENT STATE			     *
 * ------------------------------------------------------------------------- */
/* rxvt_get_ttymode() */
/* INTPROTO */
void
rxvt_get_ttymode(ttymode_t *tio, int erase)
{
#ifdef HAVE_TERMIOS_H
    /*
     * standard System V termios interface
     */
    if (GET_TERMIOS(STDIN_FILENO, tio) < 0)
    {
	/* return error - use system defaults */
	tio->c_cc[VINTR] = CINTR;
	tio->c_cc[VQUIT] = CQUIT;
	tio->c_cc[VERASE] = CERASE;
	tio->c_cc[VKILL] = CKILL;
	tio->c_cc[VSTART] = CSTART;
	tio->c_cc[VSTOP] = CSTOP;
	tio->c_cc[VSUSP] = CSUSP;
# ifdef VDSUSP
	tio->c_cc[VDSUSP] = CDSUSP;
# endif
# ifdef VREPRINT
	tio->c_cc[VREPRINT] = CRPRNT;
# endif
# ifdef VDISCRD
	tio->c_cc[VDISCRD] = CFLUSH;
# endif
# ifdef VWERSE
	tio->c_cc[VWERSE] = CWERASE;
# endif
# ifdef VLNEXT
	tio->c_cc[VLNEXT] = CLNEXT;
# endif
    }
    tio->c_cc[VEOF] = CEOF;
    tio->c_cc[VEOL] = VDISABLE;
# ifdef VEOL2
    tio->c_cc[VEOL2] = VDISABLE;
# endif
# ifdef VSWTC
    tio->c_cc[VSWTC] = VDISABLE;
# endif
# ifdef VSWTCH
    tio->c_cc[VSWTCH] = VDISABLE;
# endif
# if VMIN != VEOF
    tio->c_cc[VMIN] = 1;
# endif
# if VTIME != VEOL
    tio->c_cc[VTIME] = 0;
# endif
    if (erase != -1)
    tio->c_cc[VERASE] = (char)erase;

/* input modes */
    tio->c_iflag = (BRKINT | IGNPAR | ICRNL
# ifdef IMAXBEL
		    | IMAXBEL
# endif
		    | IXON);

/* output modes */
    tio->c_oflag = (OPOST | ONLCR);

/* control modes */
    tio->c_cflag = (CS8 | CREAD);

/* line discipline modes */
    tio->c_lflag = (ISIG | ICANON | IEXTEN | ECHO
# if defined (ECHOCTL) && defined (ECHOKE)
		    | ECHOCTL | ECHOKE
# endif
		    | ECHOE | ECHOK);
# else		    /* HAVE_TERMIOS_H */

    /*
     * sgtty interface
     */

    /* get parameters -- gtty */
    if (ioctl(STDIN_FILENO, TIOCGETP, &(tio->sg)) < 0)
    {
	tio->sg.sg_erase = CERASE;  /* ^H */
	tio->sg.sg_kill = CKILL;    /* ^U */
    }
    if (erase != -1)
	tio->sg.sg_erase = (char)erase;

    tio->sg.sg_flags = (CRMOD | ECHO | EVENP | ODDP);

    /* get special characters */
    if (ioctl(STDIN_FILENO, TIOCGETC, &(tio->tc)) < 0)
    {
	tio->tc.t_intrc = CINTR;    /* ^C */
	tio->tc.t_quitc = CQUIT;    /* ^\ */
	tio->tc.t_startc = CSTART;  /* ^Q */
	tio->tc.t_stopc = CSTOP;    /* ^S */
	tio->tc.t_eofc = CEOF;	/* ^D */
	tio->tc.t_brkc = -1;
    }

    /* get local special chars */
    if (ioctl(STDIN_FILENO, TIOCGLTC, &(tio->lc)) < 0)
    {
	tio->lc.t_suspc = CSUSP;    /* ^Z */
	tio->lc.t_dsuspc = CDSUSP;  /* ^Y */
	tio->lc.t_rprntc = CRPRNT;  /* ^R */
	tio->lc.t_flushc = CFLUSH;  /* ^O */
	tio->lc.t_werasc = CWERASE; /* ^W */
	tio->lc.t_lnextc = CLNEXT;  /* ^V */
    }

    /* get line discipline */
    ioctl(STDIN_FILENO, TIOCGETD, &(tio->line));
# ifdef NTTYDISC
    tio->line = NTTYDISC;
# endif		    /* NTTYDISC */
    tio->local = (LCRTBS | LCRTERA | LCTLECH | LPASS8 | LCRTKIL);
#endif		    /* HAVE_TERMIOS_H */

/*
 * Debugging
 */
#ifdef DEBUG
#ifdef HAVE_TERMIOS_H
/* c_iflag bits */
    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "Input flags\n"));

/* cpp token stringize doesn't work on all machines <sigh> */
# define FOO(flag,name)		    \
    if ((tio->c_iflag) & flag)	    \
        rxvt_msg (DBG_DEBUG, DBG_INIT, "%s ", name)

/* c_iflag bits */
    FOO(IGNBRK, "IGNBRK");
    FOO(BRKINT, "BRKINT");
    FOO(IGNPAR, "IGNPAR");
    FOO(PARMRK, "PARMRK");
    FOO(INPCK, "INPCK");
    FOO(ISTRIP, "ISTRIP");
    FOO(INLCR, "INLCR");
    FOO(IGNCR, "IGNCR");
    FOO(ICRNL, "ICRNL");
    FOO(IXON, "IXON");
    FOO(IXOFF, "IXOFF");
# ifdef IUCLC
    FOO(IUCLC, "IUCLC");
# endif
# ifdef IXANY
    FOO(IXANY, "IXANY");
# endif
# ifdef IMAXBEL
    FOO(IMAXBEL, "IMAXBEL");
# endif
    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "\n"));

# undef FOO
# define FOO(entry, name)		    \
    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "%-8s = %#04o\n", name, tio->c_cc [entry]))

    FOO(VINTR, "VINTR");
    FOO(VQUIT, "VQUIT");
    FOO(VERASE, "VERASE");
    FOO(VKILL, "VKILL");
    FOO(VEOF, "VEOF");
    FOO(VEOL, "VEOL");
# ifdef VEOL2
    FOO(VEOL2, "VEOL2");
# endif
# ifdef VSWTC
    FOO(VSWTC, "VSWTC");
# endif
# ifdef VSWTCH
    FOO(VSWTCH, "VSWTCH");
# endif
    FOO(VSTART, "VSTART");
    FOO(VSTOP, "VSTOP");
    FOO(VSUSP, "VSUSP");
# ifdef VDSUSP
    FOO(VDSUSP, "VDSUSP");
# endif
# ifdef VREPRINT
    FOO(VREPRINT, "VREPRINT");
# endif
# ifdef VDISCRD
    FOO(VDISCRD, "VDISCRD");
# endif
# ifdef VWERSE
    FOO(VWERSE, "VWERSE");
# endif
# ifdef VLNEXT
    FOO(VLNEXT, "VLNEXT");
# endif
    rxvt_dbgmsg ((DBG_DEBUG, DBG_INIT, "\n"));
# undef FOO
# endif		    /* HAVE_TERMIOS_H */
#endif		    /* DEBUG */
}

/*----------------------- end-of-file (C source) -----------------------*/
