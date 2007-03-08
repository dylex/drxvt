/*-----------------------------------*-C-*-------------------------------------*
 * File		: xftacs.c
 * Created	: Tue 27 Dec 2005 09:59:55 PM CST
 * Modified	: Wed 12 Apr 2006 01:33:43 AM CDT
 * Author	: Gautam Iyer <gi1242@users.sourceforge.net>
 *-----------------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 *
 * 	Copyright 2005-2006   Gautam Iyer <gi1242@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 *-----------------------------------------------------------------------------*
 *
 * DESCRIPTION
 *
 * 	Functions to draw VT100 / ACS graphics charecters. Xft functions do not
 * 	draw text graphic charecters (like XDrawText), so we have to do it
 * 	ourself. Partly plagurized with thanks from Thomas Dicky's xterm-204:
 *
 *		http://invisible-island.net/xterm/xterm.html
 *
 *	This file has nothing specific to rxvt. Instead of using XftDrawString,
 *	use xftDrawACSString. Draws the non-graphics chars using a regular Xft
 *	function, and graphics chars using XftGlyphs (if available) or line
 *	segments.
 *
 *	Before your first call to xftDrawACSString, call xftInitACS(). After
 *	your last call to xftDrawACSString, call xftCloseACS().
 *
 *	See the comments before each function definition for details.
 *
 * BUGS
 *
 *	1. Complex chars are drawn using a glyph from the Xft font. Not all
 *	   fonts have these glyphs. Some have them in a different location than
 *	   we expect. 
 *
 *	   The fonts "Andale Mono", "Bitstream Vera Sans Mono" and "Courier New"
 *	   HAVE the required glyphs at the positions below. The fonts "Courier",
 *	   "Luxi Mono" and "Nimbus Mono L" DO NOT have the required glyphs.
 *
 *----------------------------------------------------------------------------*/

#include "../config.h"
#include "rxvt.h"

#include <xftacs.h>


/*
 * Maximum number of glyphs to be drawn per call to XftDrawGlyphs.
 */
#define MAX_GLYPHS (32)

/*
 * Global variables that need to be inited on startup.
 */
GC	acsGc=0;	/* GC used for all drawing requests */
Pixmap	sPmap=0;	/* Stippled pixmap used for tiling */

/*
 * The grid is arbitrary, enough resolution that nothing's lost in
 * initialization.
 */
#define BOX_HIGH 60
#define BOX_WIDE 60

#define MID_HIGH (BOX_HIGH/2)
#define MID_WIDE (BOX_WIDE/2)

#define CHR_WIDE ((9*BOX_WIDE)/10)
#define CHR_HIGH ((9*BOX_HIGH)/10)

/*
 * ...since we'll scale the values anyway.
 */
#define SCALE_X(n) n = (n * (font_width-1)) / (BOX_WIDE-1)
#define SCALE_Y(n) n = (n * (font_height-1)) / (BOX_HIGH-1)

#define SEG(x0,y0,x1,y1) x0,y0, x1,y1

/*
 * XError handler for xftInitACS. This only sets sPmap to 0, and complains.
 */
int
acsXErrorHandler( __attribute__((unused)) Display *dpy,
	__attribute__((unused)) XErrorEvent *event)
{

    sPmap = 0;

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_XFTACS, "Could not create pixmap\n"));
    return 0;
}

/*
 * Initialize sPmap. Call this before calling xftDrawACSString(). If d or depth
 * are 0, then default values are used.
 *
 * You will probabaly get a BadMatch error if you try using xftDrawACSString on
 * a drawable of a different depth.
 */
void
xftInitACS( Display *dpy, Drawable d, unsigned depth)
{
    int (*oldXerrorHandler)( Display *, XErrorEvent *);

    rxvt_dbgmsg ((DBG_DEBUG, DBG_XFTACS, "Initing sPmap\n"));

    if(d == 0)		d = DefaultRootWindow( dpy);
    if(depth == 0)	depth=DefaultDepth( dpy, DefaultScreen( dpy));

#ifdef DEBUG
    if( sPmap != 0)
	rxvt_dbgmsg ((DBG_DEBUG, DBG_XFTACS, "sPmap not null in xftInitACS"));
#endif

    acsGc = XCreateGC( dpy, d, 0, NULL);

    oldXerrorHandler = XSetErrorHandler( (XErrorHandler) acsXErrorHandler);
    sPmap = XCreatePixmap( dpy, d, 2, 2, depth);
    XSetErrorHandler( oldXerrorHandler);

    XSetTile( dpy, acsGc, sPmap);
}

/*
 * Free sPmap. Call this after your last call to xftDrawACSString, or you WILL
 * have a memory leak.
 */
void
xftCloseACS( Display *dpy)
{
    if( sPmap )
    {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_XFTACS, "freeing sPmap\n"));

	XFreePixmap( dpy, sPmap);
	sPmap = 0;
    }
#ifdef DEBUG
    else
	rxvt_dbgmsg ((DBG_DEBUG, DBG_XFTACS, "sPmap already null in xftCloseACS"));
#endif

    XFreeGC( dpy, acsGc);
}

/*
 * INTERNAL USE ONLY. Draw a ACS graphics character on screen at x,y. Like XFT
 * functions, we do not clear the background before drawing.
 *
 * (x,y) is the bottom left corner of the character to draw.
 *
 * WARNING: If any char in *str has ascii value >= 32, then this function will
 * get stuck in an infinite loop.
 */
void
xftDrawACSChars(
	Display *dpy, Drawable d, GC gc,
	XftDraw *draw, const XftColor *color, XftFont *pub,
	int x, int y, const unsigned char *str, int len)
{

    /*
     * Line segments to draw line like chars.
     */
    static const short glyph_ht[] = {
	SEG(  0,	    0,		  0,	      5*MID_HIGH/6),	/* H */
	SEG(6*BOX_WIDE/10,  0,		6*BOX_WIDE/10,5*MID_HIGH/6),
	SEG(  0,	  5*MID_HIGH/12,6*BOX_WIDE/10,5*MID_HIGH/12),
	SEG(2*BOX_WIDE/10,  MID_HIGH,	  CHR_WIDE,	MID_HIGH),	/* T */
	SEG(6*BOX_WIDE/10,  MID_HIGH,	6*BOX_WIDE/10,	CHR_HIGH),
	-1
    }, glyph_ff[] = {
	SEG(  0,	    0,		6*BOX_WIDE/10,	0),		/* F */
	SEG(  0,	  5*MID_HIGH/12,6*CHR_WIDE/12,5*MID_HIGH/12),
	SEG(  0,	    0,		0*BOX_WIDE/3, 5*MID_HIGH/6),
	SEG(1*BOX_WIDE/3,   MID_HIGH,	  CHR_WIDE,	MID_HIGH),	/* F */
	SEG(1*BOX_WIDE/3, 8*MID_HIGH/6,10*CHR_WIDE/12,8*MID_HIGH/6),
	SEG(1*BOX_WIDE/3,   MID_HIGH,	1*BOX_WIDE/3,	CHR_HIGH),
	-1
    }, glyph_lf[] = {
	SEG(  0,	    0,		  0,	      5*MID_HIGH/6),	/* L */
	SEG(  0,	  5*MID_HIGH/6,	6*BOX_WIDE/10,5*MID_HIGH/6),
	SEG(1*BOX_WIDE/3,   MID_HIGH,	  CHR_WIDE,	MID_HIGH),	/* F */
	SEG(1*BOX_WIDE/3, 8*MID_HIGH/6,10*CHR_WIDE/12,8*MID_HIGH/6),
	SEG(1*BOX_WIDE/3,   MID_HIGH,	1*BOX_WIDE/3,	CHR_HIGH),
	-1
    }, glyph_nl[] = {
	SEG(  0,	  5*MID_HIGH/6,	  0,		0),		/* N */
	SEG(  0,	    0,		5*BOX_WIDE/6, 5*MID_HIGH/6),
	SEG(5*BOX_WIDE/6, 5*MID_HIGH/6, 5*BOX_WIDE/6,	0),
	SEG(1*BOX_WIDE/3,   MID_HIGH,	1*BOX_WIDE/3,	CHR_HIGH),	/* L */
	SEG(1*BOX_WIDE/3,   CHR_HIGH,	  CHR_WIDE,	CHR_HIGH),
	-1
    }, glyph_vt[] = {
	SEG(  0,	    0,		5*BOX_WIDE/12,5*MID_HIGH/6),	/* V */
	SEG(5*BOX_WIDE/12,5*MID_HIGH/6, 5*BOX_WIDE/6,	0),
	SEG(2*BOX_WIDE/10,  MID_HIGH,	  CHR_WIDE,	MID_HIGH),	/* T */
	SEG(6*BOX_WIDE/10,  MID_HIGH,	6*BOX_WIDE/10,	CHR_HIGH),
	-1
    }, lower_right_corner[] =
    {
	SEG(  0,	    MID_HIGH,	  MID_WIDE,	MID_HIGH),
	SEG(  MID_WIDE,	    MID_HIGH,	  MID_WIDE,	0),
	-1
    }, upper_right_corner[] =
    {
	SEG(  0,	    MID_HIGH,	  MID_WIDE,	MID_HIGH),
	SEG( MID_WIDE,	    MID_HIGH,	  MID_WIDE,	BOX_HIGH),
	-1
    }, upper_left_corner[] =
    {
	SEG(  MID_WIDE,	    MID_HIGH,	  BOX_WIDE,	MID_HIGH),
	SEG(  MID_WIDE,	    MID_HIGH,	  MID_WIDE,	BOX_HIGH),
	-1
    }, lower_left_corner[] =
    {
	SEG(  MID_WIDE,	    0,		  MID_WIDE,	MID_HIGH),
	SEG(  MID_WIDE,	    MID_WIDE,	  BOX_WIDE,	MID_HIGH),
	-1
    }, cross[] =
    {
	SEG(  0,	    MID_HIGH,	  BOX_WIDE,	MID_HIGH),
	SEG(  MID_WIDE,	    0,		  MID_WIDE,	BOX_HIGH),
	-1
    }, left_tee[] =
    {
	SEG(  MID_WIDE,	    0,		  MID_WIDE,	BOX_HIGH),
	SEG(  MID_WIDE,	    MID_HIGH,	  BOX_WIDE,	MID_HIGH),
	-1
    }, right_tee[] =
    {
	SEG(  MID_WIDE,	    0,		  MID_WIDE,	BOX_HIGH),
	SEG(  MID_WIDE,	    MID_HIGH,	  0,		MID_HIGH),
	-1
    }, bottom_tee[] =
    {
	SEG(  0,	    MID_HIGH,	  BOX_WIDE,	MID_HIGH),
	SEG(  MID_WIDE,	    0,		  MID_WIDE,	MID_HIGH),
	-1
    }, top_tee[] =
    {
	SEG(  0,	    MID_HIGH,	  BOX_WIDE,	MID_HIGH),
	SEG(  MID_WIDE,	    MID_HIGH,	  MID_WIDE,	BOX_HIGH),
	-1
    }, vertical_line[] =
    {
	SEG(  MID_WIDE,	    0,		  MID_WIDE,	BOX_HIGH),
	-1
    };

    /*
     * Pointer to line-segment structure.
     */
    static const short *lines[] = {
	NULL,			/* 00 (unused) */
	NULL,			/* 01 diamond */
	NULL,			/* 02 box */
	glyph_ht,		/* 03 HT */
	glyph_ff,		/* 04 FF */
	NULL,			/* 05 CR (not drawn) */
	glyph_lf,		/* 06 LF */
	NULL,			/* 07 degrees (small circle) */
	NULL,			/* 08 plus or minus*/
	glyph_nl,		/* 09 */
	glyph_vt,		/* 0A */
	lower_right_corner,	/* 0B */
	upper_right_corner,	/* 0C */
	upper_left_corner,	/* 0D */
	lower_left_corner,	/* 0E */
	cross,			/* 0F */
	NULL,			/* 10 overline  */
	NULL,			/* 11 topline   */
	NULL,			/* 12 midline   */
	NULL,			/* 13 botline   */
	NULL,			/* 14 underline */
	left_tee,		/* 15 */
	right_tee,		/* 16 */
	bottom_tee,		/* 17 */
	top_tee,		/* 18 */
	vertical_line,		/* 19 */
	NULL,			/* 1A leq */
	NULL,			/* 1B geq */
	NULL,			/* 1C pi */
	NULL,			/* 1D neq */
	NULL,			/* 1E pound  */
	NULL,			/* 1F bullet */
    };

    /*
     * Character number in XftFont (if any).
     */
    static const FT_UInt xftCharNo[] = {
	0,	/* 00 (unused) */
	0,	/* 01 diamond */
	0,	/* 02 box */
	0,	/* 03 HT */
	0,	/* 04 FF */
	0x8b,	/* 05 CR (drawn as (c) ) */
	0,	/* 06 LF */
	0x83,	/* 07 degrees (small circle) */
	0x93,	/* 08 plus or minus*/
	0,	/* 09 */
	0,	/* 0A */
	0,	/* 0B */
	0,	/* 0C */
	0,	/* 0D */
	0,	/* 0E */
	0,	/* 0F */
	0,	/* 10 overline  */
	0,	/* 11 topline   */
	0,	/* 12 midline   */
	0,	/* 13 botline   */
	0,	/* 14 underline */
	0,	/* 15 */
	0,	/* 16 */
	0,	/* 17 */
	0,	/* 18 */
	0,	/* 19 */
	0x94,	/* 1A leq */
	0x95,	/* 1B geq */
	0x9b,	/* 1C pi */
	0x8f,	/* 1D neq */
	0x85,	/* 1E pound  */
	0x87	/* 1F bullet */
    };

    unsigned font_width	 = pub->max_advance_width;
    unsigned font_height = pub->ascent + pub->descent;
    const short *p;
    FT_UInt glyphs[MAX_GLYPHS];

    int ytop = y - pub->ascent; /* (x, ytop) is the top left corner */

    /*
     * Update fill styles in acsGc
     */
    XSetLineAttributes(dpy, acsGc, (font_height > 16) ? font_height / 16 : 1,
	LineSolid, CapProjecting, JoinMiter);
    XCopyGC( dpy, gc, GCForeground | GCBackground, acsGc);

    /*
     * Draw the characters. A few (ones with curves / shading) need to be
     * treated specially. The rest can be drawn with the segments in lines[n].
     */
    while( 1 )
    {
	/*
	 * If even one value in str[] is >= 32, this loop will never terminate.
	 * This is checked before calling, so no need to recheck here.
	 *
	 * We use such "contorted" code to optimize for speed. Since this is
	 * done several times while refreshing the screen, we don't want to slow
	 * things down.
	 */
	int n;

	/*
	 * Find max contiguous block of chars which are present in the Xft font,
	 * and use XftDrawGlyphs to draw them.
	 */
	for( n=0; n < len && n < MAX_GLYPHS && (glyphs[n] = xftCharNo[ *str ]);
		n++, str++);
	if( n )
	{
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_XFTACS, "(%d glyphs) ", n));

	    XftDrawGlyphs( draw, color, pub, x, y, glyphs, n);

	    x += n * font_width;
	    if( !(len -= n) ) break; /* !(len -= n) iff (len -= n) <= 0 */
	}

	/*
	 * Draw contiguous stippled box (0x02)
	 */
	for( n=0; n < len && *str == 2; n++, str++);
	if( n )
	{
	    XGCValues values;

	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_XFTACS, "(%d boxes) ", n));

	    XGetGCValues( dpy, acsGc, GCForeground | GCBackground, &values);

	    XDrawPoint( dpy, sPmap, acsGc, 0, 0);
	    XDrawPoint( dpy, sPmap, acsGc, 1, 1);

	    XSetForeground( dpy, acsGc, values.background);
	    XDrawPoint( dpy, sPmap, acsGc, 0, 1);
	    XDrawPoint( dpy, sPmap, acsGc, 1, 0);

	    values.fill_style = FillTiled;
	    XChangeGC( dpy, acsGc, GCForeground | GCFillStyle, &values);

	    XFillRectangle( dpy, d, acsGc, x, ytop, n * font_width, font_height);

	    if( !(len -= n) ) break; /* !(len -= n) iff (len -= n) <= 0 */
	    x += n * font_width;
	}

	/*
	 * Draw contiguous horizontal lines.
	 */
	if( *str >= 0x10 && *str <= 0x14)
	{
	    unsigned char c = *str;
	    int xstart = x;
	    int ystart = ytop + ((c - 0x10) * (font_height-1)) / 4;

	    for( n=0; ++n < len && *(++str) == c; );

	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_XFTACS, "(%d hln)", n));
	    x += n * font_width;

	    XSetFillStyle( dpy, acsGc, FillSolid);
	    XDrawLine( dpy, d, acsGc, xstart, ystart, x-1, ystart);

	    if( !(len -= n)) break;
	}

	/*
	 * Contiguous drawing not possible for these cases.
	 */
	if (*str == 1)			/* Filled diamond */
	{
	    XPoint points[4];
	    int npoints = 4, n;

	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_XFTACS, "(1 dmd) "));

	    points[0].x = CHR_WIDE/2 + (BOX_WIDE - CHR_WIDE) / 2;
	    points[0].y = 0;

	    points[1].x = (BOX_WIDE - CHR_WIDE) / 2;
	    points[1].y = CHR_HIGH/2 + (BOX_HIGH - CHR_HIGH) / 2;

	    points[2].x = points[0].x;
	    points[2].y = CHR_HIGH + (BOX_HIGH - CHR_HIGH) / 2;

	    points[3].x = CHR_WIDE + (BOX_WIDE - CHR_WIDE) / 2;
	    points[3].y = points[1].y;

	    for (n = 0; n < npoints; n++)
	    {
		SCALE_X(points[n].x);
		SCALE_Y(points[n].y);

		points[n].x += x;
		points[n].y += ytop;
	    }

	    XSetFillStyle( dpy, acsGc, FillSolid);
	    XFillPolygon( dpy, d, acsGc, points, npoints, Convex, CoordModeOrigin);

	    if( ! (--len)) break;
	    str++;
	    x += font_width;
	}
	else if( NOT_NULL(p = lines[*str]))
	{
	    /*
	     * Draw character using segments in lines[*str]
	     */
	    int coord[4];
	    int n = 0;

	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_XFTACS, "(1 ldc) "));

	    XSetFillStyle( dpy, acsGc, FillSolid);
	    while (*p >= 0)
	    {
		coord[n++] = *p++;
		if (n == 4)
		{
		    SCALE_X(coord[0]);
		    SCALE_Y(coord[1]);
		    SCALE_X(coord[2]);
		    SCALE_Y(coord[3]);
		    XDrawLine( dpy, d, acsGc,
			x + coord[0], ytop + coord[1],
			x + coord[2], ytop + coord[3]);
		    n = 0;
		}
	    }

	    if( !(--len)) break;
	    str++;
	    x += font_width;
	}
    }
#ifdef DEBUG
    rxvt_dbgmsg ((DBG_DEBUG, DBG_XFTACS, "\n"));
#endif
}

/*
 * Draws an XFT string on screen. All characters below 32 are assumed to be ACS
 * graphics characters and are drawn by hand. Ther rest are passed to
 * xftdraw_string to be drawn by Xft. xftdraw_string should be XftDrawString8 /
 * Utf8. Changing this to accept XftDrawString16 etc is not hard, but will bloat
 * mrxvt (and isn't done here).
 */
void
xftDrawACSString ( Display *dpy, Drawable d, GC gc,
	void (*xftdraw_string)(),
	XftDraw *draw, const XftColor *color, XftFont *pub,
	int x, int y, const unsigned char *str, int len)
{
    const unsigned char *t = str;
    int chars;

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_XFTACS, "Drawing %d(%d) %sACS characters.", len, STRLEN( str), ( xftdraw_string == XftDrawString8) ? "Utf8 " : ""));

    while(len > 0)
    {
	/*
	 * Pass all non graphic chars to xftdraw_string.
	 */
	for( chars=0; *t >= 32 && chars < len; chars++, t++);
	if( chars)
	{
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_XFTACS, " [%d chars]", chars));
	    xftdraw_string( draw, color, pub, x, y, str, chars);

	    x += chars * pub->max_advance_width;
	    str = t;
	    len -= chars;
	}

	/*
	 * Draw all ACS graphics chars by hand.
	 */
	for( chars=0; *t < 32 && chars < len; chars++, t++);
	if( chars)
	{
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_XFTACS, " (%d glyphs)", chars));
	    xftDrawACSChars( dpy, d, gc, draw, color, pub, x, y, str, chars);

	    x += chars * pub->max_advance_width;
	    str = t;
	    len -= chars;
	}
    }
#ifdef DEBUG
    rxvt_dbgmsg ((DBG_DEBUG, DBG_XFTACS, "\n"));
#endif
}
/*-------------------------- end-of-file (C source) --------------------------*/
