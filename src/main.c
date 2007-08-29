/*--------------------------------*-C-*---------------------------------*
 * File:    main.c
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1992        John Bovey <jdb@ukc.ac.uk>
 * Copyright (c) 1994        Robert Nation <nation@rocket.sanders.lockheed.com>
 * Copyright (c) 1995        Garrett D'Amore <garrett@netcom.com>
 * Copyright (c) 1997        mj olesen <olesen@me.QueensU.CA>
 * Copyright (c) 1997,1998   Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (c) 1998-2001   Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2000        Xianping Ge <xge@ics.uci.edu>
 * Copyright (c) 2003-2004   Marc Lehmann <pcg@goof.com>
 * Copyright (c) 2005        Burgers A.R. <burgers@ecn.nl>
 * Copyright (c) 2004-2006   Jingmin Zhou <jimmyzhou@users.sourceforge.net>
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

#ifdef XFT_SUPPORT
# include "xftacs.h"
#endif



/*--------------------------------------------------------------------*
 *         BEGIN `INTERNAL' ROUTINE PROTOTYPES                        *
 *--------------------------------------------------------------------*/
void rxvt_alarm_signal	       ( __attribute__((unused)) int sig );
void rxvt_pre_show_init	       (rxvt_t* r);
void rxvt_clean_commands       (rxvt_t* r, int command_number);
void rxvt_free_hidden          (rxvt_t*);
void rxvt_font_up_down         (rxvt_t*, int, int);
int  rxvt_get_font_widest      (XFontStruct*);
void rxvt_set_colorfgbg        (rxvt_t*);
void rxvt_resize_sub_windows   (rxvt_t*);
#ifdef USE_XIM
void rxvt_IM_set_size          (rxvt_t*, XRectangle*);
void rxvt_IM_set_position      (rxvt_t*, XPoint*);
void rxvt_IM_set_color         (rxvt_t*, unsigned long*, unsigned long*);
Bool rxvt_IM_is_running        (rxvt_t*);
void rxvt_IM_change_fontset    (rxvt_t*, int);
void rxvt_IM_set_preedit_area  (rxvt_t*, XRectangle*, XRectangle*, XRectangle*);
void rxvt_IM_destroy_callback  (XIM, XPointer, XPointer);
Bool rxvt_IM_get_IC            (rxvt_t*);
void rxvt_IM_send_size         (rxvt_t*);
void rxvt_IM_set_status_pos    (rxvt_t*);
#endif
void rxvt_set_r                (rxvt_t*);
#ifdef XFT_SUPPORT
void xftFreeUnusedFont	       (rxvt_t*, XftFont*);
void setXftWeight	       (XftPattern *, const char *, int);
void rxvt_init_font_fixed      (rxvt_t*);
# ifndef NO_BOLDFONT
void rxvt_init_bfont_xft       (rxvt_t*, XftPattern*);
# endif
# ifdef MULTICHAR_SET
int  rxvt_init_mfont_xft       (rxvt_t*, XftPattern*, const char*);
int  isDoubleWidthFont         (Display *dpy, XftFont *font);
# endif
#endif	/* XFT_SUPPORT */
/*--------------------------------------------------------------------*
 *         END   `INTERNAL' ROUTINE PROTOTYPES                        *
 *--------------------------------------------------------------------*/



/*----------------------------------------------------------------------*/
const char**	cmd_argv;
/*----------------------------------------------------------------------*/

/*
 * Initialization done after reading command line options, and before calling
 * rxvt_create_show_windows().
 */
/* INTPROTO */
void
rxvt_pre_show_init( rxvt_t *r )
{
    /*
     * 2006-08-18 gi1242 TODO: If we're using Xft, then we don't need to
     * initialize this array.
     */
    r->pixColorsFocus = rxvt_malloc( sizeof(unsigned long) * (TOTAL_COLORS));
    r->pixColors = r->pixColorsFocus;


#ifdef XFT_SUPPORT
    if( ISSET_OPTION( r, Opt_xft ) )
    {
	r->xftColorsFocus = rxvt_malloc( sizeof(XftColor) * (TOTAL_COLORS) );
	r->xftColors = r->xftColorsFocus;
    }
    else
    {
	SET_NULL( r->xftColors );
	SET_NULL( r->xftColorsFocus );
    }
#endif

    if( r->TermWin.fade )
    {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_MAIN, "Allocating space for fade colors\n"));
	r->pixColorsUnfocus =
	    rxvt_malloc( sizeof(unsigned long) * (TOTAL_COLORS) );

# ifdef XFT_SUPPORT
	if( ISSET_OPTION( r, Opt_xft ) )
	    r->xftColorsUnfocus =
		rxvt_malloc( sizeof(XftColor) * TOTAL_COLORS );
	else
	    SET_NULL( r->xftColorsUnfocus );
# endif /* XFT_SUPPORT */
    }

    else
    {
	SET_NULL( r->pixColorsUnfocus );
# ifdef XFT_SUPPORT
	SET_NULL( r->xftColorsUnfocus );
# endif
    }
}

/* rxvt_init() */
/* LIBPROTO */
rxvt_t		*
rxvt_init(int argc, const char *const *argv)
{
    register int    i;
    register int    itnum; /* initial terminal number */
    rxvt_t*	    r;

    /* Allocate memory for "r" and initialize contents to 0 */
    r = (rxvt_t *) rxvt_calloc(1, sizeof(rxvt_t));

    /* Save "r" in _rxvt_vars. This is the only assignment to _rxvt_vars */
    rxvt_set_r(r);

    /* Initialize vars in "r" */
    if (rxvt_init_vars(r) < 0)
    {
	rxvt_msg (DBG_ERROR, DBG_MAIN,  "Could not initialize." );
	rxvt_free(r);
	return NULL;
    }

    /* save global argc and argv */
    r->global_argc = argc;
    r->global_argv = (char**) argv;

    rxvt_init_secondary(r);
    cmd_argv = rxvt_init_resources(r, argc, argv);

    rxvt_pre_show_init( r );

    rxvt_create_show_windows(r, argc, argv);

#ifdef TRANSPARENT
    if (ISSET_OPTION(r, Opt_transparent))
    {
	XSelectInput(r->Xdisplay, XROOT, PropertyChangeMask);
	/*
	 * Our "parents" will automatically be checked on the first expose and
	 * ConfigureNotify event respectively. Forcefully calling it is just a
	 * waste of time.
	 */
# if 0
	rxvt_check_our_parents(r);
# endif
    }
#endif

    rxvt_init_env(r);
    rxvt_init_command(r);
    rxvt_init_screen (r);

    /*
     * Initialize the pages
     */
    if( r->h->rs[Rs_initProfiles] )
    {
	/* Split into a comma separated string */
	char *s = (char *) r->h->rs[Rs_initProfiles];

	do
	  {
	    int profile = atoi( s );

	    rxvt_append_page( r, profile, NULL, NULL );

	    s = STRCHR( s, ',' );
	  }
	while (NULL != s++);
    }


    /* Backward compatibility: Open profiles 0 .. n-1 if tnum=n. */
    else if( r->h->rs[Rs_init_term_num] )
    {
	rxvt_msg (DBG_ERROR, DBG_MAIN,  "Option tnum is obsolete."
		" Use --initProfileList instead" );

	itnum = atoi( r->h->rs[Rs_init_term_num] );
	itnum = max( 1, itnum );
	itnum = min( itnum, MAX_PAGES );

	for (i = 0; i < itnum; i ++)
	    rxvt_append_page( r, (i < MAX_PROFILES) ? i : 0 , NULL, NULL );
    }

    /* Just open the default tab */
    else
	rxvt_append_page( r, 0, NULL, NULL );

    /* Activate the tab */
    rxvt_activate_page (r, 0);

    /* Initialize xlocale after VT is created */
    rxvt_init_xlocale(r);

    return r;
}

/* ------------------------------------------------------------------------- *
 *			    SIGNAL HANDLING & EXIT HANDLER			*
 * ------------------------------------------------------------------------- */
/*
 * Catch a SIGCHLD signal and exit if the direct child has died
 */
/* ARGSUSED */
/* EXTPROTO */
RETSIGTYPE
rxvt_Child_signal(int sig __attribute__((unused)))
{
    /*
     * Sometimes the child process could exit before we have a chance to
     * initialize the term structure. So we just count the number of calls to
     * this function here. We will wait for our dead children later.
     */
    rxvt_t  *r;

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "\e[31mrxvt_Child_signal()\e[0m\n"));

    r = rxvt_get_r();
    r->ndead_childs++;

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "done rxvt_Child_signal()\n"));
}


/*
 * Catch a fatal signal and tidy up before quitting
 */
/* EXTPROTO */
RETSIGTYPE
rxvt_Exit_signal(int sig)
{
#ifdef UTMP_SUPPORT
    register int    i;
#endif
    rxvt_t*	    r;

    rxvt_msg (DBG_INFO, DBG_MAIN, "Received signal %d\n", (int) sig);

    r = rxvt_get_r();

#ifdef UTMP_SUPPORT
    for (i = 0; i <= LTAB(r); i ++)
    {
	rxvt_privileges( RESTORE );
	rxvt_cleanutent( r, i );
	rxvt_privileges( IGNORE );
    }
#endif

    /* resend signal to default handler */
    /* kill (getpid (), sig); */
    rxvt_close_all_tabs( r );
}


void
rxvt_alarm_signal( __attribute__((unused)) int sig )
{
    int i;
    rxvt_t *r = rxvt_get_r();

    if( LTAB(r) >= 0 )
    {
	rxvt_msg (DBG_WARN, DBG_MAIN, APL_NAME ": WARNING Processes ");
	for( i=0; i <= LTAB(r); i ++ )
	    rxvt_msg (DBG_WARN, DBG_MAIN, "%d%c", PVTS(r, i)->cmd_pid,
		    i == LTAB(r) ? ' ' : ',' );
	rxvt_msg (DBG_WARN, DBG_MAIN,
	    " have not responded to SIGHUP, and are still "
	    "running. Either 'kill -9' these processes or close the "
	    APL_NAME " window again within 3 seconds.\n");
    }
}


/* INTPROTO */
void
rxvt_free_hidden( rxvt_t* r )
{
#ifdef DEBUG
    if (IS_CURSOR(r->h->bar_pointer))
    {
	XFreeCursor( r->Xdisplay, r->h->bar_pointer );
	UNSET_CURSOR(r->h->bar_pointer);
    }
# ifdef POINTER_BLANK
    if (IS_CURSOR(r->h->blank_pointer))
    {
	XFreeCursor( r->Xdisplay, r->h->blank_pointer );
	UNSET_CURSOR(r->h->blank_pointer);
    }
# endif
#endif	/* DEBUG */

#ifdef USE_XIM
    if( r->h->Input_Context )   
    {
	XDestroyIC( r->h->Input_Context );
	SET_NULL(r->h->Input_Context); 
    }
#endif
}


/*
 * If we're protecting the secondary screen, or have multiple tabs with a hidden
 * tabbar, then avoid exiting.
 */
/* EXTPROTO */
void
rxvt_exit_request( rxvt_t *r )
{
    /* Avoid exiting if there are multiple tabs with hidden tabbar */
    if( LTAB(r) > 0 &&  !rxvt_tabbar_visible( r ) )
    {
	XBell( r->Xdisplay, 0 );
	if( rxvt_tabbar_show(r) ) rxvt_resize_on_subwin (r, SHOW_TABBAR);
	return;
    }

    /* Avoid exiting if some tab is in the secondary screen */
    if(ISSET_OPTION(r, Opt2_protectSecondary))
    {
	int i, dontExit = 0;

	for( i=0; i <= LTAB(r); i++)
	{
	    if( PVTS(r, i)->current_screen == SECONDARY )
	    {
		dontExit = 1;
		if( i != ATAB(r) )
		    rxvt_tabbar_highlight_tab( r, i, False);
	    }
	}

	if( dontExit )
	{
	    XBell( r->Xdisplay, 0);
	    return;
	}
    }

    rxvt_close_all_tabs(r);
}

/*
 * Remove all held tabs, and send SIGHUP to processes in all live tabs
 *
 * 2006-09-22 gi1242: In mrxvt-0.5.1 and before, we would exit mrxvt by calling
 * rxvt_clean_exit(). This would send SIGHUP to all child processes, and then
 * call exit(). This however is flawed! If some processes do not exit on SIGHUP,
 * then they will remain in the background and the user will not know.
 *
 * To fix this issue, we call rxvt_close_all_tabs() instead of rxvt_clean_exit()
 * when we want to exit mrxvt. rxvt_close_all_tabs() will send SIGHUP's to all
 * child processes, and remove all held tabs. If mrxvt does not exit within two
 * seconds, then it will sound an alarm warning the user of the offending
 * processes which ignored SIGHUP. The user can either kill -9 those processes
 * manually, or request another exit of mrxvt within the next 3 seconds to have
 * mrxvt forcefully exit (but leaving the child processes running).
 */
/* EXTPROTO */
void
rxvt_close_all_tabs( rxvt_t *r)
{
    static struct timeval   lastRequest = {0, 0};
    struct timeval	    now;
    int			    i;

    for( i=LTAB(r); i >=0; i-- )
    {
	if( PVTS(r, i)->dead )
	    rxvt_remove_page( r, i );

	else
	{
	    PVTS(r, i)->holdOption = 0;
	    kill( PVTS(r, i)->cmd_pid, SIGHUP );
	}
    }

    gettimeofday( &now, NULL );
    if( lastRequest.tv_sec != 0 && now.tv_sec - lastRequest.tv_sec < 5 )
	/* Second request within 5 seconds. Kill mrxvt */
	rxvt_clean_exit(r);

    else
    {
	lastRequest = now;

	/* Just in case the processes don't exit on SIGHUP, warn the user */
	signal( SIGALRM, rxvt_alarm_signal );
	alarm( 2 );
    }
}

/* EXTPROTO */
void
rxvt_clean_exit (rxvt_t* r)
{
#ifdef HAVE_X11_SM_SMLIB_H
    if (ISSET_OPTION(r, Opt2_enableSessionMgt))
	rxvt_session_exit (r);
#endif

#if 0
    /*
     * Now kill all child processes, zsh puts them into background if we do not
     * do so.
     *
     * 2006-09-22 gi1242: No! See comments before rxvt_close_all_tabs().
     */
    for (i = 0; i <= LTAB(r); i ++)
	kill (PVTS(r, i)->cmd_pid, SIGHUP);
#endif

    /*
     * 2006-01-27 gi1242: Free all used resources. This used to be done only in
     * debug mode, but I think it's good practice to do it always. It can't hurt
     * anything.
     */
/* #ifdef DEBUG	*/
    rxvt_free_hidden (r);

    /* Destroy windows before other X resources */
    if (IS_WIN(r->TermWin.parent))
    {
	XDestroySubwindows (r->Xdisplay, r->TermWin.parent);
	XDestroyWindow (r->Xdisplay, r->TermWin.parent);
	UNSET_WIN(r->TermWin.parent);
    }

# ifdef HAVE_SCROLLBARS
    rxvt_scrollbar_clean_exit (r);
# endif

# ifdef HAVE_MENUBAR
    rxvt_menubar_clean_exit (r);
# endif

    rxvt_tabbar_clean_exit (r);

    if (NOT_NULL(r->TermWin.font))
	XFreeFont (r->Xdisplay, r->TermWin.font);
# ifndef NO_BOLDFONT
    if (NOT_NULL(r->TermWin.bfont) && r->TermWin.font != r->TermWin.bfont)
    {
	XFreeFont (r->Xdisplay, r->TermWin.bfont);
	SET_NULL(r->TermWin.bfont);
    }
# endif
# ifdef MULTICHAR_SET
    if (NOT_NULL(r->TermWin.mfont) && r->TermWin.font != r->TermWin.mfont)
    {
	XFreeFont (r->Xdisplay, r->TermWin.mfont);
	SET_NULL(r->TermWin.mfont);
    }
# endif
    SET_NULL(r->TermWin.font);	    /* clear font */

# ifdef XFT_SUPPORT
    if (NOT_NULL(r->TermWin.xftfont))
    {
	XftFont *fn;

	XftFontClose (r->Xdisplay, r->TermWin.xftfont);
	xftCloseACS (r->Xdisplay);

	if( (fn = r->TermWin.xftpfont) )
	{
	    SET_NULL(r->TermWin.xftpfont);
	    xftFreeUnusedFont( r, fn);
	}
	if( (fn = r->TermWin.xftPfont) )
	{
	    SET_NULL(r->TermWin.xftPfont);
	    xftFreeUnusedFont( r, fn);
	}

#  ifndef NO_BOLDFONT
	fn = r->TermWin.xftbfont;
	SET_NULL(r->TermWin.xftbfont);
	xftFreeUnusedFont( r, fn);
#  endif

#  ifdef MULTICHAR_SET
	fn = r->TermWin.xftmfont;
	SET_NULL(r->TermWin.xftmfont);
	xftFreeUnusedFont( r, fn);
#  endif
    }
    SET_NULL(r->TermWin.xftfont);   /* clear font */

    /*
     * XXX gi1242 2006-01-27: Xft bug. Patterns passed to XftFontOpenPattern
     * can't always be safely freed.
     */
#  if 0
    if( r->TermWin.xftpattern )
	XftPatternDestroy( r->TermWin.xftpattern);

#  ifdef MULTICHAR_SET
    if( r->TermWin.xftmpattern )
	XftPatternDestroy( r->TermWin.xftmpattern );
#  endif
#  endif
# endif

    if (IS_CURSOR(r->term_pointer))
    {
	XFreeCursor (r->Xdisplay, r->term_pointer);
	UNSET_CURSOR(r->term_pointer);
    }
    if (IS_GC(r->TermWin.gc))
    {
	XFreeGC (r->Xdisplay, r->TermWin.gc);
	UNSET_GC(r->TermWin.gc);
    }
    XCloseDisplay (r->Xdisplay);
    SET_NULL(r->Xdisplay);

#ifdef USE_FIFO
    if( r->fifo_fd != -1 )
    {
	close( r->fifo_fd );
	r->fifo_fd = -1;

	unlink( r->fifo_name );
	rxvt_free( r->fifo_name );  SET_NULL( r->fifo_name );
    }
#endif/*USE_FIFO*/

    rxvt_free (r->tabstop);		    SET_NULL(r->tabstop);
    rxvt_free (r->pixColorsFocus);	    SET_NULL(r->pixColorsFocus);

    if( NOT_NULL( r->pixColorsUnfocus ) )
    {
	rxvt_free (r->pixColorsUnfocus);
	SET_NULL(r->pixColorsUnfocus);
    }

#  ifdef XFT_SUPPORT
    if( NOT_NULL( r->xftColorsUnfocus ) )
    {
	rxvt_free( r->xftColorsUnfocus );
	SET_NULL( r->xftColorsUnfocus );
    }
#  endif /* XFT_SUPPORT */

# ifdef XFT_SUPPORT
    if( NOT_NULL( r->xftColors ) )
    {
	rxvt_free (r->xftColors);
	SET_NULL(r->xftColors);
    }
# endif
    rxvt_free (r->h);		    SET_NULL(r->h);
    rxvt_free (r);		    SET_NULL(r);

/* #endif */	/* DEBUG */

    exit(EXIT_SUCCESS);
}


/* ------------------------------------------------------------------------- *
 *			    PRIVILEGED OPERATIONS			     *
 * ------------------------------------------------------------------------- */

#if (defined(HAVE_SETEUID) || defined(HAVE_SETREUID)) && !defined(OS_CYGWIN)
static uid_t	    g_euid;
static gid_t	    g_egid;
#endif

/* take care of suid/sgid super-user (root) privileges */
/* EXTPROTO */
void
rxvt_privileges(int mode)
{
#if !defined(OS_CYGWIN)
# if !defined(HAVE_SETEUID) && defined(HAVE_SETREUID)
/* setreuid() is the poor man's setuid(), seteuid() */
#  define seteuid(a)	setreuid(-1, (a))
#  define setegid(a)	setregid(-1, (a))
#  define HAVE_SETEUID
# endif
# ifdef HAVE_SETEUID
    switch (mode)
    {
	case IGNORE:
	/*
	 * change effective uid/gid - not real uid/gid - so we can switch
	 * back to root later, as required
	 */
	    seteuid(getuid());
	    setegid(getgid());
	    break;
	case SAVE:
	    g_euid = geteuid();
	    g_egid = getegid();
	    break;
	case RESTORE:
	    seteuid(g_euid);
	    setegid(g_egid);
	    break;
    }
# else
    switch (mode)
    {
	case IGNORE:
	    if (setuid(getuid()) < 0)
		exit (EXIT_FAILURE);
	    if (setgid(getgid()) < 0)
		exit (EXIT_FAILURE);
	    /* FALLTHROUGH */
	case SAVE:
	    /* FALLTHROUGH */
	case RESTORE:
	    break;
    }
# endif
#endif
}


#ifdef UTMP_SUPPORT
/* EXTPROTO */
void
rxvt_privileged_utmp(rxvt_t* r, int page, char action)
{
    rxvt_msg (DBG_INFO, DBG_MAIN, "rxvt_privileged_utmp %d (%c); waiting for: %c (pid: %d)\n", page, action, PVTS(r, page)->next_utmp_action, (int) getpid());

    if (PVTS(r, page)->next_utmp_action != action ||
	(action != SAVE && action != RESTORE) ||
	ISSET_OPTION(r, Opt_utmpInhibit) ||
	IS_NULL(PVTS(r, page)->ttydev) ||
	(char) 0 == *(PVTS(r, page)->ttydev))
	return;

    rxvt_privileges(RESTORE);
    if (action == SAVE)
    {
	PVTS(r, page)->next_utmp_action = RESTORE;
	rxvt_makeutent(r, page, PVTS(r, page)->ttydev,
		r->h->rs[Rs_display_name]);
    }
    else		/* action == RESTORE */
    {
	PVTS(r, page)->next_utmp_action = IGNORE;
	rxvt_cleanutent(r, page);
    }
    rxvt_privileges(IGNORE);
}
#endif


#ifndef NO_SETOWNER_TTYDEV
/* EXTPROTO */
void
rxvt_privileged_ttydev(rxvt_t* r, int page, char action)
{
    rxvt_msg (DBG_INFO, DBG_MAIN, "rxvt_privileged_ttydev %d (r, %c); waiting for: %c (pid: %d)\n", page, action, PVTS(r, page)->next_tty_action, getpid());
    if (PVTS(r, page)->next_tty_action != action ||
	(action != SAVE && action != RESTORE) ||
	IS_NULL(PVTS(r, page)->ttydev) ||
	(char) 0 == *(PVTS(r, page)->ttydev))
	return;

    rxvt_privileges(RESTORE);

    if (action == SAVE)
    {
	PVTS(r, page)->next_tty_action = RESTORE;
# ifndef RESET_TTY_TO_COMMON_DEFAULTS
	/*
	 * store original tty status for restoration rxvt_clean_exit() -- rgg
	 * 04/12/95
	 */
	if (lstat(PVTS(r, page)->ttydev, &h->ttyfd_stat) < 0) /* you lose out */
	    PVTS(r, page)->next_tty_action = IGNORE;
	else
# endif
	{
	    /* fail silently */
	    chown(PVTS(r, page)->ttydev, getuid(), r->h->ttygid);
	    chmod(PVTS(r, page)->ttydev, PVTS(r, page)->ttymode);
# ifdef HAVE_REVOKE
	    revoke(PVTS(r, page)->ttydev);
# endif
	}
    }
    else		    /* action == RESTORE */
    {
	PVTS(r, page)->next_tty_action = IGNORE;
# ifndef RESET_TTY_TO_COMMON_DEFAULTS
	chmod(PVTS(r, page)->ttydev, PVTS(r, page)->ttyfd_stat.st_mode);
	chown(PVTS(r, page)->ttydev, PVTS(r, page)->ttyfd_stat.st_uid, PVTS(r, page)->ttyfd_stat.st_gid);
# else
	chmod(PVTS(r, page)->ttydev,
	    (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
	chown(PVTS(r, page)->ttydev, 0, 0);
# endif
    }

    rxvt_privileges(IGNORE);

# ifndef RESET_TTY_TO_COMMON_DEFAULTS
    rxvt_msg (DBG_INFO, DBG_MAIN, "%s \"%s\": mode %03o, uid %d, gid %d\n", action == RESTORE ? "Restoring" : (action == SAVE ? "Saving" : "UNKNOWN ERROR for"), PVTS(r, page)->ttydev, PVTS(r, page)->ttyfd_stat.st_mode, PVTS(r, page)->ttyfd_stat.st_uid, PVTS(r, page)->ttyfd_stat.st_gid);
# endif
}
#endif


/*----------------------------------------------------------------------*/
/*
 * Tell the teletype handler what size the window is.
 * Called after a window size change.
 */
/* EXTPROTO */
void
rxvt_tt_winsize(int fd, unsigned short col, unsigned short row, pid_t pid)
{
    struct winsize  ws;

#ifdef DEBUG
    if( pid && fd > STDERR_FILENO )
    {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_MAIN, "%s( fd=%d, col=%hu, row=%hu, pid=%d )\n", __func__, fd, col, row, pid));
    }
#endif

    if (fd < 0)
	return;

    ws.ws_col = col;
    ws.ws_row = row;
    ws.ws_xpixel = ws.ws_ypixel = 0;

    if (ioctl(fd, TIOCSWINSZ, &ws) < 0)
    {
#ifdef DEBUG
	if( pid && fd > STDERR_FILENO )
	{
	    rxvt_msg (DBG_WARN, DBG_MAIN, "Failed to send TIOCSWINSZ to fd %d\n", fd);
	}
#endif
    }
#ifdef SIGWINCH
    else if( pid )	/* force through to the command */
	kill(pid, SIGWINCH);
#endif
}


#define IDX2FNUM(i)	((FONT0_IDX + i) % MAX_NFONTS)
#define FNUM2IDX(f)	((FONT0_IDX + f) % MAX_NFONTS)


#ifdef XFT_SUPPORT
# ifndef NO_BOLDFONT
/* INTPROTO */
void
rxvt_init_bfont_xft (rxvt_t* r, XftPattern* xpold)
{
    XftResult	    fr;
    XftPattern*	    xp, *xftbpattern;
# ifdef DEBUG
    FT_Face	    face;
# endif


    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "rxvt_init_bfont_xft()"));

    xp = XftPatternDuplicate (xpold);
    if (IS_NULL(xp))
	return ;

    /* set font weight */
    XftPatternDel (xp, XFT_WEIGHT);
    setXftWeight( xp, r->h->rs[Rs_xftBwt], XFT_WEIGHT_BOLD );

    xftbpattern = XftFontMatch (r->Xdisplay, XSCREEN, xp, &fr);

    if (NOT_NULL(xftbpattern))
    {
	r->TermWin.xftbfont =
	    XftFontOpenPattern (r->Xdisplay, xftbpattern);

	if (IS_NULL(r->TermWin.xftbfont))
	{
	    /* fall back to normal font */
	    XftPatternDestroy (xftbpattern);
	    SET_NULL(xftbpattern);
	}

	else
	{
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "Opened bold font: h=%d(%d), w=%d(%d)",
			r->TermWin.xftbfont->height,
			r->TermWin.xftfont->height,
			r->TermWin.xftbfont->max_advance_width,
			r->TermWin.xftfont->max_advance_width));
#ifdef DEBUG
	    FcPatternPrint( xftbpattern );
#endif
	    /*
	     * If the bold font is of different dimensions from the regular
	     * font, we should draw it like a proportionally spaced font.
	     */
	    if (r->TermWin.xftbfont->max_advance_width < r->TermWin.fwidth)
		r->TermWin.propfont |= PROPFONT_BOLD;

	    else
	    {
		if (r->TermWin.xftbfont->max_advance_width > r->TermWin.fwidth)
		{
		    rxvt_msg (DBG_ERROR, DBG_MAIN,  "Bold font too wide. Using overstrike" );
		    XftFontClose( r->Xdisplay, r->TermWin.xftbfont );
		    SET_NULL( r->TermWin.xftbfont );
		}

		/*
		 * Now we're either overstriking, or using a correctly sized
		 * bold font, so clear PROPFONT_BOLD.
		 */
		r->TermWin.propfont &= ~PROPFONT_BOLD;
	    }
	}

# ifdef DEBUG
	{
	    if (!IS_NULL(r->TermWin.xftbfont)) 
	    {
		face = XftLockFace (r->TermWin.xftbfont);
		XftUnlockFace (r->TermWin.xftbfont);
	    }
	}
# endif
    }

    XftPatternDestroy (xp);
}
# endif	/* NO_BOLDFONT */


# ifdef MULTICHAR_SET
/* INTPROTO */
int
rxvt_init_mfont_xft (rxvt_t* r, XftPattern* xp, const char* ofname)
{
    XftResult	    fr;
    int		    len, olen;	    /* font name length */
    char*	    mfname;	    /* mfont name to open */
    FcChar8*	    omfname = NULL; /* actually opened mfont name */
    int		    width, height;
# ifdef DEBUG
    FT_Face	    face;
# endif

    /* temporary XftPattern */
    assert (NOT_NULL(xp));
    /* actually opened normal font name */
    assert (NOT_NULL(ofname));

    /*
    ** Now try to open freetype mfont
    */
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "load freetype mfont\n"));

    /* font family */
    mfname = (char*) r->h->rs[Rs_xftmfont];
    if (IS_NULL(mfname))
	mfname = rxvt_fallback_mfont_xft (r);

    /*
     * If we should not use mfont, then we always use normal font
     */
    if (ISSET_OPTION(r, Opt2_xftNomFont))
    {
	r->TermWin.xftmpattern = r->TermWin.xftpattern;
	r->TermWin.xftmfont = r->TermWin.xftfont;
	return 1;
    }

    /*
     * shortcut:
     * mfont is the same as font, just alias mfont to font
     */
    if (0 == STRCASECMP (ofname, mfname))
    {
	r->TermWin.xftmpattern = r->TermWin.xftpattern;
	r->TermWin.xftmfont = r->TermWin.xftfont;
	return 1;
    }

    /* font family */
    XftPatternDel (xp, XFT_FAMILY);
    XftPatternAddString (xp, XFT_FAMILY, mfname);

    XftPatternDel (xp, XFT_SIZE);
    /*
     * 2006-02-11 gi1242: This is however HORRIBLE for regular fonts. It's
     * probably better to fall back to xftsz. Thus the random user who knows
     * nothing about mrxvt (or encodings) will get a reasonable looking terminal
     * by default.
     */
#if 0
    /* this seems to be optimal for simsun font */
    XftPatternAddDouble (xp, XFT_SIZE, r->h->rs[Rs_xftmsz] ?
	(double) r->TermWin.xftmsize :
	(double) (r->TermWin.xftfont->height - 1));
#endif
    XftPatternAddDouble( xp, XFT_SIZE,
	    (r->h->rs[Rs_xftmsz] ?
		(double) r->TermWin.xftmsize :
		(double) r->TermWin.xftsize));

    /*
     * XXX 2006-01-26 gi1242: I don't know anything about mfont. Not sure if it
     * should be monospaced or not. Original mrxvt code did not have mono space
     * for any font, so I remove it here for the mfont.
     */
#if 0
    XftPatternDel (xp, XFT_SPACING);
#endif

    /* font pattern */
    r->TermWin.xftmpattern = XftFontMatch (r->Xdisplay, XSCREEN, xp, &fr);
    if (IS_NULL(r->TermWin.xftmpattern))
	return 0;

    /* globaladvance */
    if (ISSET_OPTION(r, Opt2_xftGlobalAdvance))
    {
	XftPatternDel (r->TermWin.xftmpattern, FC_GLOBAL_ADVANCE);
	XftPatternAddBool (r->TermWin.xftmpattern, FC_GLOBAL_ADVANCE, FcTrue);
    }

#  ifdef DEBUG
    FcPatternPrint (r->TermWin.xftmpattern);
#  endif

    /*
     * Print a warning if our matched mfont is different from the user supplied
     * font.
     */
    XftPatternGetString( r->TermWin.xftmpattern, XFT_FAMILY, 0, &omfname );

    assert (NOT_NULL(omfname));	/* shouldn't be NULL */
    len = STRLEN(mfname);
    olen = STRLEN(omfname);

    if( STRCMP( mfname, DEFAULT_XFT_FONT_NAME) 	/* Not the default font */
	    /* Not opened font */
	    && (len != olen || STRNCASECMP((char*) omfname, mfname, len))
    )
	rxvt_msg( DBG_ERROR, DBG_MAIN,
		"Cannot open mfont '%s'. Using mfont '%s' instead.\n",
	    mfname, omfname);

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MAIN, "create xftmpattern = 0x%x on mfont %d\n", (unsigned int) r->TermWin.xftmpattern, r->h->rs[Rs_xftmsz] ?  r->TermWin.xftmsize : r->TermWin.xftfont->height-1));

    /*
     * Actually open the mfont.
     */
    r->TermWin.xftmfont = XftFontOpenPattern(r->Xdisplay,
	    r->TermWin.xftmpattern);
    
    if (IS_NULL(r->TermWin.xftmfont))
	goto Failure;

# ifdef DEBUG
    face = XftLockFace (r->TermWin.xftmfont);
    XftUnlockFace (r->TermWin.xftmfont);
# endif

    width = r->TermWin.xftmfont->max_advance_width;
    if ((width & 0x01) == 1)	/* in case width is not even */
	r->TermWin.xftmono = 0;

    else if (
	      STRCASECMP (ofname, (char*) omfname)
	      && (r->TermWin.fwidth != (width >> 1))
	    )
	r->TermWin.xftmono = 0;

    else if (ISSET_OPTION(r, Opt2_xftSlowOutput))
	r->TermWin.xftmono = 0;

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "xftmono is %d\n", r->TermWin.xftmono));

    MAX_IT (r->TermWin.fwidth, (width >> 1));
    if( !r->TermWin.xftpfont )
	r->TermWin.pwidth = r->TermWin.fwidth;

    height = r->TermWin.xftmfont->ascent + r->TermWin.xftmfont->descent;
# ifndef NO_LINESPACE
    height += r->TermWin.lineSpace;
# endif
    MAX_IT (r->TermWin.fheight, height);
    if( !r->TermWin.xftpfont ) r->TermWin.pheight = r->TermWin.fheight;

    return 1;


Failure:
    if (r->TermWin.xftmpattern)
    {
	XftPatternDestroy (r->TermWin.xftmpattern);
	SET_NULL(r->TermWin.xftmpattern);
    }
    return 0;
}
# endif	/* MULTICHAR_SET */


/*
 * Set the weight of an Xft font from the weight description.
 */
/* INTPROTO */
void
setXftWeight( XftPattern *xp, const char *weightString, int defaultWeight )
{
    assert(xp);

    /* font weight */
    if ( NOT_NULL(weightString) )
    {
	if (0 == STRCASECMP (weightString, "light"))
	    XftPatternAddInteger (xp, XFT_WEIGHT, XFT_WEIGHT_LIGHT);

	else if (0 == STRCASECMP (weightString, "medium"))
	    XftPatternAddInteger (xp, XFT_WEIGHT, XFT_WEIGHT_MEDIUM);

	else if (0 == STRCASECMP (weightString, "demibold"))
	    XftPatternAddInteger (xp, XFT_WEIGHT, XFT_WEIGHT_DEMIBOLD);

	else if (0 == STRCASECMP (weightString, "bold"))
	    XftPatternAddInteger (xp, XFT_WEIGHT, XFT_WEIGHT_BOLD);

	else if (0 == STRCASECMP (weightString, "black"))
	    XftPatternAddInteger (xp, XFT_WEIGHT, XFT_WEIGHT_BLACK);

	else
	    XftPatternAddInteger (xp, XFT_WEIGHT, defaultWeight);
    }

    else
	XftPatternAddInteger( xp, XFT_WEIGHT, defaultWeight );
}


/* EXTPROTO */
int
rxvt_init_font_xft (rxvt_t* r)
{
    XftResult	    fr;
    XftPattern*	    xp;
    XGlyphInfo	    ext1, ext2;
    int		    len, olen;	    /* font name length */
    char*	    fname;	    /* font name to open */
    FcChar8*	    ofname = NULL;  /* actually opened font name */
# ifdef DEBUG
    FT_Face	    face;
# endif


    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "rxvt_init_font_xft\n"));


    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "load freetype font\n"));
    xp = XftPatternCreate ();
    if (IS_NULL(xp))
	return 0;

    /* font family */
    fname = (char*) r->h->rs[Rs_xftfont];
    if (IS_NULL(fname))
	fname = DEFAULT_XFT_FONT_NAME;
    XftPatternAddString (xp, XFT_FAMILY, fname);

    /* No spacing between lines */
    XftPatternAddBool (xp, XFT_MINSPACE, FcFalse);

    /* antialias */
    if (ISSET_OPTION(r, Opt2_xftAntialias))
	XftPatternAddBool (xp, XFT_ANTIALIAS, FcTrue);
    else
	XftPatternAddBool (xp, XFT_ANTIALIAS, FcFalse);

    /* hinting */
    if (ISSET_OPTION(r, Opt2_xftHinting))
	XftPatternAddBool (xp, FC_HINTING, FcTrue);
    else
	XftPatternAddBool (xp, FC_HINTING, FcFalse);

    /* autohint */
    if (ISSET_OPTION(r, Opt2_xftAutoHint))
	XftPatternAddBool (xp, FC_AUTOHINT, FcTrue);
    else
	XftPatternAddBool (xp, FC_AUTOHINT, FcFalse);

    /* font size, we always set it. if it's not set by the user, we
    ** have chosen a default value (12) for it */
    XftPatternAddDouble (xp, XFT_SIZE, (double) r->TermWin.xftsize);
    /* Enforce Size/Pixel_Size = 1.0 */
    XftPatternAddDouble (xp, XFT_SCALE, (double) 1.0);

    /* font width */
    if (r->h->rs[Rs_xftwd])
    {
	if (0 == STRCASECMP (r->h->rs[Rs_xftwd], "ultracondensed"))
	    XftPatternAddInteger (xp, FC_WIDTH, FC_WIDTH_ULTRACONDENSED);

	else if (0 == STRCASECMP (r->h->rs[Rs_xftwd], "condensed"))
	    XftPatternAddInteger (xp, FC_WIDTH, FC_WIDTH_CONDENSED);

	else if (0 == STRCASECMP (r->h->rs[Rs_xftwd], "normal"))
	    XftPatternAddInteger (xp, FC_WIDTH, FC_WIDTH_NORMAL);

	else if (0 == STRCASECMP (r->h->rs[Rs_xftwd], "expanded"))
	    XftPatternAddInteger (xp, FC_WIDTH, FC_WIDTH_EXPANDED);

	else if (0 == STRCASECMP (r->h->rs[Rs_xftwd], "ultraexpanded"))
	    XftPatternAddInteger (xp, FC_WIDTH, FC_WIDTH_ULTRAEXPANDED);

	else
	    XftPatternAddInteger (xp, FC_WIDTH, FC_WIDTH_NORMAL);
    }

    setXftWeight( xp, r->h->rs[Rs_xftwt], XFT_WEIGHT_MEDIUM );

    /* font slant */
    if (r->h->rs[Rs_xftst])
    {
	if (0 == STRCASECMP (r->h->rs[Rs_xftst], "roman"))
	    XftPatternAddInteger (xp, XFT_SLANT, XFT_SLANT_ROMAN);

	else if (0 == STRCASECMP (r->h->rs[Rs_xftst], "italic"))
	    XftPatternAddInteger (xp, XFT_SLANT, XFT_SLANT_ITALIC);

	else if (0 == STRCASECMP (r->h->rs[Rs_xftst], "oblique"))
	    XftPatternAddInteger (xp, XFT_SLANT, XFT_SLANT_OBLIQUE);

	else	/* default is roman */
	    XftPatternAddInteger (xp, XFT_SLANT, XFT_SLANT_ROMAN);
    }

    /* font rgba */
    if (r->h->rs[Rs_xftrgb])
    {
	if (0 == STRCASECMP (r->h->rs[Rs_xftrgb], "rgb"))
	    XftPatternAddInteger (xp, XFT_RGBA, XFT_RGBA_RGB);

	else if (0 == STRCASECMP (r->h->rs[Rs_xftrgb], "bgr"))
	    XftPatternAddInteger (xp, XFT_RGBA, XFT_RGBA_BGR);

	else if (0 == STRCASECMP (r->h->rs[Rs_xftrgb], "vrgb"))
	    XftPatternAddInteger (xp, XFT_RGBA, XFT_RGBA_VRGB);

	else if (0 == STRCASECMP (r->h->rs[Rs_xftrgb], "vbgr"))
	    XftPatternAddInteger (xp, XFT_RGBA, XFT_RGBA_VBGR);

	else
	    XftPatternAddInteger (xp, XFT_RGBA, XFT_RGBA_NONE);
    }

    /*
     * 2006-01-26 gi1242: Monospaced fonts seem a good idea.
     * 2006-01-27 gi1242: Maybe not such a good idea. When we ask for a
     * monospaced font from a propotionally spaced font, we just get the same
     * old prop font, with a bigass textwidth. That's no use to us. If it
     * returned the closest matching mono-spaced font, then that would be
     * useful.
     */
#if 0
    XftPatternAddInteger (xp, XFT_SPACING, XFT_MONO);
#endif

    r->TermWin.xftpattern = XftFontMatch (r->Xdisplay, XSCREEN, xp, &fr);

    if (IS_NULL(r->TermWin.xftpattern))
	goto Failure;

    /* globaladvance */
    if (ISSET_OPTION(r, Opt2_xftGlobalAdvance))
    {
	XftPatternDel (r->TermWin.xftpattern, FC_GLOBAL_ADVANCE);
	XftPatternAddBool (r->TermWin.xftpattern, FC_GLOBAL_ADVANCE, FcTrue);
    }

# ifdef DEBUG
    FcPatternPrint (r->TermWin.xftpattern);
# endif

    /*
     * Print a warning if our matched font is different from the user supplied
     * font.
     */
    XftPatternGetString (r->TermWin.xftpattern, XFT_FAMILY, 0, &ofname);
    assert (NOT_NULL(ofname));	/* shouldn't be NULL */

    len = STRLEN(fname);
    olen = STRLEN(ofname);
    if( STRCMP( fname, DEFAULT_XFT_FONT_NAME) 	/* Not the default font */
	    /* Not opened font */
	    && (len != olen || STRNCASECMP((char*) ofname, fname, len))
    )
	rxvt_msg( DBG_ERROR, DBG_MAIN,
		"Cannot open font '%s'. Using font '%s' instead.\n",
	    fname, ofname);

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MAIN, "create xftpattern = 0x%x on font %d\n", (unsigned int) r->TermWin.xftpattern, r->TermWin.xftsize));

    /*
     * Actually open the font.
     */
    r->TermWin.xftfont = XftFontOpenPattern( r->Xdisplay,
					    r->TermWin.xftpattern);

    /*
     * Now open the prop font.
     */
    if (IS_NULL(r->h->rs[Rs_xftpfn]))
	fname = DEFAULT_XFT_PFONT_NAME;
    else if( !STRCASECMP( r->h->rs[Rs_xftpfn], "none"))
	SET_NULL(fname);
    else
	fname = (char *) r->h->rs[Rs_xftpfn];

    if (NOT_NULL(fname) &&
	IS_NULL(r->TermWin.xftpfont) &&
	IS_NULL(r->TermWin.xftPfont))
    {
	XftPattern *ppat = XftPatternDuplicate( xp );
	XftPattern *match;

	XftPatternDel( ppat, XFT_FAMILY);
	XftPatternAddString (ppat, XFT_FAMILY, fname);

	XftPatternDel( ppat, XFT_SIZE);
	XftPatternAddDouble( ppat, XFT_SIZE, (double) r->TermWin.xftpsize);

	/*
	 * If one ever adds a XFT_MONO flag to the pattern, then it should be
	 * removed here.
	 */
#if 0
	XftPatternDel( ppat, XFT_SPACING);
#endif

	match = XftFontMatch (r->Xdisplay, XSCREEN, ppat, &fr);
	r->TermWin.xftpfont = XftFontOpenPattern( r->Xdisplay, match);

	/*
	 * XXX gi1242 2006-01-27: Xft bug. Patterns passed to XftFontOpenPattern
	 * can't always be safely freed.
	 */
#if 0
	FcPatternPrint( match);
	XftPatternDestroy( match);
#endif

	/*
	 * Try and open a bold version of the same font to use for the active
	 * tab title / etc.
	 */
	XftPatternDel	     (ppat, XFT_WEIGHT);
	XftPatternAddInteger (ppat, XFT_WEIGHT, XFT_WEIGHT_BOLD);

	match = XftFontMatch (r->Xdisplay, XSCREEN, ppat, &fr);
	r->TermWin.xftPfont = XftFontOpenPattern( r->Xdisplay, match);

	/*
	 * If we couldn't open the bold font, then use the regular font.
	 */
	if( !r->TermWin.xftPfont ) r->TermWin.xftPfont = r->TermWin.xftpfont;

	/*
	 * XXX gi1242 2006-01-27: Xft bug. Patterns passed to XftFontOpenPattern
	 * can't always be safely freed.
	 */
#if 0
	XftPatternDestroy( match);
#endif

	XftPatternDestroy( ppat);

	rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "Opened prop fonts %p, %p\n", r->TermWin.xftpfont, r->TermWin.xftPfont));
    }
    
    if (IS_NULL(r->TermWin.xftfont))
	goto Failure;

    r->TermWin.fwidth = r->TermWin.xftfont->max_advance_width;
    r->TermWin.fheight = r->TermWin.xftfont->ascent +
				r->TermWin.xftfont->descent;
# ifndef NO_LINESPACE
    r->TermWin.fheight += r->TermWin.lineSpace;
# endif

    /*
     * Failure opening prop fonts can be handled gracefully: Fall back to the
     * usual font.
     */
#ifdef MULTICHAR_SET
    if (IS_NULL(r->TermWin.xftpfont) &&
	isDoubleWidthFont(r->Xdisplay, r->TermWin.xftfont)
       )
    {
	/*
	 * 2006-04-28 gi1242: Double width fonts are only handled correctly by
	 * the screen drawing functions. The problem is that these need to be
	 * drawn one character at a time to get correct output, which is not
	 * done by the menu / tabbar functions. It will be a waste to write
	 * extra code to do that, so we pretend that we *have* a pfont.
	 *
	 * This will however make the font spacing in tab titles incorrect for
	 * double width fonts.
	 */
	rxvt_msg (DBG_WARN, DBG_MAIN, "Warning: Double width font, with no pfont\n");
	r->TermWin.xftpfont = r->TermWin.xftfont;
	r->TermWin.xftPfont = r->TermWin.xftfont;
    }
#endif /* MULTICHAR_SET */

    if( r->TermWin.xftpfont )
    {
	r->TermWin.pheight  = max( r->TermWin.xftpfont->height,
		r->TermWin.xftPfont->height);
	r->TermWin.pwidth   = r->TermWin.xftpfont->max_advance_width / 2;
    }
    else
    {
	r->TermWin.pheight  = r->TermWin.fheight;
	r->TermWin.pwidth   = r->TermWin.fwidth;
    }

#ifdef DEBUG
    face = XftLockFace (r->TermWin.xftfont);
    XftUnlockFace (r->TermWin.xftfont);
# endif

    /*
     * Do not trust the font width
     */
#ifdef MULTICHAR_SET
    if( isDoubleWidthFont( r->Xdisplay, r->TermWin.xftfont ) )
    {
	rxvt_msg (DBG_INFO, DBG_MAIN, "Got double width font. Halving width\n");
	r->TermWin.xftmono = 0;
	r->TermWin.fwidth >>= 1;
    }
    else
#endif
    {
	XftTextExtents8 (r->Xdisplay, r->TermWin.xftfont, (unsigned char*) "W",
		1, &ext1);
	XftTextExtents8 (r->Xdisplay, r->TermWin.xftfont, (unsigned char*) "i",
		1, &ext2);

	if (ext1.xOff == ext2.xOff)
	    r->TermWin.xftfnmono = r->TermWin.xftmono = 1;
	else
	    r->TermWin.xftfnmono = r->TermWin.xftmono = 0;
    }
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "xftfnmono is %d\n", r->TermWin.xftfnmono));


# ifndef NO_BOLDFONT
    rxvt_init_bfont_xft( r, xp );
# endif

# ifdef MULTICHAR_SET
    if (!rxvt_init_mfont_xft (r, xp, (char*) ofname))
	goto Failure;
# endif	/* MULTICHAR_SET */

    XftPatternDestroy (xp);

    return 1;


Failure:
    if (xp)
    {
	XftPatternDestroy (xp);
    }
    if (r->TermWin.xftpattern && r->TermWin.xftpattern != xp)
    {
	XftPatternDestroy (r->TermWin.xftpattern);
	SET_NULL(r->TermWin.xftpattern);
    }
    return 0;
}


/*----------------------------------------------------------------------*/
/* rxvt_init_font_fixed () - initialize fixed font */
/* INTPROTO */
void
rxvt_init_font_fixed (rxvt_t* r)
{
    XFontStruct*    xfont;


    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, " load font (fixed)\n"));
    xfont = XLoadQueryFont( r->Xdisplay, "fixed" );
    if (IS_NULL(xfont))
    {
	rxvt_msg( DBG_FATAL, DBG_MAIN, "Could not load font 'fixed'.\n" );
	exit(EXIT_FAILURE);
    }

    r->TermWin.font = xfont;
#ifndef NO_BOLDFONT
    /*
    r->TermWin.bfont = r->TermWin.xftbfont? xfont : NULL;
    */
    SET_NULL(r->TermWin.bfont);
#endif
#ifdef MULTICHAR_SET
    r->TermWin.mfont = xfont;
#endif
}
#endif	/* XFT_SUPPORT */


/* rxvt_init_font_x11 () - initialize font */
/* EXTPROTO */
void
rxvt_init_font_x11 (rxvt_t *r)
{
    char*	    msg = "can't load font \"%s\"\n";
    XFontStruct*    xfont;
#ifndef NO_BOLDFONT
    XFontStruct*    bfont;
    int		    ckfont;
#endif
    int		    fh = 0, fw = 0;
    int		    idx = 0;		/* index into rs_font[] */


    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "rxvt_init_font_x11 \n"));


#ifdef XFT_SUPPORT
    /* Only load fixed font if we use freetype font */
    if (ISSET_OPTION(r, Opt_xft) && r->TermWin.xftfont)
    {
	rxvt_init_font_fixed (r);
	return;
    }
#endif


    r->h->fnum = FONT0_IDX;
    idx = FNUM2IDX(r->h->fnum);

    /* OK, now it's time to load the default font */
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, " load font (%s)\n", r->h->rs[Rs_font+idx]));
    xfont = XLoadQueryFont (r->Xdisplay, r->h->rs[Rs_font+idx]);
    if (IS_NULL(xfont))
    {
	/* failed to load font */
	rxvt_msg( DBG_ERROR, DBG_MAIN, msg, r->h->rs[Rs_font+idx] );

	/* try to load fixed font */
	r->h->rs[Rs_font+idx] = "fixed";
	rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, " load font (%s)\n", r->h->rs[Rs_font+idx]));
	xfont = XLoadQueryFont(r->Xdisplay, r->h->rs[Rs_font+idx]);
	if (IS_NULL(xfont))
	{
	    /* still failed to load font */
	    rxvt_msg( DBG_ERROR, DBG_MAIN, msg, r->h->rs[Rs_font+idx] );

	    /* cannot load any font, fatal error, abort the program */
	    goto Abort;
	}
    }
    /* Font loading succeeded */
    if (NOT_NULL(xfont))
    {
	r->TermWin.font = xfont;
    }


    /* set the font sizes */
    fw = rxvt_get_font_widest (r->TermWin.font);
    fh = r->TermWin.font->ascent + r->TermWin.font->descent;
#ifndef NO_LINESPACE
    fh += r->TermWin.lineSpace;
#endif

    if (fw == r->TermWin.font->min_bounds.width)
	/* Mono-spaced (fixed width) font */
	r->TermWin.propfont &= ~PROPFONT_NORMAL;
    else
	/* Proportional font */
	r->TermWin.propfont |= PROPFONT_NORMAL;

#ifndef NO_BOLDFONT
    ckfont = !(fw == r->TermWin.fwidth && fh == r->TermWin.fheight);
#endif

    r->TermWin.fwidth  = fw;
    r->TermWin.fheight = fh;

#ifdef XFT_SUPPORT
    r->TermWin.pwidth	= fw;
    r->TermWin.pheight	= fh;
#endif

#ifndef NO_BOLDFONT
    SET_NULL(bfont);
    if (ckfont)
    {
	/* try to load boldFont, fail silently */
	if (NOT_NULL(r->h->rs[Rs_boldFont+idx]))
	{
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, " load bfont (%s)\n", r->h->rs[Rs_boldFont+idx]));
	    bfont = XLoadQueryFont (r->Xdisplay, r->h->rs[Rs_boldFont+idx]);
	}

	if (NOT_NULL(bfont))
	{
	    /* Loading bold font succeeded */
	    fw = rxvt_get_font_widest (bfont);
	    fh = bfont->ascent + bfont->descent;
#ifndef NO_LINESPACE
	    fh += r->TermWin.lineSpace;
#endif
	    if (fw <= r->TermWin.fwidth && fh <= r->TermWin.fheight)
	    {
		r->TermWin.bfont = bfont;
		if (fw == r->TermWin.fwidth)
		    r->TermWin.propfont &= ~PROPFONT_BOLD;
		else
		    r->TermWin.propfont |= PROPFONT_BOLD;
	    }
	    else
	    {
		XFreeFont (r->Xdisplay, bfont);
	    }
	}
    }
#endif /* NO_BOLDFONT */

#ifdef MULTICHAR_SET
    /* load font or substitute */
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, " load mfont (%s)\n", r->h->rs[Rs_mfont+idx]));
    xfont = XLoadQueryFont(r->Xdisplay, r->h->rs[Rs_mfont+idx]);
    if (IS_NULL(xfont))
    {
	char*	ptr;

	/* failed to load font */
	rxvt_msg (DBG_ERROR, DBG_MAIN, msg, r->h->rs[Rs_mfont+idx]);

	ptr = rxvt_fallback_mfont_x11 (r);
	rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, " load mfont (%s)\n", ptr));
	xfont = XLoadQueryFont(r->Xdisplay, ptr);
	if (NOT_NULL(xfont))
	    r->h->rs[Rs_mfont+idx] = ptr;
	else
	{
	    /* still failed to load font */
	    rxvt_msg (DBG_ERROR, DBG_MAIN, msg, ptr);
	    /* cannot load any mfont, fatal error, abort the program */
	    goto Abort;
	}
    }
    if (NOT_NULL(xfont))
    {
	r->TermWin.mfont = xfont;
    }
#endif	/* MULTICHAR_SET */

    /* Succeeded to load font, return now */
    return ;

Abort:
    rxvt_msg (DBG_ERROR, DBG_MAIN, "fatal error, aborting...");
    exit(EXIT_FAILURE);
}


#ifdef XFT_SUPPORT

# ifdef MULTICHAR_SET
/*
 * 2006-04-27 gi1242: Stolen from xterm-207. Some fonts (e.g. aquafont) report
 * their width to be double the actual width (see below). Test for this here.
 */
#  define HALF_WIDTH_TEST_STRING "1234567890"

/* '1234567890' in Chinese characters in UTF-8 */
#  define FULL_WIDTH_TEST_STRING "\xe4\xb8\x80\xe4\xba\x8c\xe4\xb8\x89" \
                               "\xe5\x9b\x9b\xe4\xba\x94" \
	           "\xef\xa7\x91\xe4\xb8\x83\xe5\x85\xab" \
	           "\xe4\xb9\x9d\xef\xa6\xb2"

/* '1234567890' in Korean script in UTF-8 */
#  define FULL_WIDTH_TEST_STRING2 "\xec\x9d\xbc\xec\x9d\xb4\xec\x82\xbc" \
                                "\xec\x82\xac\xec\x98\xa4" \
	            "\xec\x9c\xa1\xec\xb9\xa0\xed\x8c\x94" \
	            "\xea\xb5\xac\xec\x98\x81"

#  define HALF_WIDTH_CHAR1  0x0031  /* 'l' */
#  define HALF_WIDTH_CHAR2  0x0057  /* 'W' */
#  define FULL_WIDTH_CHAR1  0x4E00  /* CJK Ideograph 'number one' */
#  define FULL_WIDTH_CHAR2  0xAC00  /* Korean script syllable 'Ka' */

/*INTPROTO*/
int
isDoubleWidthFont(Display * dpy, XftFont * font)
{
    XGlyphInfo gi1, gi2;
    FcChar32 c1 = HALF_WIDTH_CHAR1, c2 = HALF_WIDTH_CHAR2;
    char *fwstr = FULL_WIDTH_TEST_STRING;
    char *hwstr = HALF_WIDTH_TEST_STRING;

    /* Some Korean fonts don't have Chinese characters at all. */
    if (!XftCharExists(dpy, font, FULL_WIDTH_CHAR1))
    {
	if (!XftCharExists(dpy, font, FULL_WIDTH_CHAR2))
	    return 0;	    /* Not a CJK font */
	else		    /* a Korean font without CJK Ideographs */
	    fwstr = FULL_WIDTH_TEST_STRING2;
    }

    XftTextExtents32(dpy, font, &c1, 1, &gi1);
    XftTextExtents32(dpy, font, &c2, 1, &gi2);
    if (gi1.xOff != gi2.xOff)	/* Not a fixed-width font */
    return 0;

    XftTextExtentsUtf8(dpy, font, (FcChar8 *) hwstr, (int) strlen(hwstr), &gi1);
    XftTextExtentsUtf8(dpy, font, (FcChar8 *) fwstr, (int) strlen(fwstr), &gi2);

    /*
     * fontconfig and Xft prior to 2.2(?) set the width of half-width characters
     * identical to that of full-width character in CJK double-width (bi-width /
     * monospace) font even though the former is half as wide as the latter.
     * This was fixed sometime before the release of fontconfig 2.2 in early
     * 2003.  See
     *
     *		http://bugzilla.mozilla.org/show_bug.cgi?id=196312
     *
     * In the meantime, we have to check both possibilities.
     */
    return ((2 * gi1.xOff == gi2.xOff) || (gi1.xOff == gi2.xOff));
}
# endif /*MULTICHAR_SET*/


/*
 * Close font f if it is not one of xftfont, xftpfont, xftbfont, xftmfont.
 */
/* INTPROTO */
void
xftFreeUnusedFont( rxvt_t *r, XftFont *f)
{
    if( f && f != r->TermWin.xftfont &&
	    f != r->TermWin.xftpfont && f != r->TermWin.xftPfont
# ifdef MULTICHAR_SET
	    && f != r->TermWin.xftmfont
# endif
# ifndef NO_BOLDFONT
	    && f != r->TermWin.xftbfont
# endif
      )
	XftFontClose( r->Xdisplay, f);
}

/* EXTPROTO */
int
rxvt_change_font_xft (rxvt_t* r, const char* fontname)
{
    XftPattern*	xp;
    XftFont	*xf, *pxf, *Pxf;
# ifdef MULTICHAR_SET
    XftPattern*	mxp;
    XftFont*	mxf;
# endif
    int		resize, oldsize = r->TermWin.xftsize;


    assert (fontname);
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "rxvt_change_font_xft (%s)\n", fontname));

    /* we only accept FONT_CMD now for XFT font ;-) */
    if (FONT_CMD != fontname[0])
	return 0;
    if ((char) 0 != fontname[1] &&
	'+' != fontname[1] &&
	'-' != fontname[1] &&
	!isdigit((int) fontname[1]))
	return 0;
    if (('+' == fontname[1]) && ((char) 0 == fontname[2]))
	resize = +1;
    else
    if (('-' == fontname[1]) && ((char) 0 == fontname[2]))
	resize = -1;
    else
	resize = atoi (fontname+1);
    r->TermWin.xftsize += resize;
    /* adjust for minimal font size */
    if (r->TermWin.xftsize < MIN_XFT_FONT_SIZE)
	r->TermWin.xftsize = MIN_XFT_FONT_SIZE;
    /* no change of font size */
    if (r->TermWin.xftsize == oldsize)
	return 0;

    /* Change pfont size and mfont size */
    r->TermWin.xftpsize += resize;
    if (r->TermWin.xftpsize < MIN_XFT_FONT_SIZE)
	r->TermWin.xftpsize = MIN_XFT_FONT_SIZE;

# ifdef MULTICHAR_SET
    if (r->h->rs[Rs_xftmsz])
    {
	r->TermWin.xftmsize += resize;
	if (r->TermWin.xftmsize < MIN_XFT_FONT_SIZE)
	    r->TermWin.xftmsize = MIN_XFT_FONT_SIZE;
    }
# endif

    /*
    ** Now reload xft font with new size using rxvt_init_font_xft.
    ** We can reuse it since we have only changed the font size.
    ** Before doing so, let us backup the old xft info in case
    ** we cannot load new xft font. In that case, we can still
    ** fallback to the old font.
    */
    xp = r->TermWin.xftpattern;	SET_NULL(r->TermWin.xftpattern);
    xf = r->TermWin.xftfont;	SET_NULL(r->TermWin.xftfont);
    pxf = r->TermWin.xftpfont;	SET_NULL(r->TermWin.xftpfont);
    Pxf = r->TermWin.xftPfont;	SET_NULL(r->TermWin.xftPfont);
# ifdef MULTICHAR_SET
    mxp = r->TermWin.xftmpattern;   SET_NULL(r->TermWin.xftmpattern);
    mxf = r->TermWin.xftmfont;	    SET_NULL(r->TermWin.xftmfont);
# endif
    if (!rxvt_init_font_xft (r))
    {
	/* fallback to old font */
	r->TermWin.xftpattern	= xp;
	r->TermWin.xftfont	= xf;
	r->TermWin.xftpfont	= pxf;
	r->TermWin.xftPfont	= Pxf;
# ifdef MULTICHAR_SET
	r->TermWin.xftmpattern	= mxp;
	r->TermWin.xftmfont	= mxf;
# endif
	return 0;
    }

    /* The old Xft font (if differnt) can now be freed */
    xftFreeUnusedFont( r, xf );
    if( pxf != xf) xftFreeUnusedFont( r, pxf );
    if( Pxf != xf && Pxf != pxf) xftFreeUnusedFont( r, Pxf);

#ifdef MULTICHAR_SET
    if( mxf != xf && mxf != pxf) xftFreeUnusedFont( r, mxf );
#endif

    /*
     * XXX It is safe now to free old XftPattern, but apparently there is some
     * problems if we change the font size in reverse direction, e.g., increase
     * font size then decrease it, or decrease font size then increase it. It
     * can crash the terminal.
     *
     * 12-19-2004: Is it because the pattern is got via XftFontMatch, which
     * actually returns a *static* pattern that we should not manually destroy?
     * Anyway, to avoid the crash, let us comment out the XftPatternDestroy
     * calls below. I hope there is a more clear documentation about these Xft
     * functions!!!
     *
     * 2006-01-27 gi1242: After grepping through the Xft source code, I think I
     * know the problem. XftFontOpenInfo destroys the font pattern if the
     * pattern corresponds to an already open font. Thus when we destroy it,
     * it's a double free leading to a friendly segfault. This is a bug in
     * Xft.
     *
     * To make matters worse, closing previously opened fonts won't work. Xft
     * caches fonts ... and only closes them when necessary. Thus calling
     * XftFontClose them won't guarentee the font is actually closed. So untill
     * the Xft folks fix this, we have to live with a temporary memory leak.
     */

#if 0
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MAIN, "Destroy %p, (xftpattern %p, xftmpattern %p)\n", xp, r->TermWin.xftpattern, r->TermWin.xftmpattern)));
    /* FcPatternPrint (xp);
    FcPatternPrint (r->TermWin.xftpattern); */
    XftPatternDestroy (xp);
# ifdef MULTICHAR_SET
    if (xp != mxp)
	XftPatternDestroy (mxp);
# endif
#endif

    return 1;
}
#endif	/* XFT_SUPPORT */


/*----------------------------------------------------------------------*/
/* rxvt_change_font_x11 () - Switch to a new font */
/*
 * init = 1   - initialize
 *
 * fontname == FONT_UP  - switch to bigger font
 * fontname == FONT_DN  - switch to smaller font
 */
/* EXTPROTO */
int
rxvt_change_font_x11 (rxvt_t* r, const char *fontname)
{
    char*	    msg = "can't load font \"%s\"";
    XFontStruct*    xfont;
#ifndef NO_BOLDFONT
    XFontStruct*    bfont;
    int		    ckfont;
#endif
    int		    fh = 0, fw = 0;
    int		    idx = 0;		/* index into rs_font[] */


    assert (fontname);
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "rxvt_change_font_x11 (%s)\n", fontname));


    switch (fontname[0])
    {
	/* special (internal) prefix for font commands */
	case FONT_CMD: /* FONT_CMD =='#' */
	    idx = atoi (fontname + 1);

	    switch (fontname[1])
	    {
		case '+':   /* corresponds to FONT_UP */
		    r->h->fnum += (idx ? idx : 1);  /* "#+" or "#+3"? */
		    r->h->fnum %= MAX_NFONTS;
		    break;

		case '-':   /* corresponds to FONT_DN */
		    r->h->fnum += (idx ? idx : -1); /* "#-" or "#-3"? */
		    r->h->fnum %= MAX_NFONTS;
		    break;

		default:
		    /* input is not a logical font number */
		    if (fontname[1] != '\0' &&
			!isdigit((int) fontname[1]))
			return 0;
		    /*
		     * input logical font number too big, but don't worry, we
		     * will handle it gracefully ;-)
		     */
		    r->h->fnum = IDX2FNUM(idx);
		    break;
	    }
	    SET_NULL(fontname);
	    break;

	default:
	    /* search for existing fontname */
	    for (idx = 0; idx < MAX_NFONTS; idx++)
	    {
		if (!STRCMP (r->h->rs[Rs_font+idx], fontname))
		{
		    r->h->fnum = IDX2FNUM(idx);
		    SET_NULL(fontname);
		    break;
		}
	    }
	    break;
    }
    /* re-position around the normal font */
    if (r->h->fnum < 0)
	r->h->fnum += (-(r->h->fnum / MAX_NFONTS - 1) * MAX_NFONTS);
    idx = FNUM2IDX(r->h->fnum);


    /*
    ** If fontname != NULL, it's some new font not in the rs_font.
    ** We try to load it and replace font in rs_font if succeed.
    */
    if (NOT_NULL(fontname))
    {
	xfont = XLoadQueryFont(r->Xdisplay, fontname);
	if (xfont)
	{
	    /* load new font succeessfully */
	    char* ptr = STRDUP (fontname);
	    if (ptr)
	    {
		if (NOT_NULL(r->h->newfont[idx]))   
		    rxvt_free (r->h->newfont[idx]);
		r->h->newfont[idx] = ptr;
		r->h->rs[Rs_font+idx] = r->h->newfont[idx];
	    }
	    else
	    {
		assert (0); /* shouldn't happen */
	    }
	    /* Free it by now */
	    XFreeFont (r->Xdisplay, xfont);
	}
    }


    /*
    ** OK, now it's time to load font or substitute
    */
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, " load font (%s)\n", r->h->rs[Rs_font+idx]));
    xfont = XLoadQueryFont (r->Xdisplay, r->h->rs[Rs_font+idx]);
    if (!xfont)
    {
	/* failed to load font */
	rxvt_msg (DBG_ERROR, DBG_MAIN, msg, r->h->rs[Rs_font+idx]);

	/* try to load fixed font */
	r->h->rs[Rs_font+idx] = "fixed";
	rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, " load font (%s)\n", r->h->rs[Rs_font+idx]));
	xfont = XLoadQueryFont(r->Xdisplay, r->h->rs[Rs_font+idx]);
	if (!xfont)
	{
	    /* still failed to load font */
	    rxvt_msg (DBG_ERROR, DBG_MAIN, msg, r->h->rs[Rs_font+idx]);
	    return 0;
	}
    }
    /* Font loading succeeded */
    if (xfont)
    {
	/*
	** if the previous font exists, replace it with new font.
	** otherwise, keep the old font, and do nothing
	*/
	if (r->TermWin.font)
	    XFreeFont (r->Xdisplay, r->TermWin.font);
	r->TermWin.font = xfont;
    }


    /* set the font sizes */
    fw = rxvt_get_font_widest (r->TermWin.font);
    fh = r->TermWin.font->ascent + r->TermWin.font->descent;
#ifndef NO_LINESPACE
    fh += r->TermWin.lineSpace;
#endif

    if (fw == r->TermWin.font->min_bounds.width)
	/* Mono-spaced (fixed width) font */
	r->TermWin.propfont &= ~PROPFONT_NORMAL;
    else
	/* Proportional font */
	r->TermWin.propfont |= PROPFONT_NORMAL;

#ifndef NO_BOLDFONT
    ckfont = !(fw == r->TermWin.fwidth && fh == r->TermWin.fheight);
#endif

#ifdef XFT_SUPPORT
    /* Set font size when XFT is not enabled */
    if (!(ISSET_OPTION(r, Opt_xft) && r->TermWin.xftfont))
#endif
    {
#ifdef XFT_SUPPORT
	r->TermWin.pwidth  = fw;
	r->TermWin.pheight = fh;
#endif
	r->TermWin.fwidth  = fw;
	r->TermWin.fheight = fh;
    }


#ifndef NO_BOLDFONT
    SET_NULL(bfont);
    if (ckfont)
    {
	if (r->TermWin.bfont)	{
	    XFreeFont (r->Xdisplay, r->TermWin.bfont);
	    SET_NULL(r->TermWin.bfont);
	}

	/* try to load boldFont, fail silently */
	if (IS_NULL(r->TermWin.bfont) &&
	    NOT_NULL(r->h->rs[Rs_boldFont+idx]))
	{
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, " load bfont (%s)\n", r->h->rs[Rs_boldFont+idx]));
	    bfont = XLoadQueryFont (r->Xdisplay, r->h->rs[Rs_boldFont+idx]);
	}

	if (bfont)
	{
	    /* Loading bold font succeeded */
	    fw = rxvt_get_font_widest (bfont);
	    fh = bfont->ascent + bfont->descent;
#ifndef NO_LINESPACE
	    fh += r->TermWin.lineSpace;
#endif
	    if (fw <= r->TermWin.fwidth && fh <= r->TermWin.fheight)
	    {
		r->TermWin.bfont = bfont;
		if (fw == r->TermWin.fwidth)
		    r->TermWin.propfont &= ~PROPFONT_NORMAL;
		else
		    r->TermWin.propfont |= PROPFONT_NORMAL;
	    }
	    else
	    {
		XFreeFont (r->Xdisplay, bfont);
	    }
	}
    }
#endif /* NO_BOLDFONT */

#ifdef HAVE_MENUBAR
    /*
     * 2006-01-29 gi1242: We should do something to resize the menubar font.
     * However just changing it in the appropriate GC from here doesn't seem to
     * work at all.
     */
    /* Resize the menubar font too. Only needed for X11 font. */
    if(
#ifdef XFT_SUPPORT
	NOTSET_OPTION(r, Opt_xft) &&
#endif
	IS_WIN(r->menuBar.win))
    {
	rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "Resized menubar font\n"));
	XSetFont( r->Xdisplay, r->menuBar.gc, r->TermWin.font->fid);
    }
#endif

#ifdef MULTICHAR_SET
    /* load font or substitute */
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, " load mfont (%s)\n", r->h->rs[Rs_mfont+idx]));
    xfont = XLoadQueryFont(r->Xdisplay, r->h->rs[Rs_mfont+idx]);
    if (!xfont)
    {
	char*	ptr;

	/* failed to load font */
	rxvt_msg (DBG_ERROR, DBG_MAIN, msg, r->h->rs[Rs_mfont+idx]);

	/* try to load default font */
	ptr = rxvt_fallback_mfont_x11 (r);
	rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, " load mfont (%s)\n", ptr));
	xfont = XLoadQueryFont(r->Xdisplay, ptr);
	if (xfont)
	    r->h->rs[Rs_mfont+idx] = ptr;
	else
	{
	    /* still failed to load font */
	    rxvt_msg (DBG_ERROR, DBG_MAIN, msg, ptr);
	    return 0;
	}
    }
    if (xfont)
    {
	/*
	** if the previous font exists, replace it with new font.
	** otherwise, keep the old font, and do nothing
	*/
	if (r->TermWin.mfont)
	    XFreeFont (r->Xdisplay, r->TermWin.mfont);
	r->TermWin.mfont = xfont;
    }
#endif	/* MULTICHAR_SET */

#ifdef USE_XIM
    rxvt_IM_change_fontset(r, idx);
#endif
    return 1;
}


/* INTPROTO */
void
rxvt_font_up_down(rxvt_t* r, int n, int direction)
{
    const char	 *p;
    int		    initial, j;

    for (j = 0; j < n; j++)
    {
	initial = r->h->fnum;
	for (;;)
	{
	    r->h->fnum += direction;
	    if (r->h->fnum == MAX_NFONTS || r->h->fnum == -1)
	    {
		r->h->fnum = initial;
		return;
	    }
	    p = r->h->rs[Rs_font + FNUM2IDX(r->h->fnum)];

	    if (NOT_NULL(p) && STRLEN(p) > 1)
		break;
	}
    }
}
#undef IDX2FNUM
#undef FNUM2IDX


/* INTPROTO */
int
rxvt_get_font_widest(XFontStruct *f)
{
    int		    i, cw, fw = 0;

    if (f->min_bounds.width == f->max_bounds.width)
	return f->min_bounds.width;
    if (IS_NULL(f->per_char))
	return f->max_bounds.width;
    for (i = f->max_char_or_byte2 - f->min_char_or_byte2; --i >= 0;)
    {
	cw = f->per_char[i].width;
	MAX_IT(fw, cw);
    }
    return fw;
}

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
#ifdef X_HAVE_UTF8_STRING
/* INTPROTO */
void
rxvt_set_utf8_property (rxvt_t* r, Atom prop, Window win, const char* str)
{
#ifdef HAVE_WCHAR_H
# if !defined (OS_FREEBSD) || _FreeBSD_version >= 500000
    wchar_t*	ws = rxvt_mbstowcs (str);
    char*	s = rxvt_wcstoutf8 (ws);

    XChangeProperty (r->Xdisplay, win, prop,
	r->h->xa[XA_UTF8_STRING], 8, PropModeReplace,
	(unsigned char*) s, STRLEN (s));

    rxvt_free (s);
    rxvt_free (ws);
# endif /* !defined (OS_FREEBSD) ||... */
#endif	/* HAVE_WCHAR_H */
}
#endif	/* X_HAVE_UTF8_STRING */


/* xterm sequences - title, iconName, color (exptl) */
/* EXTPROTO */
void
rxvt_set_win_title (rxvt_t* r, Window win, const char* str)
{
    XChangeProperty (r->Xdisplay, win, XA_WM_NAME,
	XA_STRING, 8, PropModeReplace,
	(unsigned char*) str, STRLEN (str));
#ifdef X_HAVE_UTF8_STRING
    rxvt_set_utf8_property (r, r->h->xa[XA_NET_WM_NAME],
	r->TermWin.parent, str);
#endif
}


/* EXTPROTO */
void
rxvt_set_term_title (rxvt_t* r, const unsigned char *str)
{
#ifdef SMART_WINDOW_TITLE
    XTextProperty   prop;

    if (XGetWMName (r->Xdisplay, r->TermWin.parent, &prop) == 0)
	SET_NULL(prop.value);
    if (IS_NULL(prop.value) || STRCMP(prop.value, str))
#endif
    {
	rxvt_set_win_title (r, r->TermWin.parent, (char*) str);
    }
}


/* EXTPROTO */
void
rxvt_set_icon_name (rxvt_t* r, const unsigned char *str)
{
    XTextProperty   prop;

#ifdef SMART_WINDOW_TITLE
    if (XGetWMIconName(r->Xdisplay, r->TermWin.parent, &prop) == 0)
	SET_NULL(prop.value);
    if (IS_NULL(prop.value) || STRCMP(prop.value, str))
#endif
    {
	XChangeProperty (r->Xdisplay, r->TermWin.parent,
	    XA_WM_ICON_NAME, XA_STRING, 8, PropModeReplace,
	    (unsigned char*) str, STRLEN (str));
#ifdef X_HAVE_UTF8_STRING
	rxvt_set_utf8_property (r, r->h->xa[XA_NET_WM_ICON_NAME],
	    r->TermWin.parent, (char*) str);
#endif
    }
}


#ifdef XTERM_COLOR_CHANGE
/* EXTPROTO */
void
rxvt_set_window_color(rxvt_t* r, int page, int idx, const char *color)
{
    XColor	    xcol;
    int		    color_set;
    register int    i;


    rxvt_dbgmsg ((DBG_DEBUG, DBG_MAIN, "%s( r, %d, %d, %s), ATAB=%d\n", __func__, page, idx, color, ATAB(r)));

    if (IS_NULL(color) || (char) 0 == *color)
	return;

    /*
     * Set the fg/bg colors from this page, just in case the fg/bg is to be
     * changed.
     */
    rxvt_set_fgbg_colors( r, page );

    color_set = ISSET_PIXCOLOR(r->h, idx);

    /* handle color aliases */
    if( isdigit((int) *color) )
    {
	i = atoi(color);
	if (i >= 8 && i <= 15)		/* bright colors */
	{
	    i -= 8;
# ifndef NO_BRIGHTCOLOR
	    rxvt_copy_color( r, idx, minBrightCOLOR + i );
	    goto Done;
# endif
	}
	if (i >= 0 && i <= 7)		/* normal colors */
	{
	    rxvt_copy_color( r, idx, minCOLOR + i );
	    goto Done;
	}
    }

    if( !rxvt_parse_alloc_color(r, &xcol, color) )
	/* Could not alloc color */
	return;

    /* XStoreColor (r->Xdisplay, XCMAP, XColor*); */

    /*
     * FIXME: should free colors here, but no idea how to do it so instead,
     * so just keep gobbling up the colormap
     */
# if 0 /*{{{*/
    for (i = Color_Black; i <= Color_White; i++)
	if (r->pixColors[idx] == r->pixColors[i])
	    break;

    if (i > Color_White)
    {
	fprintf (stderr, "XFreeColors: r->pixColors [%d] = %lu\n",
		idx, r->pixColors [idx]);
	XFreeColors(r->Xdisplay, XCMAP, (r->pixColors + idx), 1,
		DisplayPlanes(r->Xdisplay, XSCREEN));
    }
# endif /*}}}*/

    rxvt_set_color( r, idx, &xcol );

Done:
    if( idx == Color_bg )
    {
	PVTS( r, page )->p_bg	= r->pixColorsFocus[idx];
#ifdef XFT_SUPPORT
	if( ISSET_OPTION( r, Opt_xft ) )
	    PVTS( r, page )->p_xftbg = r->xftColorsFocus[idx];
#endif

	if( r->TermWin.fade )
	{
	    PVTS( r, page )->p_bgfade	= r->pixColorsUnfocus[idx];
#ifdef XFT_SUPPORT
	    if( ISSET_OPTION( r, Opt_xft ) )
		PVTS( r, page )->p_xftbgfade = r->xftColorsUnfocus[idx];
#endif
	}

	/*
	 * Update the GC / window background if necessary.
	 */
	if( page == ATAB(r) )
	    r->fgbg_tabnum = -1;
    }

    else if ( idx == Color_fg )
    {
	PVTS( r, page )->p_fg	= r->pixColorsFocus[idx];
#ifdef XFT_SUPPORT
	if( ISSET_OPTION( r, Opt_xft ) )
	    PVTS( r, page )->p_xftfg = r->xftColorsFocus[idx];
#endif

	if( r->TermWin.fade )
	{
	    PVTS( r, page )->p_fgfade	= r->pixColorsUnfocus[idx];
#ifdef XFT_SUPPORT
	    if( ISSET_OPTION( r, Opt_xft ) )
		PVTS( r, page )->p_xftfgfade = r->xftColorsUnfocus[idx];
#endif
	}

	if( page == ATAB(r) )
	    /* Force rxvt_set_vt_colors() to update the GC / background. */
	    r->fgbg_tabnum = -1;
    }

    /* handle Color_BD, scrollbar background, etc. */
#if 0 /*{{{*/
    /*
     * Setting the environment will not propogate to an already running child
     * process.
     */
    rxvt_set_colorfgbg(r);
#endif /*}}}*/
    if( idx == Color_pointer )
	rxvt_recolour_cursor(r);

#if defined(TRANSPARENT) || defined(BACKGROUND_IMAGE)
# ifdef TINTING_SUPPORT
    if (idx == Color_tint)
    {
#  ifdef TRANSPARENT
	if (ISSET_OPTION(r, Opt_transparent))
	    /* reset background */
	    rxvt_check_our_parents (r);
	else
#  endif
#  ifdef BACKGROUND_IMAGE
	{   /* reset background */
	    for (i = 0; i <= LTAB(r); i ++)
		rxvt_resize_pixmap (r, i);
	}
#  endif
	{   /* empty body to suppress compile error */	}
    }
# endif	/* TINTING_SUPPORT */
#endif	/* TRANSPARENT || BACKGROUND_IMAGE */

    /*
     * Restore the fg/bg colors from the active tab. Call rxvt_set_vt_colors
     * instead of rxvt_set_fgbg_colors, so that the GC fgbg & window background
     * will be refreshed if necessary.
     */
    rxvt_set_vt_colors( r, ATAB(r) );

    /*
     *  If our palette has changed, the screen must be refreshed.
     */
    XClearArea( r->Xdisplay, AVTS(r)->vt, 0, 0, 0, 0, True );
}

#else
# define rxvt_set_window_color(r, idx,color)	((void)0)
#endif	    /* XTERM_COLOR_CHANGE */


/* EXTPROTO */
void
rxvt_recolour_cursor(rxvt_t *r)
{
    XColor	    xcol[2];

    xcol[0].pixel = r->pixColorsFocus[Color_pointer];
    xcol[1].pixel = VTBG(r, 0);
    XQueryColors(r->Xdisplay, XCMAP, xcol, 2);
    XRecolorCursor(r->Xdisplay, r->term_pointer, &(xcol[0]), &(xcol[1]));

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MAIN, "%s(r): fg=%06lx, bg=%06lx\n", __func__, xcol[0].pixel, xcol[1].pixel));
}


/*----------------------------------------------------------------------*/
/*
 * find if fg/bg matches any of the normal (low-intensity) colors
 */
/* INTPROTO */
void
rxvt_set_colorfgbg(rxvt_t *r)
{
    unsigned int    i;
    const char	 *xpmb = "\0";
    char	    fstr[sizeof("default") + 1], bstr[sizeof("default") + 1];

    r->h->env_colorfgbg = rxvt_malloc(sizeof("COLORFGBG=default;default;bg")
			+ 1);
    STRCPY(fstr, "default");
    STRCPY(bstr, "default");
    for (i = Color_Black; i <= Color_White; i++)
	if (r->pixColorsFocus[Color_fg] == r->pixColorsFocus[i])
	{
	    sprintf(fstr, "%d", (i - Color_Black));
	    break;
	}
    for (i = Color_Black; i <= Color_White; i++)
	if (r->pixColorsFocus[Color_bg] == r->pixColorsFocus[i])
	{
	    sprintf(bstr, "%d", (i - Color_Black));
#ifdef BACKGROUND_IMAGE
	    xpmb = "default;";
#endif
	    break;
	}
    sprintf(r->h->env_colorfgbg, "COLORFGBG=%s;%s%s", fstr, xpmb, bstr);
    putenv(r->h->env_colorfgbg);

#ifndef NO_BRIGHTCOLOR
    r->h->colorfgbg = DEFAULT_RSTYLE;
    for (i = minCOLOR; i <= maxCOLOR; i++)
    {
	if (
		r->pixColorsFocus[Color_fg] == r->pixColorsFocus[i]
# ifndef NO_BOLD_UNDERLINE_REVERSE
		&& r->pixColorsFocus[Color_fg] == r->pixColorsFocus[Color_BD]
# endif
		/* if we wanted boldFont to have precedence */
# if 0	/* was ifndef NO_BOLDFONT */
		&& IS_NULL(r->TermWin.bfont)
# endif	
	   )
	    r->h->colorfgbg = SET_FGCOLOR(r->h->colorfgbg, i);
	if (r->pixColorsFocus[Color_bg] == r->pixColorsFocus[i])
	    r->h->colorfgbg = SET_BGCOLOR(r->h->colorfgbg, i);
    }
#endif	/* NO_BRIGHTCOLOR */
}

/*----------------------------------------------------------------------*/
/*
 * Colour determination for low colour displays, routine from
 *   Hans de Goede <hans@highrise.nl>
 */


#ifdef XFT_SUPPORT
/* EXTPROTO */
int
rxvt_alloc_xft_color( rxvt_t* r, const XColor *xcol, XftColor* xftcolor )
{
    /*
     * xcol is an already allocated color, so we don't need a round trip to the
     * server.
     */

    xftcolor->color.red	    = xcol->red;
    xftcolor->color.green   = xcol->green;
    xftcolor->color.blue    = xcol->blue;
    xftcolor->color.alpha   = 0xffff;

    xftcolor->pixel	    = xcol->pixel;

    return 1;
#if 0
    XRenderColor    render; 

    assert (xftcolor);

    render.red   = xcol->red;
    render.green = xcol->green;
    render.blue  = xcol->blue;
    render.alpha = 0xffff; 

    return XftColorAllocValue(r->Xdisplay, XVISUAL, XCMAP, &render, xftcolor);
#endif
}
#endif  /* XFT_SUPPORT */


/* EXTPROTO */
int
rxvt_parse_alloc_color(rxvt_t* r, XColor *screen_in_out, const char *colour)
{
    int		    res = 0;

    if (!XParseColor(r->Xdisplay, XCMAP, colour, screen_in_out))
	rxvt_msg (DBG_ERROR, DBG_MAIN, "can't determine colour: %s", colour);
    else
	res = rxvt_alloc_color(r, screen_in_out, colour);
    return res;
}


/* EXTPROTO */
int
rxvt_alloc_color( rxvt_t* r, XColor *screen_in_out, const char *colour )
{
    return XAllocColor( r->Xdisplay, XCMAP, screen_in_out );

#if 0 /* 2006-05-27 gi1242: Really bad code. */
    int		    res;

    if( (res = XAllocColor(r->Xdisplay, XCMAP, screen_in_out)) )
	return res;

    /* try again with closest match */
    /*
     * XXX 2006-05-25 gi1242: This is really inefficient. There must be a better
     * way!
     */
    if (XDEPTH >= 4 && XDEPTH <= 8)
    {
	int		i, numcol;
	int		best_pixel = 0;
	unsigned long   best_diff, diff;
	XColor		*colors;

#define rSQR(x)	    ((x)*(x))

	best_diff = 0;
	numcol = 0x01 << XDEPTH;
	if( (colors = rxvt_malloc(numcol * sizeof(XColor))) )
	{
	    for (i = 0; i < numcol; i++)
	    colors[i].pixel = i;

	    XQueryColors(r->Xdisplay, XCMAP, colors, numcol);
	    for (i = 0; i < numcol; i++)
	    {
		diff = rSQR(screen_in_out->red - colors[i].red)
		    + rSQR(screen_in_out->green - colors[i].green)
		    + rSQR(screen_in_out->blue - colors[i].blue);
		if (i == 0 || diff < best_diff)
		{
		    best_pixel = colors[i].pixel;
		    best_diff = diff;
		}
	    }
	    *screen_in_out = colors[best_pixel];
	    rxvt_free(colors);

	    res = XAllocColor(r->Xdisplay, XCMAP, screen_in_out);
	}
    }

    if (res == 0)
	rxvt_msg (DBG_ERROR, DBG_MAIN, "can't allocate color: %s", colour);

    return res;
#endif
}



/* -------------------------------------------------------------------- *
 * -			    X INPUT METHOD ROUTINES			- *
 * -------------------------------------------------------------------- */
#ifdef USE_XIM
/* INTPROTO */
void
rxvt_IM_set_size(rxvt_t* r, XRectangle *size)
{
    XWindowAttributes	xwa;

    XGetWindowAttributes(r->Xdisplay, AVTS(r)->vt, &xwa);
    size->x = xwa.x + r->TermWin.int_bwidth;
    size->y = xwa.y + r->TermWin.int_bwidth;
    size->width = Width2Pixel(r->TermWin.ncol);
    size->height = Row2Pixel(r->TermWin.nrow);
}

/* INTPROTO */
void
rxvt_IM_set_position(rxvt_t* r, XPoint *pos)
{
    XWindowAttributes	xwa;

    XGetWindowAttributes(r->Xdisplay, AVTS(r)->vt, &xwa);
    pos->x = xwa.x + Col2Pixel(ASCR(r).cur.col);
    pos->y = xwa.y + Row2Pixel(ASCR(r).cur.row) + r->TermWin.font->ascent;
}

/* INTPROTO */
void
rxvt_IM_set_color(rxvt_t* r, unsigned long *fg, unsigned long *bg)
{
    *fg = r->pixColorsFocus[Color_fg];
    *bg = r->pixColorsFocus[Color_bg];
}

/* Checking whether input method is running. */
/* INTPROTO */
Bool
rxvt_IM_is_running(rxvt_t *r)
{
    char		*p;
    Atom	    atom;
    Window	    win;
    char	    server[IMBUFSIZ];

    /* get current locale modifier */
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "rxvt_IM_is_running ()\n"));
    if (NOT_NULL(p = XSetLocaleModifiers(NULL)))
    {
	STRCPY(server, "@server=");
	STRNCAT(server, &(p[4]), IMBUFSIZ - 9);	/* skip "@im=" */
	if (NOT_NULL(p = STRCHR(server + 1, '@')))  /* first one only */
	    *p = '\0';

	atom = XInternAtom(r->Xdisplay, server, False);
	win = XGetSelectionOwner (r->Xdisplay, atom);
	if (IS_WIN(win))
	    return True;
    }
    return False;
}


/* EXTPROTO */
void
rxvt_IM_send_spot (rxvt_t *r)
{
    XPoint	    spot;
    XVaNestedList   preedit_attr;

    if (IS_NULL(r->h->Input_Context) ||
	!r->TermWin.focus ||
	!(r->h->input_style & XIMPreeditPosition) ||
	!(r->h->event_type == KeyPress ||
	r->h->event_type == Expose ||
	r->h->event_type == NoExpose ||
	r->h->event_type == SelectionNotify ||
	r->h->event_type == ButtonRelease ||
	r->h->event_type == FocusIn) ||
	!rxvt_IM_is_running(r))
	return;

    rxvt_IM_set_position(r, &spot);

    preedit_attr = XVaCreateNestedList(0, XNSpotLocation, &spot, NULL);
    XSetICValues(r->h->Input_Context, XNPreeditAttributes, preedit_attr, NULL);
    XFree(preedit_attr);
}


/* EXTPROTO */
void
rxvt_IM_set_fontset (rxvt_t* r, int idx)
{
    char		*string;
    size_t	    length;
    XFontSet	    prev_fontset;
    int		    success = 0;

    if (idx < 0 || idx >= MAX_NFONTS)
	return;
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "rxvt_setTermFontSet()\n"));
    prev_fontset = r->TermWin.fontset;
    SET_NULL(r->TermWin.fontset);

    length = 0;
    if (r->h->rs[Rs_font + idx])
	length += STRLEN(r->h->rs[Rs_font + idx]) + 1;
# ifdef MULTICHAR_SET
    if (r->h->rs[Rs_mfont + idx])
	length += STRLEN(r->h->rs[Rs_mfont + idx]) + 1;
# endif
    /* possible integer overflow? */
    assert (length >= 0 && length+1 > 0);
    if (length == 0 || IS_NULL(string = rxvt_malloc(length + 1)))
	SET_NULL(r->TermWin.fontset);
    else
    {
	int		missing_charsetcount;
	char		**missing_charsetlist, *def_string;

	string[0] = '\0';
	if (r->h->rs[Rs_font + idx])
	{
	    STRCAT(string, r->h->rs[Rs_font + idx]);
	    STRCAT(string, ",");
	}
# ifdef MULTICHAR_SET
	if (r->h->rs[Rs_mfont + idx])
	{
	    STRCAT(string, r->h->rs[Rs_mfont + idx]);
	    STRCAT(string, ",");
	}
# endif
	string[STRLEN(string) - 1] = '\0';
	r->TermWin.fontset = XCreateFontSet(r->Xdisplay, string,
			&missing_charsetlist,
			&missing_charsetcount,
			&def_string);
	rxvt_free(string);
	if (NOT_NULL(r->TermWin.fontset))
	    success = 1;
    }

    if (success)
    {
	if (NOT_NULL(prev_fontset))
		XFreeFontSet(r->Xdisplay, prev_fontset);
    }
    else
	r->TermWin.fontset = prev_fontset;
}

/* INTPROTO */
void
rxvt_IM_change_fontset(rxvt_t* r, int idx)
{
    XVaNestedList   preedit_attr, status_attr;

    rxvt_IM_set_fontset(r, idx);

    preedit_attr = XVaCreateNestedList(0,
		    XNFontSet, r->TermWin.fontset,
		    XNLineSpace, r->TermWin.fheight,
		    NULL);
    status_attr = XVaCreateNestedList(0,
		    XNFontSet, r->TermWin.fontset,
		    NULL);
    XSetICValues(r->h->Input_Context,
	    XNPreeditAttributes, preedit_attr,
	    XNStatusAttributes, status_attr,
	    NULL);

    XFree(preedit_attr);
    XFree(status_attr);
}

/* INTPROTO */
void
rxvt_IM_set_preedit_area(rxvt_t* r, XRectangle *preedit_rect, XRectangle *status_rect, XRectangle *needed_rect)
{
    XWindowAttributes	xwa;

    XGetWindowAttributes(r->Xdisplay, AVTS(r)->vt, &xwa);

    preedit_rect->x = xwa.x + r->TermWin.int_bwidth + needed_rect->width;
    preedit_rect->y = xwa.y + Row2Pixel(r->TermWin.nrow - 1);
    preedit_rect->width = Width2Pixel(r->TermWin.ncol) - needed_rect->width;
    preedit_rect->height = Height2Pixel(1);

    status_rect->x = xwa.x + r->TermWin.int_bwidth;
    status_rect->y = xwa.y + Row2Pixel(r->TermWin.nrow - 1);
    status_rect->width = needed_rect->width ? needed_rect->width : Width2Pixel(r->TermWin.ncol);
    status_rect->height = Height2Pixel(1);
}

/* ARGSUSED */
/* INTPROTO */
void
rxvt_IM_destroy_callback(XIM xim __attribute__((unused)), XPointer client_data __attribute__((unused)), XPointer call_data __attribute__((unused)))
{
    rxvt_t	    *r = rxvt_get_r();

    SET_NULL(r->h->Input_Context);
    /* To avoid Segmentation Fault in C locale: Solaris only? */
    if (STRCMP(r->h->locale, "C"))
	XRegisterIMInstantiateCallback(r->Xdisplay, NULL, NULL, NULL,
	    rxvt_IM_init_callback, NULL);
}

/*
 * X manual pages and include files don't match on some systems:
 * some think this is an XIDProc and others an XIMProc so we can't
 * use the first argument - need to update this to be nice for
 * both types via some sort of configure detection
 */
/* ARGSUSED */
/* EXTPROTO */
void
rxvt_IM_init_callback (Display *unused __attribute__((unused)), XPointer client_data __attribute__((unused)), XPointer call_data __attribute__((unused)))
{
    int		    i, found, had_im;
    const char	 *p;
    char	    **s;
    rxvt_t	    *r = rxvt_get_r();
    char	    buf[IMBUFSIZ];

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "rxvt_IM_init_callback()\n"));
    if (r->h->Input_Context)
	return;

    found = had_im = 0;
    p = r->h->rs[Rs_inputMethod];
    if (p && *p)
    {
	had_im = 1;
	s = rxvt_splitcommastring(p);
	for (i = 0; s[i]; i++)
	{
	    if (*s[i])
	    {
		STRCPY(buf, "@im=");
		STRNCAT(buf, s[i], IMBUFSIZ - 5);
		if (NOT_NULL(p = XSetLocaleModifiers(buf)) &&
		    *p &&
		    (rxvt_IM_get_IC(r) == True)
		   )
		{
		    found = 1;
		    break;
		}
	    }
	}
	for (i = 0; s[i]; i++)
	    rxvt_free(s[i]);
	rxvt_free(s);
    }
    if (found)
	return;

    /* try with XMODIFIERS env. var. */
    if (NOT_NULL(p = XSetLocaleModifiers("")) && *p)
    {
	rxvt_IM_get_IC(r);
	return;
    }

    /* try with no modifiers base IF the user didn't specify an IM */
    if (!had_im &&
	NOT_NULL(p = XSetLocaleModifiers("@im=none")) &&
	*p &&
	rxvt_IM_get_IC(r) == True)
	return;
}

/*
 * Try to open a XIM with the current modifiers, then see if we can
 * open a suitable preedit type
 */
/* INTPROTO */
Bool
rxvt_IM_get_IC(rxvt_t *r)
{
    int		    i, j, found;
    XIM		    xim;
    XPoint	    spot;
    XRectangle	    rect, status_rect, needed_rect;
    unsigned long   fg, bg;
    const char	 *p;
    char	    **s;
    XIMStyles	    *xim_styles;
    XVaNestedList   preedit_attr, status_attr;
    XIMCallback	 ximcallback;
    struct rxvt_hidden *h = r->h;
    long	    im_event_mask = 0;

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MAIN, "rxvt_IM_get_IC()\n"));
    xim = XOpenIM(r->Xdisplay, NULL, NULL, NULL);
    if (IS_NULL(xim))
    {
	rxvt_msg (DBG_WARN, DBG_MAIN, "Unalbe to open IM\n");
	return False;
    }

    SET_NULL(xim_styles);
    if (
	    XGetIMValues(xim, XNQueryInputStyle, &xim_styles, NULL)
	    || !xim_styles || !xim_styles->count_styles
       )
    {
	XCloseIM(xim);
	return False;
    }

    p = h->rs[Rs_preeditType] ? h->rs[Rs_preeditType]
		    : "OverTheSpot,OffTheSpot,Root";
    s = rxvt_splitcommastring(p);
    for (i = found = 0; !found && s[i]; i++)
    {
	if (!STRCMP(s[i], "OverTheSpot"))
	    h->input_style = (XIMPreeditPosition | XIMStatusNothing);
	else if (!STRCMP(s[i], "OffTheSpot"))
	    h->input_style = (XIMPreeditArea | XIMStatusArea);
	else if (!STRCMP(s[i], "Root"))
	    h->input_style = (XIMPreeditNothing | XIMStatusNothing);

	for (j = 0; j < xim_styles->count_styles; j++)
	    if (h->input_style == xim_styles->supported_styles[j])
	    {
		found = 1;
		break;
	    }
    }
    for (i = 0; s[i]; i++)
	rxvt_free(s[i]);
    rxvt_free(s);
    XFree(xim_styles);

    if (!found)
    {
	XCloseIM(xim);
	return False;
    }

    ximcallback.callback = rxvt_IM_destroy_callback;

    /* XXX: not sure why we need this (as well as IC one below) */
    XSetIMValues(xim, XNDestroyCallback, &ximcallback, NULL);

    SET_NULL(preedit_attr);
    SET_NULL(status_attr);

    if (h->input_style & XIMPreeditPosition)
    {
	rxvt_IM_set_size(r, &rect);
	rxvt_IM_set_position(r, &spot);
	rxvt_IM_set_color(r, &fg, &bg);

	preedit_attr = XVaCreateNestedList(0, XNArea, &rect,
			    XNSpotLocation, &spot,
			    XNForeground, fg,
			    XNBackground, bg,
			    XNFontSet, r->TermWin.fontset,
			    XNLineSpace, r->TermWin.fheight,
			    NULL);
    }
    else if (h->input_style & XIMPreeditArea)
    {
	rxvt_IM_set_color(r, &fg, &bg);

	/*
	 * The necessary width of preedit area is unknown
	 * until create input context.
	 */
	needed_rect.width = 0;

	rxvt_IM_set_preedit_area(r, &rect, &status_rect, &needed_rect);

	preedit_attr = XVaCreateNestedList(0, XNArea, &rect,
			    XNForeground, fg,
			    XNBackground, bg,
			    XNFontSet, r->TermWin.fontset,
			    NULL);
	status_attr = XVaCreateNestedList(0, XNArea, &status_rect,
			XNForeground, fg,
			XNBackground, bg,
			XNFontSet, r->TermWin.fontset, NULL);
    }
    h->Input_Context = XCreateIC(xim, XNInputStyle, h->input_style,
		    XNClientWindow, r->TermWin.parent,
		    XNFocusWindow, r->TermWin.parent,
		    XNDestroyCallback, &ximcallback,
		    preedit_attr ? XNPreeditAttributes : NULL,
		    preedit_attr,
		    status_attr ? XNStatusAttributes : NULL,
		    status_attr, NULL);
    if (preedit_attr)
	XFree(preedit_attr);
    if (status_attr)
	XFree(status_attr);
    if (IS_NULL(h->Input_Context))
    {
	rxvt_msg (DBG_ERROR, DBG_MAIN, "failed to create input context");
	XCloseIM(xim);
	return False;
    }
    if (h->input_style & XIMPreeditArea)
	rxvt_IM_set_status_pos (r);
    
    /* this is necessary to avoid crash after XIM quits */
    XGetICValues(h->Input_Context, XNFilterEvents, &im_event_mask, NULL);
    im_event_mask |= (KeyPressMask | StructureNotifyMask | ExposureMask
#if defined(MOUSE_WHEEL) && defined(MOUSE_SLIP_WHEELING)
	| KeyReleaseMask
#endif
#ifdef MONITOR_ENTER_LEAVE
	| EnterWindowMask | LeaveWindowMask
#endif
	| FocusChangeMask | VisibilityChangeMask);
    XSelectInput(r->Xdisplay, r->TermWin.parent, im_event_mask);


    rxvt_msg (DBG_INFO, DBG_MAIN, "rxvt_IM_get_IC() - successful connection\n");
    return True;
}

/* INTPROTO */
void
rxvt_IM_send_size(rxvt_t* r)
{
    XRectangle	size;
    XVaNestedList   preedit_attr;

    rxvt_IM_set_size(r, &size);

    preedit_attr = XVaCreateNestedList(0, XNArea, &size, NULL);
    XSetICValues(r->h->Input_Context, XNPreeditAttributes, preedit_attr, NULL);
    XFree(preedit_attr);
}

/* INTPROTO */
void
rxvt_IM_set_status_pos (rxvt_t *r)
{
    XRectangle	    preedit_rect, status_rect, *needed_rect;
    XVaNestedList   preedit_attr, status_attr;

    /* Getting the necessary width of preedit area */
    status_attr = XVaCreateNestedList(0, XNAreaNeeded, &needed_rect, NULL);
    XGetICValues(r->h->Input_Context, XNStatusAttributes, status_attr, NULL);
    XFree(status_attr);

    rxvt_IM_set_preedit_area(r, &preedit_rect, &status_rect, needed_rect);

    preedit_attr = XVaCreateNestedList(0, XNArea, &preedit_rect, NULL);
    status_attr = XVaCreateNestedList(0, XNArea, &status_rect, NULL);

    XSetICValues(r->h->Input_Context,
	    XNPreeditAttributes, preedit_attr,
	    XNStatusAttributes, status_attr, NULL);

    XFree(preedit_attr);
    XFree(status_attr);
}

/* EXTPROTO */
void
rxvt_IM_resize(rxvt_t *r)
{
    if (IS_NULL(r->h->Input_Context) || !rxvt_IM_is_running(r))
	return;

    if (r->h->input_style & XIMPreeditPosition)
	rxvt_IM_send_size(r);
    else if (r->h->input_style & XIMPreeditArea)
	rxvt_IM_set_status_pos(r);
}
#endif		    /* USE_XIM */

/*----------------------------------------------------------------------*/
static rxvt_t  *_rxvt_vars = NULL;

/* EXTPROTO */
rxvt_t*
rxvt_get_r(void)
{
    return _rxvt_vars;
}
/* INTPROTO */
void
rxvt_set_r(rxvt_t *r)
{
    _rxvt_vars = r;
}

/*----------------------- end-of-file (C source) -----------------------*/
