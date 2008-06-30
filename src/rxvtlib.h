/*--------------------------------*-H-*---------------------------------*
 * File:    rxvtlib.h
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1997-2001   Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2004        Terry Griffin <griffint@pobox.com>
 * Copyright (c) 2005        Teun Burgers <burgers@ecn.nl>
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

#ifndef __RXVTLIB_H__
#define __RXVTLIB_H__

/*
 * Boolean variables
 */
#ifdef __GNUC__
# define BOOLVAR(x, sz)	x:sz
#else
# define BOOLVAR(x, sz) x
#endif


/* type of unicode_t */
typedef uint32_t	unicode_t;


/*****************************************************************************
 *                                 SECTION 2                                 *
 *                      DO NOT TOUCH ANYTHING BELOW HERE                     *
 *****************************************************************************/

struct rxvt_vars;	/* defined later on */
struct rxvt_hidden;	/* not defined here */

typedef struct {
    int32_t         row;
    int32_t         col;
} row_col_t;

typedef unsigned char text_t;
#if defined(TTY_256COLOR) || defined(MULTICHAR_SET)
# define rend_t	    uint32_t
#else
# define rend_t	    uint16_t
#endif

/* Size of the FIFO buffer */
#define FIFO_BUF_SIZE (512)

/*
 * TermWin elements limits
 *  ncol      : 1 <= ncol       <= MAX(int16_t)
 *  nrow      : 1 <= nrow       <= MAX(int16_t)
 *  saveLines : 0 <= saveLines  <= MAX(int16_t)
 *  nscrolled : 0 <= nscrolled  <= saveLines
 *  view_start: 0 <= view_start <= nscrolled
 */

typedef struct {
    uint16_t	    fwidth,	/* font width  [pixels] */
		    fheight;	/* font height [pixels] */
#ifdef XFT_SUPPORT
    uint16_t	    pwidth,	/* propotionally spaced font width / height */
		    pheight;
#endif
    uint16_t	    propfont;	/* font proportional flags */
    uint16_t	    ncol;	/* window columns [characters] */
    uint16_t	    nrow;	/* window rows [characters] */
    uint16_t	    mapped; 	/* TermWin is mapped? */
    uint16_t	    int_bwidth; /* internal border width */
    uint16_t	    ext_bwidth; /* external border width */

    uint16_t	    maxTabWidth,    /* max width of tab title to display */
		    minVisibleTabs; /* Minimum number of tabs to try and keep
				       visible */
#ifndef NO_LINESPACE
    uint16_t	    lineSpace;	/* space between rows */
#endif

    char	    BOOLVAR(focus,1);	/* window is focused? */
    char	    enter;  /* pointer is in window? */

    Window	    parent; /* parent window */
    GC		    gc;	    /* GC for drawing text */
    XFontStruct*    font;   /* main font structure */
#ifndef NO_BOLDFONT
    XFontStruct*    bfont;  /* bold font */
#endif
#ifdef MULTICHAR_SET
    XFontStruct*    mfont;  /* Multichar font structure */
#endif
#ifdef USE_XIM
    XFontSet	    fontset;
#endif
#ifdef XFT_SUPPORT
    XftPattern*	    xftpattern;
    XftFont	    *xftfont, *xftpfont, *xftPfont;
# ifdef MULTICHAR_SET
    XftPattern*	    xftmpattern;
    XftFont*	    xftmfont;
    int		    xftmsize;
#  ifdef HAVE_ICONV_H
    iconv_t	    xfticonv;
#  endif
# endif
# ifndef NO_BOLDFONT
    XftFont*	    xftbfont;
# endif
    char	    xftfnmono;	/* font is mono? */
    char	    xftmono;	/* font and mfont are mono? */
    int		    xftsize, xftpsize;
#endif	/* XFT_SUPPORT */

# define PARENT_NUMBER	    (6)
    int		    opacity;	/* transluscent window opaque degree */
    int		    opacity_degree; /* opaque change degree */

#ifdef TRANSPARENT
    Window	    parenttree[PARENT_NUMBER];
#endif

#ifdef TINTING_SUPPORT
    int		    shade;  /* tinting shade percentage */
#endif

    int		    fade;   /* off-focus fading percentage */

#ifdef TEXT_SHADOW
    enum {
	SHADOW_NONE = 0,
	SHADOW_LEFT,
	SHADOW_RIGHT,
	SHADOW_TOP,
	SHADOW_BOTTOM,
	SHADOW_TOPLEFT,
	SHADOW_BOTRIGHT,
	SHADOW_TOPRIGHT,
	SHADOW_BOTLEFT,
    } shadow_mode;
    unsigned long   shadow; /* text shadow color */
# ifdef XFT_SUPPORT
    XftColor	    xftshadow;
# endif
#endif

#ifdef HAVE_X11_SM_SMLIB_H
    SmcConn	    sm_conn;
    IceConn	    ice_conn;
    int		    ice_fd;
    char*	    sm_client_id;
#endif

#ifndef NO_BELL
    unsigned long   vBellDuration;
#endif
} TermWin_t;



/*
 * screen accounting:
 * screen_t elements
 *   text:      Contains all text information including the scrollback buffer.
 *              Each line is length TermWin.ncol
 *   tlen:      The length of the line or -1 for wrapped lines.
 *   rend:      Contains rendition information: font, bold, colour, etc.
 *   
 *   NOTE: Each line for both text and rend are only allocated on demand, and
 *         text[x] is allocated <=> rend[x] is allocated  for all x.
 *
 *   row:       Cursor row position                   : 0 <= row < TermWin.nrow
 *   col:       Cursor column position                : 0 <= col < TermWin.ncol
 *   tscroll:   Scrolling region top row inclusive    : 0 <= row < TermWin.nrow
 *   bscroll:   Scrolling region bottom row inclusive : 0 <= row < TermWin.nrow
 *
 * selection_t elements
 *   clicks:    1, 2 or 3 clicks - 4 indicates a special condition of 1 where
 *              nothing is selected
 *   beg:       row/column of beginning of selection  : never past mark
 *   mark:      row/column of initial click           : never past end
 *   end:       row/column of one character past end of selection
 * * Note: -TermWin.nscrolled <= beg.row <= mark.row <= end.row < TermWin.nrow
 * * Note: col == -1 ==> we're left of screen
 *
 * Layout of text/rend information in the screen_t text/rend structures:
 *   Rows [0] ... [TermWin.saveLines - 1]
 *     scrollback region : we're only here if TermWin.view_start != 0
 *   Rows [TermWin.saveLines] ... [TermWin.saveLines + TermWin.nrow - 1]
 *     normal `unscrolled' screen region
 */
typedef struct {
    text_t**	    text;	/* _all_ the text */
    int16_t*	    tlen;	/* length of each text line */
    rend_t**	    rend;	/* rendition, uses RS_ flags */
    row_col_t       cur;	/* cursor position on the screen */
    uint16_t	    tscroll;	/* top of settable scroll region */
    uint16_t	    bscroll;	/* bottom of settable scroll region */
    uint16_t	    charset;	/* character set number [0..3] */
    unsigned int    flags;	/* see below */
    row_col_t	    s_cur;	/* saved cursor position */
    uint16_t	    s_charset;	/* saved character set number [0..3] */
    char	    s_charset_char;
    rend_t	    s_rstyle;	/* saved rendition style */
} screen_t;


typedef struct {
    unsigned char*  text;   /* selected text */
    uint32_t	    len;    /* length of selected text */
    enum {
	SELECTION_CLEAR = 0,/* nothing selected */
	SELECTION_INIT,	    /* marked a point */
	SELECTION_BEGIN,    /* started a selection */
	SELECTION_CONT,	    /* continued selection */
	SELECTION_DONE	    /* selection put in CUT_BUFFER0 */
    } op;		    /* current operation */
    short	    vt;	    /* VT that has the selection */
    short           screen; /* screen being used */
    short           clicks; /* number of clicks */
    row_col_t       beg;    /* beginning of selection <= mark */
    row_col_t       mark;   /* point of initial click <= end */
    row_col_t       end;    /* one character past end point */
} selection_t;

typedef enum {
    OLD_SELECT, OLD_WORD_SELECT, NEW_SELECT
} sstyle_t;

/* ------------------------------------------------------------------------- */

/* screen_t flags */
#define Screen_Relative		(1<<0)	/* relative origin mode flag */
#define Screen_VisibleCursor	(1<<1)	/* cursor visible? */
#define Screen_Autowrap		(1<<2)	/* auto-wrap flag */
#define Screen_Insert		(1<<3)	/* insert mode (vs. overstrike) */
#define Screen_WrapNext		(1<<4)	/* need to wrap for next char? */
#define Screen_DefaultFlags	(Screen_VisibleCursor|Screen_Autowrap)


#define IS_OPTION1		    (0x00000000)
#define IS_OPTION2		    (0x00000001)
#define IS_OPTION3		    (0x00000002)
#define IS_OPTION4		    (0x00000003)
#define OPTION_MASK		    (0x00000003)
#define MAX_OPTION_ARRAY	    (4)

/* rxvt_vars.Options */
/*
 * Bits corresponding to 1<<2 through 1<<30 can be used for option flags. 1<<31
 * is reserved for "reverse" options (which I think is obsolete), and the least
 * two significant bits are used to get the array index of r->Options.
 */
#define	Opt_console		    ((1LU<<2) | IS_OPTION1)
#define Opt_loginShell		    ((1LU<<3) | IS_OPTION1)
#define Opt_iconic		    ((1LU<<4) | IS_OPTION1)
#define Opt_visualBell		    ((1LU<<5) | IS_OPTION1)
#define Opt_currentTabVBell	    ((1LU<<6) | IS_OPTION1)
#define Opt_mapAlert		    ((1LU<<7) | IS_OPTION1)
#define Opt_reverseVideo	    ((1LU<<8) | IS_OPTION1)
#define Opt_utmpInhibit		    ((1LU<<9) | IS_OPTION1)
#define Opt_scrollBar		    ((1LU<<10) | IS_OPTION1)
#define Opt_scrollBar_right	    ((1LU<<11) | IS_OPTION1)
#define Opt_scrollBar_floating	    ((1LU<<12) | IS_OPTION1)
#define Opt_meta8		    ((1LU<<13) | IS_OPTION1)
#define Opt_scrollTtyOutputInhibit  ((1LU<<14) | IS_OPTION1)
#define Opt_scrollTtyKeypress	    ((1LU<<15) | IS_OPTION1)
#define Opt_transparent		    ((1LU<<16) | IS_OPTION1)
#define Opt_forceTransparent	    ((1LU<<17) | IS_OPTION1)
#define Opt_mc_hack		    ((1LU<<18) | IS_OPTION1)
#define Opt_tripleclickwords	    ((1LU<<19) | IS_OPTION1)
#define Opt_mouseWheelScrollPage    ((1LU<<20) | IS_OPTION1)
#define Opt_pointerBlank	    ((1LU<<21) | IS_OPTION1)
#define Opt_cursorBlink		    ((1LU<<22) | IS_OPTION1)
#ifdef HAVE_SCROLLBARS
# define Opt_transparent_scrollbar  ((1LU<<23) | IS_OPTION1)
#endif
#ifdef HAVE_MENUBAR
# define Opt_transparent_menubar    ((1LU<<24) | IS_OPTION1)
# define Opt_showMenu		    ((1LU<<25) | IS_OPTION1)
#endif
#define Opt_transparent_tabbar	    ((1LU<<26) | IS_OPTION1)
#define Opt_tabPixmap		    ((1LU<<27) | IS_OPTION1)
#ifdef XFT_SUPPORT
# define Opt_xft		    ((1LU<<28) | IS_OPTION1)
#endif
#ifdef USE_FIFO
# define Opt_useFifo		    ((1LU<<29) | IS_OPTION1)
#endif/*USE_FIFO*/
#define DEFAULT_OPTIONS	    \
    (Opt_scrollBar)

/* rxvt_vars.Options2 */
#define Opt2_protectSecondary	    ((1LU<<2) | IS_OPTION2)
#define Opt2_cmdAllTabs		    ((1LU<<3) | IS_OPTION2)
#ifdef XFT_SUPPORT
# ifdef MULTICHAR_SET
#  define Opt2_xftNomFont	    ((1LU<<4) | IS_OPTION2)
#  define Opt2_xftSlowOutput	    ((1LU<<5) | IS_OPTION2)
# endif
# define Opt2_xftAntialias	    ((1LU<<6) | IS_OPTION2)
# define Opt2_xftHinting	    ((1LU<<7) | IS_OPTION2)
# define Opt2_xftAutoHint	    ((1LU<<8) | IS_OPTION2)
# define Opt2_xftGlobalAdvance	    ((1LU<<9) | IS_OPTION2)
#endif
#define Opt2_syncTabTitle	    ((1LU<<10) | IS_OPTION2)
#define Opt2_syncTabIcon	    ((1LU<<11) | IS_OPTION2)
#define Opt2_hideTabbar		    ((1LU<<12) | IS_OPTION2)
#define Opt2_bottomTabbar	    ((1LU<<13) | IS_OPTION2)
#define Opt2_borderLess		    ((1LU<<14) | IS_OPTION2)
#define Opt2_overrideRedirect	    ((1LU<<15) | IS_OPTION2)
#define Opt2_broadcast		    ((1LU<<16) | IS_OPTION2)
#define Opt2_hideButtons	    ((1LU<<17) | IS_OPTION2)
#define Opt2_veryBold		    ((1LU<<18) | IS_OPTION2)
#ifndef NO_BRIGHTCOLOR
# define Opt2_boldColors	    ((1LU<<19) | IS_OPTION2)
# define Opt_veryBright		    ((1LU<<20) | IS_OPTION2)
#endif
#define Opt2_noSysConfig	    ((1LU<<21) | IS_OPTION2)
#define Opt2_disableMacros	    ((1LU<<22) | IS_OPTION2)
#ifdef HAVE_X11_SM_SMLIB_H
# define Opt2_enableSessionMgt	    ((1LU<<23) | IS_OPTION2)
#endif
#define Opt2_linuxHomeEndKey	    ((1LU<<24) | IS_OPTION2)
#define Opt2_hlTabOnBell	    ((1LU<<25) | IS_OPTION2)
#define Opt2_smoothResize	    ((1LU<<26) | IS_OPTION2)
#define Opt2_smartResize	    ((1LU<<27) | IS_OPTION2)
#define Opt2_autohideTabbar	    ((1LU<<28) | IS_OPTION2)
#define Opt2_maximized		    ((1LU<<29) | IS_OPTION2)
#define Opt2_fullscreen		    ((1LU<<30) | IS_OPTION2)


#ifdef XFT_SUPPORT
# define DEFAULT_OPTIONS2   \
    (Opt2_veryBold | Opt2_smartResize | Opt2_xftAntialias)
#else
# define DEFAULT_OPTIONS2   \
    (Opt2_veryBold | Opt2_smartResize)
#endif

#define DEFAULT_OPTIONS3    \
    (IS_OPTION3)
#define DEFAULT_OPTIONS4    \
    (IS_OPTION4)

/* place holder used for parsing command-line options */
#define Opt_Reverse	    (1LU<<31)

/* Macros to manipulate options (given an option array) */
#define ISSET_ARRAYOPT( array, option )	    \
    ( (array)[ (option) & OPTION_MASK ] & ( (option) & ~OPTION_MASK ) )

#define NOTSET_ARRAYOPT( array, option )    \
    !ISSET_ARRAYOPT( array, option )

#define SET_ARRAYOPT( array, option )	    \
    ( (array)[ (option) & OPTION_MASK ] |= ( (option) & ~OPTION_MASK ) )

#define UNSET_ARRAYOPT( array, option )	    \
    ( (array)[ (option) & OPTION_MASK ] &= ~( (option) & ~OPTION_MASK ) )

#define TOGGLE_ARRAYOPT( array, option )	    \
    ( (array)[ (option) & OPTION_MASK ] ^= ( (option) & ~OPTION_MASK ) )

/* Macros to manipulate standard options */
#define ISSET_OPTION(R, OPT)	\
    ISSET_ARRAYOPT( (R)->Options, (OPT) )

#define NOTSET_OPTION(R, OPT)	\
    !ISSET_OPTION( (R), (OPT) )

#define SET_OPTION(R, OPT)  	\
    SET_ARRAYOPT( (R)->Options, (OPT) )

#define UNSET_OPTION(R, OPT)  	\
    UNSET_ARRAYOPT( (R)->Options, (OPT) )

#define TOGGLE_OPTION(R, OPT)	\
    TOGGLE_ARRAYOPT( (R)->Options, (OPT) )



#define PROPFONT_NORMAL		    (1<<0)
#define PROPFONT_BOLD		    (1<<1)
#define PROPFONT_MULTI		    (1<<2)

/*
 * Weather or not we should use xftPfont / xftpfont while drawing text. This has
 * ABSOLUTELY NOTHING to do with PROPFONT_* or the propfont junk. This is used
 * for drawing tab titles / menubar / etc.
 */
#define NO_PFONT		    (0)
#define USE_PFONT		    (1)
#define USE_BOLD_PFONT		    (2)

/* ------------------------------------------------------------------------- */

#ifdef HAVE_MENUBAR
typedef struct {
    short           state;
    Window          win;
    GC		    gc;
# ifdef BACKGROUND_IMAGE
    Pixmap	    pixmap;
# endif
    unsigned long   fg;
    unsigned long   bg;
    unsigned long   topshadow;
    unsigned long   botshadow;

# ifdef XFT_SUPPORT
    XftDraw	    *xftDraw;
    XftColor	    xftFore;
# endif
} menuBar_t;
#endif


#ifdef HAVE_SCROLLBARS
typedef struct {
    char            state;  /* scrollbar state */
    char            init;   /* scrollbar has been initialised */
    short           beg;    /* slider sub-window begin height */
    short           end;    /* slider sub-window end height */
    short           top;    /* slider top position */
    short           bot;    /* slider bottom position */
    short           style;  /* style: rxvt, xterm, next */
    short           width;  /* scrollbar width */
    Window          win;
    int             (*update)(struct rxvt_vars *, int, int, int, int);

    GC		    gc;
# ifdef BACKGROUND_IMAGE
    Pixmap	    pixmap;
# endif
# ifdef RXVT_SCROLLBAR
    unsigned long   rxvt_fg;
    unsigned long   rxvt_bg;
    unsigned long   rxvt_topshadow;
    unsigned long   rxvt_botshadow;
# endif
# ifdef XTERM_SCROLLBAR
    unsigned long   xterm_fg;
    unsigned long   xterm_bg;
    unsigned long   xterm_shadow;
# endif
# ifdef PLAIN_SCROLLBAR
    unsigned long   plain_fg;
    unsigned long   plain_bg;
# endif
# ifdef NEXT_SCROLLBAR
    unsigned long   next_fg;	/* black */
    unsigned long   next_bg;	/* gray */
    unsigned long   next_white;
    unsigned long   next_dark;
    GC		    next_stippleGC;
    Pixmap	    next_dimple, 
		    next_upArrow,
		    next_upArrowHi,
		    next_downArrow,
		    next_downArrowHi;
# endif
# ifdef SGI_SCROLLBAR
    unsigned long   sgi_fg;	/* medium */
    unsigned long   sgi_bg;	/* light */
    unsigned long   sgi_black;
    unsigned long   sgi_white;
    unsigned long   sgi_lmedium;
    unsigned long   sgi_dark;
    unsigned long   sgi_vdark;
    GC		    sgi_stippleGC;
    Pixmap	    sgi_dimple, 
		    sgi_upArrow,
		    sgi_upArrowHi,
		    sgi_upArrowLow,
		    sgi_downArrow,
		    sgi_downArrowHi,
		    sgi_downArrowLow;
# endif
} scrollBar_t;
#endif	/* HAVE_SCROLLBARS */


typedef struct {
    char	state;	/* tabbar state */

    short	ltab;	/* last tab */
    short	atab;	/* active tab */
    short	ptab;	/* previous active tab */
    short	fvtab;	/* first visible tab */
    short	lvtab;	/* last visible tab */

    Window	win;
#ifdef BACKGROUND_IMAGE
    Bool	    hasPixmap;	/* has a background Pixmap */
#endif
    GC		gc;	/* tab background/foreground, grey25/black */
    unsigned long   fg;	    /* foreground, black */
    unsigned long   bg;	    /* background, grey25 */
    unsigned long   ifg;    /* inactive tab foreground, black */
    unsigned long   ibg;    /* inactive tab background, grey */
    char	    rsfg;   /* fg resource has changed */
    char	    rsbg;   /* bg resource has changed */
    char	    rsifg;  /* ifg resource has changed */
    char	    rsibg;  /* ibg resource has changed */
    unsigned long   frame;	/* tab frame, white */
    unsigned long   delimit;	/* delimit, dark grey */
#ifdef XFT_SUPPORT
    XftDraw*	    xftwin; /* XFT window */
    XftColor	    xftfg;  /* foreground */
    XftColor	    xftifg; /* background */
#endif
} tabBar_t;


#ifdef BACKGROUND_IMAGE
typedef struct {
    short           w, h, x, y;
    Pixmap          pixmap; 
} bgPixmap_t;
#endif


/* To suppress compile warning without xpm library */
#ifdef BACKGROUND_IMAGE
# ifndef HAVE_LIBXPM
#  define XpmCloseness	    (0)
#  define XpmColormap	    (0)
#  define XpmVisual	    (0)
#  define XpmDepth	    (0)
#  define XpmSize	    (0)
#  define XpmReturnPixels   (0)
typedef struct {
    unsigned long   valuemask;
    Visual*	    visual;
    Colormap	    colormap;
    unsigned int    depth;
    unsigned int    width;
    unsigned int    height;
    unsigned int    closeness;
} XpmAttributes;
# endif	/* HAVE_LIBXPM */
#endif	/* BACKGROUND_IMAGE */


typedef enum {
    TERMENV_XTERM = 0,
    TERMENV_RXVT,
    TERMENV_VT102,
    TERMENV_VT100,
    TERMENV_ANSI,
    TERMENV_DUMB,
} termenv_t;

struct term_t;
typedef struct {
    /*
     * Index to vts. If it's -1, then this term_t structure is not used.
     * Otherwise, it is used by pointer vts[vts_idx]. This is to improve destroy
     * performance so that we only need to do (i = page..ltab) vts[i] = vts[i+1]
     * instead of vterm[i] = vterm[i+1].
     */
    short	    vts_idx;

    unsigned char   profileNum;	    /* Profile used to init settings */
    int		    globalTabNum;   /* Number of tabs created before this tab
				       during this processes lifetime */

    /* moved from TermWin_t */
    uint16_t	    saveLines;	/* number of lines to save */
    uint16_t	    num_scr;	/* number of lines scrolled */
    uint16_t	    nscrolled;	/* number of line actually scrolled */
    uint16_t	    view_start;	/* scrollback view starts here */
    uint16_t	    mapped;	/* window state mapped? */

    /* screen structure initialized? */
    unsigned char   BOOLVAR(init_screen, 1);

    Window          vt; /* terminal window */
#ifdef XFT_SUPPORT
    XftDraw*	    xftvt;
#endif
#ifdef BACKGROUND_IMAGE
    Pixmap	    pixmap;	/* background image, NOT used by */
    bgPixmap_t	    bg;		/* transparent window!!! */
    XpmAttributes   xpm_attr;	/* original attr of image */
#endif

    /* Apparently, people like fg/bg colors for individual terminal */
    unsigned long   p_fg,
		    p_bg,
		    p_fgfade,
		    p_bgfade;
#ifdef XFT_SUPPORT
    XftColor	    p_xftfg,
		    p_xftbg,
		    p_xftfgfade,
		    p_xftbgfade;
#endif

    /* moved from rxvt_t */
    text_t**	    drawn_text;	/* text drawn on screen (characters) */
    rend_t**	    drawn_rend;	/* text drawn on screen (rendition) */
    text_t**	    buf_text;
    rend_t**	    buf_rend;
    screen_t        screen;
#if NSCREENS
    screen_t        swap;
#endif
    /* move from hidden */
    rend_t	    rstyle;

    uint16_t	    prev_ncol; /* previous columns */
    uint16_t	    prev_nrow; /* previous rows */
    /* moved from tab_t */
    short		tab_width;	/* tab width */
    char UNTAINTED *	tab_title;  	/* tab title */

    char	    *title_format;	/* Format to be used to display the tab
					   title */
    char	    *winTitleFormat;	/* Format of the window title (used when
					   syncing the tab title */

    /* moved from rxvt_t */
    int             cmd_fd; /* pty file descriptor; connected to command */
    int             tty_fd; /* tty file descriptor; connected to child */
    /* moved from hidden */
    unsigned char   BOOLVAR( current_screen, 1);    /* PRIMARY / SECONDARY */
#ifdef POINTER_BLANK
    unsigned char   BOOLVAR( hidden_pointer, 1);    /* pointer is hidden? */
#endif

    pid_t	    cmd_pid;
    char*	    ttydev;
#ifndef RESET_TTY_TO_COMMON_DEFAULTS
    struct stat     ttyfd_stat; /* original status of our tty */
#endif
#ifndef NO_SETOWNER_TTYDEV
    unsigned char   next_tty_action;
#endif

    /* moved from hidden */
    uint32_t	    PrivateModes;
    uint32_t	    SavedModes;

#ifdef UTMP_SUPPORT
#ifndef UTEMPTER_SUPPORT
# ifdef HAVE_STRUCT_UTMP
    struct utmp     ut;
# endif
# if defined(HAVE_STRUCT_UTMPX) && !defined(HAVE_STRUCT_UTMP)
    struct utmpx    utx;
# endif
# if (defined(HAVE_STRUCT_UTMP) && defined(HAVE_UTMP_PID)) || defined(HAVE_STRUCT_UTMPX)
    char            ut_id[5];
# endif
    int             utmp_pos;
#endif	/* UTEMPTER_SUPPORT */
    unsigned char   next_utmp_action;
#endif

#if 0
    char**	    command_argv;
    int		    command_argc;
#endif

    /* move from rxvt_hidden */
    ttymode_t	    tio;
    unsigned int    ttymode;
    char	    rvideo;
#ifdef MULTICHAR_SET
    char	    chstat,
    /* set ==> we only got half a glyph */
		    lost_multi,
    /* set ==> currently using 2 bytes per glyph */
		    multi_byte;
#endif
    char	    charsets[4];

    /* need to hold the terminal?
     *   hold == 0: not hold
     *   hold == 1: hold
     *   hold >  1: can destroy the terminal now
     */
    unsigned char   BOOLVAR(hold,2),
		    BOOLVAR(holdOption,3),  /* Copy of the profile option for
					       this tab.*/
		    BOOLVAR(gotEIO,1),	    /* Read on this terminal's fd
					       got EIO */
		    BOOLVAR(dead,1),	    /* the terminal is dead or alive? */
		    BOOLVAR(highlight,1);   /* the terminal is highlighted? */
    int		    status;		    /* Status of child process after it
					       exits */

    /* the terminal TERM type */
    termenv_t	    termenv;

    int		    scrolled_lines;	/* If this tab is producing lots of
					   data, this is the number of lines
					   that have scrolled without a refresh
					   request */

    short	    monitor_tab;	/* monitor tab status value, 
	                                 * see TAB_MON_* constants for possible values 
					 */

    int		    monitor_nbytes_read;    /* number of bytes read since 
                                               monitor-start */
    struct	    timeval monitor_start;  /* epoch time of monitor starttime
					       of the tab */

    /*
     * Moved from hidden: want_refresh needs to be local to each tab.
     * want_full_refresh, refresh_type (except for SMOOTH_REFRESH), and
     * want_clip_refresh should be window (not tab) variables.
     */
    unsigned char   BOOLVAR( want_refresh, 1);		/* Awaiting screen
							   refresh */

    int		    nbytes_last_read;	/* Number of bytes read on the last
					   read() call to child's fd */

    /*
     * Data we want to write into cmd_fd is buffered in here, before being
     * written by rxvt_tt_write(). [Child's input buffer]
     */
    unsigned char   *v_buffer,		/* pointer to physical buffer */
		    *v_bufstr,		/* beginning of area to write */
		    *v_bufptr,		/* end of area to write */
		    *v_bufend;		/* end of physical buffer */

    /*
     * Data read from cmd_fd is buffered in here [Child's output buffer]
     */
    unsigned char   *cmdbuf_escstart,	/* Start of an escape sequence */
		    *cmdbuf_escfail,	/* Position where processing of an
					   escape sequence last failed */
		    *cmdbuf_ptr,	/* current char */
		    *cmdbuf_endp;	/* End of read child's output */
    unsigned char   cmdbuf_base[BUFSIZ];
} term_t;

#define TAB_MON_OFF 0            /* tab monitoring off */
#define TAB_MON_ACTIVITY 1       /* monitor for activity */
#define TAB_MON_INACTIVITY 2     /* monitor for inactivity */
#define TAB_MON_AUTO 3           /* automatic discovery of monitor type */
#define TAB_MON_NOTIFICATION 4   /* inactivity/activity detected, notification needed */


/* Possible values for macros.modFlags */
#define MACRO_CTRL	(1U << 0)
#define MACRO_META	(1U << 1)
#define MACRO_SHIFT	(1U << 2)
#define MACRO_PRIMARY	(1U << 3)

/* Number of bits used by modifiers in modFlags */
#define MACRO_N_MOD_BITS    (4)
#define MACRO_MODMASK	\
    ( MACRO_CTRL | MACRO_META | MACRO_SHIFT | MACRO_PRIMARY )
#define MACRO_MAX_CHAINLEN  (0xf)

/* Get / set the macro number from modFlags */
#define MACRO_GET_NUMBER(x)	\
    (( (x) & ~MACRO_MODMASK ) >> MACRO_N_MOD_BITS)


/*
 * Action to take when a macro is called / menu item is selected.
 */
typedef struct {
    unsigned short  type;   /* must not be changed; first element */
    unsigned short  len;    /* strlen (str) */
    unsigned char   *str;   /* action to take */
} action_t;

/* Values for macro_t.type. Must sync this with macroNames from macros.c */
enum {
    MacroFnDummy=0,
    MacroFnEsc,
    MacroFnStr,
    MacroFnNewTab,
    MacroFnExec,
    MacroFnClose,
    MacroFnGoto,
    MacroFnMove,
    MacroFnScroll,
    MacroFnCopy,
    MacroFnPaste,
    MacroFnPasteFile,
    MacroFnMonitorTab,
    MacroFnToggleSubwin,
    MacroFnFont,
    MacroFnToggleVeryBold,
    MacroFnToggleBoldColors,
    MacroFnToggleVeryBright,
    MacroFnToggleTransp,
    MacroFnToggleBcst,
    MacroFnToggleHold,
    MacroFnToggleFullscreen,
    MacroFnRaise,
    MacroFnSetTitle,
    MacroFnUseFifo,
    MacroFnPrintScreen,
    MacroFnSaveConfig,
    MacroFnToggleMacros,
    NMACRO_FUNCS
} macroFnNames;

typedef unsigned char macro_priority_t;
typedef struct {
    KeySym		keysym;
    unsigned char	modFlags;	/* First 4 bits are the action order
					   number. Last four bits are the
					   modifiers */
    macro_priority_t	priority;
    action_t		action;
} macros_t;

/*
 * profile_t structure. Contains information about each profile (formerly the
 * vt%d.xx resources.
 */
typedef struct _profile_t
{
    unsigned long	fg,
			bg;
#ifdef XFT_SUPPORT
    XftColor		xftfg,
			xftbg;
#endif
    unsigned long	fg_fade,
			bg_fade;
# ifdef XFT_SUPPORT
    XftColor		xftfg_fade,
			xftbg_fade;
# endif

    int			saveLines;

    unsigned char	BOOLVAR( holdOption, 3 );

    /*
     * Each profile also has a tab title, and command associated to it. However
     * since that's already stored in our resource options, we don't need
     * pointers for it here.
     */
} profile_t;


typedef struct rxvt_vars {
    /*
     * These ``hidden'' items are not for public consumption and must not be
     * accessed externally
     *
     * 2006-02-19 gi1242: mrxvt was forked from rxvt. In rxvt, they compiled
     * common rxvt functions into a library "rxvtlib". Thus when loading
     * multiple versions of rxvt, this common code was linked to, saving some
     * memory.
     *
     * With tabs, we of course won't find this as useful. With grand dreams of
     * multiple windows, we'll never need it :).
     */
    struct rxvt_hidden *h;

    /*
     * Exposed items
     *   Changes to structure here require library version number change
     */
    TermWin_t	    TermWin;
#ifdef HAVE_SCROLLBARS
    scrollBar_t	    scrollBar;
#endif
#ifdef HAVE_MENUBAR
    menuBar_t	    menuBar;
#endif
    tabBar_t	    tabBar;
    Display*	    Xdisplay;
    uint32_t	    Options[MAX_OPTION_ARRAY];
    XSizeHints      szHint;

    /* macros */
    macros_t*	    macros;		    /* array of user defind macros */
    unsigned short  nmacros,		    /* Number of macros defined */
		    maxMacros;		    /* max # of macros that can be
					       stored in memory pointed to by
					       "macros" */

    Colormap        Xcmap;

    unsigned long   *pixColorsFocus,
		    *pixColorsUnfocus;	    /* Arrays of size TOTAL_COLORS.
					       Pixel colors to be used when
					       focused / unfocussed. */
    unsigned long   *pixColors;		    /* Array of size TOTAL_COLORS */

    int		    ntabs;		    /* Number of tabs created so far
					       during process lifetime */
    int		    fgbg_tabnum;	    /* (Global) tab number corresponding
					       to which Color_fg/bg are set */

#ifdef XFT_SUPPORT
    XftColor*	    xftColors;		    /* Array of size TOTAL_COLORS */

    XftColor	    *xftColorsFocus,	    /* Same as above, but for Xft */
		    *xftColorsUnfocus;
# endif


    profile_t	    profile[MAX_PROFILES];

    Cursor	    term_pointer;	    /* cursor for vt window */
    int		    Xdepth;
    int		    sb_shadow;		    /* scrollbar shadow width */
    int		    Xfd;		    /* file descriptor of the X
					       connection */

    /*
     * term_t structures and pointers.
     *
     * 2006-08-18 gi1242 TODO: This should be an array that grows dynamically in
     * size. Plus we should only reserve enough memory for a our currently
     * displayed term structures.
     */
    term_t	    vterm[MAX_PAGES];
    term_t*	    vts[MAX_PAGES];

    short	    tabClicked;		    /* Tab clicked by user. Used for
					       moving tabs by drag and drop. */

    unsigned char   BOOLVAR( cleanDeadChilds, 1 ),
					    /* True if we have marked some
					       children as dead, but not called
					       rxvt_clean_cmd_page() to clean
					       them out */
		    BOOLVAR( gotEIO, 1 );   /* True if some read() got EIO */

    short	    ndead_childs;	    /* number of children that have
					       died */
    short	    nAsyncChilds;	    /* Number of alive children launched
					       via rxvt_async_exec */
    short	    asyncChilds[MAX_CHILDS];
					    /* PIDS of children exec'ed via
					     * rxvt_async_exec */

    int		    num_fds;		    /* number of fd to monitor */


#ifdef USE_FIFO
    int		    fifo_fd;			/* fd to read macros from */
    char	    fifo_buf[FIFO_BUF_SIZE];	/* Buffer size of data read from
						   fifo_fd */
    char	    *fbuf_ptr;			/* Pointer where data from
						   fifo_fd should be read */
    char	    *fifo_name;			/* Name of the fifo */
#endif


    selection_t     selection;
    sstyle_t	    selection_style;

    int		    numlock_state;
    char*	    tabstop;		    /* per location: 1 == tab-stop */
    enum enc_label  encoding_method;

    char**	    global_argv;
    int		    global_argc;

} rxvt_t;


typedef enum {
    HIDE_MENUBAR = 0,
    SHOW_MENUBAR,
    HIDE_TABBAR,
    SHOW_TABBAR,
    HIDE_SCROLLBAR,
    SHOW_SCROLLBAR,
    RESIZE_FONT,
    X_CONFIGURE,
} resize_reason_t;


/* MACROS for colors of individual terminals */
#if 0
#define VTFG(R, P)	\
    ((R)->pixColors[TOTAL_COLORS + (P)])
#define VTBG(R, P)	\
    ((R)->pixColors[TOTAL_COLORS + MAX_PAGES + (P)])
#ifdef XFT_SUPPORT
# define VTXFTFG(R, P)	    \
    ((R)->xftColors[TOTAL_COLORS + (P)])
# define VTXFTBG(R, P)	    \
    ((R)->xftColors[TOTAL_COLORS + MAX_PAGES + (P)])
#endif	/* XFT_SUPPORT */
#define ISSET_VTFG(R, P)    \
    (NULL != ((R)->h->rs[Rs_color + TOTAL_COLORS + (P)]))
#define ISSET_VTBG(R, P)    \
    (NULL != ((R)->h->rs[Rs_color + TOTAL_COLORS + MAX_PAGES + (P)]))
#endif

#define VTFG(R, P)	\
    ((R)->profile[(P)].fg)
#define VTBG(R, P)	\
    ((R)->profile[(P)].bg)

#ifdef XFT_SUPPORT
# define VTXFTFG(R, P)	    \
    ((R)->profile[(P)].xftfg)
# define VTXFTBG(R, P)	    \
    ((R)->profile[(P)].xftbg)
#endif	/* XFT_SUPPORT */

# define VTFG_FADE(R, P)	    \
    ((R)->profile[(P)].fg_fade)
# define VTBG_FADE(R, P)	    \
    ((R)->profile[(P)].bg_fade)

# ifdef XFT_SUPPORT
#  define VTXFTFG_FADE(R, P)	    \
    ((R)->profile[(P)].xftfg_fade)
#  define VTXFTBG_FADE(R, P)	    \
    ((R)->profile[(P)].xftbg_fade)
# endif /* XFT_SUPPORT */

#define ISSET_VTFG(R, P)    \
    (NULL != ((R)->h->rs[Rs_foreground + (P)] ) )
#define ISSET_VTBG(R, P)    \
    (NULL != ((R)->h->rs[Rs_background + (P)] ) )



/* MACROS for tab/page number */
#define ATAB(R)	    ((R)->tabBar.atab)
#define LTAB(R)	    ((R)->tabBar.ltab)
#define FVTAB(R)    ((R)->tabBar.fvtab)
#define LVTAB(R)    ((R)->tabBar.lvtab)
#define PTAB(R)	    ((R)->tabBar.ptab)

#define APAGE(R)    ((R)->tabBar.atab)
#define LPAGE(R)    ((R)->tabBar.ltab)
#define FVPAGE(R)   ((R)->tabBar.fvtab)
#define LVPAGE(R)   ((R)->tabBar.lvtab)
#define PPAGE(R)    ((R)->tabBar.ptab)

/* MACROS for vts structure */
#define AVTS(R)	    ((R)->vts[(R)->tabBar.atab])
#define LVTS(R)	    ((R)->vts[(R)->tabBar.ltab])
#define PVTS(R, P)  ((R)->vts[(P)])

#define SEL(R)	    ((R)->selection)

#define ASCR(R)	    ((R)->vts[(R)->tabBar.atab]->screen)
#define PSCR(R, P)  ((R)->vts[(P)]->screen)



/* macros for private/saved mode of term_t */
#define ISSET_PMODE(R, P, V)   \
    ((R)->vts[(P)]->PrivateModes & (V))
#define SET_PMODE(R, P, V)  \
    ((R)->vts[(P)]->PrivateModes |= (V))
#define UNSET_PMODE(R, P, V)  \
    ((R)->vts[(P)]->PrivateModes &= ~(V))
#define ISSET_SMODE(R, P, V)   \
    ((R)->vts[(P)]->SavedModes & (V))
#define SET_SMODE(R, P, V)  \
    ((R)->vts[(P)]->SavedModes |= (V))
#define UNSET_SMODE(R, P, V)  \
    ((R)->vts[(P)]->SavedModes &= ~(V))


/* Macro to determine weather we should the i-th tab or not */
#define SHOULD_HOLD( r, i )					\
    (								\
      ( HOLD_ALWAYSBIT & PVTS((r),(i))->holdOption )		\
      ||							\
      (								\
	( HOLD_NORMALBIT & PVTS((r),(i))->holdOption )		\
	&& !WIFEXITED( PVTS((r),(i))->status )			\
      )								\
      ||							\
      (								\
	( HOLD_STATUSBIT & PVTS((r),(i))->holdOption )		\
	&& WEXITSTATUS( PVTS((r),(i))->status ) != 0		\
      )								\
    )


/*****************************************************************************
 *                                PROTOTYPES                                 *
 *****************************************************************************/
void	    rxvt_main_loop(rxvt_t *);
rxvt_t*	    rxvt_init (int, const char *const *);

#endif	    /* __RXVTLIB_H__ */
/*----------------------- end-of-file (H source) -----------------------*/
