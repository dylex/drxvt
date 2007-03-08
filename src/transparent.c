/*--------------------------------*-C-*---------------------------------*
 * File:    transparent.c
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2004-2005   Jingmin Zhou <jimmyzhou@users.sourceforge.net>
 * Copyright (c) 2005        Gautam Iyer <gi1242@users.sourceforge.net>
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


/* ------------------- Begin Internal Prototypes ---------------------- */
#ifdef TRANSPARENT
int		tempDisableTransparent	     (rxvt_t*);
void		expose_transparent_subwin    (rxvt_t*);
int		resetParentPixmap	     (rxvt_t*, unsigned, unsigned,
					      unsigned rootd);
#endif

#ifdef TINTING_SUPPORT
# ifdef HAVE_LIBXRENDER
void		xrenderShadeIntersect	     (rxvt_t*, Picture, unsigned long,
					      int, int, int, unsigned, unsigned,
					      int, int, unsigned, unsigned);
# else
static void     shade_ximage		     (rxvt_t*, XImage*);
# endif
#endif

/* -------------------------- End Internal Prototypes ----------------------- */

/* {{{1 General functions */
/* {{{2 rxvt_set_opacity(r) */
/* INTPROTO */
int
rxvt_set_opacity (rxvt_t* r)
{
    int		    k;
    unsigned int    n;
    Window	    wintree[PARENT_NUMBER];
    Window	    root;
    Window*	    list;
    CARD32	    opacity;


    /* do not set opacity */
    if (IS_NULL(r->h->rs[Rs_opacity]) ||
	NOT_ATOM(r->h->xa[XA_NET_WM_WINDOW_OPACITY]))
	return 0;

#ifdef TRANSPARENT
    /* Override pseudo-transparent in case */
    if (ISSET_OPTION(r, Opt_transparent))
	UNSET_OPTION(r, Opt_transparent);
    XSetWindowBackground(r->Xdisplay, r->TermWin.parent, VTBG(r,0) );
#endif

    opacity = (CARD32) (r->TermWin.opacity * (0xffffffff / 100));

    /*
    ** look for parent tree of top level window because the window
    ** manager may embed our window into some frame windows
    */
    wintree[0] = r->TermWin.parent;
    for (k = 1; k < PARENT_NUMBER; k++)
    {
	XQueryTree (r->Xdisplay, wintree[k-1], &root,
	    &(wintree[k]), &list, &n);
	XFree (list);
	if (wintree[k] == XROOT)
	    break;
    }

    /* Set opacity for all windows */
    if (k != PARENT_NUMBER)
    {
	while (k-- > 0)
	    XChangeProperty (r->Xdisplay, wintree[k],
		r->h->xa[XA_NET_WM_WINDOW_OPACITY], XA_CARDINAL, 32,
		PropModeReplace, (unsigned char*) &opacity, 1L);
    }

    XSync(r->Xdisplay, False);

    return 1;
}

/* {{{2 rxvt_process_reparentnotify( r, ev) */
/* EXTPROTO */
void
rxvt_process_reparentnotify (rxvt_t* r, XEvent* ev)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_TRANSPARENT, "ReparentNotify event\n"));
    
    rxvt_set_opacity (r);

#ifdef TRANSPARENT
    if (ISSET_OPTION(r, Opt_transparent))
    {
	if( r->h->bgRefreshInterval)
	    gettimeofday( &r->h->lastCNotify, NULL);
	else if( rxvt_check_our_parents(r) )
	    r->h->want_full_refresh = 1;
    }
#endif
}

/* {{{1 Transparent only functions */
#ifdef TRANSPARENT
/* tempDisableTransparent(r) {{{2
 *
 * Temporarily disable transparency (because something failed)
 */
/* INTPROTO */
int
tempDisableTransparent( rxvt_t *r)
{
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "Disabling XROOT pseudo-transparency\n"));
    if( r->h->am_transparent )
    {
	/*
	 * Need to update the window background.
	 */
	r->h->am_transparent = 0;
	r->h->want_full_refresh = 1;

	if( NOTSET_OPTION(r, Opt_forceTransparent) )
	{
	    XSetWindowBackground( r->Xdisplay, r->TermWin.parent, VTBG(r,0) );

	    expose_transparent_subwin( r );
	}

	return 1;
    }
    else return 0;
}

/* expose_transparent_subwin(r) {{{2
 *
 * Send Expose events to all transparent subwindows.
 */
void
expose_transparent_subwin( rxvt_t *r)
{
    /* Do not handle the subwindows if VT windows have not been
    ** created. Otherwise, the following will cause crash. */
    if (-1 == LTAB(r)) return;

# ifdef HAVE_SCROLLBARS
    if (ISSET_OPTION(r, Opt_transparent_scrollbar))
    {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_TRANSPARENT, "reset background image for scrollbar\n"));
	XClearWindow (r->Xdisplay, r->scrollBar.win);
	r->scrollBar.update (r, 1, r->scrollBar.top, r->scrollBar.bot,
	    r->h->scroller_len);
    }
# endif
# ifdef HAVE_MENUBAR
    if (ISSET_OPTION(r, Opt_transparent_menubar))
    {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_TRANSPARENT, "reset background image for menubar\n"));
	rxvt_menubar_expose (r);
    }
# endif
    if (ISSET_OPTION(r, Opt_transparent_tabbar))
    {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_TRANSPARENT, "reset background image for tabbar\n"));
	rxvt_tabbar_expose (r, NULL);
    }
}


/* {{{2 resetParentPixmap(r, rootw, rooth, rootd)
 *
 * Grabs the root background, shades it and sets as the windows background
 * pixmap. rwidth / height / depth are the respective attributes of the root
 * window.
 *
 * xerror_return must be set to Success before entering this function. (It's not
 * reset in this function either).
 */
/* INTPROTO */
int
resetParentPixmap( rxvt_t *r,
	unsigned rootw, unsigned rooth, unsigned rootd)
{
    Pixmap	    pmap;
    GC		    gc;

    int		    sx = r->szHint.x,	    /* coordinates to grab root img */
		    sy = r->szHint.y,
		    nx = 0, ny = 0;	    /* coordinates to put root image */

    unsigned int    nw = r->szHint.width,   /* dims of root image to grab */
		    nh = r->szHint.height;

# ifdef DEBUG
    if( sx + nw <= 0 || sx >= (int) rootw || sy + nh <= 0 || sy >= (int) rooth)
    {
        rxvt_msg (DBG_WARN, DBG_TRANSPARENT, "Possible error: grabbing offscreen (%d, %d, %u, %u)\n", sx, sy, nw, nh);
    }
# endif

    pmap = XCreatePixmap( r->Xdisplay, r->TermWin.parent,
			r->szHint.width, r->szHint.height, rootd);
    if (NOT_PIXMAP(pmap))
	return 0;

    gc = XCreateGC( r->Xdisplay, r->TermWin.parent, 0, NULL);

    if (sx < 0)
    {
	nw += sx;
	nx = -sx;
	sx = 0;
    }
    if (sy < 0)
    {
	nh += sy;
	ny = -sy;
	sy = 0;
    }
    MIN_IT(nw, (unsigned int) (rootw - sx));
    MIN_IT(nh, (unsigned int) (rooth - sy));

# if defined(HAVE_LIBXRENDER) || !defined(TINTING_SUPPORT)
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "XRender: Grab root pmap at %ux%u+%d+%d\n", nw, nh, sx, sy));
    if( r->h->rpWidth < rootw || r->h->rpHeight < rooth )
    {
	/* Tile the root background on the window. */
	XGCValues values;

	values.ts_x_origin = -sx + nx;
	values.ts_y_origin = -sy + ny;
	values.fill_style  = FillTiled;
	values.tile = r->h->rootPixmap;

	XChangeGC( r->Xdisplay, gc,
		GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin | GCTile,
		&values);

	XFillRectangle( r->Xdisplay, pmap, gc, nx, ny, nw, nh);
    }
    else
	XCopyArea( r->Xdisplay, r->h->rootPixmap, pmap, gc,
	    sx, sy, nw, nh, nx, ny);

#  if defined(HAVE_LIBXRENDER) && defined(TINTING_SUPPORT)
    xrenderShadeParentPixmap( r, pmap, nx, ny, nw, nh, True );
#  endif


# else /* if HAVE_LIBXRENDER || !TINTING_SUPPORT */
    {
	/*
	 * We need to tint the root pixmap ourself now. Since our tinting
	 * functions use XImages we have to get the root background into an
	 * ximage first.
	 */
	XImage *image;

	rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "Fast transparency -- XGetImage (%d, %d, %d, %d)\n", sx, sy, nw, nh));
	if( r->h->rpWidth < rootw || r->h->rpHeight < rooth )
	{
	    XGCValues values;
	    /*
	     * Tile the root background on the window. Xlib only tiles into
	     * pixmaps, so we tile into pmap, and then XGetImage it.
	     */

	    values.ts_x_origin = -sx;
	    values.ts_y_origin = -sy;
	    values.fill_style  = FillTiled;
	    values.tile = r->h->rootPixmap;

	    XChangeGC( r->Xdisplay, gc,
		GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin | GCTile,
		&values);

	    XFillRectangle( r->Xdisplay, pmap, gc, 0, 0, nw, nh);

	    image = XGetImage( r->Xdisplay, pmap, 0, 0, nw, nh,
		    AllPlanes, ZPixmap);
	}
	else 
	    image = XGetImage( r->Xdisplay, r->h->rootPixmap, sx, sy, nw, nh,
		AllPlanes, ZPixmap);

	if( image != NULL)
	{
	    /*
	     * Image now has our root background. We have to tint it, and put it
	     * back as a pixmap.
	     */
	    shade_ximage( r, image);
	    XPutImage(r->Xdisplay, pmap, gc, image,
		0, 0,			/* src x and y */
		nx, ny, nw, nh);	/* dst coords */

	    XDestroyImage( image);
	}
	else if( r->h->xerror_return == Success )
	{
	    /*
	     * If image == NULL, there must have been an XError. So if we got
	     * here we have a bug :)
	     */
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "Transparency messed up in function resetParentPixmap"));
	    r->h->xerror_return = LastExtensionError;
	}
    }
# endif /* HAVE_LIBXRENDER */


    /*
     * XRender or not, pmap now contains a wonderfully tinted copy of the root
     * background.
     */
    if( r->h->xerror_return == Success )
    {
	XSetWindowBackgroundPixmap( r->Xdisplay, r->TermWin.parent, pmap);

	/*
	 * If we just became transparent, then make all child terminals
	 * transparent.
	 */
	if( !r->h->am_transparent && !r->h->am_pixmap_trans )
	{
	    int i;
	    for (i = 0; i <= LTAB(r); i ++)
	    {
		XSetWindowBackgroundPixmap (r->Xdisplay,
		    PVTS(r, i)->vt, ParentRelative);
	    }
	}
    }
    /* else Caller will handle faliures */

    XFreeGC( r->Xdisplay, gc);
    XFreePixmap( r->Xdisplay, pmap);

    return r->h->xerror_return == Success;
}

/* {{{2 rxvt_toggle_transparency(r) */
/* EXTPROTO */
void
rxvt_toggle_transparency (rxvt_t* r)
{
    register int    i;


    if (ISSET_OPTION(r, Opt_transparent))
    {
	rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "unset background transparency\n"));
	UNSET_OPTION(r, Opt_transparent);
	r->h->am_transparent = 0;
	r->h->am_pixmap_trans = 0;
	r->h->bgGrabbed = False;

	XSetWindowBackground (r->Xdisplay, r->TermWin.parent, VTBG(r, 0) );

	for (i = 0; i <= LTAB(r); i ++)
	{
# ifdef BACKGROUND_IMAGE
	    if (IS_PIXMAP(PVTS(r, i)->pixmap))
		XSetWindowBackgroundPixmap (r->Xdisplay,
		    PVTS(r, i)->vt, PVTS(r, i)->pixmap);
	    else
# endif	/* BACKGROUND_IMAGE */
	    if( i == ATAB(r) )
	    {
		/* Background colors need to be forcibly reset */
		r->fgbg_tabnum = -1;
		rxvt_set_vt_colors( r, ATAB(r) );
	    }
	}

# ifdef HAVE_SCROLLBARS
	if (IS_WIN(r->scrollBar.win) &&
	    ISSET_OPTION(r, Opt_transparent_scrollbar))
	{
#  ifdef BACKGROUND_IMAGE
	    if (IS_PIXMAP(r->scrollBar.pixmap))
		XSetWindowBackgroundPixmap (r->Xdisplay,
		    r->scrollBar.win, r->scrollBar.pixmap);
	    else
#  endif    /* BACKGROUND_IMAGE */
	    {
		XSetWindowBackground (r->Xdisplay, r->scrollBar.win,
			rxvt_scrollbar_bg(r) );
	    }
	    /* The scrollbar update will not clear it's window background */
	    XClearWindow( r->Xdisplay, r->scrollBar.win);
	}
# endif	/* HAVE_SCROLLBARS */

# ifdef HAVE_MENUBAR
	if (IS_WIN(r->menuBar.win) &&
	    ISSET_OPTION(r, Opt_transparent_menubar))
	{
#  ifdef BACKGROUND_IMAGE
	    if (IS_PIXMAP(r->menuBar.pixmap))
		XSetWindowBackgroundPixmap (r->Xdisplay,
		    r->menuBar.win, r->menuBar.pixmap);
	    else
#  endif    /* BACKGROUND_IMAGE */
	    XSetWindowBackground (r->Xdisplay, r->menuBar.win, r->menuBar.bg);
	}
# endif	/* HAVE_MENUBAR */

	if (ISSET_OPTION(r, Opt_transparent_tabbar))
	{
# ifdef BACKGROUND_IMAGE
	    if (r->tabBar.hasPixmap)
	    {
		long w=0, h=0;
		Pixmap pmap;

		pmap = rxvt_load_pixmap (r, r->h->rs[Rs_tabbarPixmap], &w, &h);
		if (IS_PIXMAP(pmap))
		{
		    XSetWindowBackgroundPixmap (r->Xdisplay, r->tabBar.win,
			    pmap);
		    XFreePixmap( r->Xdisplay, pmap);
		}
		else r->tabBar.hasPixmap = False;
	    }
	    else
# endif	/* BACKGROUND_IMAGE */
	    XSetWindowBackground (r->Xdisplay, r->tabBar.win, r->tabBar.ibg);
	}
    }
    else
    {
	rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "set background transparency\n"));
	SET_OPTION(r, Opt_transparent);
	XSetWindowBackgroundPixmap (r->Xdisplay, r->TermWin.parent,
	    ParentRelative);
	for (i = 0; i <= LTAB(r); i ++)
	{
	    XSetWindowBackgroundPixmap (r->Xdisplay,
		PVTS(r, i)->vt, ParentRelative);
	}
# ifdef HAVE_SCROLLBARS
	if (IS_WIN(r->scrollBar.win) &&
	    ISSET_OPTION(r, Opt_transparent_scrollbar))
	    XSetWindowBackgroundPixmap (r->Xdisplay,
		r->scrollBar.win, ParentRelative);
# endif
# ifdef HAVE_MENUBAR
	if (IS_WIN(r->menuBar.win) &&
	    ISSET_OPTION(r, Opt_transparent_menubar))
	    XSetWindowBackgroundPixmap (r->Xdisplay,
		r->menuBar.win, ParentRelative);
# endif
	if (ISSET_OPTION(r, Opt_transparent_tabbar))
	    XSetWindowBackgroundPixmap (r->Xdisplay,
		r->tabBar.win, ParentRelative);

	XSelectInput(r->Xdisplay, XROOT, PropertyChangeMask);
	rxvt_check_our_parents (r);
    }

    /* Clear all windows */
    XClearWindow (r->Xdisplay, r->TermWin.parent);
# ifdef HAVE_SCROLLBARS
    rxvt_scrollbar_update (r, 0);
# endif
# ifdef HAVE_MENUBAR
    rxvt_menubar_expose (r);
# endif
    rxvt_tabbar_expose (r, NULL);

    rxvt_scr_clear (r, ATAB(r));
    rxvt_scr_touch (r, ATAB(r), True);
}


/* refreshRootBGVars(r) {{{2
 *
 * Refresh our information about the root pixmap and it's geometry etc. This
 * should be called every time the desktop changes.
 */
/* EXTPROTO */
void
refreshRootBGVars(rxvt_t *r )
{
    unsigned long   nitems, bytes_after;
    Atom	    atype;
    int		    aformat;
    unsigned char   *prop = NULL;

    r->h->allowedxerror = 1;
    r->h->xerror_return = Success;

    /* Get new root pixmap ID in h->rootPixmap*/
    if (
	    IS_ATOM(r->h->xa[XA_XROOTPMAPID]) &&
	    XGetWindowProperty( r->Xdisplay, XROOT,
		    r->h->xa[XA_XROOTPMAPID], 0L, 1L, False, XA_PIXMAP,
		    &atype, &aformat, &nitems, &bytes_after, &prop) == Success
	    && prop != NULL
       )
    {
	int	 u_rootx, u_rooty; /* u_ variables are unused */
	unsigned u_bw, u_depth;
	Window	 u_cr;

	r->h->rootPixmap = *( (Pixmap *) prop );
	r->h->bgGrabbed = False;

	XFree( prop);

	/* Get root pixmap geometry */
	XGetGeometry( r->Xdisplay, r->h->rootPixmap, &u_cr,
		&u_rootx, &u_rooty, &r->h->rpWidth, &r->h->rpHeight,
		&u_bw, &u_depth);
    }
    else
	/* Failed. */
	UNSET_PIXMAP(r->h->rootPixmap);

    r->h->allowedxerror = 0;
    if( r->h->xerror_return != Success ) /* Set by xerror handler */
	UNSET_PIXMAP(r->h->rootPixmap);

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "Got %snull root pixmap %lx\n", NOT_PIXMAP(r->h->rootPixmap) ? "" : "non-", r->h->rootPixmap));
}

/* {{{2 rxvt_check_our_parents(r)
 *
 * Check our parents are still who we think they are. Do transparency updates if
 * required.
 */
/* EXTPROTO */
int
rxvt_check_our_parents(rxvt_t *r)
{
    int			have_changed = 0,
			rootdepth;
    XWindowAttributes	wattr, wrootattr;
    Window		u_cr;


    /*
     * We dont' really have to check our parents if we're not transparent :)
     */
    if (NOTSET_OPTION(r, Opt_transparent))
	return have_changed;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_TRANSPARENT, "rxvt_check_our_parent ()\n"));

    /*
     * Don't try transparency if window depth is not the same as the root depth.
     */
    XGetWindowAttributes(r->Xdisplay, XROOT, &wrootattr);
    rootdepth = wrootattr.depth;
    XGetWindowAttributes(r->Xdisplay, r->TermWin.parent, &wattr);

    if (rootdepth != wattr.depth )
    {
	/*
	 * Oops. We can't be transparent (or pixmap_transparent).
	 */
	assert (-1 != LTAB(r)); /* should't happen */
	rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "Bad depth, transparency failed: (%u, %u)\n", rootdepth, wattr.depth));

	rxvt_msg (DBG_ERROR, DBG_TRANSPARENT, 
	    "Root window has different depth. Disabling transparency");

	r->h->am_pixmap_trans = 0;
	UNSET_OPTION(r, (Opt_forceTransparent | Opt_transparent));
	return tempDisableTransparent( r );
    }

    /*
     * r->szHint should contain the correct location of this window if we got
     * here from a configure notify event. If we got here from reparent_notify,
     * then we need to set it ourselves to offset WM decorations
     */
    XTranslateCoordinates( r->Xdisplay, r->TermWin.parent, XROOT,
	    0, 0, &r->szHint.x, &r->szHint.y, &u_cr);

    /*
     * Don't try refreshing our background if we're completely off screen.
     */
    if (    r->szHint.x + r->szHint.width  <= 0
	|| r->szHint.x >= (int) wrootattr.width
	|| r->szHint.y + r->szHint.height <= 0
	|| r->szHint.y >= (int) wrootattr.height
       )
	return 0;

    XSync(r->Xdisplay, False);

    r->h->allowedxerror = 1;
    r->h->xerror_return = Success;

    /*
     * Use "do" instead of "if" so we can break out if transparency updates
     * don't need to be performed.
     */
    do	/* while( 0 ) */
    {
	/*
	 * Copy XROOT pixmap transparency
	 */
	int	    retvt = 0;

	if (NOT_PIXMAP(r->h->rootPixmap))
	{
	    have_changed = tempDisableTransparent( r);
	    break;
	}

	retvt = resetParentPixmap( r,
		wrootattr.width, wrootattr.height, wrootattr.depth);

	if (retvt )
	{
	    /* All good. */
	    r->h->bgGrabbed = True;
	    r->h->prevPos.x = r->szHint.x;
	    r->h->prevPos.y = r->szHint.y;
	    r->h->prevPos.width = r->szHint.width;
	    r->h->prevPos.height = r->szHint.height;

	    have_changed = 1;
	    r->h->am_transparent = 1;
	    r->h->am_pixmap_trans = 0;

	    expose_transparent_subwin( r );
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "Enabled XROOT pseudo-transparency\n"));
	}
	else have_changed = tempDisableTransparent( r);
    } while( 0 );

    /*
     * Disable transparency if something went wrong above.
     */
    r->h->allowedxerror = 0;
    if( r->h->xerror_return != Success && r->h->am_transparent )
	/* Only call if we're still transparent (otherwise we might call
	 * tempDisableTransparent twice).
	 */
	have_changed = tempDisableTransparent( r );

    /*
     * 2006-01-07 gi1242: This code doesn't work too well. The problem is that
     * if the user sets the desktop with say xsetroot, and we recieve an expose
     * event, then we only refresh part of our window. This causes really ugly
     * effects.
     */
    if( !r->h->am_transparent && ISSET_OPTION(r, Opt_forceTransparent) )
    {
	/*
	 * Try "transparent transparency" legacy code.
	 */

	Window		    root, oldp, *list;
	unsigned int	i, n;
	/*
	 * Make the frame window set by the window manager that have the root
	 * background. Some window managers put multiple nested frame windows for
	 * each client, so we have to take care about that.
	 */
	rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "Transparent Seeking to  %08lx\n", XROOT));
	for (i = 1; i < PARENT_NUMBER; i++)
	{
	    /*
	     * Check if any of our "parents" have changed.
	     */
	    oldp = r->TermWin.parenttree[i];
	    XQueryTree(r->Xdisplay, r->TermWin.parenttree[i - 1],
		&root, &r->TermWin.parenttree[i], &list, &n);
	    XFree(list);

	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "Transparent Parent[%d] = %08lx\n", i, r->TermWin.parenttree[i] ));

	    if (r->TermWin.parenttree[i] == XROOT)
	    {
		if (IS_WIN(oldp))
		    have_changed = 1;
		break;
	    }
	    if (oldp != r->TermWin.parenttree[i])
		have_changed = 1;
	}

	n = 0;
	/*
	 * Now set background of all windows in this tree to ParentRelative.
	 */
	for (; n < (unsigned int)i; n++)
	{
	    /*
	     * Make sure we don't have any input only (or different depth)
	     * windows.
	     */
	    XGetWindowAttributes(r->Xdisplay,
		r->TermWin.parenttree[n], &wattr);
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "Transparent Checking Parent[%d]: %s\n", n, (wattr.depth == rootdepth && wattr.class != InputOnly) ?  "OK" : "FAIL"));
	    if (wattr.depth != rootdepth ||
		wattr.class == InputOnly)
	    {
		n = PARENT_NUMBER + 1;
		break;
	    }
	}

	if (n > PARENT_NUMBER)
	{
	    /*
	     * Bummer. We got an inputonly (or bad depth) window. Fail.
	     */
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "Transparent Turning off\n"));
	    if( r->h->am_pixmap_trans )
	    {
		r->h->am_pixmap_trans = 0;
		r->h->want_full_refresh = 1;
		have_changed = 1;

		XSetWindowBackground(r->Xdisplay, r->TermWin.parent,
			VTBG(r, 0) );

		expose_transparent_subwin( r );
	    }
	}
	else
	{
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "Transparent Turning on (%d parents). Geometry %ux%u+%d+%d\n", i - 1, r->szHint.width, r->szHint.height, r->szHint.x, r->szHint.y));
	    for (n = 0; n < i; n ++)
		XSetWindowBackgroundPixmap(r->Xdisplay,
		    r->TermWin.parenttree[n], ParentRelative);

	    r->h->am_pixmap_trans = 1;
	    have_changed = 1;
	    expose_transparent_subwin( r );
	}

	/*
	 * Set the tail portion of our tree to None for future calls
	 */
	for (; i < PARENT_NUMBER; i++)
	    UNSET_WIN(r->TermWin.parenttree[i]);
    }

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "am_transparent: %hhu am_pixmap_trans: %hhu have_changed %d\n", r->h->am_transparent, r->h->am_pixmap_trans, have_changed));

    return have_changed;
}

#endif	/* TRANSPARENT */

/* {{{1 Background image + Transparent functions */
#if defined(BACKGROUND_IMAGE) || defined(TRANSPARENT)
/* {{{2 rxvt_refresh_bg_image( r, page, imediate) */
/* EXTPROTO */
void
rxvt_refresh_bg_image (rxvt_t* r, int page, Bool imediate)
{
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_TRANSPARENT, "rxvt_refresh_bg_image\n"));

# ifdef TRANSPARENT
    if (ISSET_OPTION(r, Opt_transparent))
    {
	if( imediate || !r->h->bgRefreshInterval )
	{
	    /* reset background NOW */
	    rxvt_check_our_parents (r);

	    rxvt_scr_clear (r, page);
	    rxvt_scr_touch (r, page, imediate);
	}
	else
	{
	    /* Generate a timeout to refresh the bg image. TODO lastCNotify
	     * should be called something else */
	    gettimeofday( &r->h->lastCNotify, NULL);
	    r->h->bgGrabbed = False;	/* So timeout will force refresh */
	}
    }
    else
# endif	/* TRANSPARENT */
    {
# ifdef BACKGROUND_IMAGE
	/* Reset background image */
	register int	i;
	for (i = 0; i <= LTAB(r); i ++)
	    rxvt_resize_pixmap (r, i);
# endif	/* BACKGROUND_IMAGE */

	rxvt_scr_clear (r, page);
	rxvt_scr_touch (r, page, imediate);
    }
}

#endif /* BACKGROUND_IMAGE || TRANSPARENT */

/* {{{1 Tinting functions */
#ifdef TINTING_SUPPORT
# ifdef HAVE_LIBXRENDER
/* {{{2 XRender tinting functions */
/* {{{3 xrenderShadeParentPixmap( r, pmap, nx, ny, nw, nh, shadeSubwin)
 *
 * Shade the pixmap as though it were the parent window's pixmap. The
 * coordinates nx, ny, nw, nh determine the location in pmap that needs to be
 * shaded (e.g. if the window is partially off screen, we only need to shade the
 * portion of pmap that's on screen).
 *
 * If shadeSubwin is true, then areas corresponding to the tab / scroll / menu
 * bar are tinted with their respective background colors.
 *
 * pmap should have dimensions equal to the parent window.
 */
/* EXTPROTO */
void
xrenderShadeParentPixmap( rxvt_t *r, Pixmap pmap,
	int nx, int ny, unsigned nw, unsigned nh,
	Bool shadeSubwin)
{
    int		    rx = 0, ry = 0;	    /* coords of tabbar etc */
    unsigned	    rw = TWIN_WIDTH (r),    /* dims of tabbar etc */
		    rh = TWIN_HEIGHT (r);

    Picture			pic;
    XRenderPictFormat		*format;
    XRenderPictureAttributes	attrs;


    format = XRenderFindVisualFormat( r->Xdisplay, XVISUAL);
    pic = XRenderCreatePicture( r->Xdisplay, pmap, format, 0, &attrs);

    if( shadeSubwin )
    {
# ifdef HAVE_MENUBAR
	if( rxvt_menubar_visible( r ) )
	{
	    /* Shade menubar */
	    if (ISSET_OPTION(r, Opt_transparent_menubar))
		xrenderShadeIntersect( r, pic, r->menuBar.bg, r->TermWin.shade,
			nx, ny, nw, nh,
			0, 0, rw, rxvt_menubar_height(r));

	    /* Reset coordinates to shade tabbar / main window. */
	    ry += rxvt_menubar_height (r);
	    rh -= rxvt_menubar_height (r);
	}
# endif

	if( rxvt_tabbar_visible( r ) )
	{
	    /* Shade tabbar */
	    if (ISSET_OPTION(r, Opt_transparent_tabbar))
		xrenderShadeIntersect( r, pic, r->tabBar.ibg, r->TermWin.shade,
			nx, ny, nw, nh,
			0, ry + (ISSET_OPTION(r, Opt2_bottomTabbar)
					? VT_HEIGHT(r) : 0),
			rw, rxvt_tabbar_height( r ));

	    /* Reset coordinates to shade main window. */
	    rh -= rxvt_tabbar_height( r );
	    if ( NOTSET_OPTION(r, Opt2_bottomTabbar) )
		ry += rxvt_tabbar_height( r );
	}

# ifdef HAVE_SCROLLBARS
	if( rxvt_scrollbar_visible( r ) )
	{
	    /* Shade scrollbar */
	    if (ISSET_OPTION(r, Opt_transparent_scrollbar))
		xrenderShadeIntersect( r, pic, rxvt_scrollbar_bg(r),
			r->TermWin.shade,
			nx, ny, nw, nh,
			ISSET_OPTION(r, Opt_scrollBar_right) ? VT_WIDTH(r) : 0, ry,
			rxvt_scrollbar_width(r), VT_HEIGHT(r));

	    /* Reset coordinates to shade main window. */
	    rw -= rxvt_scrollbar_width(r);
	    if( NOTSET_OPTION(r, Opt_scrollBar_right) )
		rx += rxvt_scrollbar_width(r);
	}
# endif
    }

    /* Shade root background. */
    xrenderShadeIntersect( r, pic,
	    ISSET_PIXCOLOR( r->h, Color_tint ) ?
		    r->pixColorsFocus[Color_tint] : VTBG(r, 0),
	    r->TermWin.shade,
	    nx, ny, nw, nh,
	    rx, ry, rw, rh);

    /*
     * Free everything and get lost.
     */
    XRenderFreePicture( r->Xdisplay, pic);
}

/* {{{3 xrenderShadeIntersect( r, pic, pixel, shade, (rect1), (rect2))
 * Shade the intersection of two rectangles
 */
/* INTPROTO */
void
xrenderShadeIntersect( rxvt_t *r, Picture pic, unsigned long pixel, int shade,
	int x1, int y1, unsigned w1, unsigned h1,
	int x2, int y2, unsigned w2, unsigned h2)
{
    int		x, y;
    unsigned	w, h;

    XColor	    color;
    XRenderColor    trans_color;


    if( x1 + w1 < x2 || x2 + w2 < x1 ||
	    y1 + h1 < y2 || y2 + h2 < y1)
	return;	/* disjoint */

    /*
     * Get intersection in (x, y, w, h)
     */
    x = (x1 < x2) ? x2 : x1;
    y = (y1 < y2) ? y2 : y1;
    w = ((x1 + w1 < x2 + w2) ? x1 + w1 : x2 + w2) - x;
    h = ((y1 + h1 < y2 + h2) ? y1 + h1 : y2 + h2) - y;

    /*
     * Get XRenderColor from pixel and shade.
     */
    color.pixel = pixel;
    XQueryColor (r->Xdisplay, XCMAP, &color);

    trans_color.red	= color.red;
    trans_color.green	= color.green;
    trans_color.blue	= color.blue;
    trans_color.alpha	= ( (100 - shade) * (0xffff / 100) );

    /*
     * Fill translucent. Don't know if PictOpOver is the best operation to use
     * or not ...
     */
    XRenderFillRectangle( r->Xdisplay, PictOpOver, pic, &trans_color,
	    x, y, w, h);
}

/* {{{3 Unimplemented stuff */
#if 0
int
xrenderCopyShadedPixmap( rxvt_t *r, Drawable src, Drawable dst,
	XRenderColor *color, int x, int y, unsigned width, unsigned height,
	int dx, dy)
{
}
#endif
/* }}}2 */
# else /* HAVE_LIBXRENDER */
/* {{{2 Fast tinting functions */
/* {{{3 rxvt_shade_pixmap(r, src, (rect)) */
/* EXTPROTO */
void
rxvt_shade_pixmap (rxvt_t* r, Drawable src,
	int sx, int sy, unsigned nw, unsigned nh)
{
    XImage*	ximage;

    GC gc = XCreateGC (r->Xdisplay, src, 0UL, 0);

    ximage = XGetImage(r->Xdisplay, src, sx, sy, nw, nh,
		AllPlanes, ZPixmap);
    if (NULL != ximage)
    {
	shade_ximage (r, ximage);

	XPutImage(r->Xdisplay, src, gc, ximage, 0, 0, sx, sy, nw, nh);
	XDestroyImage (ximage);
    }

    XFreeGC( r->Xdisplay, gc);
}


/* {{{3 shade_ximage(r, image)
 *
 * Tint XImage with color / degree from options.
 */
/* INTPROTO */
static void
shade_ximage (rxvt_t* r, XImage* srcImage)
{
    int		    sh_r, sh_g, sh_b;
    uint32_t	    mask_r, mask_g, mask_b;
    uint32_t	    *lookup, *lookup_r, *lookup_g, *lookup_b;
    unsigned int    lower_lim_r, lower_lim_g, lower_lim_b;
    unsigned int    upper_lim_r, upper_lim_g, upper_lim_b;
    int i;
    Visual*	    visual = XVISUAL;

    XColor	    color;

    /* Former arguments */
    int		    shade, rm, gm, bm;


    if (ISSET_PIXCOLOR (r->h, Color_tint) && r->h->rs[Rs_shade])
    {
	color.pixel = r->pixColorsFocus[Color_tint];
	XQueryColor (r->Xdisplay, XCMAP, &color);

	shade = r->TermWin.shade;
    }
    else return;	    /* No shading defined */

    rm = color.red;
    gm = color.green;
    bm = color.blue;


    if (100 == shade)	    /* no need to shade */
	return;

    if( visual->class != TrueColor || srcImage->format != ZPixmap )
	return ;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_TRANSPARENT, "shade background image\n"));

    /* for convenience */
    mask_r = visual->red_mask;
    mask_g = visual->green_mask;
    mask_b = visual->blue_mask;

    /* boring lookup table pre-initialization */
    switch (srcImage->bits_per_pixel)
    {
	case 15:
	    if ((mask_r != 0x7c00) ||
		(mask_g != 0x03e0) ||
		(mask_b != 0x001f))
		return;
	    lookup = (uint32_t *) rxvt_malloc (sizeof (uint32_t)*(32+32+32));
	    lookup_r = lookup;
	    lookup_g = lookup+32;
	    lookup_b = lookup+32+32;
	    sh_r = 10;
	    sh_g = 5;
	    sh_b = 0;
	    break;

	case 16:
	    if ((mask_r != 0xf800) ||
		(mask_g != 0x07e0) ||
		(mask_b != 0x001f))
		return;
	    lookup = (uint32_t *) rxvt_malloc (sizeof (uint32_t)*(32+64+32));
	    lookup_r = lookup;
	    lookup_g = lookup+32;
	    lookup_b = lookup+32+64;
	    sh_r = 11;
	    sh_g = 5;
	    sh_b = 0;
	    break;

	case 24:
	    if ((mask_r != 0xff0000) ||
		(mask_g != 0x00ff00) ||
		(mask_b != 0x0000ff))
		return;
	    lookup = (uint32_t *) rxvt_malloc (sizeof (uint32_t)*(256+256+256));
	    lookup_r = lookup;
	    lookup_g = lookup+256;
	    lookup_b = lookup+256+256;
	    sh_r = 16;
	    sh_g = 8;
	    sh_b = 0;
	    break;

	case 32:
	    if ((mask_r != 0xff0000) ||
		(mask_g != 0x00ff00) ||
		(mask_b != 0x0000ff))
	    return;
	    lookup = (uint32_t *) rxvt_malloc (sizeof (uint32_t)*(256+256+256));
	    lookup_r = lookup;
	    lookup_g = lookup+256;
	    lookup_b = lookup+256+256;
	    sh_r = 16;
	    sh_g = 8;
	    sh_b = 0;
	    break;

	default:
	    return; /* we do not support this color depth */
    }

    /* prepare limits for color transformation (each channel is
    ** handled separately) */
    if (shade < 0)
    {
	shade = -shade;
	if (shade < 0)
	    shade = 0;
	if (shade > 100)
	    shade = 100;

	lower_lim_r = 65535-rm;
	lower_lim_g = 65535-gm;
	lower_lim_b = 65535-bm;

	lower_lim_r = 65535-(unsigned int)(((uint32_t)lower_lim_r)*((uint32_t)shade)/100);
	lower_lim_g = 65535-(unsigned int)(((uint32_t)lower_lim_g)*((uint32_t)shade)/100);
	lower_lim_b = 65535-(unsigned int)(((uint32_t)lower_lim_b)*((uint32_t)shade)/100);

	upper_lim_r = upper_lim_g = upper_lim_b = 65535;
    }
    else
    {
	if (shade < 0)
	    shade = 0;
	if (shade > 100)
	    shade = 100;

	lower_lim_r = lower_lim_g = lower_lim_b = 0;

	upper_lim_r = (unsigned int)((((uint32_t)rm)*((uint32_t)shade))/100);
	upper_lim_g = (unsigned int)((((uint32_t)gm)*((uint32_t)shade))/100);
	upper_lim_b = (unsigned int)((((uint32_t)bm)*((uint32_t)shade))/100);
    }

    /* switch red and blue bytes if necessary, we need it for some
    ** weird XServers like XFree86 3.3.3.1 */
    if ((srcImage->bits_per_pixel == 24) && (mask_r >= 0xFF0000 ))
    {
	unsigned int tmp;

	tmp = lower_lim_r;
	lower_lim_r = lower_lim_b;
	lower_lim_b = tmp;

	tmp = upper_lim_r;
	upper_lim_r = upper_lim_b;
	upper_lim_b = tmp;
    }

    /* fill our lookup tables */
    for (i = 0; i <= mask_r>>sh_r; i++)
    {
	uint32_t tmp;
	tmp = ((uint32_t)i)*((uint32_t)(upper_lim_r-lower_lim_r));
	tmp += ((uint32_t)(mask_r>>sh_r))*((uint32_t)lower_lim_r);
	lookup_r[i] = (tmp/65535)<<sh_r;
    }
    for (i = 0; i <= mask_g>>sh_g; i++)
    {
	uint32_t tmp;
	tmp = ((uint32_t)i)*((uint32_t)(upper_lim_g-lower_lim_g));
	tmp += ((uint32_t)(mask_g>>sh_g))*((uint32_t)lower_lim_g);
	lookup_g[i] = (tmp/65535)<<sh_g;
    }
    for (i = 0; i <= mask_b>>sh_b; i++)
    {
	uint32_t tmp;
	tmp = ((uint32_t)i)*((uint32_t)(upper_lim_b-lower_lim_b));
	tmp += ((uint32_t)(mask_b>>sh_b))*((uint32_t)lower_lim_b);
	lookup_b[i] = (tmp/65535)<<sh_b;
    }

    /* apply table to input image (replacing colors by newly
    ** calculated ones) */
    switch (srcImage->bits_per_pixel)
    {
    case 15:
	{
	    unsigned short *p1, *pf, *p, *pl;
	    p1 = (unsigned short *) srcImage->data;
	    pf = (unsigned short *) (srcImage->data + srcImage->height * srcImage->bytes_per_line);
	    while (p1 < pf)
	    {
		p = p1;
		pl = p1 + srcImage->width;
		for (; p < pl; p++)
		{
		    *p = lookup_r[(*p & 0x7c00)>>10] |
			lookup_g[(*p & 0x03e0)>> 5] |
			lookup_b[(*p & 0x001f)];
		}
		p1 = (unsigned short *) ((char *) p1 + srcImage->bytes_per_line);
	    }
	    break;
	}
    case 16:
	{
	    unsigned short *p1, *pf, *p, *pl;
	    p1 = (unsigned short *) srcImage->data;
	    pf = (unsigned short *) (srcImage->data + srcImage->height * srcImage->bytes_per_line);
	    while (p1 < pf)
	    {
		p = p1;
		pl = p1 + srcImage->width;
		for (; p < pl; p++)
		{
		    *p = lookup_r[(*p & 0xf800)>>11] |
			lookup_g[(*p & 0x07e0)>> 5] |
			lookup_b[(*p & 0x001f)];
		}
		p1 = (unsigned short *) ((char *) p1 + srcImage->bytes_per_line);
	    }
	    break;
	}
    case 24:
	{
	    unsigned char *p1, *pf, *p, *pl;
	    p1 = (unsigned char *) srcImage->data;
	    pf = (unsigned char *) (srcImage->data + srcImage->height * srcImage->bytes_per_line);
	    while (p1 < pf)
	    {
		p = p1;
		pl = p1 + srcImage->width * 3;
		for (; p < pl; p += 3)
		{
		    p[0] = lookup_r[(p[0] & 0xff0000)>>16];
		    p[1] = lookup_r[(p[1] & 0x00ff00)>> 8];
		    p[2] = lookup_r[(p[2] & 0x0000ff)];
		}
		p1 = (unsigned char *) ((char *) p1 + srcImage->bytes_per_line);
	    }
	    break;
	}
    case 32:
	{
	    uint32_t *p1, *pf, *p, *pl;
	    p1 = (uint32_t *) srcImage->data;
	    pf = (uint32_t *) (srcImage->data + srcImage->height * srcImage->bytes_per_line);

	    while (p1 < pf)
	    {
		p = p1;
		pl = p1 + srcImage->width;
		for (; p < pl; p++)
		{
		    *p = lookup_r[(*p & 0xff0000)>>16] |
			lookup_g[(*p & 0x00ff00)>> 8] |
			lookup_b[(*p & 0x0000ff)] |
			(*p & ~0xffffff);
		}
		p1 = (uint32_t *) ((char *) p1 + srcImage->bytes_per_line);
	    }
	    break;
	}
    }

    rxvt_free (lookup);
}
/* }}}2 */
# endif	 /* HAVE_LIBXRENDER */
#endif	/* TINTING_SUPPORT */
