/*--------------------------------*-C-*---------------------------------*
 * File:    screen.c
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1997-2001   Geoff Wing <gcw@pobox.com>
 * Copyright (C) 2000,2001   Teepanis Chachiyo <teepanis@physics.purdue.edu>
 * Copyright (c) 2001        Marius Gedminas <marius.gedminas@uosis.mif.vu.lt>
 * Copyright (c) 2003        David Hull
 * Copyright (c) 2003        Yamanobe Kiichiro <yamky@cocoa.freemail.ne.jp>
 * Copyright (c) 2003        Mamoru Komachi <usata@usata.org>
 * Copyright (c) 2005        William P. Y. Hadisoeseno <williampoetra@users.sourceforge.net>
 * Copyright (c) 2004-2006   Jingmin Zhou <jimmyzhou@users.sourceforge.net>
 * Copyright (c) 2005-2006   Gautam Iyer <gi1242@users.sourceforge.net>
 * Copyright (c) 2007		  Jehan Hysseo <hysseo@users.sourceforge.net>
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
#define INTERN_SCREEN
#include "rxvt.h"

#ifdef HAVE_X11_XKBLIB_H
#include <X11/XKBlib.h>
#endif

#ifdef XFT_SUPPORT
# include <xftacs.h>
#endif

#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif

/* ------------------------------------------------------------------------- */
#ifdef MULTICHAR_SET
#define RESET_CHSTAT(R)		    \
    if (PVTS((R))->chstat == WBYTE)	\
	PVTS((R))->chstat = SBYTE, PVTS((R))->lost_multi = 1
#else
# define RESET_CHSTAT(R)
#endif

/* ------------------------------------------------------------------------- */
#define PROP_SIZE	16384

/* ------------------------------------------------------------------------- *
 *	     GENERAL SCREEN AND SELECTION UPDATE ROUTINES		  *
 * ------------------------------------------------------------------------- */

/*
** If inhibit scrolling on tty output, we should keep the view_start.
** Otherwise, we set it to zero.
*/
#define ZERO_SCROLLBACK(R)			    \
    if (NOTSET_OPTION(R, Opt_scrollTtyOutputInhibit))	\
	PVTS((R))->view_start = 0

#define CLEAR_SELECTION(R)			\
    (R)->selection.beg.row = \
    (R)->selection.beg.col = \
    (R)->selection.end.row = \
    (R)->selection.end.col = 0

#define CLEAR_ALL_SELECTION(R)			\
    (R)->selection.beg.row = \
    (R)->selection.beg.col = \
    (R)->selection.mark.row = \
    (R)->selection.mark.col = \
    (R)->selection.end.row = \
    (R)->selection.end.col = 0

#define ROW_AND_COL_IS_AFTER(A, B, C, D)		\
    (((A) > (C)) || (((A) == (C)) && ((B) > (D))))
#define ROW_AND_COL_IS_BEFORE(A, B, C, D)		\
    (((A) < (C)) || (((A) == (C)) && ((B) < (D))))
#define ROW_AND_COL_IN_ROW_AFTER(A, B, C, D)		\
    (((A) == (C)) && ((B) > (D)))
#define ROW_AND_COL_IN_ROW_AT_OR_AFTER(A, B, C, D)	\
    (((A) == (C)) && ((B) >= (D)))
#define ROW_AND_COL_IN_ROW_BEFORE(A, B, C, D)		\
    (((A) == (C)) && ((B) < (D)))
#define ROW_AND_COL_IN_ROW_AT_OR_BEFORE(A, B, C, D)	\
    (((A) == (C)) && ((B) <= (D)))

/* these must be row_col_t */
#define RC_AFTER(X, Y)			    \
    ROW_AND_COL_IS_AFTER((X).row, (X).col, (Y).row, (Y).col)
#define RC_BEFORE(X, Y)			    \
    ROW_AND_COL_IS_BEFORE((X).row, (X).col, (Y).row, (Y).col)
#define RC_ROW_AFTER(X, Y)		    \
    ROW_AND_COL_IN_ROW_AFTER((X).row, (X).col, (Y).row, (Y).col)
#define RC_ROW_BEFORE(X, Y)		    \
    ROW_AND_COL_IN_ROW_BEFORE((X).row, (X).col, (Y).row, (Y).col)
#define RC_ROW_ATAFTER(X, Y)			\
    ROW_AND_COL_IN_ROW_AT_OR_AFTER((X).row, (X).col, (Y).row, (Y).col)
#define RC_ROW_ATBEFORE(X, Y)		    \
    ROW_AND_COL_IN_ROW_AT_OR_BEFORE((X).row, (X).col, (Y).row, (Y).col)

/*
 * CLEAR_ROWS : clear <num> rows starting from row <row>
 * CLEAR_CHARS: clear <num> chars starting from pixel position <x,y>
 * ERASE_ROWS : set <num> rows starting from row <row> to the foreground colour
 */
#define drawBuffer  (PVTS(r)->vt)

#define CLEAR_ROWS(row, num)				\
    if (r->TermWin.mapped)				\
	rxvt_clear_area (r,			\
	    r->TermWin.int_bwidth, Row2Pixel(row),	\
	    VT_WIDTH(r), (unsigned int)Height2Pixel(num))


/*
 * If already_cleared, then we got here during a clipped refresh. In this case,
 * areas we draw into are already cleared by the server. We don't need to clear
 * it ourself. (Doing so will cause problems under transparency: E.g. We want to
 * clear an entire character cell, but an expose event was generated for only
 * half the character cell. By our wonderful clippings, we redraw only half the
 * char cell. However requests to this function will clear an ENTIRE char cell,
 * causing problems.
 *
 * If garbage is found due to clipped refreshes, then we should explicitly call
 * XClearArea before generating a clipped refresh.
 */
#define CLEAR_CHARS(r, already_cleared, x, y, num)	\
	if( !already_cleared )					\
	    rxvt_clear_area( (r), (x), (y),		\
		(unsigned) Width2Pixel((num)),			\
		(unsigned) Height2Pixel(1));


#define ERASE_ROWS(row, num)			    \
    rxvt_fill_rectangle (r,		    \
	   r->TermWin.int_bwidth, Row2Pixel(row),   \
	   VT_WIDTH(r),	(unsigned int)Height2Pixel(num))


#ifdef DONT_SELECT_TRAILING_SPACES
# define STRIP_TRAILING_SPACE(str, fence)	    \
    while (str > fence && ' ' == str[-1])	    \
	str --;
#endif


/* Here are some simple macros for convenience */
#undef CURROW
#undef CURCOL
#undef SVLINES
#undef VSTART
#define CURROW	    (PSCR(r).cur.row)
#define CURCOL	    (PSCR(r).cur.col)
#define SVLINES	    (PVTS(r)->saveLines)
#define VSTART	    (PVTS(r)->view_start)


/*--------------------------------------------------------------------*
 *         BEGIN `INTERNAL' ROUTINE PROTOTYPES                        *
 *--------------------------------------------------------------------*/
static inline void rxvt_clear_area       (rxvt_t*, int x, int y, unsigned int w, unsigned int h);
static inline void rxvt_fill_rectangle   (rxvt_t*, int x, int y, unsigned int w, unsigned int h);
static void rxvt_set_font_style          (rxvt_t*);
static int  rxvt_scr_change_view         (rxvt_t*, uint16_t);
static void rxvt_scr_reverse_selection   (rxvt_t*);
static int  rxvt_selection_request_other (rxvt_t*, Atom, int);
static void rxvt_selection_start_colrow  (rxvt_t*, int, int);
static void rxvt_selection_extend_colrow (rxvt_t*, int32_t, int32_t, int, int, int);
#ifndef NO_FRILLS
static void rxvt_selection_trim          (rxvt_t*);
#endif
/*--------------------------------------------------------------------*
 *         END   `INTERNAL' ROUTINE PROTOTYPES                        *
 *--------------------------------------------------------------------*/


 
/* ------------------------------------------------------------------------- *
 *			SCREEN `COMMON' ROUTINES			   *
 * ------------------------------------------------------------------------- */

/* Fill part/all of a line with blanks. */
/* INTPROTO */
static void
rxvt_blank_line(text_t *et, rend_t *er, unsigned int width, rend_t efs)
{
    MEMSET(et, ' ', (size_t)width);
    efs &= ~RS_baseattrMask;
    for (; width--;)
	*er++ = efs;
}

/* ------------------------------------------------------------------------- */
/* Fill a full line with blanks - make sure it is allocated first */
/* INTPROTO */
static void
rxvt_blank_screen_mem(rxvt_t* r, text_t **tp, rend_t **rp,
	unsigned int row, rend_t efs)
{
    int		 width = r->TermWin.ncol;
    rend_t	 *er;

    assert ((tp[row] && rp[row]) ||
	(tp[row] == NULL && rp[row] == NULL));

    /* possible integer overflow? */
    assert (width > 0);
    assert (sizeof (text_t) * width > 0);
    assert (sizeof (rend_t) * width > 0);

    if (tp[row] == NULL)
    {
	tp[row] = rxvt_malloc(sizeof(text_t) * width);
	rp[row] = rxvt_malloc(sizeof(rend_t) * width);
    }
    MEMSET(tp[row], ' ', width);
    efs &= ~RS_baseattrMask;
    for (er = rp[row]; width--;)
	*er++ = efs;
}



/* ------------------------------------------------------------------------- *
 *			  SCREEN INITIALISATION				*
 * ------------------------------------------------------------------------- */
/* EXTPROTO */
void
rxvt_init_screen (rxvt_t* r)
{
    int	    p;
    int	    ncol = r->TermWin.ncol;

    /* first time, we don't have r->tabstop yet */
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "allocate r->tabstop as %d\n", ncol));
    assert (ncol > 0);	/* possible integer overflow? */
    r->tabstop = rxvt_malloc(ncol * sizeof(char));
    for (p = 0; p < ncol; p++)
	r->tabstop[p] = (p % TABSTOP_SIZE == 0) ? 1 : 0;
}


static void
rxvt_scr_alloc (rxvt_t* r)
{
    unsigned int    nrow, total_rows;
    unsigned int    p, q;


    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_alloc ()\n"));
    nrow = r->TermWin.nrow;
    total_rows = nrow + SVLINES;

    /*
    ** First time called so just malloc everything : don't rely on
    ** realloc
    ** Note: this is still needed so that all the scrollback lines
    ** are NULL
    */
    PVTS(r)->buf_text = rxvt_calloc(total_rows, sizeof(text_t*));
    PVTS(r)->buf_rend = rxvt_calloc(total_rows, sizeof(rend_t*));

    PVTS(r)->drawn_text = rxvt_calloc(nrow, sizeof(text_t*));
    PVTS(r)->drawn_rend = rxvt_calloc(nrow, sizeof(rend_t*));

    PSCR(r).text = rxvt_calloc(total_rows, sizeof(text_t*));
    PSCR(r).tlen = rxvt_calloc(total_rows, sizeof(int16_t));
    PSCR(r).rend = rxvt_calloc(total_rows, sizeof(rend_t*));

#if NSCREENS
    PVTS(r)->swap.text = rxvt_calloc(nrow, sizeof(text_t*));
    PVTS(r)->swap.tlen = rxvt_calloc(nrow, sizeof(int16_t));
    PVTS(r)->swap.rend = rxvt_calloc(nrow, sizeof(rend_t*));
#endif

    for (p = 0; p < nrow; p++)
    {
	q = p + SVLINES;
	rxvt_blank_screen_mem (r, PSCR(r).text,
	    PSCR(r).rend, q, DEFAULT_RSTYLE);
	PSCR(r).tlen[q] = 0;
#if NSCREENS
	rxvt_blank_screen_mem (r, PVTS(r)->swap.text,
	    PVTS(r)->swap.rend, p, DEFAULT_RSTYLE);
	PVTS(r)->swap.tlen[p] = 0;
#endif

	rxvt_blank_screen_mem (r, PVTS(r)->drawn_text,
	    PVTS(r)->drawn_rend, p, DEFAULT_RSTYLE);
    }
    PVTS(r)->nscrolled = 0;   /* no saved lines */
    PSCR(r).flags = Screen_DefaultFlags;
    PSCR(r).cur.row = 0;
    PSCR(r).cur.col = 0;
    PSCR(r).charset = 0;
    PVTS(r)->current_screen = PRIMARY;
    rxvt_scr_cursor(r, SAVE);
#if NSCREENS
    PVTS(r)->swap.flags = Screen_DefaultFlags;
    PVTS(r)->swap.cur.row = 0;
    PVTS(r)->swap.cur.col = 0;
    PVTS(r)->swap.charset = 0;
    PVTS(r)->current_screen = SECONDARY;
    rxvt_scr_cursor(r, SAVE);
    PVTS(r)->current_screen = PRIMARY;
#endif

    PVTS(r)->rstyle = DEFAULT_RSTYLE;
    PVTS(r)->rvideo = 0;
    MEMSET(&(PVTS(r)->charsets), 'B', sizeof(PVTS(r)->charsets));
#ifdef MULTICHAR_SET
    PVTS(r)->multi_byte = 0;
    PVTS(r)->lost_multi = 0;
    PVTS(r)->chstat = SBYTE;
#endif

    /* Now set screen initialization flag */
    PVTS(r)->init_screen = 1;
}



/* INTPROTO */
static void
rxvt_scr_reset_realloc(rxvt_t* r)
{
    unsigned int   total_rows, nrow;


    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_reset_realloc ()\n"));
    nrow = r->TermWin.nrow;
    total_rows = nrow + SVLINES;

    PSCR(r).text = rxvt_realloc (
	PSCR(r).text, total_rows * sizeof(text_t *));
    PSCR(r).tlen = rxvt_realloc (
	PSCR(r).tlen, total_rows * sizeof(int16_t));
    PSCR(r).rend = rxvt_realloc (
	PSCR(r).rend, total_rows * sizeof(rend_t *));

#if NSCREENS
    PVTS(r)->swap.text   = rxvt_realloc (
	PVTS(r)->swap.text, nrow * sizeof(text_t *));
    PVTS(r)->swap.tlen   = rxvt_realloc (
	PVTS(r)->swap.tlen  , total_rows * sizeof(int16_t));
    PVTS(r)->swap.rend   = rxvt_realloc (
	PVTS(r)->swap.rend, nrow * sizeof(rend_t *));
#endif

    PVTS(r)->buf_text = rxvt_realloc (
	PVTS(r)->buf_text, total_rows * sizeof(text_t *));
    PVTS(r)->buf_rend = rxvt_realloc (
	PVTS(r)->buf_rend, total_rows * sizeof(rend_t *));

    PVTS(r)->drawn_text  = rxvt_realloc (
	PVTS(r)->drawn_text, nrow * sizeof(text_t *));
    PVTS(r)->drawn_rend  = rxvt_realloc (
	PVTS(r)->drawn_rend, nrow * sizeof(rend_t *));
}


/* INTPROTO */
static void
rxvt_scr_delete_row (rxvt_t* r)
{
    unsigned int    nrow, prev_nrow;
    unsigned int    p, q;
    register int    i;


    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_delete_row ()\n"));
    nrow = r->TermWin.nrow;
    prev_nrow = PVTS(r)->prev_nrow;

    /* delete rows */
    i = min(PVTS(r)->nscrolled, prev_nrow - nrow);
    rxvt_scroll_text(r, 0, (int)prev_nrow - 1, i, 1);

    for (p = nrow; p < prev_nrow; p++)
    {
	q = p + SVLINES;
	if (PSCR(r).text[q])
	{
	    assert(PSCR(r).rend[q]);
	    rxvt_free(PSCR(r).text[q]);
	    PSCR(r).text[q] = NULL;
	    rxvt_free(PSCR(r).rend[q]);
	    PSCR(r).rend[q] = NULL;
	}
#if NSCREENS
	if (PVTS(r)->swap.text[p])
	{
	    assert(PVTS(r)->swap.rend[p]);
	    rxvt_free(PVTS(r)->swap.text[p]);
	    PVTS(r)->swap.text[p] = NULL;
	    rxvt_free(PVTS(r)->swap.rend[p]);
	    PVTS(r)->swap.rend[p] = NULL;
	}
#endif
	assert (PVTS(r)->drawn_text[p]);
	assert (PVTS(r)->drawn_rend[p]);
	rxvt_free(PVTS(r)->drawn_text[p]);
	PVTS(r)->drawn_text[p] = NULL;
	rxvt_free(PVTS(r)->drawn_rend[p]);
	PVTS(r)->drawn_rend[p] = NULL;
    }

    /* we have fewer rows so fix up cursor position */
    MIN_IT(PSCR(r).cur.row, (int32_t)nrow - 1);
#if NSCREENS
    MIN_IT(PVTS(r)->swap.cur.row, (int32_t)nrow - 1);
#endif

    rxvt_scr_reset_realloc (r);	/* realloc _last_ */
}


/* INTPROTO */
static void
rxvt_scr_add_row (rxvt_t* r, unsigned int total_rows, unsigned int prev_total_rows)
{
    unsigned int    nrow, prev_nrow;
    unsigned int    p;
    register int    i;


    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "%s( total_rows=%u, prev_total_rows=%u )\n", __func__, total_rows, prev_total_rows ));

    nrow = r->TermWin.nrow;
    prev_nrow = PVTS(r)->prev_nrow;

    /* add rows */
    rxvt_scr_reset_realloc(r);	/* realloc _first_ */

    i = min(PVTS(r)->nscrolled, nrow - prev_nrow);
    for (p = prev_total_rows; p < total_rows; p++)
    {
	PSCR(r).tlen[p] = 0;
	PSCR(r).text[p] = NULL;
	PSCR(r).rend[p] = NULL;
    }

    for (p = prev_total_rows; p < total_rows - i; p++)
	rxvt_blank_screen_mem (r, PSCR(r).text,
	    PSCR(r).rend, p, DEFAULT_RSTYLE);

    for (p = prev_nrow; p < nrow; p++)
    {
#if NSCREENS
	PVTS(r)->swap.tlen[p] = 0;
	PVTS(r)->swap.text[p] = NULL;
	PVTS(r)->swap.rend[p] = NULL;
	rxvt_blank_screen_mem (r, PVTS(r)->swap.text,
	    PVTS(r)->swap.rend, p, DEFAULT_RSTYLE);
#endif

	PVTS(r)->drawn_text[p] = NULL;
	PVTS(r)->drawn_rend[p] = NULL;
	rxvt_blank_screen_mem (r, PVTS(r)->drawn_text,
	    PVTS(r)->drawn_rend, p, DEFAULT_RSTYLE);
    }

    if (i > 0)
    {
	rxvt_scroll_text(r, 0, (int)nrow - 1, -i, 1);
	PSCR(r).cur.row += i;
	PSCR(r).s_cur.row += i;
	PVTS(r)->nscrolled -= i;
    }

    assert(PSCR(r).cur.row < r->TermWin.nrow);
    MIN_IT(PSCR(r).cur.row, nrow - 1);
#if NSCREENS
    assert(PVTS(r)->swap.cur.row < r->TermWin.nrow);
    MIN_IT(PVTS(r)->swap.cur.row, nrow - 1);
#endif
}



/* INTPROTO */
static void
rxvt_scr_adjust_col (rxvt_t* r, unsigned int total_rows)
{
    unsigned int    nrow, ncol, prev_ncol;
    unsigned int    p;


    nrow = r->TermWin.nrow;
    ncol = r->TermWin.ncol;
    prev_ncol = PVTS(r)->prev_ncol;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "%s( r, total_rows=%u ):" "ncol=%d, prev_ncol=%d, nrow=%d\n", __func__, total_rows, ncol, prev_ncol, nrow ));


    for (p = 0; p < total_rows; p++)
    {
	if (PSCR(r).text[p])
	{
	    PSCR(r).text[p] = rxvt_realloc (
		PSCR(r).text[p], ncol * sizeof(text_t));
	    PSCR(r).rend[p] = rxvt_realloc (
		PSCR(r).rend[p], ncol * sizeof(rend_t));
	    MIN_IT(PSCR(r).tlen[p], (int16_t)ncol);
	    if (ncol > prev_ncol)
		rxvt_blank_line (
		    &(PSCR(r).text[p][prev_ncol]),
		    &(PSCR(r).rend[p][prev_ncol]),
		    ncol - prev_ncol, DEFAULT_RSTYLE);
	}
    }

    for (p = 0; p < nrow; p++)
    {
	PVTS(r)->drawn_text[p] = rxvt_realloc (
	    PVTS(r)->drawn_text[p], ncol * sizeof(text_t));
	PVTS(r)->drawn_rend[p] = rxvt_realloc (
	    PVTS(r)->drawn_rend[p], ncol * sizeof(rend_t));
#if NSCREENS
	if (PVTS(r)->swap.text[p])
	{
	    PVTS(r)->swap.text[p] = rxvt_realloc (
		PVTS(r)->swap.text[p], ncol * sizeof(text_t));
	    PVTS(r)->swap.rend[p] = rxvt_realloc (
		PVTS(r)->swap.rend[p], ncol * sizeof(rend_t));
	    MIN_IT(PVTS(r)->swap.tlen[p], (int16_t)ncol);
	    if (ncol > prev_ncol)
		rxvt_blank_line(
		    &(PVTS(r)->swap.text[p][prev_ncol]),
		    &(PVTS(r)->swap.rend[p][prev_ncol]),
		    ncol - prev_ncol, DEFAULT_RSTYLE);
	}
#endif
	if (ncol > prev_ncol)
	    rxvt_blank_line(
		&(PVTS(r)->drawn_text[p][prev_ncol]),
		&(PVTS(r)->drawn_rend[p][prev_ncol]),
		ncol - prev_ncol, DEFAULT_RSTYLE);
    }
    MIN_IT(PSCR(r).cur.col, (int16_t)ncol - 1);
#if NSCREENS
    MIN_IT(PVTS(r)->swap.cur.col, (int16_t)ncol - 1);
#endif


    /*
    ** Only reset tabstop if expanding columns, save realloc in
    ** shrinking columns
    */
    if (r->tabstop && ncol > prev_ncol)
    {
	rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "expand r->tabstop to %d\n", ncol));
	r->tabstop = rxvt_realloc(r->tabstop, ncol * sizeof(char));
	for (p = prev_ncol; p < ncol; p++)
	    r->tabstop[p] = (p % TABSTOP_SIZE == 0) ? 1 : 0;
    }
}



/* EXTPROTO */
void
rxvt_scr_reset(rxvt_t* r)
{
    unsigned int    ncol, nrow, prev_ncol, prev_nrow,
		    total_rows, prev_total_rows;


    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_reset ()\n"));

    VSTART = 0;
    RESET_CHSTAT(r);
    PVTS(r)->num_scr = 0;	/* number of lines scrolled */

    prev_ncol = PVTS(r)->prev_ncol;
    prev_nrow = PVTS(r)->prev_nrow;
    if (r->TermWin.ncol == 0)
	r->TermWin.ncol = 80;
    if (r->TermWin.nrow == 0)
	r->TermWin.nrow = 24;
    ncol = r->TermWin.ncol;
    nrow = r->TermWin.nrow;
    if (PVTS(r)->init_screen &&
	ncol == prev_ncol && nrow == prev_nrow)
	return;

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_reset () refresh screen\n"));
    PVTS(r)->want_refresh = 1;

    total_rows = nrow + SVLINES;
    prev_total_rows = prev_nrow + SVLINES;

    PSCR(r).tscroll = 0;
    PSCR(r).bscroll = nrow - 1;

    if (PVTS(r)->init_screen == 0)
    {
	/* Initialize the screen structures */
	rxvt_scr_alloc (r);
    }
    else
    {
	/* B1: resize rows */
	if (nrow < prev_nrow)
	{
	    rxvt_scr_delete_row (r);
	}
	else if (nrow > prev_nrow)
	{
	    rxvt_scr_add_row (r, total_rows, prev_total_rows);
	}
	/* B2: resize columns */
	if (ncol != prev_ncol)
	{
	    rxvt_scr_adjust_col (r, total_rows);
	}
    }

    if (PVTS(r)->prev_nrow == nrow && PVTS(r)->prev_ncol == ncol)
	    return;

    PVTS(r)->prev_nrow = nrow;
    PVTS(r)->prev_ncol = ncol;

    rxvt_tt_winsize(PVTS(r)->cmd_fd, r->TermWin.ncol, r->TermWin.nrow, PVTS(r)->cmd_pid);
}




/* ------------------------------------------------------------------------- */
/*
 * Free everything.  That way malloc debugging can find leakage.
 */
/* EXTPROTO */
void
rxvt_scr_release(rxvt_t* r)
{
    unsigned int    total_rows;
    unsigned i;


    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_release ()\n"));
    total_rows = r->TermWin.nrow + SVLINES;

    for (i = 0; i < total_rows; i++)
    {
	if (PSCR(r).text[i])
	{
	    /* then so is PSCR(r).rend[i] */
	    rxvt_free(PSCR(r).text[i]);
	    PSCR(r).text[i] = NULL;
	    assert(PSCR(r).rend[i]);
	    rxvt_free(PSCR(r).rend[i]);
	    PSCR(r).rend[i] = NULL;
	}
    }

    for (i = 0; i < r->TermWin.nrow; i++)
    {
	/* if (PVTS(r)->drawn_text[i]) */
	    rxvt_free(PVTS(r)->drawn_text[i]);
	PVTS(r)->drawn_text[i] = NULL;
	/* if (PVTS(r)->drawn_rend[i]) */
	    rxvt_free(PVTS(r)->drawn_rend[i]);
	PVTS(r)->drawn_rend[i] = NULL;
#if NSCREENS
	/* if (PVTS(r)->swap.text[i]) */
	    rxvt_free(PVTS(r)->swap.text[i]);
	PVTS(r)->swap.text[i] = NULL;
	/* if (PVTS(r)->swap.rend[i])) */
	    rxvt_free(PVTS(r)->swap.rend[i]);
	PVTS(r)->swap.rend[i] = NULL;
#endif
    }

    rxvt_free(PSCR(r).text); PSCR(r).text = NULL;
    rxvt_free(PSCR(r).tlen); PSCR(r).tlen = NULL;
    rxvt_free(PSCR(r).rend); PSCR(r).rend = NULL;
    rxvt_free(PVTS(r)->drawn_text);  PVTS(r)->drawn_text = NULL;
    rxvt_free(PVTS(r)->drawn_rend);  PVTS(r)->drawn_rend = NULL;
#if NSCREENS
    rxvt_free(PVTS(r)->swap.text);   PVTS(r)->swap.text = NULL;
    rxvt_free(PVTS(r)->swap.tlen);   PVTS(r)->swap.tlen = NULL;
    rxvt_free(PVTS(r)->swap.rend);   PVTS(r)->swap.rend = NULL;
#endif
    rxvt_free(PVTS(r)->buf_text);	PVTS(r)->buf_text = NULL;
    rxvt_free(PVTS(r)->buf_rend);	PVTS(r)->buf_rend = NULL;

    /* next rxvt_scr_reset will be the first time initialization */
    PVTS(r)->init_screen = 0;

    /* clear selection if necessary */
    if (r->selection.op)
    {
	rxvt_process_selectionclear (r);
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Hard reset
 */
/* EXTPROTO */
void
rxvt_scr_poweron(rxvt_t* r)
{
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_poweron ()\n"));

    rxvt_scr_release(r);
    PVTS(r)->prev_nrow = PVTS(r)->prev_ncol = 0;
    rxvt_scr_reset(r);

    rxvt_scr_clear(r);
    rxvt_scr_refresh(r, SLOW_REFRESH);
}


/* ------------------------------------------------------------------------- *
 *			 PROCESS SCREEN COMMANDS			   *
 * ------------------------------------------------------------------------- */
/*
 * Save and Restore cursor
 * XTERM_SEQ: Save cursor   : ESC 7
 * XTERM_SEQ: Restore cursor: ESC 8
 */
/* EXTPROTO */
void
rxvt_scr_cursor(rxvt_t* r, int mode)
{
    screen_t	   *s;

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_cursor (%c)\n", mode));

#if NSCREENS && !defined(NO_SECONDARY_SCREEN_CURSOR)
    if (PVTS(r)->current_screen == SECONDARY)
	s = &(PVTS(r)->swap);
    else
#endif
    s = &(PSCR(r));
    switch (mode)
    {
	case SAVE:
	    s->s_cur.row = s->cur.row;
	    s->s_cur.col = s->cur.col;
	    s->s_rstyle = PVTS(r)->rstyle;
	    s->s_charset = s->charset;
	    s->s_charset_char = PVTS(r)->charsets[s->charset];
	    break;
	case RESTORE:
	    PVTS(r)->want_refresh = 1;
	    s->cur.row = s->s_cur.row;
	    s->cur.col = s->s_cur.col;
	    s->flags &= ~Screen_WrapNext;
	    PVTS(r)->rstyle = s->s_rstyle;
	    s->charset = s->s_charset;
	    PVTS(r)->charsets[s->charset] = s->s_charset_char;
	    rxvt_set_font_style(r);
	    break;
    }
/* boundary check in case screen size changed between SAVE and RESTORE */
    MIN_IT(s->cur.row, r->TermWin.nrow - 1);
    MIN_IT(s->cur.col, r->TermWin.ncol - 1);
    assert(s->cur.row >= 0);
    assert(s->cur.col >= 0);
    MAX_IT(s->cur.row, 0);
    MAX_IT(s->cur.col, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Swap between primary and secondary screens
 * XTERM_SEQ: Primary screen  : ESC [ ? 4 7 h
 * XTERM_SEQ: Secondary screen: ESC [ ? 4 7 l
 */
/* EXTPROTO */
int
rxvt_scr_change_screen(rxvt_t* r, int scrn)
{
#if NSCREENS
    unsigned int	i, offset;
#endif

    PVTS(r)->want_refresh = 1;

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_change_screen (%d)\n", scrn));

    VSTART = 0;
    RESET_CHSTAT(r);

    if (PVTS(r)->current_screen == scrn)
	return PVTS(r)->current_screen;

    rxvt_selection_check(r, 2);	/* check for boundary cross */

    SWAP_IT(scrn, PVTS(r)->current_screen);

#if NSCREENS
    PVTS(r)->num_scr = 0;
    offset = SVLINES;

    for (i = PVTS(r)->prev_nrow; i--;)
    {
	SWAP_IT(PSCR(r).text[i + offset],
	    PVTS(r)->swap.text[i]);
	SWAP_IT(PSCR(r).tlen[i + offset],
	    PVTS(r)->swap.tlen[i]);
	SWAP_IT(PSCR(r).rend[i + offset],
	    PVTS(r)->swap.rend[i]);
    }
    SWAP_IT(CURROW, PVTS(r)->swap.cur.row);
    SWAP_IT(CURCOL, PVTS(r)->swap.cur.col);
    assert (CURROW >= 0);
    assert (CURROW < PVTS(r)->prev_nrow);
    assert (CURCOL >= 0);
    assert (CURCOL < PVTS(r)->prev_ncol);
    MAX_IT(CURROW, 0);
    MIN_IT(CURROW, (int32_t)PVTS(r)->prev_nrow - 1);
    MAX_IT(CURCOL, 0);
    MIN_IT(CURCOL, (int32_t)PVTS(r)->prev_ncol - 1);

    SWAP_IT(PSCR(r).charset, PVTS(r)->swap.charset);
    SWAP_IT(PSCR(r).flags, PVTS(r)->swap.flags);
    PSCR(r).flags |= Screen_VisibleCursor;
    PVTS(r)->swap.flags |= Screen_VisibleCursor;

#else
# ifdef SCROLL_ON_NO_SECONDARY
    if (PVTS(r)->current_screen == PRIMARY)
	rxvt_scroll_text(r, 0, (PVTS(r)->prev_nrow - 1),
	    PVTS(r)->prev_nrow, 0);
# endif
#endif

    return scrn;
}

/* ------------------------------------------------------------------------- */
/*
 * Change the colour for following text
 */
/* EXTPROTO */
void
rxvt_scr_color(rxvt_t* r, unsigned int color, int fgbg)
{
    color &= RS_fgMask;
    if (Color_fg == fgbg)
	PVTS(r)->rstyle=SET_FGCOLOR(PVTS(r)->rstyle, color);
    else 
	PVTS(r)->rstyle=SET_BGCOLOR(PVTS(r)->rstyle, color);
}


/* ------------------------------------------------------------------------- */
/*
 * Change the rendition style for following text
 */
/* EXTPROTO */
void
rxvt_scr_rendition(rxvt_t* r, int set, rend_t style)
{
    if (set)
	PVTS(r)->rstyle |= style;
    else if (style == ~RS_None)
	PVTS(r)->rstyle = DEFAULT_RSTYLE | (PVTS(r)->rstyle & RS_fontMask);
    else
	PVTS(r)->rstyle &= ~style;
}

/* ------------------------------------------------------------------------- */
/*
 * Scroll text between <row1> and <row2> inclusive, by <count> lines
 * count positive ==> scroll up
 * count negative ==> scroll down
 * spec == 0 for normal routines
 */
/* EXTPROTO */
int
rxvt_scroll_text(rxvt_t* r, int row1, int row2, int count, int spec)
{
    int		    i, j, ret;
    unsigned int    nscrolled;
    size_t 	    size;

    if (count == 0 || (row1 > row2))
	return 0;

    PVTS(r)->want_refresh = 1;
    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN,
		"rxvt_scroll_text (%d,%d,%d,%d): %s\n", row1, row2,
		count, spec, (PVTS(r)->current_screen == PRIMARY) ?
		    "Primary" : "Secondary" ));

    if (
	  (count > 0)
	  && (row1 == 0)
	  && (PVTS(r)->current_screen == PRIMARY)
       )
    {
	nscrolled = (unsigned int) PVTS(r)->nscrolled
					    + (unsigned int) count;

	if (nscrolled > (unsigned int)SVLINES)
	    PVTS(r)->nscrolled = SVLINES;
	else
	    PVTS(r)->nscrolled = (uint16_t)nscrolled;
    }
    else if (!spec)
	row1 += SVLINES;
    row2 += SVLINES;

    if (
	  SEL(r).op
	  && PVTS(r)->current_screen == SEL(r).screen
       )
    {
	i = SEL(r).beg.row + SVLINES;
	j = SEL(r).end.row + SVLINES;
	if (
	      (i < row1 && j > row1)
	      || (i < row2 && j > row2)
	      || (i - count < row1 && i >= row1)
	      || (i - count > row2 && i <= row2)
	      || (j - count < row1 && j >= row1)
	      || (j - count > row2 && j <= row2)
	   )
	{
	    CLEAR_ALL_SELECTION(r);
	    /* XXX: too aggressive? */
	    SEL(r).op = SELECTION_CLEAR;
	}
	else if (j >= row1 && j <= row2)
	{
	    /* move selected region too */
	    SEL(r).beg.row -= count;
	    SEL(r).end.row -= count;
	    SEL(r).mark.row -= count;

	    if (SEL(r).mark.row < -SVLINES)
		SEL(r).mark.row = -SVLINES;
	    if (SEL(r).beg.row < -SVLINES)
		SEL(r).beg.row = -SVLINES;
	    if (SEL(r).end.row < -SVLINES)
		SEL(r).end.row = -SVLINES;
	}
    }

    /* _after_ PVTS(r)->nscrolled update */
    rxvt_selection_check(r, 0);

    PVTS(r)->num_scr += count;
    j = count;
    if (count < 0)
	count = -count;
    i = row2 - row1 + 1;
    MIN_IT(count, i);

    if (j > 0)
    {
	/* A: scroll up */

	/* A1: Copy lines that will get clobbered by the rotation */
	for (i = count - 1, j = row1; i >= 0; i--, j++)
	{
	    PVTS(r)->buf_text[i] = PSCR(r).text[j];
	    PVTS(r)->buf_rend[i] = PSCR(r).rend[j];
	}

	/* A2: Rotate lines */
	size = sizeof(*PSCR(r).tlen);
	MEMMOVE(&(PSCR(r).tlen[row1]), &(PSCR(r).tlen[row1+count]),
			(row2 - row1 - count + 1) * size);
	size = sizeof(*PSCR(r).text);
	MEMMOVE(&(PSCR(r).text[row1]), &(PSCR(r).text[row1+count]),
			(row2 - row1 - count + 1) * size);
	size = sizeof(*PSCR(r).rend);
	MEMMOVE(&(PSCR(r).rend[row1]), &(PSCR(r).rend[row1+count]),
			(row2 - row1 - count + 1) * size);

	j = row2 - count + 1;
	ret = i = count;
    }
    else /* if (j < 0) */
    {
	/* B: scroll down */

	/* B1: Copy lines that will get clobbered by the rotation */
	size = sizeof(*PSCR(r).text);
	MEMCPY(PVTS(r)->buf_text, &PSCR(r).text[row2 - count + 1], count * size);
	size = sizeof(*PSCR(r).rend);
	MEMCPY(PVTS(r)->buf_rend, &PSCR(r).rend[row2 - count + 1], count * size);

	/* B2: Rotate lines */
	size = sizeof(*PSCR(r).tlen);
	MEMMOVE(&(PSCR(r).tlen[row1 + count]), &(PSCR(r).tlen[row1]),
			(row2 - row1 - count + 1) * size);
	size = sizeof(*PSCR(r).text);
	MEMMOVE(&(PSCR(r).text[row1 + count]), &(PSCR(r).text[row1]),
			(row2 - row1 - count + 1) * size);
	size = sizeof(*PSCR(r).rend);
	MEMMOVE(&(PSCR(r).rend[row1 + count]), &(PSCR(r).rend[row1]),
			(row2 - row1 - count + 1) * size);

	j = row1, i = count;
	ret = -count;
    }

    /* C: Resurrect lines */
    size = sizeof(*PSCR(r).tlen);
    MEMSET(&PSCR(r).tlen[j], 0, count * size);
    size = sizeof(*PSCR(r).text);
    MEMCPY(&PSCR(r).text[j], PVTS(r)->buf_text, count * size);
    size = sizeof(*PSCR(r).rend);
    MEMCPY(&PSCR(r).rend[j], PVTS(r)->buf_rend, count * size);

    for (; i--; j++)
    {
	if (!spec)	/* line length may not equal TermWin.ncol */
	    rxvt_blank_screen_mem(r, PSCR(r).text,
		    PSCR(r).rend, (unsigned int)j,
		    PVTS(r)->rstyle);
    }

    return ret;
}

/* ------------------------------------------------------------------------- */
/*
 * Adjust the PVTS(r)->view_start so that the if nlines of text are added,
 * the view will not change.
 */
static inline void
adjust_view_start( rxvt_t *r, int nlines)
{
    if(
	 ISSET_OPTION( r, Opt_scrollTtyOutputInhibit) &&
	 VSTART != 0 && VSTART + nlines <= PVTS( r)->nscrolled
      )
	VSTART += nlines;
}

/*
 * Add text given in <str> of length <len> to screen struct
 */
/* EXTPROTO */
void
rxvt_scr_add_lines(rxvt_t* r, const unsigned char *str, int nlines,
	int len)
{
    unsigned char   checksel, clearsel;
    char	    c;
    int		 i, row, last_col;
    text_t	 *stp;
    rend_t	 *srp;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_scr_add_lines( r, %.*s, %d, %d)\n", min(len, 36), str, nlines, len ));

    if (len <= 0)	/* sanity */
	return;

    PVTS(r)->want_refresh = 1;
    last_col = r->TermWin.ncol;

    ZERO_SCROLLBACK(r);
    if (nlines > 0)
    {
	/*
	 * 2006-09-02 gi1242 TODO: The code below is *horrible*. When we call
	 * rxvt_scroll_text(), we might end up with a negative CURROW. We try
	 * and be clever using this information, but rxvt_scr_gotorc() will
	 * reset this information!
	 */
	nlines += (CURROW - PSCR(r).bscroll);
	if (
	      (nlines > 0)
	      && (PSCR(r).tscroll == 0)
	      && (PSCR(r).bscroll == (r->TermWin.nrow - 1))
	   )
	{
	    /* _at least_ this many lines need to be scrolled */
	    rxvt_scroll_text(r, PSCR(r).tscroll,
		PSCR(r).bscroll, nlines, 0);
	    adjust_view_start(r, nlines );

	    CURROW -= nlines;

	    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "\e[32mScrolling %d lines. CURROW=%d\e[0m\n", nlines, CURROW ));
	}
    }

    assert(CURCOL < last_col);
    assert(CURROW < r->TermWin.nrow);

#if 0 /*{{{ Possibly incorrection assertion */
    /*
     * XXX 2006-09-12 gi1242: I think this assertion is wrong! Note that a few
     * lines later we set CURROW to be the max of CURROW and -PVTS()->nscrolled
     */
    assert(CURROW >= -(int32_t)PVTS(r)->nscrolled);
#endif /*}}}*/

    MIN_IT(CURCOL, last_col - 1);
    MIN_IT(CURROW, (int32_t)r->TermWin.nrow - 1);
    MAX_IT(CURROW, -(int32_t)PVTS(r)->nscrolled);

    row = CURROW + SVLINES;

    checksel = (SEL(r).op &&
	PVTS(r)->current_screen == SEL(r).screen) ? 1 : 0;
    clearsel = 0;

    stp = PSCR(r).text[row];
    srp = PSCR(r).rend[row];

#ifdef MULTICHAR_SET
    if(
	 PVTS(r)->lost_multi && CURCOL > 0 &&
	 IS_MULTI1(srp[CURCOL - 1]) &&
	 *str != '\n' && *str != '\r' && *str != '\t'
      )
    {
	PVTS(r)->chstat = WBYTE;
    }
#endif

    for (i = 0; i < len;)
    {
	c = str[i++];
	switch (c)
	{
	    case '\t':
		rxvt_scr_tab(r, 1);
		continue;

	    case '\n':
		/* XXX: think about this */
		if( PSCR(r).tlen[row] != -1 )
		    MAX_IT(PSCR(r).tlen[row], CURCOL);

		PSCR(r).flags &= ~Screen_WrapNext;
		if (CURROW == PSCR(r).bscroll)
		{
		    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "%s:%d ",
				__FILE__, __LINE__ ));
		    rxvt_scroll_text(r, PSCR(r).tscroll,
			    PSCR(r).bscroll, 1, 0);
		    adjust_view_start( r, 1 );
		}
		else if (CURROW < (r->TermWin.nrow - 1))
		    row = (++CURROW) + SVLINES;

		stp = PSCR(r).text[row];  /* _must_ refresh */
		srp = PSCR(r).rend[row];  /* _must_ refresh */
		RESET_CHSTAT(r);
		continue;

	    case '\r':
		/* XXX: think about this */
		if (PSCR(r).tlen[row] != -1)
		    MAX_IT(PSCR(r).tlen[row], CURCOL);
		PSCR(r).flags &= ~Screen_WrapNext;
		CURCOL = 0;
		RESET_CHSTAT(r);
		continue;

	    default:
#ifdef MULTICHAR_SET
		if( r->encoding_method == ENC_NOENC )
		{
		    /* 2009-03-29 gi1242 TODO: Deal with ISO-8859 encodings. */
		    if (c == 127)
			continue;
		    break;
		}

		PVTS(r)->rstyle &= ~RS_multiMask;

		/* multibyte 2nd byte */
		if (PVTS(r)->chstat == WBYTE)
		{
		    /* set flag of second byte in style */
		    PVTS(r)->rstyle |= RS_multi2;
		    /* switch back to single byte for next char */
		    PVTS(r)->chstat = SBYTE;
		    if (
			  (r->encoding_method == ENC_EUCJ)
			  && ((char) stp[CURCOL-1] == (char) 0x8e)
		       )
		    {
			PVTS(r)->rstyle &= ~RS_multiMask;
			CURCOL --;
		    }
		    else
		    /* maybe overkill, but makes it selectable */
		    if ((r->encoding_method == ENC_EUCJ) ||
			(r->encoding_method == ENC_GBK) ||
			(r->encoding_method == ENC_GB))
			c |= 0x80;
		}
		/* multibyte 1st byte */
		else if (PVTS(r)->chstat == SBYTE)
		{
		    if (r->encoding_method == ENC_SJIS)
		    {
			if (
			      PVTS(r)->multi_byte
			      || (
				  (
				   (unsigned char) c >= (unsigned char) 0x81
				   && (unsigned char) c <= (unsigned char) 0x9f
				  )
				  ||
				  (
				   (unsigned char) c >= (unsigned char) 0xe0
				   && (unsigned char) c <= (unsigned char) 0xfc
				  )
				 )
			   )
			{
			    PVTS(r)->rstyle |= RS_multi1;
			    PVTS(r)->chstat = WBYTE;
			}
		    }
		    else if (PVTS(r)->multi_byte || (c & 0x80))
		    {
			/* set flag of first byte in style */
			PVTS(r)->rstyle |= RS_multi1;
			/* switch to multiple byte for next char */
			PVTS(r)->chstat = WBYTE;
			/* maybe overkill, but makes selectable */
			if (
			      (r->encoding_method == ENC_EUCJ)
			      || (r->encoding_method == ENC_GBK)
			      || (r->encoding_method == ENC_GB)
			   )
			    c |= 0x80;
		    }
		}
		else
#endif /*MULTICHAR_SET*/
		if (c == 127)
		    continue;	/* yummmm..... */
		break;
	}   /* switch */

	if (
	      checksel	    /* see if we're writing within selection */
	      && !RC_BEFORE(PSCR(r).cur, SEL(r).beg)
	      && RC_BEFORE(PSCR(r).cur, SEL(r).end)
	   )
	{
	    checksel = 0;
	    clearsel = 1;
	}

	if (PSCR(r).flags & Screen_WrapNext)
	{
	    PSCR(r).tlen[row] = -1;
	    if (CURROW == PSCR(r).bscroll)
	    {
		rxvt_scroll_text(r, PSCR(r).tscroll,
			PSCR(r).bscroll, 1, 0);
		adjust_view_start( r, 1 );
	    }
	    else if (CURROW < (r->TermWin.nrow - 1))
		row = (++CURROW) + SVLINES;
	    stp = PSCR(r).text[row];  /* _must_ refresh */
	    srp = PSCR(r).rend[row];  /* _must_ refresh */
	    CURCOL = 0;
	    PSCR(r).flags &= ~Screen_WrapNext;
	}

	if (PSCR(r).flags & Screen_Insert)
	    rxvt_scr_insdel_chars(r, 1, INSERT);

#ifdef MULTICHAR_SET
	if (
	      IS_MULTI1(PVTS(r)->rstyle)
	      && CURCOL > 0
	      && IS_MULTI1(srp[CURCOL - 1])
	   )
	{
	    stp[CURCOL - 1] = ' ';
	    srp[CURCOL - 1] &= ~RS_multiMask;
	}
	else if (
		  IS_MULTI2(PVTS(r)->rstyle)
		  && CURCOL < (last_col - 1)
		  && IS_MULTI2(srp[CURCOL + 1])
		)
	{
	    stp[CURCOL + 1] = ' ';
	    srp[CURCOL + 1] &= ~RS_multiMask;
	}
#endif

	stp[CURCOL] = c;
	srp[CURCOL] = PVTS(r)->rstyle;
	if (CURCOL < (last_col - 1))
	    CURCOL++;
	else
	{
	    PSCR(r).tlen[row] = last_col;
	    if (PSCR(r).flags & Screen_Autowrap)
		PSCR(r).flags |= Screen_WrapNext;
	}
    }	/* for */

    if (PSCR(r).tlen[row] != -1)	/* XXX: think about this */
	MAX_IT(PSCR(r).tlen[row], CURCOL);

    /*
    ** If we wrote anywhere in the selected area, kill the selection
    ** XXX: should we kill the mark too?  Possibly, but maybe that
    **	  should be a similar check.
    */
    if (clearsel)
	CLEAR_SELECTION(r);

    assert(CURROW >= 0);
    MAX_IT(CURROW, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Process Backspace.  Move back the cursor back a position, wrap if have to
 * XTERM_SEQ: CTRL-H
 */
/* EXTPROTO */
void
rxvt_scr_backspace(rxvt_t* r)
{
    RESET_CHSTAT(r);
    PVTS(r)->want_refresh = 1;
    if (CURCOL == 0)
    {
	if (CURROW > 0)
	{
#ifdef TERMCAP_HAS_BW
	    CURCOL = r->TermWin.ncol - 1;
	    CURROW--;
	    return;
#endif
	}
    } else if ((PSCR(r).flags & Screen_WrapNext) == 0)
	rxvt_scr_gotorc(r, 0, -1, RELATIVE);
    PSCR(r).flags &= ~Screen_WrapNext;
}

/* ------------------------------------------------------------------------- */
/*
 * Process Horizontal Tab
 * count: +ve = forward; -ve = backwards
 * XTERM_SEQ: CTRL-I
 */
/* EXTPROTO */
void
rxvt_scr_tab(rxvt_t* r, int count)
{
    int		 i, x;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_scr_tab (%d)\n", count));
    PVTS(r)->want_refresh = 1;
    RESET_CHSTAT(r);
    i = x = CURCOL;
    if (count == 0)
	return;
    else if (count > 0)
    {
	for (; ++i < r->TermWin.ncol; )
	    if (r->tabstop[i])
	    {
		x = i;
		if (!--count)
		    break;
	    }
	;
	if (count)
	    x = r->TermWin.ncol - 1;
    }
    else /* if (count < 0) */
    {
	for (; --i >= 0; )
	    if (r->tabstop[i])
	    {
		x = i;
		if (!++count)
		    break;
	    }
	;
	if (count)
	    x = 0;
    }

#if 0
    if (x != CURCOL)
	rxvt_scr_gotorc(r, 0, x, R_RELATIVE);
#else
    /*
     * 2006-09-02 gi1242: Don't call rxvt_scr_gotorc() because that might change
     * CURROW (if it was negative). If we're adding lines to the screen
     * structure, then CURROW is allowed to be negative.
     */
    CURCOL = x;
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Process DEC Back Index
 * XTERM_SEQ: ESC 6
 * Move cursor left in row.  If we're at the left boundary, shift everything
 * in that row right.  Clear left column.
 */
#ifndef NO_FRILLS
/* EXTPROTO */
void
rxvt_scr_backindex(rxvt_t* r)
{
    if (CURCOL > 0)
	rxvt_scr_gotorc(r, 0, -1, R_RELATIVE | C_RELATIVE);
    else
    {
	if (PSCR(r).tlen[CURROW + SVLINES] == 0)
	    return;	/* um, yeah? */
	rxvt_scr_insdel_chars(r, 1, INSERT);
    }
}
#endif
/* ------------------------------------------------------------------------- */
/*
 * Process DEC Forward Index
 * XTERM_SEQ: ESC 9
 * Move cursor right in row.  If we're at the right boundary, shift everything
 * in that row left.  Clear right column.
 */
#ifndef NO_FRILLS
/* EXTPROTO */
void
rxvt_scr_forwardindex(rxvt_t* r)
{
    int		 row;

    if (CURCOL < r->TermWin.ncol - 1)
	rxvt_scr_gotorc(r, 0, 1, R_RELATIVE | C_RELATIVE);
    else
    {
	row = CURROW + SVLINES;
	if (PSCR(r).tlen[row] == 0)
	    return;	/* um, yeah? */
	else if (PSCR(r).tlen[row] == -1)
	    PSCR(r).tlen[row] = r->TermWin.ncol;
	rxvt_scr_gotorc(r, 0, 0, R_RELATIVE);
	rxvt_scr_insdel_chars(r, 1, DELETE);
	rxvt_scr_gotorc(r, 0, r->TermWin.ncol - 1, R_RELATIVE);
    }
}
#endif

/* ------------------------------------------------------------------------- */
/*
 * Goto Row/Column
 */
/* EXTPROTO */
void
rxvt_scr_gotorc(rxvt_t* r, int row, int col, int relative)
{
    PVTS(r)->want_refresh = 1;
    ZERO_SCROLLBACK(r);
    RESET_CHSTAT(r);

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_gotorc (r:%s%d,c:%s%d): from (r:%d,c:%d)\n", (relative & R_RELATIVE ? "+" : ""), row, (relative & C_RELATIVE ? "+" : ""), col, CURROW, CURCOL));

    CURCOL = ((relative & C_RELATIVE) ? (CURCOL + col)
			 : col);
    MAX_IT(CURCOL, 0);
    MIN_IT(CURCOL, (int32_t)r->TermWin.ncol - 1);

    PSCR(r).flags &= ~Screen_WrapNext;
    if (relative & R_RELATIVE)
    {
	if (row > 0)
	{
	    if (CURROW <= PSCR(r).bscroll &&
		(CURROW + row) > PSCR(r).bscroll)
		CURROW = PSCR(r).bscroll;
	    else
		CURROW += row;
	}

	else if (row < 0)
	{
	    if (CURROW >= PSCR(r).tscroll &&
		(CURROW + row) < PSCR(r).tscroll)
		CURROW = PSCR(r).tscroll;
	    else
		CURROW += row;
	}
    }
    else
    {
	if (PSCR(r).flags & Screen_Relative)
	{
	    /* relative origin mode */
	    CURROW = row + PSCR(r).tscroll;
	    MIN_IT(CURROW, PSCR(r).bscroll);
	}
	else
	    CURROW = row;
    }
    MAX_IT(CURROW, 0);
    MIN_IT(CURROW, (int32_t)r->TermWin.nrow - 1);
}

/* ------------------------------------------------------------------------- */
/*
 * direction  should be UP or DN
 */
/* EXTPROTO */
void
rxvt_scr_index(rxvt_t* r, enum page_dirn direction)
{
    int		 dirn;

    PVTS(r)->want_refresh = 1;
    dirn = ((direction == UP) ? 1 : -1);
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_index (%d)\n", dirn));

    ZERO_SCROLLBACK(r);
    RESET_CHSTAT(r);

    PSCR(r).flags &= ~Screen_WrapNext;
    if ((CURROW == PSCR(r).bscroll && direction == UP) ||
	(CURROW == PSCR(r).tscroll && direction == DN))
	rxvt_scroll_text(r, PSCR(r).tscroll,
		PSCR(r).bscroll, dirn, 0);
    else
	CURROW += dirn;
    MAX_IT(CURROW, 0);
    MIN_IT(CURROW, (int32_t)r->TermWin.nrow - 1);
    rxvt_selection_check(r, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Erase part or whole of a line
 * XTERM_SEQ: Clear line to right: ESC [ 0 K
 * XTERM_SEQ: Clear line to left : ESC [ 1 K
 * XTERM_SEQ: Clear whole line   : ESC [ 2 K
 */
/* EXTPROTO */
void
rxvt_scr_erase_line(rxvt_t* r, int mode)
{
    unsigned int    row, col, num;

    PVTS(r)->want_refresh = 1;
    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_scr_erase_line (%d) at screen row: %d\n", mode, CURROW));
    ZERO_SCROLLBACK(r);
    RESET_CHSTAT(r);
    rxvt_selection_check(r, 1);

    PSCR(r).flags &= ~Screen_WrapNext;

    row = SVLINES + CURROW;
    switch (mode)
    {
	case 0:		/* erase to end of line */
	    col = CURCOL;
	    num = r->TermWin.ncol - col;
	    MIN_IT(PSCR(r).tlen[row], (int16_t)col);
	    if (
		  RC_ROW_ATAFTER(SEL(r).beg, PSCR(r).cur)
		  || RC_ROW_ATAFTER(SEL(r).end, PSCR(r).cur)
	       )
		CLEAR_SELECTION(r);
	    break;
	case 1:		/* erase to beginning of line */
	    col = 0;
	    num = CURCOL + 1;
	    if (
		  RC_ROW_ATBEFORE(SEL(r).beg, PSCR(r).cur)
		  || RC_ROW_ATBEFORE(SEL(r).end, PSCR(r).cur)
	       )
		CLEAR_SELECTION(r);
	    break;
	case 2:		/* erase whole line */
	    col = 0;
	    num = r->TermWin.ncol;
	    PSCR(r).tlen[row] = 0;
	    if (SEL(r).beg.row <= CURROW && SEL(r).end.row >= CURROW)
		CLEAR_SELECTION(r);
	    break;
	default:
	    return;
    }

    if (PSCR(r).text[row])
	rxvt_blank_line(&(PSCR(r).text[row][col]),
	    &(PSCR(r).rend[row][col]), num, PVTS(r)->rstyle);
    else
	rxvt_blank_screen_mem(r, PSCR(r).text,
	    PSCR(r).rend, row, PVTS(r)->rstyle);
}

/* ------------------------------------------------------------------------- */
/*
 * Erase part of whole of the screen
 * XTERM_SEQ: Clear screen after cursor : ESC [ 0 J
 * XTERM_SEQ: Clear screen before cursor: ESC [ 1 J
 * XTERM_SEQ: Clear whole screen	: ESC [ 2 J
 */
/* EXTPROTO */
void
rxvt_scr_erase_screen(rxvt_t* r, int mode)
{
    int		num;
    int32_t	row, row_offset;
    rend_t	ren;
    XGCValues	gcvalue;

    PVTS(r)->want_refresh = 1;
    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_scr_erase_screen (%d) at screen row: %d\n", mode, CURROW));
    ZERO_SCROLLBACK(r);
    RESET_CHSTAT(r);
    row_offset = (int32_t)SVLINES;

    switch (mode)
    {
	case 0:		/* erase to end of screen */
	    rxvt_selection_check(r, 1);
	    rxvt_scr_erase_line(r, 0);
	    row = CURROW + 1;	/* possible OOB */
	    num = r->TermWin.nrow - row;
	    break;
	case 1:		/* erase to beginning of screen */
	    rxvt_selection_check(r, 3);
	    rxvt_scr_erase_line(r, 1);
	    row = 0;
	    num = CURROW;
	    break;
	case 2:		/* erase whole screen */
	    /*
	     * 2006-02-15 gi1242: As pointed out by Sabit Sayeed, Gnome terminal
	     * scrolls the text off screen, instead of wiping it out completely.
	     * That's seems much better so let's do it here.
	     */
	    if( PVTS(r)->current_screen == PRIMARY )
	    {
		/*
		 * Only scroll if the primary screen is bieng cleared.
		 */
		int sr;

		/*
		 * Find the last non-empty line to save.
		 */
		for( sr = SVLINES + r->TermWin.nrow - 1; sr >= SVLINES; sr--)
		{
		    int non_empty = 0, sc;

		    for( sc = 0; sc < PSCR( r).tlen[sr]; sc++)
			if (
			      PSCR( r).text[sr][sc] != '\0'
			      && PSCR( r).text[sr][sc] != ' '
			   )
			{
			    non_empty = 1;
			    break;
			}
		    ;
		    if( non_empty ) break;
		}

		sr -=  SVLINES /* - 1 */;   /* Dump last non-empty line */
		rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "Saving %d lines\n", sr));

		if( sr > 0)
		    rxvt_scroll_text(r,
			    PSCR(r).tscroll, PSCR(r).bscroll,
			    sr, 0);
	    }
	    else rxvt_selection_check(r, 3);

	    row = 0;
	    num = r->TermWin.nrow;
	    break;
	default:
	    return;
    }

    r->h.refresh_type |= REFRESH_BOUNDS;
    if(
	 SEL(r).op &&
	 PVTS(r)->current_screen == SEL(r).screen &&
	 (
	   (SEL(r).beg.row >= row && SEL(r).beg.row <= row + num) ||
	   (SEL(r).end.row >= row && SEL(r).end.row <= row + num)
	 )
      )
    {
	CLEAR_SELECTION(r);
    }

    if (row >= r->TermWin.nrow)	/* Out Of Bounds */
	return;

    MIN_IT(num, (r->TermWin.nrow - row));
    if (PVTS(r)->rstyle & (RS_RVid | RS_Uline))
	ren = (rend_t) ~RS_None;

    else if (GET_BASEBG(PVTS(r)->rstyle) == Color_bg)
    {
	ren = DEFAULT_RSTYLE;
	CLEAR_ROWS(row, num);
    }

    else
    {
	ren = (PVTS(r)->rstyle & (RS_fgMask | RS_bgMask));

	gcvalue.foreground = r->pixColors[GET_BGCOLOR(PVTS(r)->rstyle)];
	XChangeGC(r->Xdisplay, r->TermWin.gc, GCForeground, &gcvalue);
	ERASE_ROWS(row, num);

	gcvalue.foreground = r->pixColors[Color_fg];
	XChangeGC(r->Xdisplay, r->TermWin.gc, GCForeground, &gcvalue);
    }

    for (; num--; row++)
    {
	rxvt_blank_screen_mem(r, PSCR(r).text,
		PSCR(r).rend,
		(unsigned int)(row + row_offset), PVTS(r)->rstyle);
	PSCR(r).tlen[row + row_offset] = 0;
	rxvt_blank_line(PVTS(r)->drawn_text[row],
		PVTS(r)->drawn_rend[row],
		(unsigned int) r->TermWin.ncol, ren);
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Fill the screen with `E's
 * XTERM_SEQ: Screen Alignment Test: ESC # 8
 */
/* EXTPROTO */
void
rxvt_scr_E(rxvt_t* r)
{
    int		 i, j, k;
    rend_t	 *r1, fs;

    PVTS(r)->want_refresh = 1;
    r->h.num_scr_allow = 0;
    ZERO_SCROLLBACK(r);
    RESET_CHSTAT(r);
    rxvt_selection_check(r, 3);

    fs = PVTS(r)->rstyle;
    for (k = SVLINES, i = r->TermWin.nrow; i--; k++)
    {
	/* make the `E's selectable */
	PSCR(r).tlen[k] = r->TermWin.ncol;
	MEMSET(PSCR(r).text[k], 'E', r->TermWin.ncol);
	for (r1 = PSCR(r).rend[k], j = r->TermWin.ncol; j--; )
	    *r1++ = fs;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Insert/Delete <count> lines
 */
/* EXTPROTO */
void
rxvt_scr_insdel_lines(rxvt_t* r, int count, int insdel)
{
    int		 end;

    ZERO_SCROLLBACK(r);
    RESET_CHSTAT(r);
    rxvt_selection_check(r, 1);

    if (CURROW > PSCR(r).bscroll)
	return;

    end = PSCR(r).bscroll - CURROW + 1;
    if (count > end)
    {
	if (insdel == DELETE)
	    return;
	else if (insdel == INSERT)
	    count = end;
    }
    PSCR(r).flags &= ~Screen_WrapNext;

    rxvt_scroll_text(r, CURROW,
	PSCR(r).bscroll, insdel * count, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Insert/Delete <count> characters from the current position
 */
/* EXTPROTO */
void
rxvt_scr_insdel_chars(rxvt_t* r, int count, int insdel)
{
    int		col, row;
    rend_t	tr;
    text_t*	stp;
    rend_t*	srp;
    int16_t*	slp;

    PVTS(r)->want_refresh = 1;
    ZERO_SCROLLBACK(r);
#if 0
    RESET_CHSTAT(r);
#endif
    if (count <= 0)
	return;

    rxvt_selection_check(r, 1);
    MIN_IT(count, (r->TermWin.ncol - CURCOL));

    row = CURROW + SVLINES;
    PSCR(r).flags &= ~Screen_WrapNext;

    stp = PSCR(r).text[row];
    srp = PSCR(r).rend[row];
    slp = &(PSCR(r).tlen[row]);
    switch (insdel)
    {
	case INSERT:
	    for (col = r->TermWin.ncol - 1; (col - count) >= CURCOL; col--)
	    {
		stp[col] = stp[col - count];
		srp[col] = srp[col - count];
	    }
	    if (*slp != -1)
	    {
		*slp += count;
		MIN_IT(*slp, r->TermWin.ncol);
	    }
	    if (
		  SEL(r).op
		  && PVTS(r)->current_screen == SEL(r).screen
		  && RC_ROW_ATAFTER(SEL(r).beg, PSCR(r).cur)
	       )
	    {
		if (
		      SEL(r).end.row != CURROW
		      || (SEL(r).end.col + count >= r->TermWin.ncol)
		   )
		    CLEAR_SELECTION(r);
		else	    /* shift selection */
		{
		    SEL(r).beg.col += count;
		    SEL(r).mark.col += count;	/* XXX: yes? */
		    SEL(r).end.col += count;
		}
	    }
	    rxvt_blank_line(&(stp[CURCOL]),
		&(srp[CURCOL]),
		(unsigned int)count, PVTS(r)->rstyle);
	    break;

	case ERASE:
	    CURCOL += count;	/* don't worry if > r->TermWin.ncol */
	    rxvt_selection_check(r, 1);
	    CURCOL -= count;
	    rxvt_blank_line(&(stp[CURCOL]),
		&(srp[CURCOL]),
		(unsigned int)count, PVTS(r)->rstyle);
	    break;

	case DELETE:
	    tr = srp[r->TermWin.ncol - 1]
		 & (RS_fgMask | RS_bgMask | RS_baseattrMask);
	    for (col = CURCOL; (col + count) < r->TermWin.ncol; col++)
	    {
		stp[col] = stp[col + count];
		srp[col] = srp[col + count];
	    }
	    rxvt_blank_line(&(stp[r->TermWin.ncol - count]),
		&(srp[r->TermWin.ncol - count]),
		(unsigned int)count, tr);
	    if (*slp == -1) /* break line continuation */
		*slp = r->TermWin.ncol;
	    *slp -= count;
	    MAX_IT(*slp, 0);
	    if (
		  SEL(r).op
		  && PVTS(r)->current_screen == SEL(r).screen
		  && RC_ROW_ATAFTER(SEL(r).beg, PSCR(r).cur)
	       )
	    {
		if (
		      SEL(r).end.row != CURROW
		      || (CURCOL >= SEL(r).beg.col - count)
		      || SEL(r).end.col >= r->TermWin.ncol
		   )
		    CLEAR_SELECTION(r);
		else
		{
		    /* shift selection */
		    SEL(r).beg.col -= count;
		    SEL(r).mark.col -= count;	/* XXX: yes? */
		    SEL(r).end.col -= count;
		}
	    }
	    break;
    }
#if 0
    if (IS_MULTI2(srp[0]))
    {
	srp[0] &= ~RS_multiMask;
	stp[0] = ' ';
    }
    if (IS_MULTI1(srp[r->TermWin.ncol - 1]))
    {
	srp[r->TermWin.ncol - 1] &= ~RS_multiMask;
	stp[r->TermWin.ncol - 1] = ' ';
    }
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Set the scrolling region
 * XTERM_SEQ: Set region <top> - <bot> inclusive: ESC [ <top> ; <bot> r
 */
/* EXTPROTO */
void
rxvt_scr_scroll_region(rxvt_t* r, int top, int bot)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN,
		"rxvt_scr_scroll_region( %d, %d)\n", top, bot));

    MAX_IT(top, 0);
    MIN_IT(bot, (int)r->TermWin.nrow - 1);
    if (top > bot)
	return;
    PSCR(r).tscroll = top;
    PSCR(r).bscroll = bot;
    rxvt_scr_gotorc(r, 0, 0, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Make the cursor visible/invisible
 * XTERM_SEQ: Make cursor visible  : ESC [ ? 25 h
 * XTERM_SEQ: Make cursor invisible: ESC [ ? 25 l
 */
/* EXTPROTO */
void
rxvt_scr_cursor_visible(rxvt_t* r, int mode)
{
    PVTS(r)->want_refresh = 1;
    if (mode)
	PSCR(r).flags |= Screen_VisibleCursor;
    else
	PSCR(r).flags &= ~Screen_VisibleCursor;
}

/* ------------------------------------------------------------------------- */
/*
 * Set/unset automatic wrapping
 * XTERM_SEQ: Set Wraparound  : ESC [ ? 7 h
 * XTERM_SEQ: Unset Wraparound: ESC [ ? 7 l
 */
/* EXTPROTO */
void
rxvt_scr_autowrap(rxvt_t* r, int mode)
{
    if (mode)
	PSCR(r).flags |= Screen_Autowrap;
    else
	PSCR(r).flags &= ~(Screen_Autowrap | Screen_WrapNext);
}

/* ------------------------------------------------------------------------- */
/*
 * Set/unset margin origin mode
 * Absolute mode: line numbers are counted relative to top margin of screen
 *    and the cursor can be moved outside the scrolling region.
 * Relative mode: line numbers are relative to top margin of scrolling region
 *    and the cursor cannot be moved outside.
 * XTERM_SEQ: Set Absolute: ESC [ ? 6 h
 * XTERM_SEQ: Set Relative: ESC [ ? 6 l
 */
/* EXTPROTO */
void
rxvt_scr_relative_origin(rxvt_t* r, int mode)
{
    if (mode)
	PSCR(r).flags |= Screen_Relative;
    else
	PSCR(r).flags &= ~Screen_Relative;
    rxvt_scr_gotorc(r, 0, 0, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Set insert/replace mode
 * XTERM_SEQ: Set Insert mode : ESC [ ? 4 h
 * XTERM_SEQ: Set Replace mode: ESC [ ? 4 l
 */
/* EXTPROTO */
void
rxvt_scr_insert_mode(rxvt_t* r, int mode)
{
    if (mode)
	PSCR(r).flags |= Screen_Insert;
    else
	PSCR(r).flags &= ~Screen_Insert;
}

/* ------------------------------------------------------------------------- */
/*
 * Set/Unset tabs
 * XTERM_SEQ: Set tab at current column  : ESC H
 * XTERM_SEQ: Clear tab at current column: ESC [ 0 g
 * XTERM_SEQ: Clear all tabs		 : ESC [ 3 g
 */
/* EXTPROTO */
void
rxvt_scr_set_tab(rxvt_t* r, int mode)
{
    if (mode < 0)
	MEMSET(r->tabstop, 0, r->TermWin.ncol * sizeof(char));
    else if (PSCR(r).cur.col < r->TermWin.ncol)
	r->tabstop[PSCR(r).cur.col] = (mode ? 1 : 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Set reverse/normal video
 * XTERM_SEQ: Reverse video: ESC [ ? 5 h
 * XTERM_SEQ: Normal video : ESC [ ? 5 l
 */
/* EXTPROTO */
void
rxvt_scr_rvideo_mode(rxvt_t* r, int mode)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "%s(r, mode=%d)\n", __func__, mode ));

    if (PVTS(r)->rvideo != mode)
    {
	PVTS(r)->rvideo = mode;

	SWAP_IT( PVTS(r)->p_fg, PVTS(r)->p_bg );
#ifdef XFT_SUPPORT
	if( ISSET_OPTION( r, Opt_xft ) )
	    SWAP_IT( PVTS(r)->p_xftfg, PVTS(r)->p_xftbg );
#endif
	if( r->TermWin.fade )
	{
	    SWAP_IT( PVTS(r)->p_fgfade, PVTS(r)->p_bgfade );
#ifdef XFT_SUPPORT
	    if( ISSET_OPTION( r, Opt_xft ) )
		SWAP_IT( PVTS(r)->p_xftfgfade, PVTS(r)->p_xftbgfade );
#endif
	}

	    /* Background colors need to be forcibly reset */
	    rxvt_set_vt_colors( r );

	rxvt_scr_clear( r );
	rxvt_scr_touch( r, True );
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Report current cursor position
 * XTERM_SEQ: Report position: ESC [ 6 n
 */
/* EXTPROTO */
void
rxvt_scr_report_position(rxvt_t* r)
{
    rxvt_tt_printf(r, "\033[%d;%dR", CURROW + 1, CURCOL + 1);
}

/* ------------------------------------------------------------------------- *
 *				  FONTS					*
 * ------------------------------------------------------------------------- */

/*
 * Set font style
 */
/* INTPROTO */
static void
rxvt_set_font_style(rxvt_t *r)
{
    PVTS(r)->rstyle &= ~RS_fontMask;
    switch (PVTS(r)->charsets[PSCR(r).charset])
    {
	case '0':	    /* DEC Special Character & Line Drawing Set */
	    PVTS(r)->rstyle |= RS_acsFont;
	    break;
	case 'A':	    /* United Kingdom (UK) */
	    PVTS(r)->rstyle |= RS_ukFont;
	    break;
	case 'B':	    /* United States (USASCII) */
	    break;
	case '<':	    /* Multinational character set */
	    break;
	case '5':	    /* Finnish character set */
	    break;
	case 'C':	    /* Finnish character set */
	    break;
	case 'K':	    /* German character set */
	    break;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Choose a font
 * XTERM_SEQ: Invoke G0 character set: CTRL-O
 * XTERM_SEQ: Invoke G1 character set: CTRL-N
 * XTERM_SEQ: Invoke G2 character set: ESC N
 * XTERM_SEQ: Invoke G3 character set: ESC O
 */
/* EXTPROTO */
void
rxvt_scr_charset_choose(rxvt_t* r, int set)
{
    PSCR(r).charset = set;
    rxvt_set_font_style(r);
}

/* ------------------------------------------------------------------------- */
/*
 * Set a font
 * XTERM_SEQ: Set G0 character set: ESC ( <C>
 * XTERM_SEQ: Set G1 character set: ESC ) <C>
 * XTERM_SEQ: Set G2 character set: ESC * <C>
 * XTERM_SEQ: Set G3 character set: ESC + <C>
 * See set_font_style for possible values for <C>
 */
/* EXTPROTO */
void
rxvt_scr_charset_set(rxvt_t* r, int set, unsigned int ch)
{
#ifdef MULTICHAR_SET
    PVTS(r)->multi_byte = !!(set < 0);
    set = abs(set);
#endif
    PVTS(r)->charsets[set] = (unsigned char)ch;
    rxvt_set_font_style(r);
}


/* ------------------------------------------------------------------------- *
 *			MAJOR SCREEN MANIPULATION			  *
 * ------------------------------------------------------------------------- */

/*
 * Refresh an area
 */
enum
{
    PART_BEG = 0,
    PART_END,
    RC_COUNT
};

/* EXTPROTO */
void
rxvt_scr_expose(rxvt_t* r,
	int x, int y, int width, int height,
	Bool refresh)
{
    int		 i;
    row_col_t	   rc[RC_COUNT];

    if (PVTS(r)->drawn_text == NULL)  /* sanity check */
	return;

    x = max(x, (int)r->TermWin.int_bwidth);
    x = min(x, (int)r->szHint.width);
    y = max(y, (int)r->TermWin.int_bwidth);
    y = min(y, (int)r->szHint.height);

    /* round down */
    rc[PART_BEG].col = Pixel2Col(x);
    rc[PART_BEG].row = Pixel2Row(y);
    /* round up */
    rc[PART_END].col = Pixel2Width(x + width + r->TermWin.fwidth - 1);
    rc[PART_END].row = Pixel2Row(y + height + r->TermWin.fheight - 1);

    /* sanity checks */
    for (i = PART_BEG; i < RC_COUNT; i++)
    {
	MIN_IT(rc[i].col, r->TermWin.ncol - 1);
	MIN_IT(rc[i].row, r->TermWin.nrow - 1);
    }

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_expose (x:%d, y:%d, w:%d, h:%d) area (c:%d,r:%d)-(c:%d,r:%d)\n", x, y, width, height, rc[PART_BEG].col, rc[PART_BEG].row, rc[PART_END].col, rc[PART_END].row));

	{
		register int	j = rc[PART_BEG].col;
		register int	k = rc[PART_END].col - rc[PART_BEG].col + 1;

		for (i = rc[PART_BEG].row; i <= rc[PART_END].row; i++)
		{
			rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, " memset drawn_text[%d][%d], len=%d\n", i, j, k));
			MEMSET(&(PVTS(r)->drawn_text[i][j]), 0, k);
		}
	 }

    if (refresh)
    {
	rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "Forcing immediate screen refresh"));
	rxvt_scr_refresh(r, SLOW_REFRESH | REFRESH_BOUNDS);
    }
}


/* ------------------------------------------------------------------------- */
/*
 * Refresh the entire screen
 */
/* EXTPROTO */
void
rxvt_scr_touch(rxvt_t* r, Bool refresh)
{
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_touch\n"));
    rxvt_scr_expose(r, 0, 0, VT_WIDTH(r), VT_HEIGHT(r), refresh);
}

/* ------------------------------------------------------------------------- */
/*
 * Move the display so that the line represented by scrollbar value Y is at
 * the top of the screen
 */
/* EXTPROTO */
int
rxvt_scr_move_to(rxvt_t* r, int y, int len)
{
    long	    p = 0;
    uint16_t	    oldviewstart;

    oldviewstart = VSTART;
    if (y < len)
    {
	p = (r->TermWin.nrow + PVTS(r)->nscrolled) * (len - y) / len;
	p -= (long)(r->TermWin.nrow - 1);
	p = max(p, 0);
    }
    VSTART = (uint16_t)min(p, PVTS(r)->nscrolled);
    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_scr_move_to (%d, %d) view_start:%d\n", y, len, VSTART));

    return rxvt_scr_change_view(r, oldviewstart);
}

/* ------------------------------------------------------------------------- */
/*
 * Page the screen up/down nlines
 * direction should be UP or DN
 */
/* EXTPROTO */
int
rxvt_scr_page(rxvt_t* r, enum page_dirn direction, int nlines)
{
    int		n;
    uint16_t	oldviewstart;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_scr_page (%s, %d) view_start:%d\n", ((direction == UP) ? "UP" : "DN"), nlines, VSTART));

    oldviewstart = VSTART;
    if (direction == UP)
    {
	n = VSTART + nlines;
	VSTART = min(n, PVTS(r)->nscrolled);
    }
    else
    {
	n = VSTART - nlines;
	VSTART = max(n, 0);
    }
    return rxvt_scr_change_view(r, oldviewstart);
}


/* INTPROTO */
static int
rxvt_scr_change_view(rxvt_t* r, uint16_t oldviewstart)
{
    if (VSTART != oldviewstart)
    {
	PVTS(r)->want_refresh = 1;
	PVTS(r)->num_scr -= (VSTART - oldviewstart);
    }
    return (int)(VSTART - oldviewstart);
}


/* ------------------------------------------------------------------------- */
/* EXTPROTO */
void
rxvt_scr_bell(rxvt_t *r)
{
#ifndef NO_BELL

#if defined(THROTTLE_BELL_MSEC) && THROTTLE_BELL_MSEC > 0
    /* Maximal number of bell per pre-defined time interval */
    static int		    bellcount	= 0;
    static struct timeval   lastBell	= {0, 0};
    struct timeval	    tvnow	= {0, 0};
    long		    tminterval;

#ifdef HAVE_NANOSLEEP
    struct timespec	    rqt;

    rqt.tv_sec = r->TermWin.vBellDuration / 1000000000ul;
    rqt.tv_nsec = r->TermWin.vBellDuration % 1000000000ul;
#endif

    if (gettimeofday (&tvnow, NULL) >= 0)
    {
	if (0 == lastBell.tv_sec && 0 == lastBell.tv_usec)
	    /* first time bell, try avoid integer overflow */
	    tminterval = 0;

	else
	    tminterval = (tvnow.tv_sec - lastBell.tv_sec) * 1000 +
			(tvnow.tv_usec - lastBell.tv_usec) / 1000;

	lastBell = tvnow;
	if (tminterval > THROTTLE_BELL_MSEC)
	    bellcount = 1;

	else if (bellcount ++ >= THROTTLE_BELL_COUNT)
	    return;
    }
#endif	/* THROTTLE_BELL_MSEC && THROTTLE_BELL_MSEC > 0 */

# ifndef NO_MAPALERT
#  ifdef MAPALERT_OPTION
    if (ISSET_OPTION(r, Opt_mapAlert))
#  endif
	XMapWindow(r->Xdisplay, r->TermWin.parent);
# endif
    if(
	    ISSET_OPTION(r, Opt_visualBell)
      )
    {
	/*
	 * Visual bells don't need to be rung on windows which are not visible.
	 */
	if( r->h.refresh_type == NO_REFRESH )
	    return;

	{
	    /* refresh also done */
	    rxvt_scr_rvideo_mode(r, !PVTS(r)->rvideo);

#ifdef HAVE_NANOSLEEP
	    rxvt_scr_refresh( r,  r->h.refresh_type );
	    XSync( r->Xdisplay, False );
	    if( r->TermWin.vBellDuration )
		nanosleep(&rqt, NULL);
#endif
	    rxvt_scr_rvideo_mode(r, !PVTS(r)->rvideo);
	}
    }

    else
    {
	/*
	 * gi1242: With Xorg-1.7.4 and evdev keyboards, XBell is ignored. Use
	 * XkbBell for now instead. See bug #24503 on freedesktop.
	 */
#ifdef HAVE_X11_XKBLIB_H
	XkbBell( r->Xdisplay, PVTS(r)->vt, 0, None );
#else
	XBell(r->Xdisplay, 0);
#endif
    }
#endif /* NO_BELL */
}

/* ------------------------------------------------------------------------- */

static inline void
rxvt_clear_area (rxvt_t* r, int x, int y, unsigned int w, unsigned int h)
{
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "clear area (%d, %d, %d, %d)\n", x,y,w,h));

    XClearArea (r->Xdisplay, drawBuffer, x, y, w, h, False);
}


static inline void
rxvt_fill_rectangle (rxvt_t* r, int x, int y, unsigned int w, unsigned int h)
{
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "fill rectangle (%d, %d, %d, %d)\n", x,y,w,h));
    XFillRectangle (r->Xdisplay, drawBuffer, r->TermWin.gc, x, y, w, h);
}


/* ------------------------------------------------------------------------- */
/*
 * Refresh the screen
 * PVTS(r)->drawn_text/PVTS(r)->drawn_rend contain the screen information before the update.
 * PSCR(r).text/PSCR(r).rend contain what the screen will change to.
 */


#define X11_DRAW_STRING_8	    (1)
#define X11_DRAW_STRING_16	    (2)
#define X11_DRAW_IMAGE_STRING_8	    (3)
#define X11_DRAW_IMAGE_STRING_16    (4)
#define XFT_DRAW_STRING_8	    (5)
#define XFT_DRAW_STRING_16	    (6)
#define XFT_DRAW_STRING_32	    (7)
#define XFT_DRAW_STRING_UTF8	    (8)
#define XFT_DRAW_IMAGE_STRING_8	    (9)
#define XFT_DRAW_IMAGE_STRING_16    (10)
#define XFT_DRAW_IMAGE_STRING_32    (11)
#define XFT_DRAW_IMAGE_STRING_UTF8  (12)

#ifdef XFT_SUPPORT
#define XFTDRAW_STRING(xdraw, color, font, x, y, str, len)		    \
    ( ( rend & RS_acsFont) ?						    \
      (xftDrawACSString( r->Xdisplay, d, gc,				    \
			 xftdraw_string,				    \
			 (xdraw), (color), (font), (x), (y),		    \
			 (unsigned char*) (str), (len))) :		    \
      (xftdraw_string( (xdraw), (color), (font), (x), (y), (str), (len))))
/*
 * len: number of characters to draw. for UTF-8 string, it is the
 *      number of characters * 2 of the original 16-bits string
 *
 * pfont: Weather to use xftpfn (if defined) or not.
 *
 * refreshRegion: If set, then any changes made to clipping are undone by
 * resetting them to this. (Pass None for no region).
 */
/* EXTPROTO */
static void
rxvt_draw_string_xft (rxvt_t* r, Drawable d, GC gc, Region refreshRegion,
	rend_t rend, int pfont,
	XftDraw* win, XftColor* fore, int x, int y, char* str, int len,
	void (*xftdraw_string)())
{
    XftFont *font;

    /*
     * If "multichar" stuff is needed in tab titles etc, then xftpfont /
     * xftPfont must be multichar capable. If that's not an option, then set
     * xftpfont to NULL, and the correct multichar font will be used.
     */
    if( pfont && r->TermWin.xftpfont )
    {
	font = ( pfont == USE_BOLD_PFONT) ?
	    r->TermWin.xftPfont : r->TermWin.xftpfont;
    }
#ifdef MULTICHAR_SET
    else if( xftdraw_string == XftDrawStringUtf8 )
	font = r->TermWin.xftmfont;
#endif
    else font = r->TermWin.xftfont;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "Draw: 0x%8x %p: '%.40s'\n", rend, font, str ));

#ifdef MULTICHAR_SET
    if( xftdraw_string == XftDrawStringUtf8 )
	len = STRLEN( str);
#endif

    XFTDRAW_STRING (win, fore, font, x, y, str, len);
}
#undef XFTDRAW_STRING
#endif	/* XFT_SUPPORT */


/*
 *  len : number of characters in the string
 */
/* EXTPROTO */
static void
rxvt_draw_string_x11 (rxvt_t* r, Window win, GC gc, Region refreshRegion,
	int x, int y, char* str, int len, int (*draw_string)())
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "output entire string: %s\n", str));
    draw_string (r->Xdisplay, win, gc, x, y, str, len);
}


/* 
** Draw the string:
**    x, y:  top left corner of the string
**    str :  actual string to draw
**    len :  actual number of characters in the string. It is NOT
**           the byte length!
**    drawfunc: function to draw string
**    fore, back: color index to r->pixColors array. It is only
**           used by the XFT drawing, not X11 drawing
**    rend:  rendition value (text attributes). Only used for the RS_acsFont
**	     attribute with Xft for correct drawing of ACS graphics characters.
*/
/* INTPROTO */
static void
rxvt_scr_draw_string (rxvt_t* r,
	int x, int y, char* str, int len, int drawfunc,
	uint16_t fore, uint16_t back,
	__attribute__((unused)) rend_t rend, Region refreshRegion)
{
#ifdef XFT_SUPPORT
    int	    fillback = 0;
    int	    adjust;
    void    (*xftdraw_string) () = NULL;

    switch (drawfunc)
    {
	case	XFT_DRAW_IMAGE_STRING_8:
	    fillback = 1;
	case	XFT_DRAW_STRING_8:
	    xftdraw_string = XftDrawString8; break;

	case	XFT_DRAW_IMAGE_STRING_16:
	    fillback = 1;
	case	XFT_DRAW_STRING_16:
	    xftdraw_string = XftDrawString16; break;
    }

    /*
     * adjust is a variable that records whether each character of the string is
     * 8 bits or 16 bits
     */
    adjust = (XftDrawString8 == xftdraw_string) ? 0 : 1;

    if (ISSET_OPTION(r, Opt_xft) && PVTS(r)->xftvt && xftdraw_string)
    {
	register int	loop;	    /* loop iteration number */
	register int	loopitem;   /* each iteration increasing # */
	register int	i;
	/*
	** xft_draw_string_xft should call these two parameters
	*/
	register char*	pstr;	    /* string to print */
	register int	plen;	    /* string length */
	char*		newstr;
#ifdef MULTICHAR_SET
#  ifdef HAVE_ICONV_H
	char		pbuf[1024]; /* buffer to save UTF-8 string */
#  endif
#endif


	/*
	 * Xft does not support XDrawImageString, so we need to clear the
	 * background of text by ourselves.
	 */
	if (fillback)
	    XftDrawRect( PVTS(r)->xftvt, &(r->xftColors[back]),
		    x, y, Width2Pixel(len * (1 + adjust)), Height2Pixel(1));

	/* We use TermWin.xftfont->ascent here */
	y += r->TermWin.xftfont->ascent;

	/*
	 * Xft does not support XftDrawString16, so we need to convert the
	 * string to UTF-8. Here we reencode the string before conversion
	 */
# ifdef MULTICHAR_SET
#  ifdef HAVE_ICONV_H
	if (adjust && (iconv_t) -1 != r->TermWin.xfticonv)
	{
	    register int    j, newlen = (len << 1);
	    switch (r->encoding_method)
	    {
		case ENC_EUCJ:
		case ENC_GB:
		    for (j = 0; j < newlen; j ++)
			str[j] |= 0x80;
		    break;
		case ENC_GBK:	/* need to do nothing */
		case ENC_BIG5:	/* need to do nothing */
		default:
		    break;
	    }
	    /* we will use utf8 routine to draw string */
	    xftdraw_string = XftDrawStringUtf8;
	}
#  endif
# endif	/* MULTICHAR_SET */


	/*
	** If the font is monospace, we print the entire string once,
	** otherwise, print the characters one by one
	*/
	if (r->TermWin.xftmono)
	{
	    /* print string once for mono font */
	    loop = 1;
	    loopitem = len;
	    /*
	    ** If XftDrawString8 == xftdraw_string
	    **     string length == character number
	    ** If XftDrawString16 == xftdraw_string
	    **     string length == 2 * character number, but we do
	    ** not need to multiply loopitem by 2 because  the
	    ** XftDrawString16 takes character numbers.
	    **
	    ** If XftDrawStringUtf8 == xftdraw_string
	    **     string length == 2 * character number, but we need
	    ** to multiply loopitem by 2 because iconv need string
	    ** length as parameter, not character number.
	    */
	    if (XftDrawStringUtf8 == xftdraw_string)
		loopitem <<= 1;
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "output entire mono string\n"));
	}
	/*
	** Non monospace font, but still we can improve the performance
	** by print it once under certain conditions
	*/
# ifdef MULTICHAR_SET
	else
	if (
	      NOTSET_OPTION(r, Opt2_xftSlowOutput)
	      && (XftDrawStringUtf8 == xftdraw_string)
	      && (
		  r->TermWin.xftmfont->max_advance_width ==
		  	(r->TermWin.fwidth << 1)
		 )
	   )
	{
	    /* print string once for multichar string */
	    loop = 1;
	    /*
	    ** If XftDrawStringUtf8 == xftdraw_string
	    **     string length == 2 * character number, but we need
	    ** to multiply loopitem by 2 because iconv need string
	    ** length as parameter, not character number.
	    */
	    loopitem = (len << 1);
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "output entire UTF-8 string\n"));
	}
	else
	if (
	      NOTSET_OPTION(r, Opt2_xftSlowOutput)
	      && (XftDrawString16 == xftdraw_string)
	      && (
		  r->TermWin.xftmfont->max_advance_width ==
		  	(r->TermWin.fwidth << 1)
		 )
	   )
	{
	    /* print string once for 16-bits string */
	    loop = 1;
	    loopitem = len;
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "output entire 16-bits string\n"));
	}
# endif	/* MULTICHAR_SET */
	else
	if (
	      r->TermWin.xftfnmono && (XftDrawString8 == xftdraw_string)
	      && (r->TermWin.xftfont->max_advance_width == r->TermWin.fwidth)
	   )
	{
	    /* print string once for 8-bits string */
	    loop = 1;
	    loopitem = len;
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "output entire 8-bits string\n"));
	}
	else
	{
	    /* print string one by one character */
	    loop = len;
	    loopitem = 1 + adjust;
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "output characters one by one\n"));
	}


	newstr = str;	/* string beginning in each iteration */
	for (i = 0; i < loop; i ++)
	{
# ifdef MULTICHAR_SET
#  ifdef HAVE_ICONV_H
	    if (XftDrawStringUtf8 == xftdraw_string)
	    {
		/* We should convert the string to UTF-8 */
		char*	buf = pbuf;		/* always init it */
		size_t	buflen = sizeof(pbuf)-1;/* always init it */
		size_t	newlen = loopitem;	/* always init it */
		char*	oldstr = newstr;
		iconv (r->TermWin.xfticonv, &newstr, &newlen, &buf, &buflen);
		*buf = (char) 0;    /* set end of string */
		pstr = pbuf;
		/*
		 * we should use the length of original string, not UTF-8 string
		 * here!!!
		 */
		plen = loopitem;
		/*
		 * reset newstr to old position, we will increase it later
		 */
		newstr = oldstr;
	    }
	    else
#  endif
# endif	/* MULTICHAR_SET */
	    {
		/* We do not need to convert the string to UTF-8 */
		pstr = newstr;
		plen = loopitem;
	    }

	    rxvt_draw_string_xft(r, PVTS( r)->vt, r->TermWin.gc,
		    refreshRegion, rend, NO_PFONT,
		    PVTS(r)->xftvt, &(r->xftColors[fore]),
		    x, y, pstr, plen, xftdraw_string);

	    x += Width2Pixel (loopitem);
	    newstr += loopitem;	/* next string to display */
	}
    }
    else
#endif	/* XFT_SUPPORT */
    {
	int	(*draw_string) ();
	switch (drawfunc)
	{
	    case    X11_DRAW_STRING_8:
		draw_string = XDrawString; break;
	    case    X11_DRAW_STRING_16:
		draw_string = XDrawString16; break;
	    case    X11_DRAW_IMAGE_STRING_8:
		draw_string = XDrawImageString; break;
	    case    X11_DRAW_IMAGE_STRING_16:
		draw_string = XDrawImageString16; break;

	    case    XFT_DRAW_STRING_8:	    /* fall back to X11 */
		draw_string = XDrawString; break;
	    case    XFT_DRAW_STRING_16:	    /* fall back to X11 */
	    case    XFT_DRAW_STRING_32:	    /* fall back to X11 */
	    case    XFT_DRAW_STRING_UTF8:   /* fall back to X11 */
		draw_string = XDrawString16; break;

	    case    XFT_DRAW_IMAGE_STRING_8:	/* fall back to X11 */
		draw_string = XDrawImageString; break;
	    case    XFT_DRAW_IMAGE_STRING_16:	/* fall back to X11 */
	    case    XFT_DRAW_IMAGE_STRING_32:	/* fall back to X11 */
	    case    XFT_DRAW_IMAGE_STRING_UTF8:	/* fall back to X11 */
		draw_string = XDrawImageString16; break;

	    default:
		draw_string = NULL; break;
	}

	/* We use TermWin.font->ascent here */
	y += r->TermWin.font->ascent;

	/* Now draw the string */
	if (draw_string)
	    rxvt_draw_string_x11 (r, PVTS(r)->vt, r->TermWin.gc,
		    refreshRegion, x, y, str, len, draw_string);
    }
}


/*
 * 2006-08-19 gi1242: Don't display blinking text with the bold attribute. This
 * causes problems all over the code: When we unset the bold attribute, thinking
 * "we've taken care of it", the blink attribute might cause us to do over
 * striking. Plus it causes trouble in the pixel dropping avoidance stuff.
 */
#define MONO_BOLD(x)							    \
    (ISSET_OPTION(r, Opt2_veryBold) ?					    \
     ((x) & RS_Bold) :							    \
     (((x) & (RS_Bold | RS_fgMask)) == (RS_Bold | Color_fg))		    \
    )
#define MONO_BOLD_FG(x, fg)						    \
    (ISSET_OPTION(r, Opt2_veryBold) ?					    \
     ((x) & RS_Bold) :							    \
     (((x) & RS_Bold) && (fg) == Color_fg)				    \
    )


#define FONT_WIDTH(X, Y)						    \
    (X)->per_char[(Y) - (X)->min_char_or_byte2].width
#define FONT_RBEAR(X, Y)						    \
    (X)->per_char[(Y) - (X)->min_char_or_byte2].rbearing
#define FONT_LBEAR(X, Y)						    \
    (X)->per_char[(Y) - (X)->min_char_or_byte2].lbearing
#define IS_FONT_CHAR(X, Y)						    \
    ((Y) >= (X)->min_char_or_byte2 && (Y) <= (X)->max_char_or_byte2)

/*
 * This function needs to be optimized and sped up more
 *
 * 2006-02-11 gi1242: Added a CLIPPED_REFRESH option. If set, only clipped areas
 * are redrawn. This adds a TREMENDOUS speedup when dragging some window over
 * mrxvt's window. (Not to mention got rid of that annoying flicker).
 */
/* EXTPROTO */
void
rxvt_scr_refresh(rxvt_t* r, unsigned char refresh_type)
{
    unsigned char
		clearfirst, /* first character writes before cell */
		clearlast,  /* last character writes beyond cell */
		must_clear, /* use drawfunc not image_drawfunc */
		already_cleared=0, /* Use XClearArea or no-op */
#ifndef NO_BOLDFONT
		usingBoldFt, /* we've changed font to bold font */
		loadedBoldFt,	/* If a bold font is loaded */
#endif
		rvid,	    /* reverse video this position */
		wbyte,	    /* we're in multibyte */
		showcursor; /* show the cursor */


    signed char	morecur = 0;/* */
#ifdef TTY_RGBCOLOR
    uint32_t
#elif defined(TTY_256COLOR)
    uint16_t
#else
    unsigned char
#endif
		fore, back; /* desired foreground/background */
    int16_t	col, row,   /* column/row we're processing */
		ocrow,	    /* old cursor row */
		len, wlen;  /* text length screen/buffer */
    int		i,	    /* tmp */
		row_offset; /* basic offset in screen structure */
#ifndef NO_CURSORCOLOR
    rend_t	cc1 = 0;    /* store colours at cursor position(s) */
# ifdef MULTICHAR_SET
    rend_t	cc2 = 0;    /* store colours at cursor position(s) */
			    /* 2007-07-30 gi1242: NULL assignment to suppress
			     * compile warning. */
# endif
#endif
    XGCValues	gcvalue;    /* Graphics Context values */
    XFontStruct*    wf;	    /* font structure */
    rend_t*	drp;	    /* drawn_rend pointer */
    rend_t*	srp;	    /* screen-rend-pointer */
    text_t*	dtp;	    /* drawn-text pointer */
    text_t*	stp;	    /* screen-text-pointer */
    char*	buffer;	    /* local copy of r->h.buffer */
    /*
    int		(*drawfunc) () = XDrawString;
    int		(*image_drawfunc) () = XDrawImageString;
    */
    int		drawfunc, image_drawfunc;

    if( !(refresh_type & CLIPPED_REFRESH) )
	PVTS( r)->scrolled_lines = 0;

    if (refresh_type == NO_REFRESH)
    {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "Skipping refresh (%d)\n", refresh_type));

	return;
    }

    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_scr_refresh ()\n"));

    /*
    ** A: set up vars
    */
#ifdef XFT_SUPPORT
    if (ISSET_OPTION(r, Opt_xft) && PVTS(r)->xftvt)
    {
	drawfunc = XFT_DRAW_STRING_8;
	image_drawfunc = XFT_DRAW_IMAGE_STRING_8;
    }
    else
#endif
    {
	drawfunc = X11_DRAW_STRING_8;
	image_drawfunc = X11_DRAW_IMAGE_STRING_8;
    }

    clearfirst = clearlast = must_clear = wbyte = 0;
#ifndef NO_BOLDFONT
    usingBoldFt = 0;

    /* Determine if we have a bold font loaded or not */
    if(
# ifdef XFT_SUPPORT
        ISSET_OPTION( r, Opt_xft )		    ?
	    NOT_NULL( r->TermWin.xftbfont )	    :
	    NOT_NULL( r->TermWin.bfont )
# else
        NOT_NULL( r->TermWin.bfont )
# endif
      )
	loadedBoldFt = 1;
    else
	loadedBoldFt = 0;
#endif /* NO_BOLDFONT */

    if (r->h.currmaxcol < r->TermWin.ncol)
    {
	r->h.currmaxcol = r->TermWin.ncol;
	r->h.buffer = rxvt_realloc(r->h.buffer, sizeof(char) * (r->h.currmaxcol + 1));
    }
    buffer = r->h.buffer;

    row_offset = SVLINES - VSTART;
#ifdef XFT_SUPPORT
    if (!(ISSET_OPTION(r, Opt_xft) && r->TermWin.xftfont))
#endif
    {
	/* always go back to the base font - it's much safer */
	XSetFont(r->Xdisplay, r->TermWin.gc, r->TermWin.font->fid);
	wf = r->TermWin.font;
    }

    if ((refresh_type & REFRESH_BOUNDS))
    {
	clearfirst = clearlast = 1;
	r->h.refresh_type &= ~REFRESH_BOUNDS;
    }
    /* is there an old outline cursor on screen? */
    ocrow = r->h.oldcursor.row;

    /*
     * set base colours to avoid check in "single glyph writing" below
     */
    gcvalue.foreground = r->pixColors[Color_fg];
    gcvalue.background = r->pixColors[Color_bg];

    /*
     * Set clippings on our XftDrawables and GC's to make sure we don't waste
     * time drawing pixels outside this clipping. (This probably happened
     * because of an expose event).
     */
    if( (refresh_type & CLIPPED_REFRESH) && IS_REGION(r->h.refreshRegion))
    {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "Doing clipped refresh (Region %p)\n", r->h.refreshRegion));

	/*
	 * We must wait till refresh is complete before destroying the
	 * region because the clipping is reset when text shadow is used.
	 */
	XSetRegion( r->Xdisplay, r->TermWin.gc, r->h.refreshRegion);
#ifdef XFT_SUPPORT
	if( ISSET_OPTION(r, Opt_xft) && PVTS(r)->xftvt )
	    XftDrawSetClip( PVTS(r)->xftvt, r->h.refreshRegion);
#endif
	/* Remember we don't need to call XClearArea on exposed regions */
	if( must_clear ) already_cleared = 1;
    }

    /*
    ** B: reverse any characters which are selected
    */
    rxvt_scr_reverse_selection(r);


    /*
    ** C: set the cursor character(s)
    */
    {
	unsigned char	setoldcursor;
	rend_t		ccol1,	/* Cursor colour */
			ccol2;	/* Cursor colour2 */

	showcursor = (PSCR(r).flags & Screen_VisibleCursor);
#ifdef CURSOR_BLINK
	if (r->h.hidden_cursor)
	    showcursor = 0;
#endif
	if (showcursor && r->TermWin.focus)
	{
	    int	    currow = CURROW + SVLINES;
	    srp = &(PSCR(r).rend[currow][CURCOL]);

	    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "Setting solid cursor\n"));

	    *srp ^= RS_RVid;

#ifndef NO_CURSORCOLOR
	    cc1 = *srp & (RS_fgMask | RS_bgMask);
	    if (XDEPTH > 2 && ISSET_PIXCOLOR(r, Color_cursor))
		ccol1 = Color_cursor;
	    else
#ifdef CURSOR_COLOR_IS_RENDITION_COLOR
		ccol1 = GET_FGCOLOR(
			    PVTS(r)->drawn_rend[CURROW][CURCOL]);
		/*
		ccol1 = GET_FGCOLOR(PVTS(r)->rstyle);
		*/
#else
		ccol1 = Color_fg;
#endif
	    if(
		 XDEPTH > 2
		 && ISSET_PIXCOLOR( r, Color_cursor )
		 && ISSET_PIXCOLOR( r, Color_cursor2 )
	      )
		ccol2 = Color_cursor2;
	    else
	    {
#ifdef CURSOR_COLOR_IS_RENDITION_COLOR
# ifdef SIMULATE_LINUX_CONSOLE_CURSOR_COLOR
		ccol2 = GET_FGCOLOR(PVTS(r)->drawn_rend[CURROW][CURCOL]);
# else
		ccol2 = GET_BGCOLOR(PVTS(r)->drawn_rend[CURROW][CURCOL]);
# endif	/* SIMULATE_LINUX_CONSOLE_CURSOR_COLOR */
		/*
		ccol2 = GET_BGCOLOR(PVTS(r)->rstyle);
		*/
#else
		ccol2 = Color_bg;
#endif
	    }

	    *srp = SET_FGCOLOR(*srp, ccol1);
	    *srp = SET_BGCOLOR(*srp, ccol2);
#endif /* NO_CURSORCOLOR */

#ifdef MULTICHAR_SET
	    if (IS_MULTI1(*srp))
	    {
		if (CURCOL < r->TermWin.ncol - 2 && IS_MULTI2(*++srp))
		    morecur = 1;
	    }
	    else if (IS_MULTI2(*srp))
	    {
		if (CURCOL > 0 && IS_MULTI1(*--srp))
		    morecur = -1;
	    }
	    if (morecur)
	    {
		*srp ^= RS_RVid;
# ifndef NO_CURSORCOLOR
		cc2 = *srp & (RS_fgMask | RS_bgMask);
		*srp = SET_FGCOLOR(*srp, ccol1);
		*srp = SET_BGCOLOR(*srp, ccol2);
# endif
	    }
#endif
	}

	/* make sure no outline cursor is left around */
	setoldcursor = 0;
	if (ocrow != -1)
	{
	    if (CURROW + VSTART != ocrow || CURCOL != r->h.oldcursor.col)
	    {
		if (
		      ocrow < r->TermWin.nrow
		      && r->h.oldcursor.col < r->TermWin.ncol
		   )
		{
		    PVTS(r)->drawn_rend[ocrow][r->h.oldcursor.col] ^=
			(RS_RVid | RS_Uline);
#ifdef MULTICHAR_SET
		    if (r->h.oldcursormulti)
		    {
			col = r->h.oldcursor.col + r->h.oldcursormulti;
			if (col < r->TermWin.ncol)
			    PVTS(r)->drawn_rend[ocrow][col] ^=
				(RS_RVid | RS_Uline);
		    }
#endif
		}
		if (r->TermWin.focus || !showcursor)
		    r->h.oldcursor.row = -1;
		else
		    setoldcursor = 1;
	    }
	}
	else if (!r->TermWin.focus)
	    setoldcursor = 1;

	if (setoldcursor)
	{
	    if (CURROW + VSTART >= r->TermWin.nrow)
		r->h.oldcursor.row = -1;
	    else
	    {
		r->h.oldcursor.row = CURROW + VSTART;
		r->h.oldcursor.col = CURCOL;
#ifdef MULTICHAR_SET
		r->h.oldcursormulti = morecur;
#endif
	    }
	}
    }
    /* End of C */


#ifndef NO_SLOW_LINK_SUPPORT
    /*
    ** D: CopyArea pass - very useful for slower links
    **	This has been deliberately kept simple.
    */
    i = PVTS(r)->num_scr;
    if (refresh_type == FAST_REFRESH &&
	    r->h.num_scr_allow &&
	    i &&
	    abs(i) < r->TermWin.nrow &&
	    !must_clear)
    {
	int16_t	     nits;
	int	     j;
	rend_t	     *drp2;
	text_t	     *dtp2;

	rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "Trying slowlink copyarea pass\n"));

	j = r->TermWin.nrow;
	wlen = len = -1;
	row = i > 0 ? 0 : j - 1;
	for (; j-- >= 0; row += (i > 0 ? 1 : -1))
	{
	    if (row + i >= 0 && row + i < r->TermWin.nrow && row + i != ocrow)
	    {
		stp = PSCR(r).text[row + row_offset];
		srp = PSCR(r).rend[row + row_offset];
		dtp = PVTS(r)->drawn_text[row];
		dtp2 = PVTS(r)->drawn_text[row + i];
		drp = PVTS(r)->drawn_rend[row];
		drp2 = PVTS(r)->drawn_rend[row + i];
		for (nits = 0, col = r->TermWin.ncol; col--; )
		    if (stp[col] != dtp2[col] ||
			srp[col] != drp2[col])
			nits--;
		    else if (stp[col] != dtp[col] ||
			srp[col] != drp[col])
			nits++;
		if (nits > 8)	    /* XXX: arbitrary choice */
		{
		    for (col = r->TermWin.ncol; col--; )
		    {
			*dtp++ = *dtp2++;
			*drp++ = *drp2++;
		    }
		    if (len == -1)
		    len = row;
		    wlen = row;
		    continue;
		}
	    }

	    if (len != -1)
	    {
		/* also comes here at end if needed because of >= above */
		if (wlen < len)
		    SWAP_IT(wlen, len);

		rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_scr_refresh (): " "XCopyArea: %d -> %d (height: %d)\n", len + i, len, wlen - len + 1));
		XCopyArea(r->Xdisplay, PVTS(r)->vt,
		    PVTS(r)->vt, r->TermWin.gc,
		    0, Row2Pixel(len + i),
		    TWIN_WIDTH(r),
		    (unsigned int)Height2Pixel(wlen-len+1),
		    0, Row2Pixel(len));
		len = -1;
	    }
	}
    }	/* End of D */
#endif	/* !NO_SLOW_LINK_SUPPORT */


    /*
    ** E: main pass across every character
    */
    for (row = 0; row < r->TermWin.nrow; row++)
    {
	unsigned char	clear_next = 0;
	int		j,
			/* x offset for start of drawing (font) */
			xpixel,
			/* y offset for top of drawing */
			ypixelc;
	unsigned long	gcmask;	 /* Graphics Context mask */


	stp = PSCR(r).text[row + row_offset];
	srp = PSCR(r).rend[row + row_offset];
	dtp = PVTS(r)->drawn_text[row];
	drp = PVTS(r)->drawn_rend[row];


#ifndef NO_PIXEL_DROPPING_AVOIDANCE
	/*
	 * E1: Pixel dropping avoidance.  Do this before the main refresh on the
	 * line. Require a refresh where pixels may have been dropped into our
	 * area by a neighbour character which has now changed
	 *
	 * TODO: This could be integrated into E2 but might be too messy
	 */
	for (col = 0; col < r->TermWin.ncol; col++)
	{
	    unsigned char   is_font_char, is_same_char;
	    text_t	    t;

	    t = dtp[col];
	    is_same_char = (t == stp[col] && drp[col] == srp[col]);
	    if (!clear_next &&
		(is_same_char || t == 0 || t == ' '))
		/* screen cleared elsewhere */
		continue;

	    if (clear_next)
	    {
		/* previous char caused change here */
		clear_next = 0;
		dtp[col] = 0;

		/* don't cascade into next char */
		if (is_same_char)
		    continue;
	    }

#ifdef XFT_SUPPORT
	    if( ISSET_OPTION(r, Opt_xft) )
	    {
		/*
		 * Only text drawn by over striking needs to be watched for
		 * "dropped pixels".
		 * 
		 * XXX This does not take into account Color_BD
		 */
		if( !loadedBoldFt && ( drp[col] & RS_Bold ) )
		{
		    if( col == r->TermWin.ncol - 1 ) clearlast = 1;
		    else clear_next = 1;
		}
	    }
	    else
#endif
	    {
		j = MONO_BOLD(drp[col]) ? 1 : 0;
# ifndef NO_BOLDFONT
		wf = (j && r->TermWin.bfont) ?
			    r->TermWin.bfont : r->TermWin.font;
# endif

		/*
		** TODO: consider if anything special needs to happen
		** with:
		** #if defined(MULTICHAR_SET) &&
		** !defined(NO_BOLDOVERSTRIKE_MULTI)
		*/
		is_font_char = (wf->per_char && IS_FONT_CHAR(wf, t)) ? 1:0;
		if (!is_font_char || FONT_LBEAR(wf, t) < 0)
		{
		    if (col == 0)
			clearfirst = 1;
		    else
			dtp[col - 1] = 0;
		}
		if (!is_font_char ||
			(FONT_WIDTH(wf, t) < (FONT_RBEAR(wf, t) + j)))
		{
		    if (col == r->TermWin.ncol - 1)
			clearlast = 1;
		    else
			clear_next = 1;
		}
	    }
	}
#endif	/* NO_PIXEL_DROPPING_AVOIDANCE */
	/* End of E1 */


	/*
	** E2: OK, now the real pass
	*/
	ypixelc = (int)Row2Pixel(row);

	for (col = 0; col < r->TermWin.ncol; col++)
	{
			    /* current font size != base font size */
	    unsigned char   fontdiff,
			    /* proportional font used */
			    fprop;
			    /* rendition value */
	    rend_t	    rend;

	    /* screen rendition (target rendtion) */
	    rend = srp[col];

	    /*
	     * compare new text with old - if exactly the same then continue
	     */
	    if (
		 /* Must match characters to skip. */
		 stp[col] == dtp[col] &&
		 /* Either rendition the same or   */
		 (
		   rend == drp[col] ||
		   /* space w/ no background change  */
		   (
		     stp[col] == ' ' &&
		     	 GET_BGATTR(rend) == GET_BGATTR(drp[col])
		   )
		 )
	       )    /* if */
	    {
		if (!IS_MULTI1(rend))
		    continue;
#ifdef MULTICHAR_SET
		else
		{
		    /* first byte is Kanji so compare second bytes */
		    if (stp[col + 1] == dtp[col + 1])
		    {
			/* assume no corrupt characters on the screen */
			col++;
			continue;
		    }
		}
#endif
	    }
	    /* redraw one or more characters */

	    fontdiff = 0;
	    len = 0;
	    buffer[len++] = dtp[col] = stp[col];
	    drp[col] = rend;
	    xpixel = Col2Pixel(col);

	    /*
	     * Find out the longest string we can write out at once
	     */
#ifndef NO_BOLDFONT
	    if (MONO_BOLD(rend) && r->TermWin.bfont != NULL)
		fprop = (r->TermWin.propfont & PROPFONT_BOLD);
	    else
#endif
		fprop = (r->TermWin.propfont & PROPFONT_NORMAL);

#ifdef MULTICHAR_SET
	    if (
		  IS_MULTI1(rend) && col < r->TermWin.ncol - 1
		  && IS_MULTI2(srp[col + 1])
	       )
	    {
		if (!wbyte && r->TermWin.mfont)
		{
		    wbyte = 1;
		    XSetFont(r->Xdisplay, r->TermWin.gc,
			 r->TermWin.mfont->fid);
		    fontdiff = (r->TermWin.propfont & PROPFONT_MULTI);
#ifdef XFT_SUPPORT
		    if ( ISSET_OPTION(r, Opt_xft) && PVTS(r)->xftvt )
		    {
			drawfunc = XFT_DRAW_STRING_16;
			image_drawfunc = XFT_DRAW_IMAGE_STRING_16;
		    }
		    else
#endif
		    {
			drawfunc = X11_DRAW_STRING_16;
			image_drawfunc = X11_DRAW_IMAGE_STRING_16;
		    }
		}

		if (r->TermWin.mfont == NULL)
		{
		    buffer[0] = buffer[1] = ' ';
		    len = 2;
		    col++;
		}
		else
		{
		    /* double stepping - we're in multibyte font mode */
		    for (; ++col < r->TermWin.ncol;)
		    {
			/* XXX: could check sanity on 2nd byte */
			dtp[col] = stp[col];
			drp[col] = srp[col];
			buffer[len++] = stp[col];
			col++;
			/* proportional multibyte font mode */
			if (fprop)
			    break;
			if ((col == r->TermWin.ncol) ||
			    (srp[col] != rend))
			    break;
			if ((stp[col] == dtp[col]) &&
			    (srp[col] == drp[col]) &&
			    (stp[col + 1] == dtp[col + 1]))
			    break;
			if (len == r->h.currmaxcol)
			    break;
			dtp[col] = stp[col];
			drp[col] = srp[col];
			buffer[len++] = stp[col];
		    }
		    col--;
		}

		if (buffer[0] & 0x80)
		{
		    (r->h.multichar_decode)( (unsigned char*) buffer, len);
		}
		wlen = len / 2;
	    }
	    else
	    {
		if (rend & RS_multi1)
		{
		    /* corrupt character - you're outta there */
		    rend &= ~RS_multiMask;
		    drp[col] = rend; /* TODO check: may also want */
		    dtp[col] = ' '; /* to poke into stp/srp   */
		    buffer[0] = ' ';
		}
		if (wbyte)
		{
		    wbyte = 0;
#ifdef XFT_SUPPORT
		    if (!(ISSET_OPTION(r, Opt_xft) && r->TermWin.xftfont))
#endif
			XSetFont(r->Xdisplay, r->TermWin.gc,
				r->TermWin.font->fid);
#ifdef XFT_SUPPORT
		    if ( ISSET_OPTION(r, Opt_xft) && PVTS(r)->xftvt )
		    {
			drawfunc = XFT_DRAW_STRING_8;
			image_drawfunc = XFT_DRAW_IMAGE_STRING_8;
		    }
		    else
#endif
		    {
			drawfunc = X11_DRAW_STRING_8;
			image_drawfunc = X11_DRAW_IMAGE_STRING_8;
		    }
		}   /* if (wbyte) */
#else /*MULTICHAR_SET*/
	    { /* add } for correct % bouncing */
#endif /*MULTICHAR_SET*/
		if (!fprop)
		{
		    int echars;

		    /* single stepping - `normal' mode */
		    for (i = 0, echars=0; ++col < r->TermWin.ncol - 1;)
		    {
			/*
			 * Instead of simply getting the longest string that
			 * needs to be refreshed, we do some caching.
			 *
			 * i is the number of trailing chars that we read (in an
			 * attempt to cache) that DO NOT need to be refreshed.
			 * These had better be dumped.
			 *
			 * echars are the number of extra chars we drew that did
			 * not need to be drawn. When echars get's too high,
			 * then we should break out.
			 */
			if (rend != srp[col])
			    /* Different attributes. */
			    break;
			buffer[len++] = stp[col];

			if ( (stp[col] != dtp[col]) || (srp[col] != drp[col]) )
			{
			    /* This position needed to be refreshed anyway */
			    /* if (must_clear && (i++ > (len / 2))) break; */

			    dtp[col] = stp[col];
			    drp[col] = srp[col];
			    i = 0; /* Set trailing chars to 0 */
			}
			else /* if (must_clear ||
				(stp[col] != ' ' && ++i > 32)) */
			{
			    /*
			     * This position did not require a refresh. Let's do
			     * some caching.
			     */
			    i++;
			    /*
			     * 25% (arbitarily choosen) of our drawn string can
			     * be extra chars.
			     */
			    if( ++echars > (len >> 2) ) break;
			}

		    }	/* for */
		    col--;	/* went one too far.  move back */
		    len -= i;	/* dump any matching trailing chars */

		    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "Drawing %d(%d) chars: %.*s\n", len, echars-i, (len > 55) ? 55 : len, buffer));
		} /* if (!fprop) */
		wlen = len;
	    }
	    buffer[len] = '\0';

	    /*
	     * Determine the attributes for the string
	     */
	    fore = GET_FGCOLOR(rend);
	    back = GET_BGCOLOR(rend);
	    rend = GET_ATTR(rend);

	    switch (rend & RS_fontMask)
	    {
		case RS_acsFont:
		    for (i = 0; i < len; i++)
			/*
			 * Xterm leaves 0x5f (_) unchanged in graphics mode.
			 */
			if (buffer[i] > 0x5f && buffer[i] < 0x7f)
			    buffer[i] -= 0x5f;
		    break;
		case RS_ukFont:
		    for (i = 0; i < len; i++)
			if (buffer[i] == '#')
			    buffer[i] = 0x1e;	/* pound sign */
		    break;
	    }

#ifndef NO_BOLD_UNDERLINE_REVERSE
	    /*
	     * Bold / underline fonts. We use Color_BD / Color_UL only if we're
	     * displaying the font with normal colors.
	     */
	    if( fore == Color_fg && back == Color_bg )
	    {
		/*
		 * TODO: Should probably add a Color_BDUL here.
		 */
		if ( rend & RS_Bold )
		{
		    if (
			  XDEPTH > 2 && ISSET_PIXCOLOR(r, Color_BD)
			  && PIXCOLOR(r, fore) != r->pixColors[Color_BD]
			  && PIXCOLOR(r, back) != r->pixColors[Color_BD]
		       )
		    {

			fore = Color_BD;
			/*
			 * 2006-03-27 gi1242: Ignore veryBold when Color_BD is
			 * set. veryBold will only be used when displaying
			 * colored / blinking text.
			 */
#if 0
			if (NOTSET_OPTION(r, Opt2_veryBold))
#endif
			rend &= ~RS_Bold;   /* we've taken care of it */
		    }
		}
		else if ( rend & RS_Uline )
		{
		    if (
			  XDEPTH > 2 && ISSET_PIXCOLOR(r, Color_UL)
			  && PIXCOLOR(r, fore) != r->pixColors[Color_UL]
			  && PIXCOLOR(r, back) != r->pixColors[Color_UL]
		       )
		    {
			fore = Color_UL;
			rend &= ~RS_Uline;  /* we've taken care of it */
		    }
		}
	    }
#endif
	    rvid = (rend & RS_RVid) ? 1 : 0;
#ifdef OPTION_HC
	    /*
	     * Use Color_HC for anything blinking. TODO: Add a seperate
	     * attribute for the XSelection, so that the user can distinguish
	     * blinking text from the selection. (XTerm does this)
	     */
	    if ( (rend & RS_Blink))
	    {
		if (
		      XDEPTH > 2 && ISSET_PIXCOLOR(r, Color_HC)
		      && PIXCOLOR(r, fore) != r->pixColors[Color_HC]
		      && PIXCOLOR(r, back) != r->pixColors[Color_HC]
# ifndef NO_CURSORCOLOR
			/* Don't do this for the cursor */
		      && (
			    !ISSET_PIXCOLOR(r, Color_cursor)
			    || !r->TermWin.focus || !showcursor
			    || CURROW != row || CURCOL != col
			 )
# endif
		   )
		{
		    if( rvid) rvid = 0;
		    back = Color_HC;
		}
		else
		    rvid = 1;	/* fall back */
	    }
#endif

	    /*
	     * Reverse Video. If defined, Color_RV for background and leave
	     * foreground untouched. Done last so that RV-BD text will have
	     * Color_BD background if set (like in XTerm).
	     */
	    if( rvid )
	    {
#ifndef NO_BOLD_UNDERLINE_REVERSE
		if (
		      XDEPTH > 2 && ISSET_PIXCOLOR(r, Color_RV)
		      && PIXCOLOR(r, fore) != r->pixColors[Color_RV]
		      && PIXCOLOR(r, back) != r->pixColors[Color_RV]
# ifndef NO_CURSORCOLOR
		      /* Don't do this for the cursor */
		      && (
			    !ISSET_PIXCOLOR(r, Color_cursor)
			    || !r->TermWin.focus || !showcursor
			    || CURROW != row || CURCOL != col
			 )
# endif
		   )
		    back = Color_RV;
		else
#endif
		{
		    SWAP_IT(fore, back);
		}
	    }

#ifndef NO_BRIGHTCOLOR
	    /* Use bright colors for bold primary colors */
	    if( ((rend & RS_Bold) && (NOTSET_OPTION( r, Opt2_boldColors ) || PIXCOLOR(r, fore) == PIXCOLOR(r, back)))
#ifdef BLINK_BRIGHTCOLOR
			    || (rend & RS_Blink)
#endif
			    )
	    {
		if( fore == Color_fg )
		{
		    if (
			  XDEPTH > 2 && ISSET_PIXCOLOR(r, Color_BD)
			  && r->pixColors[fore] != r->pixColors[Color_BD]
			  && r->pixColors[back] != r->pixColors[Color_BD]
		       )
		    {
			fore = Color_BD;
		    }
		}
		else if( fore >= minCOLOR && fore < minBrightCOLOR )
		{
		    fore += minBrightCOLOR - minCOLOR;
		    if( NOTSET_OPTION( r, Opt_veryBright ) )
			rend &= ~RS_Bold;
		}
#if defined(TTY_256COLOR) && (defined(BOLD_BRIGHTENS_256_COLORS) || defined(BLINK_BRIGHTCOLOR))
#ifndef BOLD_BRIGHTENS_256_COLORS
		else if (!(rend & RS_Blink));
#endif
		/* If fore is in the 6x6x6 color cube, try and brighten it */
		else if(
			 fore >= min256COLOR			    &&
			 fore <= min256COLOR + 4*36 + 4*6 + 4	    &&
			 (fore - min256COLOR) % 6 < 5		    &&
			 ((fore - min256COLOR)/6) % 6 < 5
		       )
		{
		    fore += 36 + 6 + 1;

#ifdef BOLD_BRIGHTENS_256_COLORS
		    if( NOTSET_OPTION( r, Opt_veryBright ) )
			rend &= ~RS_Bold;
#endif
		}

		/* Brighten up colors in the grey-scale ramp. */
		else if(
			fore >= min256COLOR + 6*6*6		    &&
			fore < max256COLOR
		       )
		{
		    if( fore >= max256COLOR -3 )
			fore = min256COLOR + 6*6*6 - 1;
		    else
			fore += 4;

#ifdef BOLD_BRIGHTENS_256_COLORS
		    if( NOTSET_OPTION( r, Opt_veryBright ) )
			rend &= ~RS_Bold;
#endif
		}
#endif /*TTY_256COLOR && BOLD_BRIGHTENS_256_COLORS*/
	    }
#endif /*NO_BRIGHTCOLOR*/


	    /*
	     * fore and back should now have the correct colors.
	     */
	    gcmask = 0;
	    if (back != Color_bg)
	    {
		gcvalue.background = PIXCOLOR(r, back);
		gcmask = GCBackground;
	    }
	    if (fore != Color_fg)
	    {
		gcvalue.foreground = PIXCOLOR(r, fore);
		gcmask |= GCForeground;
	    }
#ifndef NO_BOLD_UNDERLINE_REVERSE
	    else if (rend & RS_Bold && ISSET_PIXCOLOR( r, Color_BD) )
	    {
# ifdef XFT_SUPPORT
		/*
		 * XFT won't use the colors from the GC, so we need to set
		 * fore.
		 */
		if( ISSET_OPTION(r, Opt_xft) && PVTS(r)->xftvt )
		    fore = Color_BD;
		else
# endif
		{
		    gcvalue.foreground = r->pixColors[Color_BD];
		    gcmask |= GCForeground;
		}

		/*
		 * If veryBold is not set, then don't render colored text in
		 * bold.
		 */
		if (NOTSET_OPTION(r, Opt2_veryBold))
		    rend &= ~RS_Bold;
	    }
	    else if (rend & RS_Uline && ISSET_PIXCOLOR( r, Color_UL) )
	    {
# ifdef XFT_SUPPORT
		if( ISSET_OPTION(r, Opt_xft) && PVTS(r)->xftvt )
		    fore = Color_UL;
		else
# endif
		{
		    gcvalue.foreground = r->pixColors[Color_UL];
		    gcmask |= GCForeground;
		}
		rend &= ~RS_Uline;  /* we've taken care of it */
	    }
#endif


	    if (gcmask)
		XChangeGC(r->Xdisplay, r->TermWin.gc, gcmask, &gcvalue);
#ifndef NO_BOLDFONT
	    /*
	     * Switch to the bold font if we are rendering bold text.
	     *
	     * NOTE: We only deal with bold fonts for non-multichar text.
	     * Multichar bold text will have to be done by over striking (or
	     * some other shmuck must code it) -- gi1242 2006-08-19.
	     */
	    if ( MONO_BOLD_FG(rend, fore) && !wbyte )
	    {
		if( usingBoldFt )
		    rend &= ~RS_Bold;	/* We've taken care of it */

		else if( loadedBoldFt )
		{
		    usingBoldFt = 1;

# ifdef XFT_SUPPORT
		    if( ISSET_OPTION(r, Opt_xft) )
		    {
			SWAP_IT( r->TermWin.xftfont, r->TermWin.xftbfont );
		    }
		    else
# endif
		    {
			XSetFont(r->Xdisplay, r->TermWin.gc,
				r->TermWin.bfont->fid);
		    }

		    fontdiff = (r->TermWin.propfont & PROPFONT_BOLD);
		    rend &= ~RS_Bold;   /* we've taken care of it */
		}
	    }

	    /*
	     * If we are using the bold font, but don't want to render bold
	     * text, then we should restore the original font.
	     */
	    else if( usingBoldFt && !MONO_BOLD_FG( rend, fore ) )
	    {
		usingBoldFt = 0;

		/*
		 * If we're not showing a multi byte char, then we reset
		 * fontdiff to 0. If we're showing a multi byte char, then font
		 * diff will have been set elsewhere, and we should not reset
		 * it.
		 */
		if( !wbyte )
		    fontdiff    = 0;

# ifdef XFT_SUPPORT
		if( ISSET_OPTION(r, Opt_xft) )
		{
		    SWAP_IT( r->TermWin.xftfont, r->TermWin.xftbfont);
		}
		else
# endif
		{
		    XSetFont(r->Xdisplay, r->TermWin.gc, r->TermWin.font->fid);
		}
	    }
#endif

	    /*
	     * Actually do the drawing of the string here
	     */
	    if (back == Color_bg && must_clear)
	    {
		CLEAR_CHARS( r, already_cleared,
			xpixel, ypixelc, len);
		for (i = 0; i < len; i++)
		    /* don't draw empty strings */
		    if (buffer[i] != ' ')
		    {
			rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "CL Drawing '%.60s' (%d)\n", buffer, len));

			rxvt_scr_draw_string (r, xpixel, ypixelc,
				buffer, wlen, drawfunc,
				fore, back, rend,
				((refresh_type & CLIPPED_REFRESH) ?
					r->h.refreshRegion : None ));
			break;
		    }
	    }
	    else if (fprop || fontdiff)
	    {
		/* single glyph writing */
		unsigned long   pixel;

		pixel = gcvalue.foreground;
		gcvalue.foreground = gcvalue.background;
		XChangeGC(r->Xdisplay, r->TermWin.gc, GCForeground, &gcvalue);
		rxvt_fill_rectangle (r, xpixel, ypixelc,
		    (unsigned int) Width2Pixel(len),
		    (unsigned int) (Height2Pixel(1)
		    /* - r->TermWin.lineSpace */));
		gcvalue.foreground = pixel;
		XChangeGC(r->Xdisplay, r->TermWin.gc, GCForeground, &gcvalue);

		rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "PF Drawing '%.60s' (%d)\n", buffer, len));
		rxvt_scr_draw_string (r,
			xpixel, ypixelc, buffer, wlen, drawfunc,
			fore, back, rend,
			((refresh_type & CLIPPED_REFRESH) ?
				r->h.refreshRegion : None ));
	    }
	    else
	    {
		rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "NC Drawing '%.60s' (%d)\n", buffer, len));
		rxvt_scr_draw_string (r,
			xpixel, ypixelc, buffer, wlen, image_drawfunc,
			fore, back, rend,
			((refresh_type & CLIPPED_REFRESH) ?
				r->h.refreshRegion : None ));
	    }

#ifndef NO_BOLDOVERSTRIKE
# ifdef NO_BOLDOVERSTRIKE_MULTI
	    if (!wbyte)
# endif
	    if (MONO_BOLD_FG(rend, fore))
	    {
		/*
		 * If we still need to draw a bold chars, then all else has
		 * failed. Fall back to overstriking.
		 */
		rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "Overstriking %s\n", buffer ));
		rxvt_scr_draw_string (r,
			xpixel + 1, ypixelc, buffer, wlen, drawfunc,
			fore, back, rend,
			((refresh_type & CLIPPED_REFRESH) ?
				r->h.refreshRegion : None ));
	    }
#endif
	    if (rend & RS_Uline)
	    {
#ifdef XFT_SUPPORT
		if (ISSET_OPTION(r, Opt_xft) && PVTS(r)->xftvt)
		{
		    if (r->TermWin.xftfont->descent > 1)
			XDrawLine(r->Xdisplay, drawBuffer,
			    r->TermWin.gc,
			    xpixel,
			    ypixelc + r->TermWin.xftfont->ascent + 1,
			    xpixel + Width2Pixel(len) - 1,
			    ypixelc + r->TermWin.xftfont->ascent + 1);
		}
		else
#endif
		if (r->TermWin.font->descent > 1)
		    XDrawLine(r->Xdisplay, drawBuffer, r->TermWin.gc,
			xpixel,
			ypixelc + r->TermWin.font->ascent + 1,
			xpixel + Width2Pixel(len) - 1,
			ypixelc + r->TermWin.font->ascent + 1);
	    }

	    if (gcmask)	    /* restore normal colours */
	    {
		gcvalue.foreground = r->pixColors[Color_fg];
		gcvalue.background = r->pixColors[Color_bg];
		XChangeGC(r->Xdisplay, r->TermWin.gc, gcmask, &gcvalue);
	    }
	} /* for (col....) */

	/* End of E2 */
    } /* for (row....) */

    /*
     * If we've completed our refresh, and are using the bold font, we need to
     * reset it. Only needed when using XFT.
     */
    if( usingBoldFt )
    {
	usingBoldFt = 0;

#ifdef XFT_SUPPORT
	if( ISSET_OPTION(r, Opt_xft) )
	{
	    SWAP_IT( r->TermWin.xftfont, r->TermWin.xftbfont);
	}
	else
# endif
	{
	    XSetFont(r->Xdisplay, r->TermWin.gc, r->TermWin.font->fid);
	}
    }
    /* End of E */


    /*
    ** G: cleanup cursor and display outline cursor in necessary
    */
    if (showcursor)
    {
	if (r->TermWin.focus)
	{
	    int	    currow = CURROW + SVLINES;
	    srp = &(PSCR(r).rend[currow][CURCOL]);
	    *srp ^= RS_RVid;

#ifndef NO_CURSORCOLOR
	    *srp = (*srp & ~(RS_fgMask | RS_bgMask)) | cc1;
#endif
#ifdef MULTICHAR_SET
	    if (morecur)
	    {
		assert (0 == morecur || -1 == morecur || 1 == morecur);
		srp += morecur;
		*srp ^= RS_RVid;
# ifndef NO_CURSORCOLOR
		*srp = (*srp & ~(RS_fgMask | RS_bgMask)) | cc2;
# endif
	    }
#endif
	}
	else if (r->h.oldcursor.row >= 0)
	{
#ifndef NO_CURSORCOLOR
	    unsigned long   gcmask; /* Graphics Context mask */

	    gcmask = 0;
	    if (XDEPTH > 2 && ISSET_PIXCOLOR(r, Color_cursor))
	    {
		gcvalue.foreground = r->pixColors[Color_cursor];
		gcmask = GCForeground;
		XChangeGC(r->Xdisplay, r->TermWin.gc, gcmask, &gcvalue);
		    gcvalue.foreground = r->pixColors[Color_fg];
	    }
#endif

	    XDrawRectangle(r->Xdisplay, drawBuffer, r->TermWin.gc,
		Col2Pixel(r->h.oldcursor.col + morecur),
		Row2Pixel(r->h.oldcursor.row),
		(unsigned int)(Width2Pixel(1 + (morecur?1:0)) - 1),
		(unsigned int)(Height2Pixel(1)
		/* - r->TermWin.lineSpace*/ - 1));

#ifndef NO_CURSORCOLOR
	    if (gcmask)	    /* restore normal colours */
		XChangeGC(r->Xdisplay, r->TermWin.gc, gcmask, &gcvalue);
#endif
	}
    }
    /* End of G */


    /*
    ** H: cleanup selection
    */
    rxvt_scr_reverse_selection(r);


    /*
    ** I: other general cleanup
    */
    /* 
    ** clear the whole screen height, note that width == 0 is treated
    ** specially by XClearArea
    */
    if (clearfirst && r->TermWin.int_bwidth)
	rxvt_clear_area (r, 0, 0,
	    (unsigned int)r->TermWin.int_bwidth, VT_HEIGHT(r));
    /* 
    ** clear the whole screen height, note that width == 0 is treated
    ** specially by XClearArea
    */
    if (clearlast && r->TermWin.int_bwidth)
	rxvt_clear_area (r,
	    TWIN_WIDTH(r) + r->TermWin.int_bwidth, 0,
	    (unsigned int)r->TermWin.int_bwidth, VT_HEIGHT(r));

    if (refresh_type & SMOOTH_REFRESH)
	XSync(r->Xdisplay, False);

    if( (refresh_type & CLIPPED_REFRESH) && IS_REGION(r->h.refreshRegion))
    {
	/*
	 * A clipped refresh is complete. Don't restrict future refreshes.
	 */
	rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "Completed clipped refresh\n"));

	/*
	 * XSetRegion( r->Xdisplay, r->TermWin.gc, None) causes a segfault.
	 * Probably because GC's dont' accept the None region as gracefully.
	 */
	XSetClipMask( r->Xdisplay, r->TermWin.gc, None);
#ifdef XFT_SUPPORT
	if( ISSET_OPTION(r, Opt_xft) && PVTS( r)->xftvt)
	    XftDrawSetClip( PVTS( r)->xftvt, None);
#endif
    }
    else
	/* If we performed an unclipped refresh, then the screen is current */
	PVTS(r)->want_refresh = 0;

    r->h.refresh_type &= ~CLIPPED_REFRESH;
    r->h.want_clip_refresh = 0; /* clipping is current (regardless of wether we
				 performed a clipped refresh or not. */

    /* Clipping regions will now carry stale information. */
    if (IS_REGION(r->h.refreshRegion))
    {
	XDestroyRegion( r->h.refreshRegion );
	UNSET_REGION(r->h.refreshRegion);
    }

    PVTS(r)->num_scr = 0;
    r->h.num_scr_allow = 1;
}


#undef X11_DRAW_STRING_8
#undef X11_DRAW_STRING_16
#undef X11_DRAW_IMAGE_STRING_8
#undef X11_DRAW_IMAGE_STRING_16
#undef XFT_DRAW_STRING_8
#undef XFT_DRAW_STRING_16
#undef XFT_DRAW_STRING_32
#undef XFT_DRAW_STRING_UTF8

/* ------------------------------------------------------------------------- */


/* EXTPROTO */
void
rxvt_scr_clear(rxvt_t* r)
{
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_scr_clear()\n"));

    r->h.num_scr_allow = 0;
    PVTS(r)->want_refresh = 1;
    XClearWindow(r->Xdisplay, PVTS(r)->vt);
}

/* ------------------------------------------------------------------------- */
/* INTPROTO */
static void
rxvt_scr_reverse_selection(rxvt_t* r)
{
    int		 i, col, row, end_row;
    rend_t	 *srp;

    if (
	  SEL(r).op &&
	  PVTS(r)->current_screen == SEL(r).screen
       )
    {
	end_row = SVLINES - VSTART;

	i = SEL(r).beg.row + SVLINES;
	row = SEL(r).end.row + SVLINES;

	if (i >= end_row)
	    col = SEL(r).beg.col;
	else
	{
	    col = 0;
	    i = end_row;
	}

	end_row += r->TermWin.nrow;
	for (; i < row && i < end_row; i++, col = 0)
	    for (srp = PSCR(r).rend[i]; col < r->TermWin.ncol; col++)
#ifndef OPTION_HC
		srp[col] ^= RS_RVid;
#else
		srp[col] ^= RS_Blink;
#endif
	if (i == row && i < end_row)
	    for (srp = PSCR(r).rend[i]; col < SEL(r).end.col; col++)
#ifndef OPTION_HC
		srp[col] ^= RS_RVid;
#else
		srp[col] ^= RS_Blink;
#endif
    }
}

/* ------------------------------------------------------------------------- *
 *			   CHARACTER SELECTION				 *
 * ------------------------------------------------------------------------- */

/*
 * -PVTS(r)->nscrolled <= (selection row) <= r->TermWin.nrow - 1
 */
/* EXTPROTO */
void
rxvt_selection_check(rxvt_t* r, int check_more)
{
    row_col_t	   pos;

    if (!SEL(r).op ||
	SEL(r).screen != PVTS(r)->current_screen)
	return;

    pos.row = pos.col = 0;
    if (
	    (SEL(r).beg.row < -(int32_t)PVTS(r)->nscrolled) ||
	    (SEL(r).beg.row >= r->TermWin.nrow) ||
	    (SEL(r).mark.row < -(int32_t)PVTS(r)->nscrolled) ||
	    (SEL(r).mark.row >= r->TermWin.nrow) ||
	    (SEL(r).end.row < -(int32_t)PVTS(r)->nscrolled) ||
	    (SEL(r).end.row >= r->TermWin.nrow) ||
	    ( check_more == 1 &&
	      PVTS(r)->current_screen == SEL(r).screen &&
	      !RC_BEFORE(PSCR(r).cur, SEL(r).beg) &&
	      RC_BEFORE(PSCR(r).cur, SEL(r).end)) ||
	    ( check_more == 2 &&
	      RC_BEFORE(SEL(r).beg, pos) &&
	      RC_AFTER(SEL(r).end, pos)) ||
	    ( check_more == 3 &&
	      RC_AFTER(SEL(r).end, pos)) ||
	    ( check_more == 4	/* screen width change */ &&
	      ( SEL(r).beg.row != SEL(r).end.row ||
		SEL(r).end.col > r->TermWin.ncol))
       )
    {
	CLEAR_SELECTION(r);
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Paste a selection direct to the command fd
 */
/* INTPROTO */
static void
rxvt_paste_str(rxvt_t* r,
	const unsigned char *data, unsigned int nitems)
{
    unsigned int    i, j, n;
    unsigned char  *ds = rxvt_malloc(PROP_SIZE);
    
    /*
     * Convert normal newline chars into common keyboard Return key sequence
     */
    for (i = 0; i < nitems; i += PROP_SIZE)
    {
	n = min(nitems - i, PROP_SIZE);
	MEMCPY(ds, data + i, n);
	for (j = 0; j < n; j++)
	    if (ds[j] == '\n')
		ds[j] = '\r';
	rxvt_tt_write(r, ds, (int)n);
    }
    rxvt_free(ds);
}


/* ------------------------------------------------------------------------- */
/*
 * Respond to a notification that a primary selection has been sent
 * EXT: SelectionNotify
 */
/* EXTPROTO */
int
rxvt_selection_paste(rxvt_t* r, Window win, Atom prop, Bool delete_prop)
{
    long	    nread = 0;
    unsigned long   bytes_after;
    XTextProperty   ct;
#ifdef MULTICHAR_SET
    int		    dummy_count;
    char**	    cl;
#endif

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_selection_paste (%08lx, %lu, %d), wait=%2x\n", win, (unsigned long)prop, (int)delete_prop, r->h.selection_wait));

    if (NOT_ATOM(prop))	    /* check for failed XConvertSelection */
    {
#ifdef MULTICHAR_SET
	if ((r->h.selection_type & Sel_CompoundText))
	{
	    int	    selnum = r->h.selection_type & Sel_whereMask;

	    r->h.selection_type = 0;
	    if (selnum != Sel_direct)
		rxvt_selection_request_other(r, XA_STRING, selnum);
	}
#endif
	return 0;
    }

    for (;;)
    {
	if(
	     XGetWindowProperty( r->Xdisplay, win, prop, (long) (nread/4),
		    (long) (PROP_SIZE / 4), delete_prop, AnyPropertyType,
		    &ct.encoding, &ct.format, &ct.nitems, &bytes_after,
		    &ct.value)
		!= Success
	  )
	    break;
	if( ct.encoding == None )
	{
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_selection_paste: property didn't exist!\n"));
	    break;
	}

	if (ct.value == NULL)
	{
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_selection_paste: property shooting blanks!\n"));
	    continue;
	}

	if (ct.nitems == 0)
	{
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_selection_paste: property empty - also INCR end\n"));

	    if( r->h.selection_wait == Sel_normal && nread == 0 )
	    {
		/*
		 * pass through again trying CUT_BUFFER0 if we've come from
		 * XConvertSelection() but nothing was presented
		 */
		rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_selection_request: pasting CUT_BUFFER0\n"));
		rxvt_selection_paste(r, XROOT, XA_CUT_BUFFER0, False);
	    }
	    nread = -1;	    /* discount any previous stuff */
	    break;
	}

	nread += ct.nitems;
#ifdef MULTICHAR_SET
	if (
	      XmbTextPropertyToTextList(r->Xdisplay, &ct, &cl, &dummy_count)
	      	    == Success
	      && cl
	   )
	{
	    rxvt_paste_str(r, (const unsigned char*) cl[0],
		    STRLEN(cl[0]));
	    XFreeStringList(cl);
	}
	else
#endif
	    rxvt_paste_str(r, ct.value, (unsigned int) ct.nitems);

	if( bytes_after == 0 )
	    break;

	XFree(ct.value);
	ct.value = 0;
    }

    if (ct.value)
	XFree(ct.value);

    if (r->h.selection_wait == Sel_normal)
	r->h.selection_wait = Sel_none;

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_selection_paste: bytes written: %ld\n", nread));
    return (int)nread;
}


/*
 * INCR support originally provided by Paul Sheer <psheer@obsidian.co.za>
 */
/* EXTPROTO */
void
rxvt_selection_property(rxvt_t* r, Window win, Atom prop)
{
    int		 reget_time = 0;

    if (NOT_ATOM(prop))
	return;

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_selection_property(%08lx, %lu)\n", win, (unsigned long)prop));
    if (r->h.selection_wait == Sel_normal)
    {
	int	     a, afmt;
	Atom		atype;
	unsigned long   bytes_after, nitems;
	unsigned char  *s = NULL;

	a = XGetWindowProperty(r->Xdisplay, win, prop, 0L, 1L, False,
		   r->h.xa[XA_INCR], &atype, &afmt, &nitems,
		   &bytes_after, &s);
	if (s)
	    XFree(s);
	if (a != Success)
	    return;
#ifndef OS_CYGWIN
	if (atype == r->h.xa[XA_INCR])	    /* start an INCR transfer */
	{
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_selection_property: INCR: starting transfer\n"));
	    XDeleteProperty(r->Xdisplay, win, prop);
	    XFlush(r->Xdisplay);
	    reget_time = 1;
	    r->h.selection_wait = Sel_incr;
	}
#endif
    }
    else if (r->h.selection_wait == Sel_incr)
    {
	reget_time = 1;
	if (rxvt_selection_paste(r, win, prop, True) == -1)
	{
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_selection_property: INCR: clean end\n"));
	    r->h.selection_wait = Sel_none;
	    r->h.timeout[TIMEOUT_INCR].tv_sec = 0; /* turn off timer */
	}
    }
    if (reget_time)	/* received more data so reget time */
    {
	gettimeofday( &(r->h.timeout[TIMEOUT_INCR]), NULL);
	/* ten seconds wait */
	r->h.timeout[TIMEOUT_INCR].tv_sec += 10;
    }
}


/* ------------------------------------------------------------------------- */
/*
 * Request the content of a selection buffer: 
 *
 * EXT: button 2 release
 */
/* EXTPROTO */
void
rxvt_selection_request_by_sel(rxvt_t* r, Time tm, int x, int y,int sel)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_selection_request (%lu, %d, %d)\n", tm, x, y ));

    if (x < 0 || x >= VT_WIDTH(r) || y < 0 || y >= VT_HEIGHT(r))
	return;		/* outside window */

	r->h.selection_request_time = tm;
	r->h.selection_wait = Sel_normal;

#ifdef MULTICHAR_SET
   r->h.selection_type = Sel_CompoundText;
#else
   r->h.selection_type = 0;
#endif
   rxvt_selection_request_other(r,
#ifdef MULTICHAR_SET
	 r->h.xa[XA_COMPOUND_TEXT],
#else
	 XA_STRING,
#endif
		 sel);
}


/* ------------------------------------------------------------------------- */
/*
 * Request the current selection: 
 * Order: > internal selection if available
 *	> PRIMARY, SECONDARY, CLIPBOARD if ownership is claimed (+)
 *	> CUT_BUFFER0
 * (+) if ownership is claimed but property is empty, rxvt_selection_paste()
 *   will auto fallback to CUT_BUFFER0
 * EXT: button 2 release
 */
/* EXTPROTO */
void
rxvt_selection_request(rxvt_t* r, Time tm, int x, int y)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_selection_request (%lu, %d, %d)\n", tm, x, y ));

    if (x < 0 || x >= VT_WIDTH(r) || y < 0 || y >= VT_HEIGHT(r))
	return;		/* outside window */

    if( SEL(r).text != NULL )	    /* internal selection */
    {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_selection_request: pasting internal\n" ));
	rxvt_paste_str( r, SEL(r).text, SEL(r).len );
	return;
    }
    else
    {
	int	     i;

	r->h.selection_request_time = tm;
	r->h.selection_wait = Sel_normal;

	for (i = Sel_Primary; i <= Sel_Clipboard; i++)
	{
#ifdef MULTICHAR_SET
	    r->h.selection_type = Sel_CompoundText;
#else
	    r->h.selection_type = 0;
#endif
	    if (rxvt_selection_request_other(r,
#ifdef MULTICHAR_SET
		 r->h.xa[XA_COMPOUND_TEXT],
#else
		 XA_STRING,
#endif
		 i))
	    return;
	}
    }

    /* don't loop in rxvt_selection_paste() */
    r->h.selection_wait = Sel_none;
    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_selection_request: pasting CUT_BUFFER0\n" ));
    rxvt_selection_paste(r, XROOT, XA_CUT_BUFFER0, False);
}


/* INTPROTO */
static int
rxvt_selection_request_other(rxvt_t* r, Atom target, int selnum)
{
    Atom	    sel;
#ifdef DEBUG
    char	   *debug_xa_names[] = { "PRIMARY", "SECONDARY", "CLIPBOARD" };
#endif

    r->h.selection_type |= selnum;
    if (selnum == Sel_Primary)
	sel = XA_PRIMARY;
    else if (selnum == Sel_Secondary)
	sel = XA_SECONDARY;
    else
	sel = r->h.xa[XA_CLIPBOARD];
    if (XGetSelectionOwner(r->Xdisplay, sel) != None)
    {
	rxvt_dbgmsg(( DBG_DEBUG, DBG_SCREEN, "rxvt_selection_request_other: "
		    "pasting %s\n", debug_xa_names[selnum] ));

	XConvertSelection(r->Xdisplay, sel, target,
	    r->h.xa[XA_VT_SELECTION], PVTS(r)->vt,
	    r->h.selection_request_time);
	return 1;
    }
    return 0;
}


/* ------------------------------------------------------------------------- */
/*
 * Paste the content of the file specified by filename to the 
 * currently active tab 
 * EXT: button 2 release
 */
/* EXTPROTO */
void
rxvt_paste_file(rxvt_t* r, Time tm, int x, int y, char* filename)
{
	rxvt_dbgmsg(( DBG_DEBUG, DBG_SCREEN,
				"rxvt_paste_file (%lu, %d, %d) %s\n", tm, x, y,
				filename ));

	if (x < 0 || x >= VT_WIDTH(r) || y < 0 || y >= VT_HEIGHT(r))
		return;		/* outside window */

	char buffer[BUFSIZ];
	char TAINTED * str;
	FILE * fdpaste;

#ifdef HAVE_WORDEXP_H
	wordexp_t p;
	int wordexp_result;
	char* filename_original_ptr = filename;

	/* perform a shell-like expansion of the provided filename */
	wordexp_result = wordexp(filename, &p, 0);
	if( wordexp_result == 0 && p.we_wordc == 1 )
		filename = *p.we_wordv;
	else
	{
		rxvt_msg( DBG_ERROR, DBG_SCREEN,
				"Error expanding %s, or possibly ambiguous expansion\n",
				filename );
		rxvt_msg( DBG_INFO, DBG_SCREEN, "wordexp_result=%i\n", wordexp_result );
	}
#endif

	if (NOT_NULL(fdpaste = fopen( filename , "r")))
	{
		while (NOT_NULL(str = fgets(buffer, sizeof(buffer), fdpaste)))
			rxvt_paste_str( r, (const unsigned char*) str , STRLEN(str));

		fclose(fdpaste);
	}
	else
	{
		rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN,
					"rxvt_paste_file : unable to open file '%s'\n", filename));
	}

#ifdef HAVE_WORDEXP_H
	filename = filename_original_ptr;
	wordfree(&p);
#endif
	return;
}



/* ------------------------------------------------------------------------- */
/*
 * Clear all selected text
 * EXT: SelectionClear
 */
/* EXTPROTO */
void
rxvt_process_selectionclear(rxvt_t* r)
{
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_process_selectionclear ()\n"));

    PVTS(r)->want_refresh = 1;
    if (SEL(r).text)
	rxvt_free(SEL(r).text);
    SEL(r).text = NULL;
    SEL(r).len = 0;
    CLEAR_SELECTION(r);

    SEL(r).op = SELECTION_CLEAR;
    SEL(r).screen = PRIMARY;
    SEL(r).clicks = 0;
}


/* ------------------------------------------------------------------------- */
/*
 * Copy a selection into the cut buffer
 * EXT: button 1 or 3 release
 */
/* EXTPROTO */
void
rxvt_selection_make(rxvt_t* r, Time tm)
{
    int		    i, col, end_col, row, end_row;
    unsigned char*  new_selection_text;
    unsigned char*  str;
    text_t*	    t;
#ifdef MULTICHAR_SET
    rend_t*	    tr;
#endif
#ifdef ACS_ASCII
    rend_t*	    re;
#endif

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_selection_make (): sel.op=%d, sel.clicks=%d\n", SEL(r).op, SEL(r).clicks));
    switch (SEL(r).op)
    {
	case SELECTION_CONT:
	    break;
	case SELECTION_INIT:
	    CLEAR_SELECTION(r);
	    /* FALLTHROUGH */
	case SELECTION_BEGIN:
	    SEL(r).op = SELECTION_DONE;
	    /* FALLTHROUGH */
	default:
	    return;
    }
    SEL(r).op = SELECTION_DONE;

    if (SEL(r).clicks == 4)
	return;		/* nothing selected, go away */

    assert ((SEL(r).end.row - SEL(r).beg.row + 1) > 0);
    assert ((r->TermWin.ncol + 1) > 0);
    i = (SEL(r).end.row - SEL(r).beg.row + 1)
	* (r->TermWin.ncol + 1) + 1;
    /* possible integer overflow? */
    assert (i > 0);
    str = rxvt_malloc(i * sizeof(char));

    new_selection_text = (unsigned char *)str;

    col = SEL(r).beg.col;
    MAX_IT(col, 0);
    row = SEL(r).beg.row + SVLINES;
    end_row = SEL(r).end.row + SVLINES;

    /*
    ** A: rows before end row
    */
    for (; row < end_row; row++, col = 0)
    {
	t = &(PSCR(r).text[row][col]);
#ifdef MULTICHAR_SET
	tr = &(PSCR(r).rend[row][col]);
#endif	/* MULTICHAR_SET */
#ifdef ACS_ASCII
	re = &(PSCR(r).rend[row][col]);
#endif
	if ((end_col = PSCR(r).tlen[row]) == -1)
	    end_col = r->TermWin.ncol;


	/*
	** Looks like a completely mess. Think about the logic here
	** carefully. ;-)
	** Patch source:
	** http://gentoo.nedlinux.nl/distfiles/rxvt-2.7.10-rk.patch
	*/
	for (; col < end_col; col++, str++, t++)
	{
#ifdef MULTICHAR_SET
	    if (
		  (ENC_EUCJ == r->encoding_method) && (*t & 0x80)
		  && !(*tr & RS_multiMask)
	       )
	    {
		*str++ = 0x8E;
	    }
	    tr ++;
#endif	/* MULTICHAR_SET */
#ifdef ACS_ASCII
	    if ((*re++ & RS_acsFont) && *t >= 0x60 && *t < 0x80)
		*str = r->h.rs[Rs_acs_chars][(*t) - 0x60];
	    else
#endif	/* ACS_ASCII */
	    *str = *t;
	}


	if (PSCR(r).tlen[row] != -1)
	{
#ifdef DONT_SELECT_TRAILING_SPACES
	    STRIP_TRAILING_SPACE(str, new_selection_text);
#endif
	    *str++ = '\n';
	}
    }

    /*
    ** B: end row
    */
    t = &(PSCR(r).text[row][col]);
#ifdef MULTICHAR_SET
    tr = &(PSCR(r).rend[row][col]);
#endif	/* MULTICHAR_SET */
#ifdef ACS_ASCII
    re = &(PSCR(r).rend[row][col]);
#endif
    end_col = PSCR(r).tlen[row];
    if (end_col == -1 || SEL(r).end.col <= end_col)
	end_col = SEL(r).end.col;
    MIN_IT(end_col, r->TermWin.ncol);	/* CHANGE */


    /*
    ** Looks like a completely mess. Think about the logic here
    ** carefully. ;-)
    ** Patch source:
    ** http://gentoo.nedlinux.nl/distfiles/rxvt-2.7.10-rk.patch
    */
    for (; col < end_col; col++, str++, t++)
    {
#ifdef MULTICHAR_SET
	if (
	      (ENC_EUCJ == r->encoding_method) && (*t & 0x80)
	      && !(*tr & RS_multiMask)
	   )
	{
	    *str++ = 0x8E;
	}
	tr ++;
#endif	/* MULTICHAR_SET */
#ifdef ACS_ASCII
	if ((*re++ & RS_acsFont) && *t >= 0x60 && *t < 0x80)
	    *str = r->h.rs[Rs_acs_chars][(*t) - 0x60];
	else
#endif	/* ACS_ASCII */
	*str = *t;
    }

#ifdef DONT_SELECT_TRAILING_SPACES
    STRIP_TRAILING_SPACE(str, new_selection_text);
#endif


#ifndef NO_OLD_SELECTION
    if (r->selection_style == OLD_SELECT)
	if (end_col == r->TermWin.ncol)
	{
	    *str++ = '\n';
	}
#endif
#ifndef NO_NEW_SELECTION
    if (r->selection_style != OLD_SELECT)
	if (end_col != SEL(r).end.col)
	{
	    *str++ = '\n';
	}
#endif
    *str = '\0';
    if ((i = STRLEN((char *)new_selection_text)) == 0)
    {
	rxvt_free(new_selection_text);
	return;
    }
    SEL(r).len = i;
    if (SEL(r).text)
	rxvt_free(SEL(r).text);
    SEL(r).text = new_selection_text;

    XSetSelectionOwner(r->Xdisplay, XA_PRIMARY, PVTS(r)->vt, tm);
    if (XGetSelectionOwner(r->Xdisplay, XA_PRIMARY) != PVTS(r)->vt)
	rxvt_msg (DBG_ERROR, DBG_SCREEN, "can't get primary selection");
    XChangeProperty(r->Xdisplay, XROOT, XA_CUT_BUFFER0, XA_STRING, 8,
	PropModeReplace, SEL(r).text, (int)SEL(r).len);
    r->h.selection_time = tm;
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_selection_make (): sel.len=%d\n", SEL(r).len));
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "sel.text=%s\n", SEL(r).text));
}


/* ------------------------------------------------------------------------- */
/*
 * Mark or select text based upon number of clicks: 1, 2, or 3
 * EXT: button 1 press
 */
/* EXTPROTO */
void
rxvt_selection_click(rxvt_t* r, int clicks, int x, int y)
{
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_selection_click (%d, %d, %d)\n", clicks, x, y));

    clicks = ((clicks - 1) % 3) + 1;
    SEL(r).clicks = clicks; /* save clicks so extend will work */

    rxvt_selection_start_colrow(r, Pixel2Col(x), Pixel2Row(y));

    if (clicks == 2 || clicks == 3)
	rxvt_selection_extend_colrow(r, SEL(r).mark.col,
	    SEL(r).mark.row + VSTART,
	    0,	/* button 3  */
	    1,	/* button press */
	    0);	/* click change */
}


/* ------------------------------------------------------------------------- */
/*
 * Mark a selection at the specified col/row
 */
/* INTPROTO */
static void
rxvt_selection_start_colrow(rxvt_t* r, int col, int row)
{
    PVTS(r)->want_refresh = 1;
    SEL(r).mark.col = col;
    SEL(r).mark.row = row - VSTART;
    MAX_IT(SEL(r).mark.row, -(int32_t)PVTS(r)->nscrolled);
    MIN_IT(SEL(r).mark.row, (int32_t)r->TermWin.nrow - 1);
    MAX_IT(SEL(r).mark.col, 0);
    MIN_IT(SEL(r).mark.col, (int32_t)r->TermWin.ncol - 1);

    if (SEL(r).op)
    {
	/* clear the old selection */
	SEL(r).beg.row = SEL(r).end.row = SEL(r).mark.row;
	SEL(r).beg.col = SEL(r).end.col = SEL(r).mark.col;
    }
    SEL(r).op = SELECTION_INIT;
    SEL(r).screen = PVTS(r)->current_screen;
}


/* ------------------------------------------------------------------------- */
/*
 * Word select: select text for 2 clicks
 * We now only find out the boundary in one direction
 */

/* what do we want: spaces/tabs are delimiters or cutchars or non-cutchars */
#define DELIMIT_TEXT(x) \
    (((x) == ' ' || (x) == '\t') ? 2 : (STRCHR(r->h.rs[Rs_cutchars], (x)) != NULL))
#ifdef MULTICHAR_SET
# define DELIMIT_REND(x)    (((x) & RS_multiMask) ? 1 : 0)
#else
# define DELIMIT_REND(x)    1
#endif

/* INTPROTO */
static void
rxvt_selection_delimit_word(rxvt_t* r, enum page_dirn dirn, const row_col_t *mark, row_col_t *ret)
{
    int		 col, row, dirnadd, tcol, trow, w1, w2;
    row_col_t	   bound;
    text_t	 *stp;
    rend_t	 *srp;


    if (dirn == UP)
    {
	bound.row = SVLINES - PVTS(r)->nscrolled - 1;
	bound.col = 0;
	dirnadd = -1;
    }
    else
    {
	bound.row = SVLINES + r->TermWin.nrow;
	bound.col = r->TermWin.ncol - 1;
	dirnadd = 1;
    }
    row = mark->row + SVLINES;
    col = mark->col;
    MAX_IT(col, 0);
/* find the edge of a word */
    stp = &(PSCR(r).text[row][col]);
    w1 = DELIMIT_TEXT(*stp);

    if (r->selection_style != NEW_SELECT)
    {
	if (w1 == 1)
	{
	    stp += dirnadd;
	    if (DELIMIT_TEXT(*stp) == 1)
		goto Old_Word_Selection_You_Die;
	    col += dirnadd;
	}
	w1 = 0;
    }
    srp = (&PSCR(r).rend[row][col]);
    w2 = DELIMIT_REND(*srp);

    for (;;)
    {
	for (; col != bound.col; col += dirnadd)
	{
	    stp += dirnadd;
	    if (DELIMIT_TEXT(*stp) != w1)
		break;
	    srp += dirnadd;
	    if (DELIMIT_REND(*srp) != w2)
		break;
	}
	if ((col == bound.col) && (row != bound.row))
	{
	    if (PSCR(r).tlen[(row - (dirn == UP ? 1 : 0))] == -1)
	    {
		trow = row + dirnadd;
		tcol = dirn == UP ? r->TermWin.ncol - 1 : 0;
		if (PSCR(r).text[trow] == NULL)
		    break;
		stp = &(PSCR(r).text[trow][tcol]);
		srp = &(PSCR(r).rend[trow][tcol]);
		if (DELIMIT_TEXT(*stp) != w1 ||
		    DELIMIT_REND(*srp) != w2)
		    break;
		row = trow;
		col = tcol;
		continue;
	    }
	}
	break;
    }

Old_Word_Selection_You_Die:
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_selection_delimit_word (%s,...) @ (r:%3d, c:%3d) has boundary (r:%3d, c:%3d)\n", (dirn == UP ? "up    " : "down"), mark->row, mark->col, row - SVLINES, col));

    if (dirn == DN)
	col++;		/* put us on one past the end */

/* Poke the values back in */
    ret->row = row - SVLINES;
    ret->col = col;
}


/* ------------------------------------------------------------------------- */
/*
 * Extend the selection to the specified x/y pixel location
 * EXT: button 3 press; button 1 or 3 drag
 * flag == 0 ==> button 1
 * flag == 1 ==> button 3 press
 * flag == 2 ==> button 3 motion
 */
/* EXTPROTO */
void
rxvt_selection_extend(rxvt_t* r, int x, int y, int flag)
{
    int		 col, row;

    col = Pixel2Col(x);
    row = Pixel2Row(y);
    MAX_IT(row, 0);
    MIN_IT(row, (int)r->TermWin.nrow - 1);
    MAX_IT(col, 0);
    MIN_IT(col, (int)r->TermWin.ncol);

#ifndef NO_NEW_SELECTION
/*
 * If we're selecting characters (single click) then we must check first
 * if we are at the same place as the original mark.  If we are then
 * select nothing.  Otherwise, if we're to the right of the mark, you have to
 * be _past_ a character for it to be selected.
 */
    if (r->selection_style != OLD_SELECT)
    {
	if (
	      ((SEL(r).clicks % 3) == 1) && !flag
	      && (
		    col == SEL(r).mark.col
		    && (row == SEL(r).mark.row + VSTART)
		 )
	   )
	{
	    /* select nothing */
	    SEL(r).beg.row = SEL(r).end.row = 0;
	    SEL(r).beg.col = SEL(r).end.col = 0;
	    SEL(r).clicks = 4;
	    PVTS(r)->want_refresh = 1;
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_SCREEN, "rxvt_selection_extend () sel.clicks = 4\n"));
	    return;
	}
    }
#endif
    if (SEL(r).clicks == 4)
	SEL(r).clicks = 1;
    rxvt_selection_extend_colrow(r, col, row, !!flag,
	/* ? button 3	  */
	 flag == 1 ? 1 : 0, /* ? button press  */
	 0);	/* no click change */
}


#ifdef MULTICHAR_SET
/* INTPROTO */
static void
rxvt_selection_adjust_kanji(rxvt_t* r)
{
    int		 c1, r1;

    if (SEL(r).beg.col > 0)
    {
	r1 = SEL(r).beg.row + SVLINES;
	c1 = SEL(r).beg.col;
	if (IS_MULTI2(PSCR(r).rend[r1][c1]) &&
	    IS_MULTI1(PSCR(r).rend[r1][c1 - 1]))
	    SEL(r).beg.col--;
    }
    if (SEL(r).end.col < r->TermWin.ncol)
    {
	r1 = SEL(r).end.row + SVLINES;
	c1 = SEL(r).end.col;
	if (IS_MULTI1(PSCR(r).rend[r1][c1 - 1]) &&
	    IS_MULTI2(PSCR(r).rend[r1][c1]))
	    SEL(r).end.col++;
    }
}
#endif		    /* MULTICHAR_SET */


/* ------------------------------------------------------------------------- */
/*
 * Extend the selection to the specified col/row
 */
/* INTPROTO */
static void
rxvt_selection_extend_colrow(rxvt_t* r, int32_t col, int32_t row, int button3, int buttonpress, int clickchange)
{
    unsigned int    ncol = r->TermWin.ncol;
    row_col_t	    pos;
#ifndef NO_NEW_SELECTION
    int		 end_col;
    enum {
	LEFT, RIGHT
    } closeto = RIGHT;
#endif


    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_selection_extend_colrow (c:%d, r:%d, %d, %d) clicks:%d, op:%d\n", col, row, button3, buttonpress, SEL(r).clicks, SEL(r).op));
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_selection_extend_colrow () ENT  b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)\n", SEL(r).beg.row, SEL(r).beg.col, SEL(r).mark.row, SEL(r).mark.col, SEL(r).end.row, SEL(r).end.col));

    PVTS(r)->want_refresh = 1;
    switch (SEL(r).op)
    {
	case SELECTION_INIT:
	    CLEAR_SELECTION(r);
	    SEL(r).op = SELECTION_BEGIN;
	    /* FALLTHROUGH */
	case SELECTION_BEGIN:
	    if (row != SEL(r).mark.row ||
		col != SEL(r).mark.col ||
		(!button3 && buttonpress))
		SEL(r).op = SELECTION_CONT;
	    break;
	case SELECTION_DONE:
	    SEL(r).op = SELECTION_CONT;
	    /* FALLTHROUGH */
	case SELECTION_CONT:
	    break;
	case SELECTION_CLEAR:
	    rxvt_selection_start_colrow(r, col, row);
	    /* FALLTHROUGH */
	default:
	    return;
    }

    if (
	  SEL(r).beg.col == SEL(r).end.col
	  && SEL(r).beg.col != SEL(r).mark.col
	  && SEL(r).beg.row == SEL(r).end.row
	  && SEL(r).beg.row != SEL(r).mark.row
       )
    {
	SEL(r).beg.col = SEL(r).end.col = SEL(r).mark.col;
	SEL(r).beg.row = SEL(r).end.row = SEL(r).mark.row;
	rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN,
	    "rxvt_selection_extend_colrow () "
	    "ENT2 b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)\n",
	    SEL(r).beg.row, SEL(r).beg.col, SEL(r).mark.row,
	    SEL(r).mark.col, SEL(r).end.row, SEL(r).end.col));
    }

    pos.col = col;
    pos.row = row;

    pos.row -= VSTART;	/* adjust for scroll */


#ifndef NO_OLD_SELECTION
    /*
    ** This mimics some of the selection behaviour of version 2.20
    ** and before.
    ** There are no ``selection modes'', button3 is always character
    ** extension.
    ** Note: button3 drag is always available, c.f. v2.20
    ** Selection always terminates (left or right as appropriate) at
    ** the mark.
    */
    if (r->selection_style == OLD_SELECT)
    {
	if (SEL(r).clicks == 1 || button3)
	{
	    if (r->h.hate_those_clicks)
	    {
		r->h.hate_those_clicks = 0;
		if (SEL(r).clicks == 1)
		{
		    SEL(r).beg.row = SEL(r).mark.row;
		    SEL(r).beg.col = SEL(r).mark.col;
		}
		else
		{
		    SEL(r).mark.row = SEL(r).beg.row;
		    SEL(r).mark.col = SEL(r).beg.col;
		}
	    }

	    if (RC_BEFORE(pos, SEL(r).mark))
	    {
		SEL(r).end.row = SEL(r).mark.row;
		SEL(r).end.col = SEL(r).mark.col + 1;
		SEL(r).beg.row = pos.row;
		SEL(r).beg.col = pos.col;
	    }
	    else
	    {
		SEL(r).beg.row = SEL(r).mark.row;
		SEL(r).beg.col = SEL(r).mark.col;
		SEL(r).end.row = pos.row;
		SEL(r).end.col = pos.col + 1;
	    }
# ifdef MULTICHAR_SET
	    rxvt_selection_adjust_kanji(r);
# endif		    /* MULTICHAR_SET */
	}
	else if (SEL(r).clicks == 2)
	{
	    rxvt_selection_delimit_word(r, UP, &(SEL(r).mark),
		&(SEL(r).beg));
	    rxvt_selection_delimit_word(r, DN, &(SEL(r).mark),
		&(SEL(r).end));
	    r->h.hate_those_clicks = 1;
	}
	else if (SEL(r).clicks == 3)
	{
	    SEL(r).beg.row = SEL(r).end.row = SEL(r).mark.row;
	    SEL(r).beg.col = 0;
	    SEL(r).end.col = ncol;
	    r->h.hate_those_clicks = 1;
	}

	rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_selection_extend_colrow () EXIT b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)\n", SEL(r).beg.row, SEL(r).beg.col, SEL(r).mark.row, SEL(r).mark.col, SEL(r).end.row, SEL(r).end.col));
	return;
    }
#endif		    /* ! NO_OLD_SELECTION */


#ifndef NO_NEW_SELECTION
    /* selection_style must not be OLD_SELECT to get here */
    /*
    ** This is mainly xterm style selection with a couple of
    ** differences, mainly in the way button3 drag extension
    ** works.
    ** We're either doing: button1 drag; button3 press; or
    ** button3 drag
    **  a) button1 drag : select around a midpoint/word/line -
    ** that point/word/line is always at the left/right edge
    ** of the SEL(r).
    **  b) button3 press: extend/contract character/word/line
    ** at whichever edge of the selection we are closest to.
    **  c) button3 drag : extend/contract character/word/line
    ** - we select around a point/word/line which is either
    ** the start or end of the selection and it was decided
    ** by whichever point/word/line was `fixed' at the time
    ** of the most recent button3 press
    */
    if (button3 && buttonpress)	    /* button3 press */
    {
	/* first determine which edge of the selection we are
	** closest to
	*/
	if (RC_BEFORE(pos, SEL(r).beg) ||
	    (!RC_AFTER(pos, SEL(r).end) &&
	     (((pos.col - SEL(r).beg.col) +
	       ((pos.row - SEL(r).beg.row) * ncol)) <
	      ((SEL(r).end.col - pos.col) +
	       ((SEL(r).end.row - pos.row) * ncol)))))
	     closeto = LEFT;

	if (closeto == LEFT)
	{
	    SEL(r).beg.row = pos.row;
	    SEL(r).beg.col = pos.col;
	    SEL(r).mark.row = SEL(r).end.row;
	    SEL(r).mark.col = SEL(r).end.col
		    - (SEL(r).clicks == 2);
	}
	else
	{
	    SEL(r).end.row = pos.row;
	    SEL(r).end.col = pos.col;
	    SEL(r).mark.row = SEL(r).beg.row;
	    SEL(r).mark.col = SEL(r).beg.col;
	}
    }
    else	    /* button1 drag or button3 drag */
    {
	if (RC_AFTER(SEL(r).mark, pos))
	{
	    if ((SEL(r).mark.row == SEL(r).end.row) &&
		(SEL(r).mark.col == SEL(r).end.col) &&
		clickchange && SEL(r).clicks == 2)
		SEL(r).mark.col--;
	    SEL(r).beg.row = pos.row;
	    SEL(r).beg.col = pos.col;
	    SEL(r).end.row = SEL(r).mark.row;
	    SEL(r).end.col = SEL(r).mark.col
	       + (SEL(r).clicks == 2);
	}
	else
	{
	    SEL(r).beg.row = SEL(r).mark.row;
		SEL(r).beg.col = SEL(r).mark.col;
		SEL(r).end.row = pos.row;
		SEL(r).end.col = pos.col;
	}
    }


    if (SEL(r).clicks == 1)
    {
	end_col = PSCR(r).tlen[SEL(r).beg.row + SVLINES];
	if (end_col != -1 && SEL(r).beg.col > end_col)
	{
#if 1
	    SEL(r).beg.col = ncol;
#else
	    if (SEL(r).beg.row != SEL(r).end.row)
		SEL(r).beg.col = ncol;
	    else
		SEL(r).beg.col = SEL(r).mark.col;
#endif
	}
	end_col = PSCR(r).tlen[SEL(r).end.row +
		    SVLINES];
	if (end_col != -1 && SEL(r).end.col > end_col)
	    SEL(r).end.col = ncol;

# ifdef MULTICHAR_SET
	rxvt_selection_adjust_kanji(r);
# endif		    /* MULTICHAR_SET */
    }
    else if (SEL(r).clicks == 2)
    {
	if (RC_AFTER(SEL(r).end, SEL(r).beg))
	    SEL(r).end.col--;
	rxvt_selection_delimit_word(r, UP, &(SEL(r).beg),
	    &(SEL(r).beg));
	rxvt_selection_delimit_word(r, DN, &(SEL(r).end),
	    &(SEL(r).end));
    }
    else if (SEL(r).clicks == 3)
    {
#ifndef NO_FRILLS
	if (ISSET_OPTION(r, Opt_tripleclickwords))
	{
	    int		 end_row;

	    rxvt_selection_delimit_word(r, UP, &(SEL(r).beg),
		&(SEL(r).beg));
	    end_row = PSCR(r).tlen[SEL(r).mark.row +
			SVLINES];
	    for (
		  end_row = SEL(r).mark.row;
		  end_row < r->TermWin.nrow;
		  end_row++
		)
	    {
		end_col = PSCR(r).tlen[end_row + SVLINES];
		if (end_col != -1)
		{
		    SEL(r).end.row = end_row;
		    SEL(r).end.col = end_col;
		    rxvt_selection_trim(r);
		    break;
		}
	    }	/* for */
	}
	else
#endif
	{
	    if (RC_AFTER(SEL(r).mark, SEL(r).beg))
		SEL(r).mark.col++;
	    SEL(r).beg.col = 0;
	    SEL(r).end.col = ncol;
	}
    }	/* if (ISSET_OPTION(r, Opt_tripleclickwords)) */

    if (button3 && buttonpress)
    {
	/* mark may need to be changed */
	if (closeto == LEFT)
	{
	    SEL(r).mark.row = SEL(r).end.row;
	    SEL(r).mark.col = SEL(r).end.col - (SEL(r).clicks == 2);
	}
	else
	{
	    SEL(r).mark.row = SEL(r).beg.row;
	    SEL(r).mark.col = SEL(r).beg.col;
	}
    }

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_SCREEN, "rxvt_selection_extend_colrow () EXIT b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)\n", SEL(r).beg.row, SEL(r).beg.col, SEL(r).mark.row, SEL(r).mark.col, SEL(r).end.row, SEL(r).end.col));

#endif		    /* ! NO_NEW_SELECTION */
}


#ifndef NO_FRILLS
/* INTPROTO */
static void
rxvt_selection_trim(rxvt_t* r)
{
    int32_t	 end_col, end_row;
    text_t	 *stp; 

    end_col = SEL(r).end.col;
    end_row = SEL(r).end.row;
    for ( ; end_row >= SEL(r).beg.row; )
    {
	stp = PSCR(r).text[end_row + SVLINES];
	while (--end_col >= 0)
	{
	    if (stp[end_col] != ' ' && stp[end_col] != '\t')
		break;
	}
	if (end_col >= 0 || PSCR(r).tlen[end_row - 1 + SVLINES] != -1)
	{
	    SEL(r).end.col = end_col + 1;
	    SEL(r).end.row = end_row;
	    break;
	}
	end_row--;
	end_col = r->TermWin.ncol;
    }

    if (SEL(r).mark.row > SEL(r).end.row)
    {
	SEL(r).mark.row = SEL(r).end.row;
	SEL(r).mark.col = SEL(r).end.col;
    }
    else if (SEL(r).mark.row == SEL(r).end.row &&
	SEL(r).mark.col > SEL(r).end.col)
	SEL(r).mark.col = SEL(r).end.col;
}
#endif


/* ------------------------------------------------------------------------- */
/*
 * Double click on button 3 when already selected
 * EXT: button 3 double click
 */
/* EXTPROTO */
void
rxvt_selection_rotate(rxvt_t* r, int x, int y)
{
    SEL(r).clicks = SEL(r).clicks % 3 + 1;

    rxvt_selection_extend_colrow (r, Pixel2Col(x),
	Pixel2Row(y), 1, 0, 1);
}



/* ------------------------------------------------------------------------- */
/*
 * Respond to a request for our current selection
 * EXT: SelectionRequest
 */
/* EXTPROTO */
void
rxvt_process_selectionrequest (rxvt_t* r, const XSelectionRequestEvent *rq)
{
    XSelectionEvent ev;
#ifdef USE_XIM
    Atom	  target_list[4];
#else
    Atom	  target_list[3];
#endif
    Atom	    target;
    XTextProperty   ct;
#ifdef USE_XIM
    XICCEncodingStyle style;
#endif
    char	   *cl[2], dummy[1];

    ev.type = SelectionNotify;
    ev.property = None;
    ev.display = rq->display;
    ev.requestor = rq->requestor;
    ev.selection = rq->selection;
    ev.target = rq->target;
    ev.time = rq->time;

    if (rq->target == r->h.xa[XA_TARGETS])
    {
	target_list[0] = r->h.xa[XA_TARGETS];
	target_list[1] = XA_STRING;
	target_list[2] = r->h.xa[XA_TEXT];
#ifdef USE_XIM
	target_list[3] = r->h.xa[XA_COMPOUND_TEXT];
#endif
	XChangeProperty(r->Xdisplay, rq->requestor, rq->property,
	    XA_ATOM, 32, PropModeReplace,
	    (unsigned char *)target_list,
	    (sizeof(target_list) / sizeof(target_list[0])));
	ev.property = rq->property;
    }
    else if (rq->target == r->h.xa[XA_MULTIPLE])
    {
	/* TODO: Handle MULTIPLE */
    }
    else if (rq->target == r->h.xa[XA_TIMESTAMP] && SEL(r).text)
    {
	XChangeProperty(r->Xdisplay, rq->requestor, rq->property,
	    XA_INTEGER,
	    sizeof(Time) > 4 ? 32 : (8 * sizeof(Time)),
	    PropModeReplace, (unsigned char*)&r->h.selection_time,
	    sizeof(Time) > 4 ? sizeof(Time)/4 : 1);
	ev.property = rq->property;
    }
    else if (
	      rq->target == XA_STRING
	      || rq->target == r->h.xa[XA_COMPOUND_TEXT]
	      || rq->target == r->h.xa[XA_TEXT]
	    )
    {
#ifdef USE_XIM
	short	       freect = 0;
#endif
	int	     selectlen;

#ifdef USE_XIM
	if (rq->target != XA_STRING)
	{
	    target = r->h.xa[XA_COMPOUND_TEXT];
	    style = (rq->target == r->h.xa[XA_COMPOUND_TEXT])
		? XCompoundTextStyle : XStdICCTextStyle;
	} else
#endif
	{
	    target = XA_STRING;
#ifdef USE_XIM
	    style = XStringStyle;
#endif
	}
	if (SEL(r).text)
	{
	    cl[0] = (char *)SEL(r).text;
	    selectlen = SEL(r).len;
	}
	else
	{
	    cl[0] = dummy;
	    *dummy = '\0';
	    selectlen = 0;
	}
#ifdef USE_XIM
	if (XmbTextListToTextProperty(r->Xdisplay, cl, 1, style, &ct) == Success)	/* if we failed to convert then send it raw */
	    freect = 1;
	else
#endif
	{
	    ct.value = (unsigned char *)cl[0];
	    ct.nitems = selectlen;
	}
	XChangeProperty(r->Xdisplay, rq->requestor, rq->property,
	    target, 8, PropModeReplace,
	    ct.value, (int)ct.nitems);
	ev.property = rq->property;
#ifdef USE_XIM
	if (freect)
	    XFree(ct.value);
#endif
    }
    XSendEvent(r->Xdisplay, rq->requestor, False, 0L, (XEvent *)&ev);
}

/* ------------------------------------------------------------------------- *
 *			      MOUSE ROUTINES				   *
 * ------------------------------------------------------------------------- */

/*
 * return col/row values corresponding to x/y pixel values
 */
/* EXTPROTO */
void
rxvt_pixel_position(rxvt_t* r, int *x, int *y)
{
    *x = Pixel2Col(*x);
/* MAX_IT(*x, 0); MIN_IT(*x, (int)r->TermWin.ncol - 1); */
    *y = Pixel2Row(*y);
/* MAX_IT(*y, 0); MIN_IT(*y, (int)r->TermWin.nrow - 1); */
}

/*----------------------- end-of-file (C source) -----------------------*/
