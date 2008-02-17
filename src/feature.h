/*--------------------------------*-H-*---------------------------------*
 * File:    feature.h
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1997-1998   Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (c) 2005        William P. Y. Hadisoeseno
 * 					<williampoetra@users.sourceforge.net>
 * Copyright (c) 2004-2006   Jingmin Zhou <jimmyzhou@users.sourceforge.net>
 * Copyright (c) 2005-2006   Gautam Iyer <gautam@math.uchicago.edu>
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

#ifndef __FEATURE_H__
#define __FEATURE_H__

#ifndef X11USRLIBDIR
# define X11USRLIBDIR	    "/usr/X11R6/lib"
#endif
#ifndef X11LIBDIR
# define X11LIBDIR	X11USRLIBDIR "/X11"
#endif
#ifndef XAPPLOADDIR
# define XAPPLOADDIR	    X11LIBDIR "/app-defaults" 
# define XAPPLOADDIRLOCALE  X11LIBDIR "/%-.*s/app-defaults"
#endif

/* Check ~/.Xdefaults and ~/.Xresources as well */
#define HAVE_EXTRA_XRESOURCE_FILES


/*-----------------------SCREEN OPTIONS AND COLOURS-----------------{{{1*/
/*
 * Define the name of the environment variable to be used in
 * addition to the "PATH" environment and the `path' resource.
 * Usually it should point to where you keep your background pixmaps and/or
 * your menu files
 */
#define PATH_ENV    "RXVTPATH"

/*
 * Avoid enabling the colour cursor (-cr, cursorColor, cursorColor2)
 */
/* #define NO_CURSORCOLOR */

/*
 * Suppress use of BOLD and BLINK attributes for setting bright foreground
 * and background, respectively.  Simulate BOLD using colorBD, boldFont or
 * overstrike characters.
 */
/* #define NO_BRIGHTCOLOR */

/*
 * Try and brighten colors in the 16--255 range (if possible) instead of using
 * the bold attribute.
 */
/* #define BOLD_BRIGHTENS_256_COLORS */

/*
 * Make colours match xterm colours instead of `traditional' rxvt colours
 */
#define XTERM_COLORS

/*
 * Disable separate colours for bold, underline and reverse video
 */
/* #define NO_BOLD_UNDERLINE_REVERSE */

/*
 * Disable using simulated bold using overstrike.  You can also turn off
 * overstrike just for multi char fonts
 * Note: NO_BOLDOVERSTRIKE implies NO_BOLDOVERSTRIKE_MULTI
 */
/* #define NO_BOLDOVERSTRIKE */
/* #define NO_BOLDOVERSTRIKE_MULTI */

/*
 * Don't bother cleaning up pixel droppings.  If you don't use bold
 * overstrike then you don't usually need this
 */
/* #define NO_PIXEL_DROPPING_AVOIDANCE */

/*
 * Compile without support for real bold fonts
 */
/* #define NO_BOLDFONT */

/*
 * Add support for '-hc colour' for background highlight of selection
 */
#define OPTION_HC

/*
 * Define maximum possible columns and rows
 */
#define MAX_COLS    1000
#define MAX_ROWS    1000

/*
 * Define default colours for certain items.  If you have a low colour
 * display, then consider using colours which are already pre-allocated:
 *   Black	(#000000)
 *   Red3	(#CD0000)   + these
 *   Green3	(#00CD00)   + colours
 *   Yellow3	    (#CDCD00)	+ are
 *   Blue3	(#0000CD)   + not
 *   Magenta3	    (#CD00CD)	+ pre-allocated
 *   Cyan3	(#00CDCD)   + if
 *   AntiqueWhite   (#FAEBD7)	+ NO_BRIGHTCOLOR
 *   Grey25	(#404040)   + defined
 *   Red	(#FF0000)
 *   Green	(#00FF00)
 *   Yellow	(#FFFF00)
 *   Blue	(#0000FF)
 *   Magenta	    (#FF00FF)
 *   Cyan	(#00FFFF)
 *   White	(#FFFFFF)
 */
/* These colours MUST be defined */
#define COLOR_FOREGROUND    "Black"
#define COLOR_BACKGROUND    "White"
#define COLOR_SCROLLBAR	    "#B2B2B2"	/* scrollColor match Netscape */
#define COLOR_SCROLLTROUGH  "#969696"
/*
 * The cursor colours are special.  Be very careful about setting these:
 * foreground/background colours may be modified by command line or resources
 * prior to this allocation.  Also, they are not valid if NO_CURSORCOLOR is
 * defined
 */
#define COLOR_CURSOR_FOREGROUND	NULL	/* if NULL, use background colour */
#define COLOR_CURSOR_BACKGROUND	NULL	/* if NULL, use foreground colour */

/*
 * Define to remove support for XCopyArea() support.  XCopyArea() is useful
 * for scrolling on non-local X displays
 */
/* #define NO_SLOW_LINK_SUPPORT */

/*
 * Printer pipe which will be used for emulation of attached vt100 printer
 */
#define PRINTPIPE   "lpr"

/*
 * Allow 80/132 mode switching on startup
 */
/* #define ALLOW_132_MODE */
 
/*------------------------------RESOURCES---------------------------{{{1*/
/*
 * Define to find installed application defaults for rxvt
 */
/* #define USE_XAPPLOADDIR */

/*
 * Add support for the Offix DND (Drag 'n' Drop) protocol
 */
/* #define OFFIX_DND */

/*---------------------------------KEYS-----------------------------{{{1*/

/*
 * Define defaults for backspace and delete keys - unless they have been
 * configured out with --disable-backspace-key / --disable-delete-key
 */
/* #define DEFAULT_BACKSPACE	"DEC"	    */ /* SPECIAL */
/* #define DEFAULT_BACKSPACE	"\177"	    */
/* #define DEFAULT_DELETE   "\033[3~"	*/

/*
 * If defined, then escape sequences for keys with a modifier pressed will be
 * the same as those sent by Xterm, instead of those sent by rxvt.
 */
#define XTERM_KEYS

/*
 * If defined, then when numlock is on, the DEC terminal sequences sent by all
 * the keypad keys (including +,-,*, etc) will be the sequences sent with the
 * shift modifier ignored. Otherwise only the numeric keys and the decimal point
 * will ignore the shift modifier.
 */
#define NUMLOCK_ALWAYS_IGNORES_SHIFT

/*
 * If defined, macros will be allowed to be "UNSHIFTED" (i.e. your macros don't
 * necessarily need a modifier key).
 */
#define UNSHIFTED_MACROS

/* (Hops) Set to choose a number of lines of context between pages
 *      (rather than a proportion (1/5) of savedlines buffer)
 *      when paging the savedlines with SHIFT-{Prior,Next} keys.
 */
#define PAGING_CONTEXT_LINES (1)

/*
 * Have either Ctrl+Tab or Mod4+Tab emit \e\t
 * Useful when window manager grabs Alt+Tab   -- mg
 */
/* #define CTRL_TAB_MAKES_META */
/* #define MOD4_TAB_MAKES_META */

/*
 * default mode switch when greek keyboard is enabled (i.e. GREEK_SUPPORT)
 */
#ifndef GREEK_KEYBOARD_MODESWITCH
# define GREEK_KEYBOARD_MODESWITCH XK_Mode_switch
#endif

/*--------------------------------MOUSE-----------------------------{{{1*/
/*
 *  Enable jump scrolling when use mouse wheel
 */
#ifdef MOUSE_WHEEL
# define JUMP_MOUSE_WHEEL
#endif

/*
 * Disable sending escape sequences (up, down, page up/down)
 * from the scrollbar when XTerm mouse reporting is enabled
 */
/* #define NO_SCROLLBAR_REPORT */

/*
 * Default separating chars for multiple-click selection
 * Space and tab are separate separating characters and are not settable
 */
#define CUTCHARS    "\"&'()*,;<=>?@[\\]^`{|}~"

/*
 * Add run-time support for changing the cutchars for double click selection
 */
#define CUTCHAR_RESOURCE

/*
 * Have mouse reporting include double-click info for button1
 */
/* #define MOUSE_REPORT_DOUBLECLICK */

/*
 * Set delay between multiple click events [default: 500 milliseconds]
 */
/* #define MULTICLICK_TIME 500 */

/*
 * Time factor to slow down a `jumpy' mouse.  Motion isn't recognised until
 * this long after the last mouse click [default: 50 milliseconds]
 */
#define MOUSE_THRESHOLD	    50

/*
 * If mouse wheel is defined, then scrolling is by 5 lines (or 1 line
 * if the shift key is down).  Scrolling can be smooth or jump scrolling
 */
/* #define JUMP_MOUSE_WHEEL */

/*
 * Set delay periods for continuous scrolling with scrollbar buttons
 */
/* #define SCROLLBAR_INITIAL_DELAY 40 */
/* #define SCROLLBAR_CONTINUOUS_DELAY 2 */

/*
 * The speed of selection scrolling is proportional to the distance
 * the mouse pointer is out of the text window.  This is the max
 * number of lines to scroll at a time.
 */
#define SELECTION_SCROLL_MAX_LINES 2

/*
 * The number of lines (measured in character's heights, not pixels)
 * the pointer must be out of the window for each increase in the
 * number of lines scrolled.
 */
#define SELECTION_SCROLL_LINE_SPEEDUP 4

/*--------------------------------MISC------------------------------{{{1*/
/*
 * Disable to reset tty device to pre-incovation state
 */
#define RESET_TTY_TO_COMMON_DEFAULTS

/*
 * Only log in wtmp file when we're a login shell (-ls option)
 */
#define WTMP_ONLY_ON_LOGIN

/*
 * When pasting ACS characters, convert to similar normal characters
 */
#define ACS_ASCII

/*
 * Default ascii characters to which ACS symbols are converted
 */
#ifdef ACS_ASCII
# define ACS_CHARS ".#    o+  ----+---__++--|<> =#. "
#endif

/*
 * When executing external commands (e.g. via NewTab macro) use attempt to word
 * split at quotes internally.
 */
/* #define INTERNAL_ARGV_SPLIT */

/*
 * Maximum number of process (executed via Exec / NewTab macros, or -blc option)
 * that can be alive at one time.
 */
#define MAX_CHILDS  (16)

/*
 * Quick timeout default. The amount of time (micro seconds) mrxvt should wait
 * for X events when a screen refresh is pending. Screen refreshes (especially
 * with Xft and anti-aliasing) is quite expensive. The higher the value, the
 * fewer screen refreshes are preformed. If you set it too high, you might find
 * typing text a little jerky. If you set it too low, bye bye CPU. Try something
 * in the range of 500 -- 50000
 */
#define TIMEOUT_USEC (10000)

/*--------------------------------BELL------------------------------{{{1*/
/*
 * Disable all bell indications
 */
/* #define NO_BELL */

/*
 * Disable automatic de-iconify when a bell is received
 */
/* #define NO_MAPALERT */

/*
 * Have mapAlert behaviour selectable with mapAlert resource
 */
#define MAPALERT_OPTION

/*------------------------------TAB BAR-----------------------------{{{1*/

/* Default title of new tabs if unspecified */
#define DEFAULT_TAB_TITLE   "Terminal"

/*-----------------------------SCROLL BAR---------------------------{{{1*/
/*
 * Choose the rxvt style scrollbar width
 * - should be an even number [default: 10]
 */
/* #define SB_WIDTH_RXVT 10 */

/*
 * Minimum and maximum widths of the scrollbar (all styles)
 */
#define SB_WIDTH_MINIMUM    5
#define SB_WIDTH_MAXIMUM    100

/*
 * When using Rxvt scrollbar, clicking above or below the slider will move
 * 1/4 of the screen height, if possible.  Setting RXVT_SCROLL_FULL will move
 * it one screen height less one line, if possible
 */
#define RXVT_SCROLL_FULL 1

/*
 * (Hops) draw an internal border line on inside edge of the scrollbar
 */
/* #define SB_BORDER */

/*------------------------------MENU BAR----------------------------{{{1*/
/*
 * Choose how many of (experimental) menuBars you want to be able to stack at
 * one time.
 *  A value of 1 disables menuBar stacking.
 *  A value of 0 disables menuBar all together.
 *  Note that the amount of memory overhead is the same for any value >= 2.
 */
/*
 * 2006-02-04 gi1242: The user interface (using escape sequences) for modifying
 * menus has been disabled due to a security hole. Thus stacking menubars is
 * absolutely pointless and a waste of memory. The stacking code has been
 * removed.
 */
/* #define MENUBAR_MAX 8 */

/*
 * Change the default shadow style
 */
/* #define MENUBAR_SHADOW_IN */

/*
 * Change the default shadow style
 */
#define MENU_SHADOW_IN

/*---------------------------MULTILINGUAL---------------------------{{{1*/
/*
 * Allow run-time selection of Meta (Alt) to set the 8th bit on
 */
#define META8_OPTION

/*---------------------------DISPLAY OPTIONS------------------------{{{1*/
/*
 * Force local connection to be socket (or other local) communication
 */
/* #define LOCAL_X_IS_UNIX */

/*
 * Have DISPLAY environment variable & "\E[7n" transmit display with IP number
 */
/* #define DISPLAY_IS_IP */

/*
 * Have "\E[7n" transmit the display name.
 * This has been cited as a potential security hole.
 */
/* #define ENABLE_DISPLAY_ANSWER */

/*
 * Change what ESC Z transmits instead of the default "\E[?1;2c"
 */
/* #define ESCZ_ANSWER	"\033[?1;2C" */

/*
 * Check the current value of the window-time/icon-name and avoid
 * re-setting it to the same value -- avoids unnecessary window refreshes
 */
#define SMART_WINDOW_TITLE

/*
 * Allow colors to be changed with an xterm escape sequence.
 */
#define XTERM_COLOR_CHANGE

/*
 * Remove secondary screen's independent cursor position, a la xterm
 */
/* #define NO_SECONDARY_SCREEN_CURSOR */

/*
 * If no secondary screen is available (NO_SECONDARY_SCREEN in config.h),
 * then scroll the text up on a window change request
 */
/* #define SCROLL_ON_NO_SECONDARY */

/* Width of the term internal border */
#define DEFAULT_INTERNALBORDERWIDTH (0)
#define MAX_INTERNALBORDERWIDTH	    (20)

/* Width of the term external border */
#define DEFAULT_EXTERNALBORDERWIDTH (0)
#define MAX_EXTERNALBORDERWIDTH	    (20)

/* Default number of extra dots between lines */
#define DEFAULT_LINESPACE	    (0)
#define MAX_LINESPACE		    (20)

/* Default seconds of pointer blank delay */
#define DEFAULT_BLANKDELAY	    (2)
#define MAX_BLANKDELAY		    (20)

/* Default number of lines in the scrollback buffer */
#define DEFAULT_SAVELINES	    (100)
#define MAX_SAVELINES		    (0xffff)

/* Default bell throttle interval */
#define THROTTLE_BELL_MSEC	    (500)
#define THROTTLE_BELL_COUNT	    (3)

/* Default tabstop size */
#define TABSTOP_SIZE		    (8)

/* Maximal length and display length of tab title */
#define MAX_TAB_TXT		    (507)
#define MAX_DISPLAY_TAB_TXT	    (40)
#define MAX_TAB_PIXEL_WIDTH	    (200)

#define DEFAULT_DISPLAY_TAB_TXT	    (27)
#define DEFAULT_MIN_VISIBLE_TABS    (6)

/* Minimum Xft font size (pixel) */
#define MIN_XFT_FONT_SIZE	    (1)

/* Default Xft font name and size */
#define DEFAULT_XFT_FONT_SIZE	    (12)
#define DEFAULT_XFT_FONT_NAME	    "Monospace"

/* Default Xft propotional font name and size (used for menubar / tabs) */
#define DEFAULT_XFT_PFONT_SIZE	    (10)
#define DEFAULT_XFT_PFONT_NAME	    "Sans"

/* Default cursor blinking time (ms) */
#define MIN_BLINK_TIME		    (100)
#define DEFAULT_BLINK_TIME	    (500)
#define MAX_BLINK_TIME		    (5000)

/*
 * Make text cursor highlighting be the same as color of char that the cursor is
 * currently on.
 */
#define CURSOR_COLOR_IS_RENDITION_COLOR
/* #define SIMULATE_LINUX_CONSOLE_CURSOR_COLOR */

/* Set tab title on Xterm escape sequence to change window title */
#define SET_TAB_TITLE_ON_XTERM_SEQUENCE
/* Set tab title on Xterm escape sequence but not change window title */
#define SET_TAB_TITLE_NOT_WIN_TITLE

/* Enable dead key. To do so, set option "XkbLayout" to "us_intl"
** in the keyboard device section. Then disable multichar encoding
(-km noenc) and use ISO8859-X fonts. */
#define USE_DEADKEY

/* Strip trailing spaces from selection */
#define DONT_SELECT_TRAILING_SPACES

/*
 * Default interval (micro secs) to wait before refreshing the background under
 * pseudo transparency.
 */
#define DEFAULT_BG_REFRESH_INTERVAL 100000L

/*
 * Default number of pages to skip when jump scrolling.
 */
#define DEFAULT_SKIP_PAGES	(25)

/*
 * If an active tab produces fewer than these many characters, a screen refresh
 * is requested immediately. Otherwise it is delayed slightly (about 10ms) to
 * ensure that the active tab is not "flat out" scrolling.
 */
#define DEFAULT_REFRESH_LIMIT	(0)

/*
 * Provide termcap/terminfo bw support
 */
/* #define TERMCAP_HAS_BW */

#endif	/* __FEATURE_H__ */

/*}}}1*/
/* vim: set fdm=marker: */
/*----------------------- end-of-file (H source) -----------------------*/
