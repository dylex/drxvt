/*--------------------------------*-C-*---------------------------------*
 * File:    tabbar.c
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2002        Alexis <materm@tuxfamily.org>
 * Copyright (c) 2004        Terry Griffin <griffint@pobox.com>
 * Copyright (c) 2004        Sergey Popov <p_sergey@jungo.com>
 * Copyright (c) 2004-2006   Jingmin Zhou <jimmyzhou@users.sourceforge.net>
 * Copyright (c) 2005        Mark Olesen <Mark.Olesen@gmx.net>
 * Copyright (c) 2005-2006   Gautam Iyer <gi1242@users.sourceforge.net>
 * Copyright (C) 2008		  Jehan Hysseo <hysseo@users.sourceforge.net>
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
#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif


#ifdef HAVE_LIBXPM

#include "close_term.xpm"
#include "term.xpm"
#include "right.xpm"
#include "left.xpm"
#include "close_term_d.xpm"
#include "term_d.xpm"
#include "right_d.xpm"
#include "left_d.xpm"

#else

#include "close_term.xbm"
#include "term.xbm"
#include "right.xbm"
#include "left.xbm"

#endif /* HAVE_LIBXPM */


#ifdef XFT_SUPPORT
# define    FHEIGHT pheight
# define    FWIDTH  pwidth
#else
# define    FHEIGHT fheight
# define    FWIDTH  fwidth
#endif

/* border between the tabs */
#define TAB_BORDER	((int) 1)
/* margin around the text of the tab */
#define TXT_MARGIN	((int) 3)
/*
 * Parameters to draw top tabbar
 */
/* space between top window border and tab top */
#define TAB_TOPOFF	((int) 0)
/* Extra height of the active tab. */
#define ATAB_EXTRA	((int) (ATAB_EXTRA_PERCENT * r->TermWin.FHEIGHT / 100))
/* space between top window border and tab bottom */
#define TAB_BOTOFF	((int) (r->TermWin.FHEIGHT + 2*TXT_MARGIN) + ATAB_EXTRA)

/* Radius of tab corners */
#define TAB_RADIUS	(TAB_RADIUS_PERCENT * TXT_XOFF / 100 )


/* X offset of text in tab */
#define TXT_XOFF	((int) (r->TermWin.FWIDTH - TAB_BORDER))
/* height of text in tab */
#define TXT_YOFF	((int) (r->TermWin.FHEIGHT + TXT_MARGIN + TAB_BORDER))

/* width of No. idx tab */
#define TAB_WIDTH(idx)	((int) (TAB_BORDER + r->vts[idx]->tab_width))

/* size of button */
#define BTN_WIDTH	((int) 18)
#define BTN_HEIGHT	((int) 18)
/* space between top window border and button top */
#define BTN_TOPOFF	(max (0, ((TAB_BOTOFF - BTN_HEIGHT)/2)))
/* space between buttons */
#define BTN_SPACE	((int) 5)

/* width of tabbar that can be used to draw tabs */
#define TAB_SPACE	(TWIN_WIDTH(r)- \
    (ISSET_OPTION(r, Opt2_hideButtons) ? 0 : 1) * \
    (4 * (BTN_WIDTH+BTN_SPACE) + TAB_BORDER))


#define CHOOSE_GC_FG(R, PIXCOL)	\
    XSetForeground ((R)->Xdisplay, (R)->tabBar.gc, (PIXCOL))

/******************************************************************************
*			Begin internal routine prototypes.		      *
******************************************************************************/
/******************************************************************************
*			End internal routine prototypes.		      *
******************************************************************************/

enum {XPM_TERM,XPM_CLOSE,XPM_LEFT,XPM_RIGHT,NB_XPM};

#ifdef HAVE_LIBXPM
static char** xpm_name[] =
{
    term_xpm,close_term_xpm,
    left_xpm,right_xpm
};
static char** xpm_d_name[] =
{
    term_d_xpm,close_term_d_xpm,
    left_d_xpm,right_d_xpm
};
#else
static unsigned char *xbm_name[] =
{
    term_bits,close_term_bits,
    left_bits,right_bits
};
#endif
    
static Pixmap img[NB_XPM];
#ifdef HAVE_LIBXPM
static Pixmap img_e[NB_XPM]; /* enable image */
static Pixmap img_emask[NB_XPM]; /* shape mask image */
static Pixmap img_d[NB_XPM]; /* disable image */
static Pixmap img_dmask[NB_XPM]; /* shape mask image */
#endif
    
extern char **cmd_argv;


/*
 * Width between two tabs:
 * From the left of the first tab to the right of the second tab
 */
/* INTPROTO */
static int
width_between (rxvt_t* r, int start, int end)
{
    register int    i, w=0;
    
    for (i = start; i <= end; i++)
	w += TAB_WIDTH(i);
    
    return w;
}


/*
 * Find most left tab within specified distance. Note that the
 * distance does not include the width of tab[start]. It means
 * distance = (beginning of tab[start] - 0)
 */
/* INTPROTO */
static int
find_left_tab (rxvt_t* r, int start, int distance)
{
    register int    i, left;

    /* Sanatization */
    if (0 == start)
	return 0;

    /* BUG: tab overlap with button */
    if (distance < 0)
	return start;

    left = distance;
    for (i = start - 1; i >= 0; i --)
    {
	if (left < TAB_WIDTH(i))
	    break;
	left -= (TAB_WIDTH(i));
    }
    return (i + 1);
}



/*
 * Find most right tab within specified distance. Note that the
 * distance does not include the width of tab[start]. It means
 * distance = (beginning of first button - end of tab[start])
 */
/* INTPROTO */
static int
find_right_tab (rxvt_t* r, int start, int distance)
{
    register int    i, left;

    /* Sanatization */
    if (LTAB(r) == start)
	return start;

    /* BUG: tab overlap with button */
    if (distance < 0)
	return start;

    left = distance;
    for (i = start + 1; i <= LTAB(r); i ++)
    {
	if (left < TAB_WIDTH(i))
	    break;
	left -= (TAB_WIDTH(i));
    }
    return (i - 1);
}


/* EXTPROTO */
/*
 * If refresh is true, then the respective parts of the tabbar are redrawn.
 * NOTE: This function redraws parts of the tabbar soley based on wether the tab
 * position / width has changed. It does not check to see if the tab titles /
 * etc has changed.
 */
void
rxvt_tabbar_set_visible_tabs (rxvt_t* r, Bool refresh)
{
    assert( LTAB(r) >= 0 );

    /*
     * For Firefox style tabs, we should recompute all tabwidths.
     */
#ifdef XFT_SUPPORT
    if( ISSET_OPTION(r, Opt_xft) && r->TermWin.xftpfont )
    {
	int	i;
	short	tabWidth = rxvt_tab_width( r, NULL);	/* Firefox style tabs
							   don't need the tab
							   title */
	int	numVisible = (TAB_SPACE - TAB_BORDER)
				/ (TAB_BORDER + tabWidth);

	int	oldTabWidth = PVTS(r,0)->tab_width,
		oldFVtab    = FVTAB(r),
		oldLVtab    = LVTAB(r);

	/*
	 * Reset the widths of all tabs (visible or not).
	 */
	for (i = 0; i <= LTAB(r); i ++) PVTS(r, i)->tab_width = tabWidth;

	/*
	 * Set visible tabs. First make sure the active tab is visible
	 */
	if( numVisible == 1 )
	    FVTAB(r) = LVTAB(r) = ATAB(r);
	else
	{
	    if( ATAB(r) < FVTAB(r) )
		/* Make ATAB second last tab that's visible */
		FVTAB(r) = max( ATAB(r) - numVisible + 2, 0);
	    else if ( ATAB(r) >= FVTAB(r) + numVisible )
		/* Make ATAB the second tab that's visible */
		FVTAB(r) = max( ATAB(r) - 1, 0);

	    /*
	     * Active tab is now visible. Try and make as many other tabs
	     * visible.
	     */
	    if( FVTAB(r) + numVisible - 1 > LTAB(r) )
	    {
		LVTAB(r) = LTAB(r);
		FVTAB(r) = max( LVTAB(r) - numVisible + 1, 0);
	    }
	    else
		LVTAB(r) = FVTAB(r) + numVisible - 1;
	}

	if( refresh && IS_WIN(r->tabBar.win))
	{
	    /* Clear out the parts of the tabbar that have changed. Expose
	     * events will be sent to the tabbar. */
	    if( tabWidth != oldTabWidth || FVTAB(r) != oldFVtab )
		/* Refresh all tabs */
		XClearArea( r->Xdisplay, r->tabBar.win,
			0, 0, TAB_SPACE, 0, True);

	    else if( oldLVtab != LVTAB(r) )
	    {
		int x = TAB_BORDER +
		    (TAB_BORDER + tabWidth) * min( oldLVtab, LVTAB(r));
		XClearArea( r->Xdisplay, r->tabBar.win,
			x, 0, TAB_SPACE - x + 1, 0, True);
	    }
	}
    }
    else
#endif
    {
	/* set first visible tab to active tab */
	FVTAB(r) = ATAB(r);

	/* always try visualize the right tabs */
	LVTAB(r) = find_right_tab (r, FVTAB(r),
	    TAB_SPACE - TAB_WIDTH(FVTAB(r)));

	if (LVTAB(r) == LTAB(r) && 0 != FVTAB(r))
	{
	    /* now try to visualize the left tabs */
	    register int size = TAB_SPACE -
		width_between (r, FVTAB(r), LVTAB(r));

	    FVTAB(r) = find_left_tab (r, FVTAB(r), size);
	}
	if( refresh && IS_WIN(r->tabBar.win))
	    XClearArea( r->Xdisplay, r->tabBar.win, 0, 0, TAB_SPACE, 0, True);
    }
}


/*
 *  x, y      : starting position of string, no need to adjust y
 *  str       : string to draw
 *  len       : byte length of the string, not number of characters!
 *  multichar : whether the string is multichar string
 *  active    : active or inactive tab
 *
 *  Returns the pixel width of the string drawn.
 */
/* INTPROTO */
static int
draw_string (rxvt_t* r, Region clipRegion,
	int x, int y, char* str, int len,
	__attribute__((unused)) int multichar, int active)
{
#ifdef XFT_SUPPORT
    XGlyphInfo	ginfo;
#endif

#ifdef MULTICHAR_SET
    if (multichar)
    {
	/*
	 * Draw the multichar string
	 */
# ifdef XFT_SUPPORT

	if (ISSET_OPTION(r, Opt_xft) && (NULL != r->tabBar.xftwin))
	{
#  ifdef HAVE_ICONV_H
	    if (
		    ENC_NOENC != r->encoding_method
		    && (iconv_t) -1 != r->TermWin.xfticonv
	       )
	    {
		char		buf[1024];
		int		plen = 1023;
		char*		pstr = buf;
		int		olen = len;
		char*		ostr = str;

		/* convert to UTF-8 */
		iconv (r->TermWin.xfticonv, (char**) &ostr,
		    (size_t*) &olen, &pstr, (size_t*) &plen);
		*pstr = (char) 0;   /* set end of string */

		rxvt_draw_string_xft (r, r->tabBar.win, r->tabBar.gc,
			clipRegion, RS_None, 
			active ? USE_BOLD_PFONT : USE_PFONT,
			r->tabBar.xftwin,
			active ? &(r->tabBar.xftfg) : &(r->tabBar.xftifg),
			x, y, buf, len, XftDrawStringUtf8);
		if( r->TermWin.xftpfont )
		{
		    XftTextExtentsUtf8( r->Xdisplay, r->TermWin.xftpfont,
			    (unsigned char*) buf, pstr - buf, &ginfo);
		    return ginfo.width;
		}
		else return Width2Pixel( pstr - buf );
	    }
	    else
#  endif
	    {
		rxvt_dbgmsg ((DBG_VERBOSE, DBG_TABBAR, "XFT non-iconv tab title\n"));
		rxvt_draw_string_xft (r, r->tabBar.win, r->tabBar.gc,
			clipRegion, RS_None,
			active ? USE_BOLD_PFONT : USE_PFONT,
			r->tabBar.xftwin,
			active ? &(r->tabBar.xftfg) : &(r->tabBar.xftifg),
			x, y, str, len, XftDrawString8);

		if( r->TermWin.xftpfont )
		{
		    XftTextExtents8( r->Xdisplay, r->TermWin.xftpfont,
			    (unsigned char*) str, len, &ginfo);
		    return ginfo.width;
		}
		else return Width2Pixel( len );
	    }
	}
	else
# endif	/* XFT_SUPPORT */
	{
	    if (ENC_NOENC != r->encoding_method)
	    {
		XSetFont (r->Xdisplay, r->tabBar.gc, r->TermWin.mfont->fid);
		r->h->multichar_decode ( (unsigned char*) str, len);
		rxvt_draw_string_x11 (r, r->tabBar.win, r->tabBar.gc,
			clipRegion, x, y, str, len/2, XDrawString16);
	    }
	    else
	    {
		XSetFont (r->Xdisplay, r->tabBar.gc, r->TermWin.font->fid);
		rxvt_draw_string_x11 (r, r->tabBar.win, r->tabBar.gc,
			clipRegion, x, y, str, len, XDrawString);
	    }
	    return Width2Pixel( len );
	}
    } /* if (multichar) */

    else
#endif /* MULTICHAR_SET */
    {
	/*
	 * Draw the non-multichar string
	 */
# ifdef XFT_SUPPORT
	if (ISSET_OPTION(r, Opt_xft) && (NULL != r->tabBar.xftwin))
	{
	    rxvt_draw_string_xft (r, r->tabBar.win, r->tabBar.gc,
		    clipRegion, RS_None,
		    active ? USE_BOLD_PFONT : USE_PFONT,
		    r->tabBar.xftwin,
		    active ? &(r->tabBar.xftfg) : &(r->tabBar.xftifg),
		    x, y, str, len, XftDrawString8);

	    if( r->TermWin.xftpfont )
	    {
		XftTextExtents8( r->Xdisplay, r->TermWin.xftpfont,
			(unsigned char*) str, len, &ginfo);
		return ginfo.width;
	    }
	    else return Width2Pixel( len );
	}
	else
# endif	/* XFT_SUPPORT */
	{
	    XSetFont (r->Xdisplay, r->tabBar.gc, r->TermWin.font->fid);
	    rxvt_draw_string_x11 (r, r->tabBar.win, r->tabBar.gc,
		    clipRegion, x, y, str, len, XDrawString);
	    return Width2Pixel( len );
	}
    }
}


/*
 * Draw tab title string
 *
 * If region is non-empty, we assume that the caller has set the GC's clipping
 * to region, and we honor it.
 */
/* INTPROTO */
static void
draw_title (rxvt_t* r, int x, int y, int tnum, Region region)
{
    Region	clipRegion;
    char	str[MAX_DISPLAY_TAB_TXT + 1];

#ifdef MULTICHAR_SET
    char	buf[MAX_TAB_TXT + 1];
    const char*	sptr;
    const char*	ptr;
    int		multichar;
    int		len;
#endif

    UNSET_REGION( clipRegion );

    /*
     * Adjust y offset, and make sure output is restricted to the current tab
     * title.
     */
#ifdef XFT_SUPPORT
    if (ISSET_OPTION(r, Opt_xft) && (NULL != r->tabBar.xftwin))
    {
	if( r->TermWin.xftpfont )
	{
	    /*
	     * If we use pfont to draw tab titles, the we dont' know how many
	     * characters will fit on the title. So we should clip the output
	     * correctly.
	     */
	    XRectangle rect;

	    rect.x = x;
	    rect.y = y - r->TermWin.pheight;
	    rect.width = PVTS(r, tnum)->tab_width - 2*TXT_XOFF;
	    rect.height = r->TermWin.pheight;

	    clipRegion = XCreateRegion();
	    XUnionRectWithRegion( &rect, clipRegion, clipRegion);

	    if (IS_REGION(region))
		XIntersectRegion( clipRegion, region, clipRegion);

	    XftDrawSetClip( r->tabBar.xftwin, clipRegion);

	    y -= r->TermWin.xftpfont->descent;
	}
	else y -= r->TermWin.xftfont->descent;
    }
    else
#endif /* XFT_SUPPORT */
	y -= r->TermWin.font->descent;

    /*
     * Get the title into str. Under Xft, we use the format specified by
     * title_format.
     */
#ifdef XFT_SUPPORT
    if(
	  NOTSET_OPTION( r, Opt_xft )			||
	  IS_NULL( r->TermWin.xftpfont )		||
	  IS_NULL( PVTS(r, tnum)->title_format )   	||
	  rxvt_percent_interpolate( r, tnum,
		PVTS(r, tnum)->title_format,
		STRLEN( PVTS(r, tnum)->title_format ),
		str, r->TermWin.maxTabWidth ) <= 1
      )
#endif /* XFT_SUPPORT */
    {
	/*
	 * If % interpolation was not possible, or returned a 1 byte long
	 * string, then just copy the title over.
	 *
	 * 2008-08-04 Jim Diamond: Show the suffix of the title, not the prefix,
	 * if chopEnd is enabled. This only affects the non-xft case.
	 */
	if( ISSET_OPTION( r, Opt3_chopEnd ) )
	{
	    STRNCPY( str, PVTS(r,tnum)->tab_title , r->TermWin.maxTabWidth );
	    str[r->TermWin.maxTabWidth] = '\0';
	}
	else
	{
	    int title_len = STRLEN(PVTS(r,tnum)->tab_title);
	    int excess = max(title_len - r->TermWin.maxTabWidth, 0);

	    STRNCPY( str, PVTS(r,tnum)->tab_title + excess,
		     r->TermWin.maxTabWidth + 1 );  /* + 1 ensures we get \0 */
	}
    }


    /*
     * Draw the string (different code for multichar / non-multichar).
     */
#ifdef MULTICHAR_SET
    sptr = ptr = str;
    multichar = (*ptr & 0x80);
    while (*ptr)
    {
	if (multichar && (*ptr & 0x80))	    /* multichar */
	    ptr ++;
	else if (!multichar && !(*ptr & 0x80))	/* single char */
	    ptr ++;
	else
	{
	    len = ptr - sptr;
	    /* adjust bytes, must be 2x for multichar */
	    if (multichar && (len % 2) != 0)
	    {
		len ++; ptr ++;
		/* continue to next byte, we shouldn't stop here */
		continue;
	    }
	    assert (len <= MAX_TAB_TXT);

	    memcpy (buf, sptr, len);
	    buf[len] = (char) 0;
	    x += draw_string (r, clipRegion,
		    x, y, buf, len, multichar, tnum == ATAB(r));

	    /* adjust start position */
	    /* x += Width2Pixel(len); */
	    /*
#ifdef XFT_SUPPORT
	    if (ISSET_OPTION(r, Opt_xft) && r->tabBar.xftwin)
	    {
		x += Width2Pixel(len);
	    }
	    else
#endif
	    {
		if (multichar)
		    x += XTextWidth (r->TermWin.mfont, buf, len/2);
		else
		    x += XTextWidth (r->TermWin.font, buf, len);
	    }
	    */

	    /* ok, now the next sub-string */
	    sptr = ptr;
	    multichar = (*ptr & 0x80);
	    if ((char) 0 == *ptr)
		break;	/* in case ptr is increased at line 356 */
	    ptr ++;
	}
    }

    /* last sub-string */
    len = ptr - sptr;
    if (0 != len)	/* in case last sub-string is empty */
    {
	memcpy (buf, sptr, len);
	buf[len] = (char) 0;
	draw_string (r, clipRegion,
		x, y, buf, len, multichar, tnum == ATAB(r));
    }

#else	/* MULTICHAR_SET */
    draw_string (r, clipRegion,
	    x, y, str, STRLEN(str), False, tnum == ATAB(r));
#endif	/* MULTICHAR_SET */

    /*
     * Restore clipping of the xftdrawable / gc.
     */
    if (IS_REGION(clipRegion))
    {
	XDestroyRegion( clipRegion);

	if (NOT_REGION(region))
	    XSetClipMask( r->Xdisplay, r->tabBar.gc, None);
	else
	    XSetRegion( r->Xdisplay, r->tabBar.gc, region);
#ifdef XFT_SUPPORT
	if (r->tabBar.xftwin)
	    XftDrawSetClip( r->tabBar.xftwin, region);
#endif
    }
}


#define SET_ARC( arc, ax, ay, awidth, aheight, aangle1, aangle2)    \
    (arc).x	    = (short) (ax);				    \
    (arc).y	    = (short) (ay);				    \
    (arc).width	    = (unsigned short) (awidth);		    \
    (arc).height    = (unsigned short) (aheight);		    \
    (arc).angle1    = (short) (aangle1);			    \
    (arc).angle2    = (short) (aangle2)

#define SET_POINT( point, ax, ay)   \
    point.x	    = (short) ax;   \
    point.y	    = (short) ay

/*
 * Refresh title of tab "page"
 */
/* EXTPROTO */
void
refresh_tabbar_tab( rxvt_t *r, int page)
{
    int		i;
    XRectangle	rect;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR, "Refreshing tabbar title of page %d\n", page));

    if( page < FVTAB(r) || page > LVTAB(r) ) return;

    for( i=FVTAB(r), rect.x=TAB_BORDER; i < page; i++)
	rect.x += TAB_WIDTH(i);
    
    rect.y	= TAB_TOPOFF;
    rect.width	= TAB_WIDTH( page);
    rect.height	= 0;

    /* Clear the tab completely, and send expose events */
    XClearArea( r->Xdisplay, r->tabBar.win,
	    rect.x, rect.y, rect.width, rect.height, True);
}

/*
 * Draw all visible tabs at top. If region is not none, then we clip output to
 * it.
 */
/* INTPROTO */
void rxvt_draw_tabs (rxvt_t* r, Region region)
{
    int	    page, x;

    if (LTAB(r) < 0 || NOT_WIN(r->tabBar.win) || !r->tabBar.state)
	/*
	 * Nothing to do here :)
	 */
	return;

    /* Sanatization */
    assert( LTAB(r)  >= 0	 );
    assert( FVTAB(r) >= 0	 );
    assert( FVTAB(r) <= LTAB(r)	 );
    assert( LVTAB(r) >= 0	 );
    assert( LVTAB(r) <= LTAB(r)	 );
    assert( ATAB(r)  >= FVTAB(r) );
    assert( ATAB(r)  <= LVTAB(r) );


    if (IS_REGION(region))
	XSetRegion( r->Xdisplay, r->tabBar.gc, region);

    for( page=FVTAB(r), x=TAB_BORDER; page <= LVTAB(r); page++)
    {
	/*
	 * Draw the tab corresponding to "page".
	 */
	XArc	 	arcs[2];
	XPoint	 	points[8];

	/*
	 * Color the tab as active if the tab is active 
	 * or
	 * activity/inactivity is detected by the MonitorTab macro
	 */

	if( (page == ATAB(r)) || 
 	    ((NOT_NULL(&PVTS(r, page)->monitor_tab)) && 
	     (PVTS(r,page)->monitor_tab == TAB_MON_NOTIFICATION)) 
	  )
	{
	    /* 
	     * disable activity/inactivity notification if current tab is
	     * active tab
	     */
	    if ((page == ATAB(r)) && 
		(PVTS(r,page)->monitor_tab == TAB_MON_NOTIFICATION))
	    {
	      rxvt_msg (DBG_INFO, DBG_MACROS,  
		      "Macro MonitorTab: monitored tab %i is now the active tab", page);
	      PVTS(r,page)->monitor_tab = TAB_MON_OFF;
            }
	    /*
	     * Draw the active tab, and bottom line of the tabbar.
	     */

	    int		    clear = 0;	/* use ClearArea or FillRectangle */

	    if (ISSET_OPTION(r, Opt2_bottomTabbar))
	    {
		/* Top tabbar line & left of active tab */
		SET_POINT( points[0], 0, TAB_TOPOFF);
		SET_POINT( points[1], x, TAB_TOPOFF);
		SET_POINT( points[2], x, TAB_BOTOFF - TAB_RADIUS);

		/* Arc coordinates for rounded tab tops :) */
		SET_ARC( arcs[0], x, TAB_BOTOFF - 2*TAB_RADIUS,
			2*TAB_RADIUS, 2*TAB_RADIUS, 180*64, 90*64);
		SET_ARC( arcs[1],
			x + AVTS(r)->tab_width - 2*TAB_RADIUS,
			TAB_BOTOFF - 2*TAB_RADIUS,
			2*TAB_RADIUS, 2*TAB_RADIUS, 270*64, 90*64);

		/* Coordinates for horizontal line below tab. */
		SET_POINT( points[3], x + TAB_RADIUS, TAB_BOTOFF);
		SET_POINT( points[4],
			x + AVTS(r)->tab_width - TAB_RADIUS, TAB_BOTOFF);

		/* Right line of tab and top of tabbar. */
		SET_POINT( points[5],
			x + AVTS(r)->tab_width, TAB_BOTOFF - TAB_RADIUS);
		SET_POINT( points[6], x + AVTS(r)->tab_width, TAB_TOPOFF);
		SET_POINT( points[7], TWIN_WIDTH(r), TAB_TOPOFF);
	    }

	    else    /* if (ISSET_OPTION(r, Opt2_bottomTabbar)) */
	    {
		/*
		 * Coordinates for the draw bottom line to the left of active
		 * tab, and left verticle line of the active tab.
		 */
		SET_POINT( points[0], 0, TAB_BOTOFF);
		SET_POINT( points[1], x, TAB_BOTOFF);
		SET_POINT( points[2], x, TAB_TOPOFF + TAB_RADIUS);

		/* Arc coordinates for rounded tab tops :) */
		SET_ARC( arcs[0], x, TAB_TOPOFF,
			2*TAB_RADIUS, 2*TAB_RADIUS, 180*64, -90*64);
		SET_ARC( arcs[1],
			x + AVTS(r)->tab_width - 2*TAB_RADIUS, TAB_TOPOFF,
			2*TAB_RADIUS, 2*TAB_RADIUS, 90*64, -90*64);

		/* Coordinates for horizontal line above tab. */
		SET_POINT( points[3], x + TAB_RADIUS, TAB_TOPOFF);
		SET_POINT( points[4],
			x + AVTS(r)->tab_width - TAB_RADIUS, TAB_TOPOFF);

		/*
		 * Coordinates for vertical line on the right of the active tab, and
		 * bottom line of tab bar after active tab.
		 */
		SET_POINT( points[5], x + AVTS(r)->tab_width,
			TAB_TOPOFF + TAB_RADIUS);
		SET_POINT( points[6], x + AVTS(r)->tab_width, TAB_BOTOFF);
		SET_POINT( points[7], TWIN_WIDTH(r), TAB_BOTOFF);
	    }

#ifdef BACKGROUND_IMAGE
	    if( r->tabBar.hasPixmap  && ISSET_OPTION(r, Opt_tabPixmap))
		clear = 1;  /* use background image */
#endif
#ifdef TRANSPARENT
	    if ( ( r->h->am_transparent || r->h->am_pixmap_trans ) &&
		ISSET_OPTION(r, Opt_transparent_tabbar))
		clear = 1;  /* transparent override background image */
#endif

	    if( !clear )
	    {
		/*
		 * Fill the ATAB with the background color.
		 */
		CHOOSE_GC_FG( r, r->tabBar.bg);

		XFillArcs( r->Xdisplay, r->tabBar.win, r->tabBar.gc,
			arcs, 2);
		XFillPolygon( r->Xdisplay, r->tabBar.win, r->tabBar.gc,
			points+1, 6, Convex, CoordModeOrigin);

		/*
		 * This misses the bottom of the ATAB, so we should color it
		 * ourselves.
		 *
		 * 2006-02-14 gi1242: Drawing with XDrawLine is not enough. For
		 * some reason a thin line below is still missed. Be super safe
		 * and XFillRectangle it.
		 *
		 * 2006-05-26 gi1242: The thin line looks kinda nice actually...
		 */
#if 0
		XDrawLine( r->Xdisplay, r->tabBar.win, r->tabBar.gc,
			points[1].x, points[1].y,
			points[6].x, points[6].y);

		XFillRectangle( r->Xdisplay, r->tabBar.win, r->tabBar.gc,
			points[1].x, points[1].y,
			points[6].x - points[1].x, 2);
#endif
	    }


	    /*
	     * Finally, draw the (boundary) of ATAB here.
	     */
	    CHOOSE_GC_FG( r, r->tabBar.frame);

	    /* Tabbar line + left of ATAB */
	    XDrawLines( r->Xdisplay, r->tabBar.win, r->tabBar.gc,
		    points, 3, CoordModeOrigin);
	    /* Rounded tab tops :) */
	    XDrawArcs( r->Xdisplay, r->tabBar.win, r->tabBar.gc, arcs, 2);
	    /* Top line of ATAB */
	    XDrawLines( r->Xdisplay, r->tabBar.win, r->tabBar.gc,
		    points + 3, 2, CoordModeOrigin);
	    /* Right of ATAB + tab bar line */
	    XDrawLines( r->Xdisplay, r->tabBar.win, r->tabBar.gc,
		    points + 5, 3, CoordModeOrigin);

	    /* Draw the tab title. */
	    CHOOSE_GC_FG( r, r->tabBar.fg);
	    draw_title (r, x + TXT_XOFF, ATAB_EXTRA / 2 + TXT_YOFF,
		    page, region);
	}

	else /* if( page == ATAB(r) ) */
	{
	    /*
	     * Draw the inactive tabs.
	     */
	    CHOOSE_GC_FG( r, r->tabBar.delimit);

	    if (ISSET_OPTION(r, Opt2_bottomTabbar))
	    {
		/* Left vertical line */
		XDrawLine( r->Xdisplay, r->tabBar.win, r->tabBar.gc,
			x, TAB_TOPOFF + 1, /* Dont' interupt tabbar line */
			x, TAB_BOTOFF - TAB_RADIUS - ATAB_EXTRA);

		/* Draw rounded tab bottoms :). */
		SET_ARC( arcs[0], x, TAB_BOTOFF - ATAB_EXTRA - 2*TAB_RADIUS,
			2*TAB_RADIUS, 2*TAB_RADIUS, 180*64, 90*64);
		SET_ARC( arcs[1],
			x + PVTS(r, page)->tab_width - 2*TAB_RADIUS,
			TAB_BOTOFF - ATAB_EXTRA - 2*TAB_RADIUS,
			2*TAB_RADIUS, 2*TAB_RADIUS, 270*64, 90*64);
		XDrawArcs( r->Xdisplay, r->tabBar.win, r->tabBar.gc, arcs, 2);

		/* Horizontal line below tab. */
		XDrawLine( r->Xdisplay, r->tabBar.win, r->tabBar.gc,
			x + TAB_RADIUS, TAB_BOTOFF - ATAB_EXTRA,
			x + PVTS(r, page)->tab_width - TAB_RADIUS,
			TAB_BOTOFF - ATAB_EXTRA);

		/* Right vertical line */
		XDrawLine( r->Xdisplay, r->tabBar.win, r->tabBar.gc,
			x + PVTS(r, page)->tab_width,
			TAB_BOTOFF - TAB_RADIUS - ATAB_EXTRA,
			x + PVTS(r, page)->tab_width, TAB_TOPOFF + 1);
	    }

	    else /* if (ISSET_OPTION(r, Opt2_bottomTabbar)) */
	    {
		/* Left vertical line */
		XDrawLine( r->Xdisplay, r->tabBar.win, r->tabBar.gc,
			x, TAB_BOTOFF-1, x,
			TAB_TOPOFF + TAB_RADIUS + ATAB_EXTRA);

		/* Draw rounded tab tops :). */
		SET_ARC( arcs[0], x, TAB_TOPOFF + ATAB_EXTRA,
			2*TAB_RADIUS, 2*TAB_RADIUS, 180*64, -90*64);
		SET_ARC( arcs[1],
			x + PVTS(r, page)->tab_width - 2*TAB_RADIUS,
			TAB_TOPOFF + ATAB_EXTRA,
			2*TAB_RADIUS, 2*TAB_RADIUS, 90*64, -90*64);
		XDrawArcs( r->Xdisplay, r->tabBar.win, r->tabBar.gc, arcs, 2);

		/* Horizontal line above tab. */
		XDrawLine( r->Xdisplay, r->tabBar.win, r->tabBar.gc,
			x + TAB_RADIUS, TAB_TOPOFF + ATAB_EXTRA,
			x + PVTS(r, page)->tab_width - TAB_RADIUS,
			TAB_TOPOFF + ATAB_EXTRA);

		/* Right vertical line */
		XDrawLine( r->Xdisplay, r->tabBar.win, r->tabBar.gc,
			x + PVTS(r, page)->tab_width,
			TAB_TOPOFF + TAB_RADIUS + ATAB_EXTRA,
			x + PVTS(r, page)->tab_width, TAB_BOTOFF-1);
	    }

	    /* Choose GC foreground for tab title. */
	    CHOOSE_GC_FG( r, r->tabBar.ifg);
	    draw_title (r, x + TXT_XOFF,
		    ISSET_OPTION(r, Opt2_bottomTabbar) ?
		     	    TXT_YOFF : ATAB_EXTRA + TXT_YOFF,
		    page, region);

	    /* Highlight the tab if necessary */
	    if( PVTS(r, page)->highlight )
		rxvt_tabbar_highlight_tab( r, page, True);
	}


	x += TAB_WIDTH(page);
    }

    if (IS_REGION(region))
	XSetClipMask( r->Xdisplay, r->tabBar.gc, None);
}


/* EXTPROTO */
void
rxvt_tabbar_highlight_tab (rxvt_t* r, short page, Bool force)
{
    register int    i, x;
    int		    sx, sy;
    unsigned int    rw, rh;
    XGCValues	    gcvalues;


    /* Sanatization */
    assert (LTAB(r) >= 0);
    assert (FVTAB(r) >= 0);
    assert (FVTAB(r) <= LTAB(r));
    assert (LVTAB(r) >= 0);
    assert (LVTAB(r) <= LTAB(r));
    assert (ATAB(r) >= FVTAB(r));
    assert (ATAB(r) <= LVTAB(r));

    assert (page <= LTAB(r));

    /* highlight flag is already set, simply return */
    if ( !force && PVTS(r, page)->highlight)
	return;	

    /* set highlight flag */
    PVTS(r, page)->highlight = 1;

    if (LTAB(r) < 0 || NOT_WIN(r->tabBar.win) || !r->tabBar.state)
	return ;

    /* do not highlight invisible/active tab */
    if (page < FVTAB(r) || page > LVTAB(r) || page == ATAB(r))
	return;

    for (i = FVTAB(r), x=TAB_BORDER; i < page; x += TAB_WIDTH(i), i++);

    /* set dash-line attributes */
    XGetGCValues( r->Xdisplay, r->tabBar.gc,
	    GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle,
	    &gcvalues);
    XSetLineAttributes (r->Xdisplay, r->tabBar.gc,
	    1, LineOnOffDash, CapButt, JoinMiter);

    XSetForeground (r->Xdisplay, r->tabBar.gc, r->tabBar.ifg);

    /* Set dimensions of the highlighted tab rectangle */
    sx = x + ( TXT_XOFF / 2 );
    sy = ISSET_OPTION(r, Opt2_bottomTabbar) ?
		TAB_TOPOFF + 1			:
		TAB_TOPOFF + ATAB_EXTRA + 1;
    rw = PVTS(r, page)->tab_width - TXT_XOFF;
    rh = TAB_BOTOFF - TAB_TOPOFF - ATAB_EXTRA - 3;

    XDrawRectangle (r->Xdisplay, r->tabBar.win, r->tabBar.gc,
	sx, sy, rw, rh);

    /* restore solid-line attributes */
    XChangeGC( r->Xdisplay, r->tabBar.gc,
	    GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle,
	    &gcvalues);
}

/*
 * Buttons
 */
/* EXTPROTO */
void
rxvt_tabbar_draw_buttons (rxvt_t* r)
{
    register int    i;
    int		    topoff;
    unsigned long   frame;


    if (LTAB(r) < 0)
	return;
    if (NOT_WIN(r->tabBar.win))
	return;
    if (!r->tabBar.state)
	return;

    /* whether the buttons are hidden */
    if (ISSET_OPTION(r, Opt2_hideButtons))
	return;

    topoff = BTN_TOPOFF;
#if 0
    frame = NOTSET_OPTION(r, Opt2_bottomTabbar) ?
		r->tabBar.frame : r->tabBar.delimit;
#endif
    frame = r->tabBar.frame;

    CHOOSE_GC_FG (r, r->tabBar.fg);
    for (i = NB_XPM; i >= 1; i--)
    {
#ifdef HAVE_LIBXPM
	register int	curimg = NB_XPM - i;

	switch (curimg)
	{
	    case XPM_TERM:
		/*img[XPM_TERM] = (LTAB(r) == MAX_PAGES - 1) ? 
		    img_d[XPM_TERM] : img_e[XPM_TERM];*/
			 /* 08-08-20 Jehan: 
			  * As there is no more maximum tab, there is no disable image here. */
			 img[XPM_TERM] = img_e[XPM_TERM];
		break;
	    case XPM_CLOSE:
		img[XPM_CLOSE] = (ISSET_OPTION(r, Opt2_protectSecondary) &&
				PRIMARY != AVTS(r)->current_screen) ?
			img_d[XPM_CLOSE] : img_e[XPM_CLOSE];
		break;
	    case XPM_LEFT:
		img[XPM_LEFT] = (FVTAB(r) == 0) ? 
		    img_d[XPM_LEFT] : img_e[XPM_LEFT];
		break;
	    case XPM_RIGHT:
		img[XPM_RIGHT] = (LVTAB(r) == LTAB(r)) ? 
		    img_d[XPM_RIGHT] : img_e[XPM_RIGHT];
		break;
	}
#endif
	if (IS_PIXMAP(img[NB_XPM-i]))
	{
	    XCopyArea  (r->Xdisplay, img[NB_XPM-i], r->tabBar.win,
		r->tabBar.gc, 0, 0,
		BTN_WIDTH, BTN_HEIGHT,
		TWIN_WIDTH(r)-(i*(BTN_WIDTH+BTN_SPACE)), topoff);
	}
    }


    CHOOSE_GC_FG (r, r->tabBar.frame);
    for (i = NB_XPM; i >= 1; i--)
    {
	/*
	XDrawRectangle (r->Xdisplay, r->tabBar.win,
	    r->tabBar.gc,
	    TWIN_WIDTH(r)-(i*(BTN_WIDTH+BTN_SPACE)), topoff,
	    BTN_WIDTH, BTN_HEIGHT);
	*/
	int	sx = TWIN_WIDTH(r) - (i*(BTN_WIDTH+BTN_SPACE));
	/* draw top line */
	XDrawLine (r->Xdisplay, r->tabBar.win, r->tabBar.gc,
	    sx, topoff, sx + BTN_WIDTH, topoff);
	/* draw left line */
	XDrawLine (r->Xdisplay, r->tabBar.win, r->tabBar.gc,
	    sx, topoff, sx, topoff + BTN_HEIGHT);
    }
    CHOOSE_GC_FG (r, r->tabBar.delimit);
    for (i = NB_XPM; i >= 1; i--)
    {
	int	sx = TWIN_WIDTH(r) - (i*(BTN_WIDTH+BTN_SPACE));
	/* draw bottom line */
	XDrawLine (r->Xdisplay, r->tabBar.win, r->tabBar.gc,
	    sx, topoff+BTN_HEIGHT, sx+BTN_WIDTH, topoff+BTN_HEIGHT);
	/* draw right line */
	XDrawLine (r->Xdisplay, r->tabBar.win, r->tabBar.gc,
	    sx+BTN_WIDTH, topoff, sx+BTN_WIDTH, topoff+BTN_HEIGHT);
    }
}


/*
 * Initialize global data structure of all tabs
 */
/* INTPROTO */
static void
init_tabbar (rxvt_t* r)
{
    r->tabBar.state = 0;    /* not mapped yet */

    LTAB(r) = -1;   /* the last tab */
    r->tabBar.atab = 0;	/* the active tab */
    FVTAB(r) = 0;   /* first visiable tab */
    LVTAB(r) = 0;   /* last visiable tab */
    r->tabBar.ptab = 0;	    /* previous active tab */

    /* Make sure that font has been initialized */
#ifdef XFT_SUPPORT
    if (ISSET_OPTION (r, Opt_xft))
	assert (NULL != r->TermWin.xftfont);
    else
#endif
    assert (NULL != r->TermWin.font);
    assert (r->TermWin.FHEIGHT > 0);

    /* resource string are static, needn't to free */
    r->tabBar.rsfg =
    r->tabBar.rsbg =
    r->tabBar.rsifg =
    r->tabBar.rsibg = 0;
}


/* INTPROTO */
void
rxvt_kill_page (rxvt_t* r, short page)
{
    kill (PVTS(r, page)->cmd_pid, SIGHUP);
}


/*
 * Reduce r->num_fds so that select() is more efficient
 */
/* EXTPROTO */
void
rxvt_adjust_fd_number( rxvt_t* r )
{
    int	    num_fds = STDERR_FILENO;
    int	    i;

    for( i=0; i <= LTAB(r); i++ )
	MAX_IT( num_fds, PVTS(r, i)->cmd_fd );
    rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR, "LTAB=%d, stderr_fd=%d, num_fds=%d. ", LTAB(r), STDERR_FILENO, num_fds));

    MAX_IT( num_fds, r->Xfd );
#ifdef USE_FIFO
    MAX_IT( num_fds, r->fifo_fd );
#endif/*USE_FIFO*/
#ifdef HAVE_X11_SM_SMLIB_H
    MAX_IT( num_fds, r->TermWin.ice_fd );
#endif

#if 0
    MAX_IT( num_fds, r->num_fds-1 );
#endif
#ifdef OS_IRIX
    /* Alex Coventry says we need 4 & 7 too */
    MAX_IT( num_fds, 7 );
#endif
    r->num_fds = num_fds + 1;	/* counts from 0 */
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TABBAR, "Adjust num_fds to %d\n", r->num_fds));
}


/*
 * Append a new tab after the last tab. If command is not NULL, run that
 * command in the tab. If command begins with '!', then run the shell first.
 */
/* EXTPROTO */
void
rxvt_append_page( rxvt_t* r, int profile,
	const char TAINTED *title, const char *command )
{
    int	    num_cmd_args = 0; /* Number of args we got from parsing command */
    char**  argv;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR, "rxvt_append_page( r, %d, %s, %s )\n", profile, title ? title : "(nil)", command ? command : "(nil)" ));

    if( profile < 0 || profile >= MAX_PROFILES )
    {
	rxvt_msg (DBG_WARN, DBG_TABBAR,  "Warning: Profile '%d' out of range; use profile '0' instead.", profile );
	profile = 0;
    }

    //LTAB(r)++;

    /*
     * Use command specified with -e only if we're opening the first tab, or the
     * --cmdAllTabs option is specified, and we're not given a command to
     *  execute (e.g. via the NewTab cmd macro).
     */
    if(
	    cmd_argv	    /* Argument specified via -e option */
	    && command == NULL /* No command specified (e.g. via NewTab macro) */
	    && (
		//LTAB(r) == 0			    /* First tab */
		LTAB (r) == - 1
		|| ISSET_OPTION(r, Opt2_cmdAllTabs)  /* -at option */
	       )
      )
	argv = cmd_argv;
    else
    {
	/* load tab command if necessary*/
	if( command == NULL )
	    command = getProfileOption( r, profile, Rs_command );

	if( command != NULL && *command != '!' )
	{
	    const char *cmd = command;

	    /* If "command" starts with '!', we should run it in the shell. */
	    if( cmd[0] == '\\' && cmd[1] == '!' )
		cmd++;

	    argv = rxvt_string_to_argv( cmd, &num_cmd_args );
	}
	else
	    argv = NULL;
    }
    rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR, "\tForking command=%s, argv[0]=%s\n", command ? command : "(nil)", ( argv && argv[0] ) ? argv[0] : "(nil)" ));

    /*
     * Set the tab title.
     */
    if( title == NULL || *title == '\0' )
    {
	title = getProfileOption( r, profile, Rs_tabtitle );
	if( title == NULL || *title == '\0' )
	{
	    if( command && *command != '\0' )
		title = command;
	    else if( argv && argv[0] && *argv[0] != '\0' )
		title = argv[0];
	}
    }
    if (!rxvt_create_termwin( r, LTAB(r) + 1, profile, title ))
    {
	rxvt_dbgmsg ((DBG_ERROR, DBG_TABBAR,
		    "\tThe initialization of the new tab failed.\n"));
	return;
    }

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TABBAR, "Last page is %d.\n", LTAB(r)));

    /*
     * Run the child process.
     *
     * 2006-02-17 gi1242: Bug -- If the child produces some output and exits
     * quickly, then some of that output is sometimes lost.
     */
    if( getProfileOption( r, profile, Rs_cwd ) != NULL )
    {
	const char  *cwdOption	= getProfileOption( r, profile, Rs_cwd );
	char	    cwd[PATH_MAX] = "",
		    child_cwd[PATH_MAX] = "";
	int	    len = 0;


	getcwd (cwd, PATH_MAX);

	if( !STRCMP( cwdOption, "." ) )
	{
	    if( ATAB(r) != LTAB(r) )
	    {
		/*
		 * Copy working directory of the current tab into child_cwd.
		 */
		char	proc_cwd[32];		/* 16 is enough */

		sprintf( proc_cwd, "/proc/%d/cwd", AVTS(r)->cmd_pid );
		if( (len = readlink( proc_cwd, child_cwd, PATH_MAX-1) ) > 0 )
		    /* readlink does not null terminate */
		    child_cwd[len] = 0;
	    }
	}

	else
	{
#ifdef HAVE_WORDEXP_H
	    wordexp_t p;
	    int wordexp_result = wordexp(cwdOption, &p, 0);
	    char *filename;

	    if( wordexp_result == 0 )
	    {
		if( p.we_wordc > 1)
		    rxvt_msg( DBG_ERROR, DBG_TABBAR,
			    "Too many words when expanding %s\n", cwdOption );
		else
		{
		    filename = *p.we_wordv;

		    len  = STRLEN( filename );
		    MIN_IT( len, PATH_MAX - 1 );
		    STRNCPY( child_cwd, filename, len );
		    child_cwd[len] = 0;
		}

		wordfree( &p );
	    }

	    else
	    {
		rxvt_dbgmsg(( DBG_VERBOSE, DBG_TABBAR,
			    "wordexp error code '%i', expanding '%s'\n",
			    wordexp_result, filename ));
	    }
#endif /* HAVE_WORDEXP_H */
	}

	if( len > 0 && chdir( child_cwd ) == 0 )
	{
	    /* Now in working directory of ATAB */
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR,
			"Running child in directory: %s\n", child_cwd ));

	    /* Run command in this new directory. */
	    LVTS(r)->cmd_fd =
		rxvt_run_command( r, LTAB(r), (const char**) argv );

	    /* Restore old working directory. */
	    chdir( cwd );
	}

	else
	{
	    /* Exec command in original directory. */
	    rxvt_dbgmsg(( DBG_DEBUG, DBG_TABBAR,
			"Running child in original directory\n"));

	    LVTS(r)->cmd_fd =
		rxvt_run_command( r, LTAB(r), (const char**) argv );
	}
    }
    else
	LVTS(r)->cmd_fd = rxvt_run_command (r, LTAB(r), (const char**) argv);


    /*
     * In case we allocated memory for argv using rxvt_string_to_argv (because a
     * command was specified), then free it.
     */
    if( num_cmd_args > 0)
    {
	char **s;

	for( s = argv; *s != NULL; s++) rxvt_free(*s);
	rxvt_free( argv );
    }

    /*
     * If run command failed, rollback
     */
    assert( -1 != LVTS(r)->cmd_fd );
    if (-1 == LVTS(r)->cmd_fd)
    {
	rxvt_dbgmsg ((DBG_WARN, DBG_TABBAR,
		    "\tThe command failed.\n"));
	rxvt_destroy_termwin (r, LTAB(r));
	return;
    }
    rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR,"page %d's cmd_fd is %d\n", LTAB(r), LVTS(r)->cmd_fd));


    /* adjust number of file descriptors to listen */
    rxvt_adjust_fd_number (r);

    /*
     * Initialize the screen data structures
     */
    rxvt_scr_reset (r, LTAB(r));
    rxvt_scr_refresh (r, LTAB(r), FAST_REFRESH);

    /*
     * Now we actually execute the command after executing shell, but we need
     * careful check first.
     */
    if( command != NULL && *command == '!' )
    {
	command++;  /* Skip leading '!' */
	rxvt_tt_write( r, LTAB(r), (const unsigned char*) command,
		STRLEN(command) );
	rxvt_tt_write( r, LTAB(r), (const unsigned char*) "\n", 1 );
    }

    /*
     * Now update active page information
     */
    PTAB(r) = ATAB(r); /* set last previous tab */
    ATAB(r) = LTAB(r); /* set the active tab */

    /* update mapped flag */
    AVTS(r)->mapped = 1;

    /* first tab is special since ptab = atab now */
    if (PTAB(r) != ATAB(r))
	PVTS(r, r->tabBar.ptab)->mapped = 0;

    /* Adjust visible tabs */
    rxvt_tabbar_set_visible_tabs (r, True); /* Send expose events to tabbar */
    refresh_tabbar_tab( r, PTAB(r));	    /* PTAB will need to be drawn as
					       inactive */

    /*
     * Auto show tabbar if we have exactly two tabs.
     */
    if(
	    !r->tabBar.state && LTAB(r) == 1
	    && ISSET_OPTION(r, Opt2_autohideTabbar)
	    && rxvt_tabbar_show( r )
      )
	rxvt_resize_on_subwin( r, SHOW_TABBAR);

    /* synchronize terminal title with tab title */
    if (ISSET_OPTION(r, Opt2_syncTabTitle))
	sync_tab_title( r, ATAB(r) );

    /* synchronize icon name to tab title */
    if (ISSET_OPTION(r, Opt2_syncTabIcon))
	rxvt_set_icon_name (r,
		(const unsigned char*) PVTS(r, ATAB(r))->tab_title);
}


/*
 * Called by the handler of SIGCHLD; destroy the terminal and its tab
 */
/* EXTPROTO */
void
rxvt_remove_page (rxvt_t* r, short page)
{
    register int    i;

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TABBAR,"rxvt_remove_page (r, %d)\n", page));

    /* clean utmp/wtmp entry */
#ifdef UTMP_SUPPORT
    rxvt_privileges (RESTORE);
    rxvt_cleanutent (r, page); 
    rxvt_privileges (IGNORE);
#endif

    /* free virtual terminal related resources */
    assert (PVTS(r, page)->ttydev);
    rxvt_free (PVTS(r, page)->ttydev);
    assert (PVTS(r, page)->cmd_fd >= 0);
    close (PVTS(r, page)->cmd_fd);

    if (PVTS(r, page)->inbuf_base)
    {
	rxvt_free (PVTS(r, page)->inbuf_base);
	PVTS(r, page)->inbuf_base = NULL;
    }

    /* free screen structure */
    rxvt_scr_release (r, page);

    /* destroy the virtual terminal window */
    rxvt_destroy_termwin (r, page);

    if (LTAB (r) == 0)
    {
	/* quit the last the terminal, exit the application */
	rxvt_free (r->vts);
	SET_NULL (r->vts);
	rxvt_clean_exit (r);
    }

    /* update total number of tabs */
    LTAB(r)--;
    rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR, "\tThe last tab is %d.", LTAB(r)));

    if (FVTAB(r) > page)
	FVTAB(r)--;
    if (LVTAB(r) > page)
	LVTAB(r)--;
    /* Reorganize the tabs array. */
    /* update TermWin and tab_widths */
    for (i = page; i <= LTAB(r); i++)
    {
	PVTS(r, i) = PVTS(r, i+1);
	PVTS(r, i)->vts_idx = i;
	refresh_tabbar_tab( r, i);
    }

    {
	term_t** temp_vts = rxvt_realloc (r->vts, (LTAB (r) + 1) * sizeof (term_t*));
	if (temp_vts)
	{
	    r->vts = temp_vts;
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR,
			"\tThe tab array has been reallocated to (%d * sizeof (term_t*)).\n", LTAB(r) + 1));
	}
	else
	    rxvt_dbgmsg ((DBG_WARN, DBG_TABBAR,
			"\tAfter removing a tab, the tab array could not be reallocated. If you see often this message, this can be a problem.\n"));
	// if the realloc failed, this is not fatale as we can imagine it may be reallocated
	// at the next change on tabs but display all the same a warning.
    }

    /* update selection */
    if (page == r->selection.vt)
	rxvt_process_selectionclear (r, page);
    else if (r->selection.vt > page)
	r->selection.vt --;

    /*
     * Now we try to set correct atab, ptab, fvtab, and lvtab
     * Must be careful here!!!
     */
    /* update previous active tab */
    if (PTAB(r) > page)
	PTAB(r)--;
    /* in case PTAB is invalid */
    if (PTAB(r) > LTAB(r))
	PTAB(r) = LTAB(r);

    /* update active tab */
    if( ATAB(r) == page )
    {
	/* Fall back to previous active */
	ATAB(r) = PTAB(r);

	/* Make the previous active tab the previous / next tab if possible. */
	if( PTAB(r) > 0 )
	    PTAB(r)--;
	else if (PTAB(r) < LTAB(r) )
	    PTAB(r)++;
    }
    else if( ATAB(r) > page)
	ATAB(r)--;

    /* always set mapped flag */
    AVTS(r)->mapped = 1;

    /* Adjust the number of FD's we select() for.  */
    rxvt_adjust_fd_number(r);

    /* adjust visible tabs */
    rxvt_tabbar_set_visible_tabs (r, True);
    refresh_tabbar_tab( r, ATAB(r));	/* Active tab has changed */

    /* redraw the tabs and buttons */
    if (r->tabBar.state)
    {
	if( LTAB(r) == 0 && ISSET_OPTION(r, Opt2_autohideTabbar) 
		&& rxvt_tabbar_hide( r ))
	    /*
	     * Only one tab left. Auto hide tabbar.
	     */
	    rxvt_resize_on_subwin (r, HIDE_TABBAR);
    }

    /* Switch fg/bg colors */
    rxvt_set_vt_colors( r, ATAB(r) );
    XMapRaised( r->Xdisplay, AVTS(r)->vt );

    /*
     * We don't need to touch the screen here. XMapRaised will generate a
     * MapNotify and Expose events, which will refresh the screen as needed.
     * Touching the screen unnecessarily causes a flicker (and is *horrible*
     * under slow connections).
     */
    /* rxvt_scr_touch (r, ATAB(r), True); */

    /* synchronize terminal title with tab title */
    if (ISSET_OPTION(r, Opt2_syncTabTitle))
	sync_tab_title( r, ATAB(r) );

    /* synchronize icon name to tab title */
    if (ISSET_OPTION(r, Opt2_syncTabIcon))
	rxvt_set_icon_name(r,
		(const unsigned char*) PVTS(r, ATAB(r))->tab_title);
}


/*
 * Set new title for a tab
 */
/* EXTPROTO */
void
rxvt_tabbar_set_title (rxvt_t* r, short page, const unsigned char TAINTED * str)
{
    char UNTAINTED *	    n_title;

    assert (str);
    assert (page >= 0 && page <= LTAB(r));
    assert (PVTS(r, page)->tab_title);

    n_title = STRNDUP (str, MAX_TAB_TXT);
    /*
     * If strdup succeeds, set new title
     */
    if (NULL != n_title)
    {
	rxvt_free (PVTS(r, page)->tab_title);
	PVTS(r, page)->tab_title = n_title;

	/* Compute the new width of the tab */
	PVTS(r, page)->tab_width = rxvt_tab_width (r, n_title);
    }

    /*
     * If visible tab's title is changed, refresh tab bar
     */
    if (page >= FVTAB(r) && page <= LVTAB(r))
    {
	/* adjust visible tabs */
	rxvt_tabbar_set_visible_tabs (r, True);
	refresh_tabbar_tab(r, page);
    }

    /* synchronize terminal title with active tab title */
    if (ISSET_OPTION(r, Opt2_syncTabTitle) &&
	(page == ATAB(r)))
	sync_tab_title( r, ATAB(r) );

    /* synchronize icon name to tab title */
    if (ISSET_OPTION(r, Opt2_syncTabIcon) &&
	(page == ATAB(r)))
	rxvt_set_icon_name(r,
		(const unsigned char*) PVTS(r, ATAB(r))->tab_title);
}


/*
 * Activate a page terminal
 */
/* EXTPROTO */
void
rxvt_activate_page (rxvt_t* r, short index)
{
    /* shortcut */
    if (/* !r->tabBar.state ||
	NOT_WIN(r->tabBar.win) || */
	index == ATAB(r))
	return;

    AVTS(r)->mapped = 0;
    r->tabBar.ptab = ATAB(r);
    ATAB(r) = index;
    AVTS(r)->mapped = 1;
    AVTS(r)->highlight = 0; /* clear highlight flag */
    
    /*
     * Now the visible tabs may be changed, recompute the visible
     * tabs before redrawing.
     */
    if (index < FVTAB(r) || index > LVTAB(r))
    {
	/* adjust visible tabs */
	rxvt_tabbar_set_visible_tabs (r, True);
    }
    refresh_tabbar_tab( r, ATAB(r));
    refresh_tabbar_tab( r, PTAB(r));

    /* Switch VT fg/bg colors */
    rxvt_set_vt_colors( r, ATAB(r) );
    XMapRaised( r->Xdisplay, AVTS(r)->vt );
    /*
     * We don't need to touch the screen here. XMapRaised will generate a
     * MapNotify and Expose events, which will refresh the screen as needed.
     * Touching the screen unnecessarily causes a flicker (and is *horrible*
     * under slow connections).
     */
    /* rxvt_scr_touch (r, ATAB(r), True); */
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TABBAR, "active page is %d\n",ATAB(r)));

    /* synchronize terminal title with tab title */
    if (ISSET_OPTION(r, Opt2_syncTabTitle))
	sync_tab_title( r, ATAB(r) );

    /* synchronize icon name to tab title */
    if (ISSET_OPTION(r, Opt2_syncTabIcon))
	rxvt_set_icon_name (r,
		(const unsigned char*) PVTS(r, ATAB(r))->tab_title);
}


/*
 * Change the width of the tab bar
 */
/* EXTPROTO */
void
rxvt_tabbar_resize (rxvt_t* r)
{
    register int    i;
    int		    sx, sy;


    sx = 0;
    sy = 0;
#ifdef HAVE_MENUBAR
    sy += rxvt_menubar_height (r);
#endif
    if (ISSET_OPTION(r, Opt2_bottomTabbar))
	sy += VT_HEIGHT(r);
    XMoveResizeWindow  (r->Xdisplay, r->tabBar.win,
	sx, sy, TWIN_WIDTH(r), rxvt_tabbar_rheight (r));

    /* recompute width of each tab */
    for (i = 0; i <= LTAB(r); i ++)
	PVTS(r, i)->tab_width = rxvt_tab_width (r, PVTS(r, i)->tab_title);

    /* adjust visible tabs */
    rxvt_tabbar_set_visible_tabs (r, False);
    /* redraw the tabs and buttons */
    XClearArea( r->Xdisplay, r->tabBar.win,
	    0, 0, 0, 0, True);
}


/*
 * Determine the position of pointer click and dispatch the event
 */
/* EXTPROTO */
void
rxvt_tabbar_dispatcher (rxvt_t* r, XButtonEvent* ev)
{
    register int    x, y, z, but;

    x = ev->x;
    y = ev->y;
    but = -1;
    rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR, "click in (%d,%d)\n", x, y));

    /* Button4/Button5 of wheel mouse activate the left/right tab as
       Mozilla firefox */
    switch ( ev->button )
    {
#ifdef HAVE_MENUBAR
	case Button3:
	    if( r->h->popupMenu[0] )
	    {
		int	x, y;
		Window	unused_cr;

		r->h->showingMenu |= POPUP_MENU;

		XTranslateCoordinates( r->Xdisplay, ev->window,
			r->TermWin.parent, ev->x, ev->y, &x, &y, &unused_cr);

		r->h->ActiveMenu = r->h->popupMenu[0];

		r->h->ActiveMenu->x = x;
		r->h->ActiveMenu->y = y;

		XDefineCursor(r->Xdisplay, AVTS(r)->vt, r->h->bar_pointer);
		rxvt_menu_show(r);
		return;
	    }
	    break;
#endif
	case Button4: /* activate left tab */
	    if (0 != ATAB(r))
		rxvt_activate_page (r, ATAB(r)-1);
	    else if (0 != LTAB(r))
		rxvt_activate_page (r, LTAB(r));
	    return;

	case Button5: /* activate right tab */
	    if( ATAB(r) != LTAB(r) )
		rxvt_activate_page( r, ATAB(r) + 1 );
	    else if( 0 != LTAB(r) )
		rxvt_activate_page( r, 0 );
	    return;

	default:
	    break;
    }


    /* let's decode where the user click */
    z = TWIN_WIDTH(r) - x;
    if (
	    NOTSET_OPTION(r, Opt2_hideButtons)
	    && z < 4*(BTN_WIDTH+BTN_SPACE)
	    && (z%(BTN_WIDTH+BTN_SPACE)) > BTN_SPACE
       )
    {
	but = z/(BTN_WIDTH+BTN_SPACE);

	/* we should only handle left-mouse-button clicks */
	if ( ev->button != Button1 )
	{
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TABBAR,"skip non-left-mouse-button click\n"));
	    return;
	}

	rxvt_dbgmsg ((DBG_VERBOSE, DBG_TABBAR,"click on button %d\n",but));
	switch(but)
	{
	    case 0 : /* right shift */
		if (r->tabBar.atab < LTAB(r))
		    rxvt_activate_page (r, r->tabBar.atab+1);
		break;

	    case 1 : /* left shift */
		if (r->tabBar.atab > 0)
		    rxvt_activate_page (r, r->tabBar.atab-1);
		break;

	    case 2 : /* delete the active vt if it's in primary screen */
		if(
		    NOTSET_OPTION(r, Opt2_protectSecondary) ||
		    (
		       ISSET_OPTION(r, Opt2_protectSecondary) &&
		       PRIMARY == AVTS(r)->current_screen
		    )
		  )
		{
		    /*
		     * 2006-09-19 gi1242: If user presses the X button, don't
		     * hold the tab open, regardless of exit status.
		     */
		    AVTS(r)->holdOption &= ~(HOLD_NORMALBIT|HOLD_STATUSBIT);
		    rxvt_kill_page (r, ATAB(r));
		}
		break;

	    case 3 : /* create a new vt*/
		rxvt_append_page (r, 0, NULL, NULL);
		break;

	    default :
		break;
	}
    }
    else if ( x < TAB_SPACE && LTAB(r) >= 0)
    {
	register int	w = 0;
	register int	i;
	for ( i = FVTAB(r); w < x && i <= LVTAB(r); i++)
	    w += TAB_WIDTH(i);

	if( w - TAB_BORDER >= x )
	{
	    but = i - 1;

	    rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR,"click on tab %d\n", but));
	    switch( ev->button )
	    {
		case Button1:
		    /* activate the selected tab */
		    rxvt_activate_page (r, but);
		    r->tabClicked = but;
		    break;

		case Button2:
		    /* change tab title on middle click */
		    if (NULL != r->selection.text)
			rxvt_tabbar_set_title (r, but, r->selection.text);
		    break;
	    }
	}
	else
	{
	    /* change tab title of active tab on middle click */
	    if ((Button2 == ev->button) && (NULL != r->selection.text))
		rxvt_tabbar_set_title (r, ATAB(r), r->selection.text);
	}
    }
}


/*
 * Check if we're dragging a tab. If yes, then move the tab.
 *
 * TODO: Set a different cursor when dragging a tab.
 */
/* EXTPROTO */
void
rxvt_tabbar_button_release( rxvt_t *r, XButtonEvent *ev)
{
    int	    w, droppedTab;

    do	/* while( 0 ) */
    {
	if (
		ev->button != Button1		    /* Ignore everything except
						       left clicks */
		|| r->tabClicked == -1		    /* If we're not dragging a
						       tab then nothing to do */
		|| ev->y < 0
		|| ev->y > rxvt_tabbar_rheight( r ) /* If we drag off the
						       tabbar. (Coordinates in
						       ev are relative to the
						       tabbar window) */
	   )
	    break;

	/* Figure out where the user released the mouse */
	for (
		droppedTab = FVTAB(r), w=0;
		w < ev->x && droppedTab <= LVTAB(r);
		droppedTab++
	    )
	    w += TAB_WIDTH( droppedTab );

	rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR, "Dragged tab %d to %d (%d, %d)\n", r->tabClicked, droppedTab - 1, ev->x, ev->y));

	/* Move active tab there */
	rxvt_tabbar_move_tab( r, droppedTab - 1 );
    } while( 0 );

    r->tabClicked = -1;
}


/*
 * Is the tabbar visible
 */
/* EXTPROTO */
int
rxvt_tabbar_visible (rxvt_t* r)
{
    return (IS_WIN(r->tabBar.win) && r->tabBar.state);
}


/*
 * Expose handler for tabbar
 */
/* EXTPROTO */
void
rxvt_tabbar_expose (rxvt_t* r, XEvent *ev)
{
	Region region;
	UNSET_REGION(region);
	rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR,  "rxvt_tabbar_expose (r, ev)\n"));

	if( ev && ev->type == Expose)
	{
		region = XCreateRegion();

		do
		{
			XRectangle rect;
			Region region_temp = XCreateRegion ();

			rect.x	= ev->xexpose.x;
			rect.y	= ev->xexpose.y;
			rect.width	= ev->xexpose.width;
			rect.height	= ev->xexpose.height;

			/* Not sure this is necessary,
			 * but I had a segmentation fault issue without this. */
			XUnionRectWithRegion( &rect, region_temp, region_temp);
			XUnionRegion(region_temp, region, region);
			XDestroyRegion( region_temp );
		}
		while( XCheckTypedWindowEvent( r->Xdisplay, r->tabBar.win,
					Expose, ev));
	}
	else XClearWindow (r->Xdisplay, r->tabBar.win);

	/* draw the tabs and blank space*/
	rxvt_draw_tabs(r, region);

	/* draw the buttons */
	rxvt_tabbar_draw_buttons (r);

	if (IS_REGION(region))
		XDestroyRegion( region );
}


/*
 * Hide the tabbar
 */
/* EXTPROTO */
int
rxvt_tabbar_hide (rxvt_t* r)
{
    int	    changed = 0;

    assert (IS_WIN(r->tabBar.win));
    changed = r->tabBar.state;
    XUnmapWindow  (r->Xdisplay, r->tabBar.win);
    r->tabBar.state = 0;

    return (changed);
}


/*
 * Show the tabbar
 */
/* EXTPROTO */
int
rxvt_tabbar_show (rxvt_t* r)
{
    int	    changed = 0;

    assert (IS_WIN(r->tabBar.win));
    changed = !r->tabBar.state;
    XMapWindow  (r->Xdisplay, r->tabBar.win);
    r->tabBar.state = 1;

    return (changed);
}


/*
 * Create the tab bar window
 */
/* EXTPROTO */
void
rxvt_tabbar_create (rxvt_t* r)
{
    XColor	    color, bgcolor;
    XGCValues	    gcvalue;
    unsigned long   gcmask;
    register int    i;
    int		    sx, sy;
#ifdef HAVE_LIBXPM
    XpmAttributes   xpm_attr;
    /*
     * Make sure symbol `background' exists in all .xpm files! This elimate the
     * background color so that the buttons look transparent.
     */
    XpmColorSymbol  xpm_color_sym = {"background", NULL, 0};
#endif


    init_tabbar (r);
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TABBAR,"Creating tabbar\n"));


    /* initialize the colors */
    if (XDEPTH <= 2)
    {
	r->tabBar.fg = r->pixColorsFocus[Color_fg];
	r->tabBar.bg = r->pixColorsFocus[Color_bg];
	r->tabBar.ifg = r->pixColorsFocus[Color_fg];
	r->tabBar.ibg = r->pixColorsFocus[Color_bg];
	r->tabBar.frame = r->pixColorsFocus[Color_bg];
	r->tabBar.delimit = r->pixColorsFocus[Color_fg];
    }
    else 
    {
	/* create the foreground color */
	if(
	     r->h->rs[Rs_tabfg] && 
	     rxvt_parse_alloc_color (r, &color, r->h->rs[Rs_tabfg])
	  )
	{
	    r->tabBar.fg = color.pixel;
#ifdef XFT_SUPPORT
	    if( ISSET_OPTION( r, Opt_xft ) )
		rxvt_alloc_xft_color( r, &color, &r->tabBar.xftfg );
#endif
	}

	else
	{
	    r->tabBar.fg = r->pixColorsFocus[Color_Black];
#ifdef XFT_SUPPORT
	    if( ISSET_OPTION( r, Opt_xft ) )
		r->tabBar.xftfg = r->xftColorsFocus[Color_Black];
#endif
	}

	/*
	 * create the background color
	 */
	if(
	     r->h->rs[Rs_tabbg]	&&
	     rxvt_parse_alloc_color (r, &color, r->h->rs[Rs_tabbg])
	  )
	{
	    r->tabBar.bg = color.pixel;
	}

	else
	{
	    color.red	= 0xd300;
	    color.green = 0xd300;
	    color.blue	= 0xdd00;
	    if( rxvt_alloc_color (r, &color, "Active_Tab") )
		r->tabBar.bg = color.pixel;
	    else
		r->tabBar.bg = VTBG(r,0);
	}

	/* create the tab frame color */
	r->tabBar.frame = r->pixColorsFocus[Color_fg];

	/* create the inactive tab foreground color */
	if(
	     r->h->rs[Rs_itabfg]
	     && rxvt_parse_alloc_color (r, &color, r->h->rs[Rs_itabfg])
	  )
	{
	    r->tabBar.ifg = color.pixel;
#ifdef XFT_SUPPORT
	    if( ISSET_OPTION( r, Opt_xft ) )
		rxvt_alloc_xft_color( r, &color, &r->tabBar.xftifg );
#endif
	}
	else
	{
	    r->tabBar.ifg = r->pixColorsFocus[Color_Black];
#ifdef XFT_SUPPORT
	    if( ISSET_OPTION( r, Opt_xft ) )
		r->tabBar.xftifg = r->xftColorsFocus[Color_Black];
#endif
	}

	/* create the inactive tab background color */
	if(
	     r->h->rs[Rs_itabbg]
	     && rxvt_parse_alloc_color( r, &color, r->h->rs[Rs_itabbg] )
	  )
	    r->tabBar.ibg = color.pixel;

	else
	{
	    color.red	= 0xa100;
	    color.green = 0xa100;
	    color.blue	= 0xac00;
	    if( rxvt_alloc_color( r, &color, "Inactive_Tab_Bg" ) )
		r->tabBar.ibg = color.pixel;
	    else
		r->tabBar.ibg = VTBG(r,0);
	}

	/* create the delimit color (average of 3*fg & bg) */
	color.pixel	= r->pixColorsFocus[Color_fg];
	XQueryColor( r->Xdisplay, XCMAP, &color );

	bgcolor.pixel	= r->pixColorsFocus[Color_bg];
	XQueryColor( r->Xdisplay, XCMAP, &bgcolor );

	color.red   = ( bgcolor.red	+ 3 * color.red	    ) / 4;
	color.green = ( bgcolor.green	+ 3 * color.green   ) / 4;
	color.blue  = ( bgcolor.blue	+ 3 * color.blue    ) / 4;

	if( rxvt_alloc_color( r, &color, "Tab_Delimit" ) )
	    r->tabBar.delimit = color.pixel;
	else
	    r->tabBar.delimit = VTFG(r,0);

	rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR, "Delimit color: %hx, %hx, %hx (#%lx)\n", color.red, color.green, color.blue, r->tabBar.delimit));
    }


    sx = 0;
    sy = 0;
#ifdef HAVE_MENUBAR
    sy += rxvt_menubar_height (r);
#endif
    if (ISSET_OPTION(r, Opt2_bottomTabbar))
	sy += VT_HEIGHT(r);
    /*
     * create the window of the tabbar. Use ifg and ibg for the background of
     * the tabBar so that the active tab stands out better.
     */
    r->tabBar.win = XCreateSimpleWindow( r->Xdisplay, r->TermWin.parent,
			sx, sy, TWIN_WIDTH(r), rxvt_tabbar_rheight( r ),
			0, r->tabBar.ifg, r->tabBar.ibg );
    assert(IS_WIN(r->tabBar.win));

#ifdef XFT_SUPPORT
    if (ISSET_OPTION(r, Opt_xft))
    {
	r->tabBar.xftwin = XftDrawCreate (r->Xdisplay, r->tabBar.win,
				XVISUAL, XCMAP);
    }
#endif

#ifdef DEBUG_X
    rxvt_set_win_title (r, r->tabBar.win, "tabbar");
#endif


#ifdef BACKGROUND_IMAGE
    r->tabBar.hasPixmap = False;    /* initialize it to None */
    if (
#ifdef TRANSPARENT
	    /* Transparency overrides background */
	    !(
		ISSET_OPTION(r, Opt_transparent)
		&& ISSET_OPTION(r, Opt_transparent_tabbar)
	     )
	    &&
#endif
	    r->h->rs[Rs_tabbarPixmap]
       )
    {
	long	w = 0, h = 0;
	Pixmap	pmap;

	pmap = rxvt_load_pixmap (r, r->h->rs[Rs_tabbarPixmap], &w, &h);
	if (IS_PIXMAP(pmap))
	{
	    XSetWindowBackgroundPixmap (r->Xdisplay, r->tabBar.win, pmap);
	    XFreePixmap( r->Xdisplay, pmap);

	    r->tabBar.hasPixmap = True;
	}
	else r->tabBar.hasPixmap = False;
    }
#endif

#ifdef TRANSPARENT
    if (
	    ISSET_OPTION(r, Opt_transparent)
	    && ISSET_OPTION(r, Opt_transparent_tabbar)
       )
	XSetWindowBackgroundPixmap( r->Xdisplay, r->tabBar.win, ParentRelative);
#endif


    /* create the GC for the tab window */
    gcvalue.foreground	= r->tabBar.fg;
    gcvalue.line_width	= 0;
    gcvalue.line_style	= LineSolid;
    gcvalue.cap_style	= CapButt;
    gcvalue.join_style	= JoinMiter;
    gcvalue.arc_mode	= ArcChord;	/* For coloring ATAB */
    gcvalue.fill_style	= FillSolid;	/* Probably default ... */

    gcmask = GCForeground | GCLineWidth
		| GCLineStyle | GCCapStyle | GCJoinStyle
		| GCArcMode | GCFillStyle;

#ifdef TRANSPARENT
    /* set background color when there's no transparent */
    if (!(( r->h->am_transparent || r->h->am_pixmap_trans) &&
	ISSET_OPTION(r, Opt_transparent_tabbar)))
#endif
#ifdef BACKGROUND_IMAGE
	/* set background color when there's no bg image */
	if ( ! r->tabBar.hasPixmap )
#endif
	{
	    gcvalue.background = r->tabBar.bg;
	    gcmask |= GCBackground;
	}

    r->tabBar.gc = XCreateGC (r->Xdisplay, r->tabBar.win,
	gcmask, &gcvalue);
    assert (IS_GC(r->tabBar.gc));


    XDefineCursor (r->Xdisplay, r->tabBar.win, r->h->bar_pointer);
    XSelectInput (r->Xdisplay, r->tabBar.win,
	    ExposureMask | ButtonPressMask | ButtonReleaseMask
#ifdef HAVE_MENUBAR
		| Button3MotionMask
#endif
	);

#ifdef XFT_SUPPORT
    if (NOTSET_OPTION(r, Opt_xft))
#endif
    XSetFont (r->Xdisplay, r->tabBar.gc, r->TermWin.font->fid);


#ifdef HAVE_LIBXPM
    xpm_color_sym.pixel = r->tabBar.bg;
    xpm_attr.colorsymbols = &xpm_color_sym;
    xpm_attr.numsymbols = 1;
    xpm_attr.visual = XVISUAL;
    xpm_attr.colormap = XCMAP;
    xpm_attr.depth = XDEPTH;
    xpm_attr.closeness = 65535;
    xpm_attr.valuemask = XpmVisual | XpmColormap | XpmDepth |
	XpmCloseness | XpmReturnPixels | XpmColorSymbols;
#endif

    /* now, create the buttons */
    for (i = 0; i < NB_XPM; i++)
    {
#ifdef HAVE_LIBXPM
	XpmCreatePixmapFromData (r->Xdisplay, r->tabBar.win,
	    xpm_name[i], &img_e[i], &img_emask[i], &xpm_attr);
	assert (IS_PIXMAP(img_e[i]));
	XpmCreatePixmapFromData (r->Xdisplay, r->tabBar.win,
	    xpm_d_name[i], &img_d[i], &img_dmask[i], &xpm_attr);
	assert (IS_PIXMAP(img_d[i]));
#else
	img[i] = XCreatePixmapFromBitmapData (r->Xdisplay,
	    r->tabBar.win, (char *) xbm_name[i], BTN_WIDTH, BTN_HEIGHT,
	    r->tabBar.fg, r->tabBar.bg, XDEPTH);
	assert (IS_PIXMAP(img[i]));
#endif
    }

    rxvt_dbgmsg ((DBG_DEBUG, DBG_TABBAR, "TXT_XOFF=%d, TXT_YOFF=%d, ATAB_EXTRA=%d, TAB_RADIUS=%d\n", TXT_XOFF, TXT_YOFF, ATAB_EXTRA, TAB_RADIUS));
}


/*
 * Create the tab bar window
 */
/* EXTPROTO */
void
rxvt_tabbar_clean_exit (rxvt_t* r)
{
    register int    i;


    UNSET_WIN(r->tabBar.win);	/* destroyed by XDestroySubwindows */

    /* free resource strings */
    if (r->tabBar.rsfg)
	rxvt_free ((void*) r->h->rs[Rs_tabfg]);
    if (r->tabBar.rsbg)
	rxvt_free ((void*) r->h->rs[Rs_tabbg]);
    if (r->tabBar.rsifg)
	rxvt_free ((void*) r->h->rs[Rs_itabfg]);
    if (r->tabBar.rsibg)
	rxvt_free ((void*) r->h->rs[Rs_itabbg]);

    if (IS_GC(r->tabBar.gc))
    {
	XFreeGC (r->Xdisplay, r->tabBar.gc);
	UNSET_GC(r->tabBar.gc);
    }

    for (i = 0; i < NB_XPM; i ++)
    {
#ifdef HAVE_LIBXPM
	if (IS_PIXMAP(img_e[i]))
	{
	    XFreePixmap (r->Xdisplay, img_e[i]);
	    UNSET_PIXMAP(img_e[i]);
	}
	if (IS_PIXMAP(img_emask[i]))
	{
	    XFreePixmap (r->Xdisplay, img_emask[i]);
	    UNSET_PIXMAP(img_emask[i]);
	}
	if (IS_PIXMAP(img_d[i]))
	{
	    XFreePixmap (r->Xdisplay, img_d[i]);
	    UNSET_PIXMAP(img_d[i]);
	}
	if (IS_PIXMAP(img_dmask[i]))
	{
	    XFreePixmap (r->Xdisplay, img_dmask[i]);
	    UNSET_PIXMAP(img_dmask[i]);
	}
#else
	if (IS_PIXMAP(img[i]))
	    XFreePixmap (r->Xdisplay, img[i]);
#endif
	UNSET_PIXMAP(img[i]);
    }	/* for */
}


/* EXTPROTO */
unsigned short
rxvt_tabbar_height (rxvt_t* r)
{
    if (NOT_WIN(r->tabBar.win) || !r->tabBar.state)
	return 0;
    return (rxvt_tabbar_rheight(r));
}


/* EXTPROTO */
unsigned short
rxvt_tabbar_rheight (rxvt_t* r)
{
    return (r->TermWin.FHEIGHT + 2*TXT_MARGIN + 2*TAB_BORDER + ATAB_EXTRA);
}


/* EXTPROTO */
unsigned int
rxvt_tab_width (rxvt_t *r, const char *str)
{
#ifdef XFT_SUPPORT
    if ( ISSET_OPTION (r, Opt_xft) && r->TermWin.xftpfont)
    {
	/*
	 * With a proportionally spaced font defined, let's try and make the
	 * tabs look like firefox. All tabs have the same width. The more tabs
	 * there are, the narrower the width becomes. The width does not depend
	 * on the tab title.
	 */
	if( LTAB(r) >= 0 )
	{
	    int twidth = (TAB_SPACE - TAB_BORDER)
			    / min( LTAB(r) + 1, r->TermWin.minVisibleTabs )
			    - TAB_BORDER;
	    return min( twidth, MAX_TAB_PIXEL_WIDTH);
	}
	else return MAX_TAB_PIXEL_WIDTH;
    }
    else
#endif
    {
	int	    len;
	uint16_t    maxw = r->TermWin.maxTabWidth;

	assert (str);
	len = STRLEN (str);
	if (len > maxw)
	    len = maxw;
#ifdef XFT_SUPPORT
	if (ISSET_OPTION (r, Opt_xft) && (NULL != r->tabBar.xftwin))
	{
	    return (2 * TXT_XOFF + Width2Pixel(len));
	}
	else
#endif	/* XFT_SUPPORT */
	return (2 * TXT_XOFF + XTextWidth (r->TermWin.font, str, len));
    }
}


/* EXTPROTO */
int
rxvt_is_tabbar_win (rxvt_t* r, Window w)
{
    return (w == r->tabBar.win);
}


/* EXTPROTO */
void
rxvt_tabbar_change_color (rxvt_t* r, int item, const char* str)
{
    XColor	xcol;
    int		changed = 0;


    switch (item)
    {
	case MRxvt_tabfg:
	    if (r->h->rs[Rs_tabfg] &&
		!STRCASECMP(str, r->h->rs[Rs_tabfg]))
		break;	/* no color change */

	    if (rxvt_parse_alloc_color (r, &xcol, str))
	    {
		r->tabBar.fg = xcol.pixel;
#ifdef XFT_SUPPORT
		rxvt_alloc_xft_color( r, &xcol, &(r->tabBar.xftfg) );
#endif
		if (r->tabBar.rsfg)	/* free previous string */
		    rxvt_free ((void*) r->h->rs[Rs_tabfg]);
		r->h->rs[Rs_tabfg] = STRDUP(str);
		r->tabBar.rsfg = 1;	/* free resource string later */
		changed = 1;
	    }
	    break;

	case MRxvt_tabbg:
	    if (r->h->rs[Rs_tabbg] &&
		!STRCASECMP(str, r->h->rs[Rs_tabbg]))
		break;	/* no color change */

	    if (rxvt_parse_alloc_color (r, &xcol, str))
	    {
		r->tabBar.bg = xcol.pixel;
		if (r->tabBar.rsbg)	/* free previous string */
		    rxvt_free ((void*) r->h->rs[Rs_tabbg]);
		r->h->rs[Rs_tabbg] = STRDUP(str);
		r->tabBar.rsbg = 1;	/* free resource string later */
		changed = 1;
	    }
	    break;

	case MRxvt_itabfg:
	    if (r->h->rs[Rs_itabfg] &&
		!STRCASECMP(str, r->h->rs[Rs_itabfg]))
		break;	/* no color change */

	    if (rxvt_parse_alloc_color (r, &xcol, str))
	    {
		r->tabBar.ifg = xcol.pixel;
#ifdef XFT_SUPPORT
		rxvt_alloc_xft_color( r, &xcol, &(r->tabBar.xftifg) );
#endif
		if (r->tabBar.rsifg)	/* free previous string */
		    rxvt_free ((void*) r->h->rs[Rs_itabfg]);
		r->h->rs[Rs_itabfg] = STRDUP(str);
		r->tabBar.rsifg = 1;	/* free resource string later */
		changed = 1;
	    }
	    break;

	case MRxvt_itabbg:
	    if (r->h->rs[Rs_itabbg] && !STRCASECMP(str, r->h->rs[Rs_itabbg]))
		break;

	    if (rxvt_parse_alloc_color (r, &xcol, str))
	    {
		r->tabBar.ibg = xcol.pixel;
		if (r->tabBar.rsibg)	/* free previous string */
		    rxvt_free ((void*) r->h->rs[Rs_itabbg]);
		r->h->rs[Rs_itabbg] = STRDUP(str);
		r->tabBar.rsibg = 1;	/* free resource string later */
		changed = 1;
	    }
	    break;
	
	default:
	    break;
    }

    if (changed)
    {
	if (MRxvt_itabbg == item)
	{
#if defined(TRANSPARENT) || defined(BACKGROUND_IMAGE)
	    if (
# ifdef TRANSPARENT
		    (
		     (r->h->am_transparent || r->h->am_pixmap_trans)
		     && ISSET_OPTION (r, Opt_transparent_tabbar)
		    )
# endif
# if defined(TRANSPARENT) && defined(BACKGROUND_IMAGE)
		||
# endif
# ifdef BACKGROUND_IMAGE
		( r->tabBar.hasPixmap )
# endif
	       )
	    {
# ifdef HAVE_LIBXRENDER
		/* Background image needs to be regrabed */
		rxvt_refresh_bg_image(r, ATAB(r), False);
# endif
	    }
	    else
#endif
	    {
		XSetWindowBackground (r->Xdisplay, r->tabBar.win,
			r->tabBar.ibg);
	    }
	}

	/*
	 * Better to put the expose event on the queue, than expose immediately.
	 * Expose events can be expensive when using XRender transparency.
	 */
	XClearArea( r->Xdisplay, r->tabBar.win, 0, 0, 0, 0, True);
    }
}

/*
 * Move active tab to position newPage.
 */
/* EXTPROTO */
void
rxvt_tabbar_move_tab (rxvt_t* r, short newPage)
{
    short   curPage	= ATAB(r);
    short   i;

    
    if (
	    0 == LTAB(r) ||			/* Only one tab (no move
						   possible) */
	    newPage == curPage ||		/* Move to itself */
	    newPage < 0 || newPage > LTAB(r)	/* Out of range */
       )
	return;

    if( newPage < curPage )
    {
	term_t* temp_vt = r->vts[curPage];

	/* Shift pages newPage .. curPage-1 one to the right. */
	for( i = curPage; i > newPage; i--)
	    r->vts[i] = r->vts[i-1];

	r->vts[newPage] = temp_vt;

	/* Update selection */
	if( r->selection.vt >= newPage && r->selection.vt < curPage )
	    r->selection.vt++;
	else if( r->selection.vt == curPage )
	    r->selection.vt = newPage;
    }
    else
    {
	term_t* temp_vt = r->vts[curPage];

	/* Shift pages curPage+1 .. newPage one to the left. */
	for( i = curPage; i < newPage; i++)
	    r->vts[i] = r->vts[i+1];

	r->vts[newPage] = temp_vt;

	/* Update selection */
	if( r->selection.vt > curPage && r->selection.vt <= newPage)
	    r->selection.vt--;
	else if( r->selection.vt == curPage )
	    r->selection.vt = newPage;
    }

    /* adjust active tab */
    ATAB(r) = newPage;
    /* adjust previous active tab */
    if (PTAB(r) == newPage) PTAB(r) = curPage;

    /* refresh tabbar */
    if (newPage < FVTAB(r) || newPage > LVTAB(r))
	rxvt_tabbar_set_visible_tabs (r, True);
    else
    {
	/*
	 * If the width of newPage is different from that of curPage, then all
	 * tabs in between newPage and curPage will have to be refreshed.
	 */
	for( i = min( newPage, curPage ); i <= max( newPage, curPage ); i++ )
	    refresh_tabbar_tab( r, i);
    }

    if( ISSET_OPTION( r, Opt2_syncTabTitle ) )
	sync_tab_title( r, ATAB(r) );
}

/*
 * Synchronize the window title to the title of the tab "page".
 */
void
sync_tab_title( rxvt_t *r, int page )
{
    char wintitle[MAX_TAB_TXT];

    if(
	 IS_NULL( PVTS(r,page)->winTitleFormat )	||
	 rxvt_percent_interpolate( r, page,
	     PVTS(r,page)->winTitleFormat, STRLEN(PVTS(r,page)->winTitleFormat),
	     wintitle, MAX_TAB_TXT ) <= 1
      )
    {
	/* % interpolation failed / not possible */
	rxvt_set_term_title( r, (unsigned char*) PVTS(r, page)->tab_title );
    }
    else
	rxvt_set_term_title( r, (unsigned char*) wintitle );
}

/*----------------------- end-of-file (C source) -----------------------*/
