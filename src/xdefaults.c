/*--------------------------------*-C-*---------------------------------*
 * File:    xdefaults.c
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1994        Robert Nation <nation@rocket.sanders.lockheed.com>
 * Copyright (c) 1997,1998   mj olesen <olesen@me.queensu.ca>
 * Copyright (c) 2004        Terry Griffin <griffint@pobox.com>
 * Copyright (c) 2005        Grant McDorman <grmcdorman@users.sourceforge.net>
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


/*--------------------------------------------------------------------*
 *         BEGIN `INTERNAL' ROUTINE PROTOTYPES                        *
 *--------------------------------------------------------------------*/
#ifndef NO_RESOURCES
void  rxvt_get_xdefaults (rxvt_t*, FILE*, const char*, macro_priority_t);
#endif
/*--------------------------------------------------------------------*
 *         END   `INTERNAL' ROUTINE PROTOTYPES                        *
 *--------------------------------------------------------------------*/

static const char *emptyResource    = "";

static const char *const xnames[3] = {
    ".mrxvtrc",
    ".Xdefaults",
    ".Xresources",
};

/*{{{ monolithic option/resource structure: */
/*
 * `string' options MUST have a usage argument
 * `switch' and `boolean' options have no argument
 * if there's no desc(ription), it won't appear in rxvt_usage()
 */

/* INFO() - descriptive information only */
#define INFO(opt, arg, desc)			\
    {0, -1, NULL, (opt), (arg), (desc), 0}

/* STRG() - command-line option, with/without resource */
#define STRG(rsp, kw, opt, arg, desc, multiple)		    \
    {0, (rsp), (kw), (opt), (arg), (desc), (multiple)}

/* RSTRG() - resource/long-option */
#define RSTRG(rsp, kw, arg, multiple)			\
    {0, (rsp), (kw), NULL, (arg), NULL, (multiple)}

/* BOOL() - regular boolean `-/+' flag */
#define BOOL(kw, opt, flag, desc)		    \
    {(flag), -1, (kw), (opt), NULL, (desc), 0}

/* SWCH() - `-' flag */
#define SWCH(kw, opt, flag, desc)		    \
    {(flag), -1, (kw), (opt), NULL, (desc), 0}


/* convenient macros */
#define optList_STRLEN(i)			\
    (optList[i].flag ? 0 : (optList[i].arg ? STRLEN(optList[i].arg) : 1))
#define optList_isBool(i)		    \
    (optList[i].flag)
#define optList_isReverse(i)		    \
    (optList[i].flag & Opt_Reverse)
#define optList_isMultiple(i)		    \
    (optList[i].multiple)
#define optList_size()			    \
    (sizeof(optList) / sizeof(optList[0]))

static const struct {
    const uint32_t  flag;	/* Option flag */
    const int	    doff;	/* data offset */
    const char*	    kw;		/* keyword */
    const char*	    opt;	/* option */
    const char*	    arg;	/* argument */
    const char*	    desc;	/* description */

    const char	    BOOLVAR( multiple, 1 ); /* multiple values for profiles */
} optList[] = {
    /* Options for each profile */
#ifdef BACKGROUND_IMAGE
    STRG(Rs_backgroundPixmap, "Pixmap", "pixmap", "file[;geom]",
	    "background image for a tab", 1),
#endif
    STRG(Rs_tabtitle, "tabTitle", "tt", "string", "title name for tab", 1),
    STRG(Rs_saveLines, "saveLines", "sl", "number",
	    "number of scrolled lines to save for tab", 1),
    STRG(Rs_command, "command", "e", "string",
	    "command to execute for a tab", 1),
    STRG( Rs_foreground, "foreground", "fg", "color",
	    "foreground color for a tab", 1),
    STRG( Rs_background, "background", "bg", "color",
	    "background color for a tab", 1),
    STRG( Rs_cwd, "workingDirectory", "wd", "dir",
	    "Working directory to open new tabs in", 1 ),
    STRG(Rs_holdExit, "holdExit", "hold", "number",
	    "Keep tab open after child exits, based on childs exit status", 1 ),
    STRG(Rs_holdExitTxt, "holdExitText", "het",
	"string", "text to show while holding the terminal", 1),
    STRG(Rs_holdExitTtl, "holdExitTitle", "heT",
	"string", "Tab title of exited tabs", 1),
    STRG(Rs_titleFormat, "titleFormat", "tf",
	"string", "Displayed tab title format", 1),
    STRG( Rs_winTitleFormat, "winTitleFormat", "wtf", "string",
	    "format of window title (when syncing to tab title)", 1 ),

    /* Global options */
    STRG(Rs_maxTabWidth, "maxTabWidth", "mtw", "number", "maximum (char) title width of all tabs", 0),
    STRG(Rs_minVisibleTabs, "minVisibleTabs", "mvt", "number",
	    "minimum # of tabs to keep visible (requires xftpfn)", 0),
    BOOL( "highlightTabOnBell", "htb", Opt2_hlTabOnBell,
	    "highlighting inactive tabs only when bell sounds" ),
    BOOL( "syncTabTitle", "stt", Opt2_syncTabTitle,
	    "synchronizing terminal title with tab title" ),
    BOOL( "hideTabbar", "ht", Opt2_hideTabbar,
	    "hiding tabbar on initialization" ),
    BOOL( "autohideTabbar", "aht", Opt2_autohideTabbar,
	    "auto hide/show the tabbar depending on number of open tabs" ),
    BOOL( "bottomTabbar", "bt", Opt2_bottomTabbar,
	    "showing tabbar at bottom" ),
    BOOL( "hideButtons", "hb", Opt2_hideButtons,
	    "hide buttons on tabbar" ),
    BOOL( "syncTabIcon", "sti", Opt2_syncTabIcon,
	    "synchronizing icon name with tab title" ),
    BOOL( "veryBoldFont", "vbf", Opt2_veryBold,
	    "showing color text with bold font" ),
#ifndef NO_BRIGHTCOLOR
    BOOL( "boldColors", NULL, Opt2_boldColors, NULL),	/* Show bold primary
							   colors using bright
							   colors */
    BOOL( "veryBright", NULL, Opt_veryBright, NULL),	/* Show bold primary
							   colors using bold
							   font also */
#endif


    STRG(Rs_container_window, NULL, "into", "WinID", "Parent window id", 0),
    STRG(Rs_display_name, NULL, "d", NULL, NULL, 0),
    STRG(Rs_display_name, NULL, "display", "string", "X server to contact", 0),
    STRG(Rs_term_name, "termName", "tn", "string", "the TERM environment variable", 0),
    STRG(Rs_title, "title", "title", "string", "title name for window", 0),
    STRG(Rs_geometry, NULL, "g", NULL, NULL, 0),
    STRG(Rs_geometry, "geometry", "geometry", "geometry", "size (in characters) and position", 0),
    BOOL( "interceptConsole", "C", Opt_console, "intercept console messages" ),
    BOOL( "startIconic", "iconic", Opt_iconic, "start iconic" ),
    BOOL( "maximized", "m", Opt2_maximized, "Start maximized" ),
    BOOL( "fullscreen", "fs", Opt2_fullscreen, "Start fullscreen" ),
    BOOL( "reverseVideo", "rv", Opt_reverseVideo, "reverse video" ),
    BOOL( "loginShell", "ls", Opt_loginShell, "login shell" ),

    RSTRG( Rs_refreshLimit, "refreshLimit", "nchars", 0 ),
    RSTRG( Rs_skipPages, "skipPages", "%d", 0 ),

#ifdef TINTING_SUPPORT
    STRG(Rs_color + Color_tint, "tintColor", "tint", "color", "tint color", 0),
    STRG(Rs_shade, "shading", "shade", "%", "shade background by %x when tinting", 0),
    STRG(Rs_shade, "backgroundFade", "bgfade", "%",
	"fade background image or transparent background by %x (obsolete)", 0),
#endif
#ifdef TRANSPARENT
    RSTRG(Rs_bgRefreshInterval, "bgRefreshInterval", "ms", 0),
#endif

    STRG(Rs_fade, "fading", "fade", "%", "make colors x% darker when losing focus", 0),

#ifdef HAVE_SCROLLBARS
    BOOL( "scrollBar", "sb", Opt_scrollBar, "scrollbar" ),
    BOOL( "scrollbarRight", "sr", Opt_scrollBar_right, "scrollbar right" ),
    BOOL( "scrollbarFloating", "st", Opt_scrollBar_floating,
	    "scrollbar without a trough" ),
    STRG(Rs_scrollBar_thickness, "scrollbarThickness", "sbt", "number",
	"scrollbar thickness/width in pixels", 0),
    STRG(Rs_scrollBar_style, "scrollbarStyle", "ss", "mode",
	"scrollbar style = plain|xterm|rxvt|next|sgi", 0),
    STRG(Rs_scrollBar_align, "scrollbarAlign", "sa", "mode", 
	"scrollbar alignment = top|bottom", 0),
# ifdef BACKGROUND_IMAGE
    STRG(Rs_scrollbarPixmap, "scrollbarPixmap", "sbpixmap",
	"file[;geom]", "scrollbar background image", 0),
# endif
#endif
    BOOL( "scrollTtyOutputInhibit", "si", Opt_scrollTtyOutputInhibit,
	    "scroll-on-tty-output inhibit" ),
    BOOL( "scrollTtyKeypress", "sk", Opt_scrollTtyKeypress,
	    "scroll-on-keypress" ),

    STRG(Rs_opacity, "opacity", "o", "%",
	"transluscent window (true transparent) opaque degree", 0),
    STRG(Rs_opacityDegree, "opacityDegree", "od", "%",
	"transluscent window opaque degree interval", 0),

#ifdef TRANSPARENT
    BOOL( "transparent", "tr", Opt_transparent, "transparent" ),
    BOOL( "transparentForce", "trf", Opt_forceTransparent,
	    "forcefully transparent" ),
# ifdef HAVE_SCROLLBARS
    BOOL( "transparentScrollbar", "trs", Opt_transparent_scrollbar,
	    "transparent scrollbar" ),
# endif
# ifdef HAVE_MENUBAR
    BOOL( "transparentMenubar", "trm", Opt_transparent_menubar,
	    "transparent menubar" ),
# endif
    BOOL( "transparentTabbar", "trt", Opt_transparent_tabbar,
	    "transparent tabbar" ),
#endif	/* TRANSPARENT */

#ifdef BACKGROUND_IMAGE
    STRG(Rs_tabbarPixmap, "tabbarPixmap", "tbpixmap", "file[;geom]", "tabbar background image", 0),
    BOOL( "tabUsePixmap", "tupixmap", Opt_tabPixmap,
	    "use tabbar background image for tabs" ),
    STRG(Rs_appIcon, "appIcon", "ic", "file[;geom]", "application icon file", 0),
#endif	/* BACKGROUND_IMAGE */

    BOOL( "utmpInhibit", "ut", Opt_utmpInhibit,
	    "utmp inhibit - do not log to utmp" ),
    STRG(Rs_confFile, NULL, "cf", "file",
	"Configuration file instead of ~/.mrxvtrc", 0),

#ifndef NO_BELL
    BOOL( "visualBell", "vb", Opt_visualBell, "visual bell" ),
    BOOL( "currentTabVBell", "ctvb", Opt_currentTabVBell,
	    "visual bell only for active tab in focused window" ),
    RSTRG( Rs_vBellDuration, "vBellDuration",
	    "Duration (ms) of the visual bell", 0),
    STRG( Rs_bellCommand, "bellCommand", "blc",
	"string", "command to execute instead of beeping", 0),
    STRG( Rs_monitorCommand, "monitorCommand", "mcmd",
	"string", "command to execute if activity/inactivity was detected while monitoring", 0),
    STRG( Rs_monitorTimeout, "monitorTimeout", "mto", "number",
	"timeout in milliseconds for tab-monitoring", 0),
# if ! defined(NO_MAPALERT) && defined(MAPALERT_OPTION)
    BOOL( "mapAlert", NULL, Opt_mapAlert, NULL ),
# endif
#endif
#ifdef META8_OPTION
    BOOL( "meta8", "m8", Opt_meta8, "meta8" ),
#endif
#ifdef MOUSE_WHEEL
    BOOL( "mouseWheelScrollPage", "mp", Opt_mouseWheelScrollPage,
	    "mouse wheel scrolling a page" ),
#endif
#ifdef MULTICHAR_SET
    BOOL( "multibyte_cursor", "mcc", Opt_mc_hack,
	    "Multibyte character cursor movement" ),
#endif
#ifndef NO_FRILLS
    BOOL( "tripleclickwords", "tcw", Opt_tripleclickwords,
	    "triple click word selection" ),
#endif
    STRG(Rs_color + Color_ufbg, "ufBackground", "ufbg", "color", "unfocused background color", 0),
#ifdef TEXT_SHADOW
    STRG(Rs_textShadow, "textShadow", "ts", "color", "text shadow color", 0),
    STRG(Rs_textShadowMode, "textShadowMode", "tsm", "mode",
	"shadow mode = "
	"top|bottom|left|right|topleft|topright|botleft|botright", 0),
#endif
    STRG(Rs_tabfg, "tabForeground", "tabfg", "color", "tabbar active tab foreground color", 0),
    STRG(Rs_tabbg, "tabBackground", "tabbg", "color", "tabbar and active tab background color", 0),
    STRG(Rs_itabfg, "itabForeground", "itabfg", "color", "tabbar inactive tab foreground color", 0),
    STRG(Rs_itabbg, "itabBackground", "itabbg", "color", "tabbar inactive tab background color", 0),
    RSTRG(Rs_color + minCOLOR + 0, "color0", "color", 0),
    RSTRG(Rs_color + minCOLOR + 1, "color1", "color", 0),
    RSTRG(Rs_color + minCOLOR + 2, "color2", "color", 0),
    RSTRG(Rs_color + minCOLOR + 3, "color3", "color", 0),
    RSTRG(Rs_color + minCOLOR + 4, "color4", "color", 0),
    RSTRG(Rs_color + minCOLOR + 5, "color5", "color", 0),
    RSTRG(Rs_color + minCOLOR + 6, "color6", "color", 0),
    RSTRG(Rs_color + minCOLOR + 7, "color7", "color", 0),
#ifndef NO_BRIGHTCOLOR
    RSTRG(Rs_color + minBrightCOLOR + 0, "color8", "color", 0),
    RSTRG(Rs_color + minBrightCOLOR + 1, "color9", "color", 0),
    RSTRG(Rs_color + minBrightCOLOR + 2, "color10", "color", 0),
    RSTRG(Rs_color + minBrightCOLOR + 3, "color11", "color", 0),
    RSTRG(Rs_color + minBrightCOLOR + 4, "color12", "color", 0),
    RSTRG(Rs_color + minBrightCOLOR + 5, "color13", "color", 0),
    RSTRG(Rs_color + minBrightCOLOR + 6, "color14", "color", 0),
    RSTRG(Rs_color + minBrightCOLOR + 7, "color15", "color", 0),
#endif		    /* NO_BRIGHTCOLOR */
#ifndef NO_BOLD_UNDERLINE_REVERSE
    RSTRG(Rs_color + Color_BD, "colorBD", "color", 0),
    RSTRG(Rs_color + Color_UL, "colorUL", "color", 0),
    RSTRG(Rs_color + Color_RV, "colorRV", "color", 0),
#endif		    /* ! NO_BOLD_UNDERLINE_REVERSE */
#ifdef KEEP_SCROLLCOLOR
    RSTRG(Rs_color + Color_scroll, "scrollColor", "color", 0),
    RSTRG(Rs_color + Color_trough, "troughColor", "color", 0),
#endif		    /* KEEP_SCROLLCOLOR */
#ifdef OPTION_HC
    RSTRG(Rs_color + Color_HC, "highlightColor",  "color", 0),
#endif
#ifndef NO_CURSORCOLOR
    STRG(Rs_color + Color_cursor, "cursorColor", "cr", "color", "cursor color", 0),
    RSTRG(Rs_color + Color_cursor2, "cursorColor2", "color", 0),
#endif		    /* NO_CURSORCOLOR */
    STRG(Rs_color + Color_pointer, "pointerColor", "pr", "color", "pointer color", 0),
    STRG(Rs_color + Color_border, "borderColor", "bd", "color", "border color", 0),

#if defined (BACKGROUND_IMAGE) || defined(HAVE_MENUBAR)
    RSTRG(Rs_path, "path", "search path", 0),
#endif		    /* defined (BACKGROUND_IMAGE) || (HAVE_MENUBAR) */
#ifdef HAVE_MENUBAR
    STRG(Rs_menu, "menu", "menu",
	"filename[;tag]", "menubar definition file", 0),
    BOOL( "showMenu", "showmenu", Opt_showMenu, "show menubar" ),
# ifdef BACKGROUND_IMAGE
    STRG(Rs_menubarPixmap, "menubarPixmap", "mbpixmap",
	"file[;geom]", "menubar background image", 0),
# endif
#endif

#ifndef NO_BOLDFONT
    STRG(Rs_boldFont, "boldFont", "fb", "fontname", "bold text font", 0),
# if MAX_NFONTS > 1
    RSTRG(Rs_boldFont + 1, "boldFont1", "fontname", 0),
#endif
# if MAX_NFONTS > 2
    RSTRG(Rs_boldFont + 2, "boldFont2", "fontname", 0),
#endif
# if MAX_NFONTS > 3
    RSTRG(Rs_boldFont + 3, "boldFont3", "fontname", 0),
#endif
# if MAX_NFONTS > 4
    RSTRG(Rs_boldFont + 4, "boldFont4", "fontname", 0),
#endif
# if MAX_NFONTS > 5
    RSTRG(Rs_boldFont + 5, "boldFont5", "fontname", 0),
#endif
# if MAX_NFONTS > 6
    RSTRG(Rs_boldFont + 6, "boldFont6", "fontname", 0),
#endif
# if MAX_NFONTS > 7
    RSTRG(Rs_boldFont + 7, "boldFont7", "fontname", 0),
#endif
#endif

    STRG(Rs_font + 0, "font", "fn", "fontname", "normal text font", 0),
#if MAX_NFONTS > 1
    RSTRG(Rs_font + 1, "font1", "fontname", 0),
#endif
#if MAX_NFONTS > 2
    RSTRG(Rs_font + 2, "font2", "fontname", 0),
#endif
#if MAX_NFONTS > 3
    RSTRG(Rs_font + 3, "font3", "fontname", 0),
#endif
#if MAX_NFONTS > 4
    RSTRG(Rs_font + 4, "font4", "fontname", 0),
#endif
#if MAX_NFONTS > 5
    RSTRG(Rs_font + 5, "font5", "fontname", 0),
#endif
#if MAX_NFONTS > 6
    RSTRG(Rs_font + 6, "font6", "fontname", 0),
#endif
#if MAX_NFONTS > 7
    RSTRG(Rs_font + 7, "font7", "fontname", 0),
#endif
#ifdef MULTICHAR_SET
    STRG(Rs_mfont + 0, "mfont", "fm", "fontname", "multichar font", 0),
# if MAX_NFONTS > 1
    RSTRG(Rs_mfont + 1, "mfont1", "fontname", 0),
# endif
# if MAX_NFONTS > 2
    RSTRG(Rs_mfont + 2, "mfont2", "fontname", 0),
# endif
# if MAX_NFONTS > 3
    RSTRG(Rs_mfont + 3, "mfont3", "fontname", 0),
# endif
# if MAX_NFONTS > 4
    RSTRG(Rs_mfont + 4, "mfont4", "fontname", 0),
# endif
# if MAX_NFONTS > 5
    RSTRG(Rs_mfont + 5, "mfont5", "fontname", 0),
# endif
# if MAX_NFONTS > 6
    RSTRG(Rs_mfont + 6, "mfont6", "fontname", 0),
# endif
# if MAX_NFONTS > 7
    RSTRG(Rs_mfont + 7, "mfont7", "fontname", 0),
# endif
#endif		    /* MULTICHAR_SET */

#ifdef XFT_SUPPORT
    BOOL( "xft", "xft", Opt_xft, "use freetype font" ),
    STRG(Rs_xftfont, "xftFont", "xftfn", "fontname", "freetype font", 0),
# ifdef MULTICHAR_SET
    STRG(Rs_xftmfont, "xftmFont", "xftfm", "fontname", "freetype multichar font", 0),
    STRG(Rs_xftmsz, "xftmSize", "xftmsz", "number",
	"freetype multichar font size", 0),
    BOOL( "xftNomFont", "xftnfm", Opt2_xftNomFont,
	    "use freetype font as freetype mfont" ),
    BOOL( "xftSlowOutput", "xftslow", Opt2_xftSlowOutput,
	    "multichar string display in slow mode for better visual effect" ),
# endif
    BOOL( "xftAntialias", "xftaa", Opt2_xftAntialias,
	    "antialias of freetype font" ),
    BOOL( "xftHinting", "xftht", Opt2_xftHinting,
	    "hinting of freetype font" ),
    BOOL( "xftAutoHint", "xftah", Opt2_xftAutoHint,
	    "autohint of freetype font" ),
    BOOL( "xftGlobalAdvance", "xftga", Opt2_xftGlobalAdvance,
	    "global advance of freetype font" ),
    STRG(Rs_xftwt, "xftWeight", "xftwt", "style",
	"Xft font weight", 0),
    STRG(Rs_xftBwt, "xftBoldWeight", "xftbwt", "style",
	"Xft bold font weight", 0),
    STRG(Rs_xftst, "xftSlant", "xftst", "style",
	"slant style = roman|italic|oblique", 0),
    STRG(Rs_xftsz, "xftSize", "xftsz", "number",
	"freetype font size", 0),
    STRG(Rs_xftwd, "xftWidth", "xftwd", "style",
	"freetype font width = "
	"ultracondensed|condensed|normal|expanded|ultraexpended", 0),
    STRG(Rs_xftrgb, "xftRGBA", "xftrgb", "style",
	"freetype font sub-pixel order = rgb|bgr|vrgb|vbgr|none", 0),
    STRG(Rs_xftpfn, "xftPFont", "xftpfn", "fontname",
	"propotionally spaced freetype font to use for tabbar / menubar", 0),
    STRG(Rs_xftpsz, "xftPSize", "xftpsz", "size",
	"size of propotional freetype font", 0),
#endif

    BOOL( "cmdAllTabs", "at", Opt2_cmdAllTabs,
	    "running -e command for all tabs" ),
    BOOL( "protectSecondary", "ps", Opt2_protectSecondary,
	    "protecting tab that uses the secondary screen from being closed" ),

#ifdef MULTICHAR_SET
    STRG(Rs_multichar_encoding, "multichar_encoding", "km", "mode",
	"multichar encoding mode = eucj|sjis|big5|gb|gbk|kr|noenc", 0),
#endif		    /* MULTICHAR_SET */

#ifdef USE_XIM
    STRG(Rs_inputMethod, "inputMethod", "im", "name", "name of input method", 0),
    STRG(Rs_preeditType, "preeditType", "pt", "style",
	"input style = OverTheSpot|OffTheSpot|Root", 0),
#endif		    /* USE_XIM */

#ifdef GREEK_SUPPORT
    STRG(Rs_greek_keyboard, "greek_keyboard", "grk", "mode",
	"greek keyboard mapping mode = iso|ibm", 0),
    RSTRG(Rs_greektoggle_key, "greektoggle_key", "keysym", 0),
#endif

    STRG(Rs_name, "clientName", "name", "string",
	"client instance, icon, and title strings", 0),
    STRG(Rs_title, NULL, "T", NULL, NULL, 0),	/* short form */
    STRG(Rs_iconName, "iconName", "in", "string",
	"icon name for window", 0),
    BOOL( "borderLess", "bl", Opt2_borderLess,
	    "borderless window" ),
    BOOL( "overrideRedirect", "or", Opt2_overrideRedirect,
	    "override_redirect flag" ),
    STRG(Rs_desktop, "desktop", "desktop",
	"number", "desktop to place the program", 0),
    BOOL( "broadcast", "bcst", Opt2_broadcast,
	    "broadcast input to all terminals" ),

#ifndef NO_FRILLS
    STRG(Rs_ext_bwidth, "externalBorder", "w", "number",
	"external border in pixels", 0),
    STRG(Rs_ext_bwidth, NULL, "bw", NULL, NULL, 0),
    STRG(Rs_ext_bwidth, NULL, "borderwidth", NULL, NULL, 0),
    STRG(Rs_int_bwidth, "internalBorder", "b", "number",
	"internal border in pixels", 0),

    /* Reposition window on resize to stay on screen */
    BOOL( "smartResize", NULL, Opt2_smartResize,
	    NULL ),
    /* Resize window smooothly (not in character steps) */
    BOOL( "smoothResize", NULL, Opt2_smoothResize,
	    NULL ),
#endif
#ifndef NO_LINESPACE
    STRG(Rs_lineSpace, "lineSpace", "lsp", "number", "number of extra pixels between rows", 0),
#endif

#ifdef POINTER_BLANK
    BOOL( "pointerBlank", "pb", Opt_pointerBlank,
	    "blank pointer" ),
    RSTRG(Rs_pointerBlankDelay, "pointerBlankDelay", "number", 0),
#endif
#ifdef CURSOR_BLINK
    BOOL( "cursorBlink", "bc", Opt_cursorBlink, "blinking cursor" ),
    STRG(Rs_cursorBlinkInterval, "cursorBlinkInterval", "bci",
	"number", "cursor blinking interval (ms)", 0),
#endif

#ifndef NO_BACKSPACE_KEY
    RSTRG(Rs_backspace_key, "backspaceKey", "string", 0),
#endif

#ifndef NO_DELETE_KEY
    RSTRG(Rs_delete_key, "deleteKey", "string", 0),
#endif

    RSTRG(Rs_selectstyle, "selectStyle",
	"select style mode = old|oldword", 0),

#ifdef PRINTPIPE
    RSTRG(Rs_print_pipe, "printPipe", "string", 0),
#endif

    BOOL( "noSysConfig", "nsc", Opt2_noSysConfig,
	    "reading /etc/mrxvt/mrxvtrc." ),
    BOOL( "disableMacros", "dm", Opt2_disableMacros,
	    "all keyboard shortcuts (macros)" ),
    BOOL( "linuxHomeEndKey", "lk", Opt2_linuxHomeEndKey,
	    "enable Linux console Home/End keys" ),

    STRG(Rs_modifier, "modifier", "mod", "modifier",
	"meta modifier = alt|meta|hyper|super|mod1|...|mod5", 0),
    INFO("xrm", "string", "X resource"),

#ifdef CUTCHAR_RESOURCE
    RSTRG(Rs_cutchars, "cutChars", "string", 0),
#endif	/* CUTCHAR_RESOURCE */

#ifdef ACS_ASCII
    RSTRG(Rs_acs_chars, "acsChars", "string", 0),
#endif	/* ACS_ASCII */

    RSTRG(Rs_answerbackstring, "answerbackString", "string", 0),

#ifdef HAVE_X11_SM_SMLIB_H
    BOOL( "sessionMgt", "sm", Opt2_enableSessionMgt,
	    "enabling X session management" ),
    STRG(Rs_smClientID, "smClientID", "sid", "string",
	"client id of mrxvt for X session management", 0),
#endif	/* HAVE_X11_SM_SMLIB_H */

    /* Initial number of terminals */
    STRG( Rs_init_term_num, "initTermNumber", "tnum", "number",
	"Initial number of tabs/terminals", 0),
    STRG( Rs_initProfiles, "initProfileList", "ip", "profile list",
	"List of profiles to load on startup", 0 ),

    STRG(Rs_debug_masks, "dbgMasks", "dmask", "string", 
	"List of debug masks separated by coma", 0),
    STRG(Rs_debug_level, "dbgLevel", "dlevel", "string", 
	"Name of number of the debug level", 0),

#ifdef USE_FIFO
    BOOL( "useFifo", NULL, Opt_useFifo, NULL ),
#endif/*USE_FIFO*/
    INFO("e", "command arg ...", "command to execute")
};

/* Previously set options */
static uint32_t	pSetOpts[ MAX_OPTION_ARRAY ] = { 0u, 0u, 0u, 0u };

#undef INFO
#undef STRG
#undef RSTRG
#undef SWCH
#undef BOOL
/*}}} */

static const char releasestring[] = "Mrxvt v" VERSION "\n";
static const char optionsstring[] = "Options: "
#if defined(BACKGROUND_IMAGE)
# ifdef HAVE_LIBXPM
    "XPM,"
# endif
# ifdef USE_JPEG
    "Jpeg,"
# endif
# ifdef USE_JPEG
    "PNG,"
# endif
#endif
#if defined(TRANSPARENT)
    "transparent,"
#endif
    "fade,"
#if defined(BACKGROUND_IMAGE) || defined(TRANSPARENT)
# if defined(TINTING_SUPPORT)
    "tint,"
# endif
#endif
#if defined(TEXT_SHADOW)
    "textshadow,"
#endif
#if defined(UTMP_SUPPORT)
    "utmp,"
#endif
#if defined(HAVE_MENUBAR)
    "menubar,"
#endif
#if defined(USE_XIM)
    "XIM,"
#endif
#if defined(MULTICHAR_SET)
    "multichar_languages,"
#endif
    "scrollbars="
#if !defined(HAVE_SCROLLBARS)
    "NONE"
#else
# if defined(RXVT_SCROLLBAR)
    "rxvt"
#  if defined(NEXT_SCROLLBAR) || defined(XTERM_SCROLLBAR) || defined(SGI_SCROLLBAR) || defined(PLAIN_SCROLLBAR)
    "+"
#  endif
# endif	/* RXVT_SCROLLBAR */
# if defined(NEXT_SCROLLBAR)
    "NeXT"
#  if defined(XTERM_SCROLLBAR) || defined(SGI_SCROLLBAR) || defined(PLAIN_SCROLLBAR)
    "+"
#  endif
# endif	/* NEXT_SCROLLBAR */
# if defined(XTERM_SCROLLBAR)
    "xterm"
#  if defined(SGI_SCROLLBAR) || defined(PLAIN_SCROLLBAR)
    "+"
#  endif
# endif	/* XTERM_SCROLLBAR */
# if defined(SGI_SCROLLBAR)
    "sgi"
#  if defined(PLAIN_SCROLLBAR)
    "+"
#  endif
# endif	/* SGI_SCROLLBAR */
# if defined(PLAIN_SCROLLBAR)
    "plain"
# endif	/* PLAIN_SCROLLBAR */
#endif	/* HAVE_SCROLLBARS */
    ","
#ifdef XFT_SUPPORT
    "xft,"
#endif
#if defined(GREEK_SUPPORT)
    "Greek,"
#endif
#if defined(NO_BACKSPACE_KEY)
    "no_backspace,"
#endif
#if defined(NO_DELETE_KEY)
    "no_delete,"
#endif
#ifdef OUR_STRINGS
    "strings,"
#endif
#ifdef OUR_MALLOC
    "fast memory,"
#endif
#if !defined(NO_FRILLS)
    "frills,"
#endif
#if !defined(NO_LINESPACE)
    "linespace,"
#endif
#if defined(PREFER_24BIT)
    "24bit,"
#endif
#if defined(SELECTION_SCROLLING)
    "selectionscrolling,"
#endif
#if defined(TTY_256COLOR)
    "256colour,"
#endif
#if defined(CURSOR_BLINK)
    "cursorBlink,"
#endif
#if defined(POINTER_BLANK)
    "pointerBlank,"
#endif
#ifdef HAVE_X11_SM_SMLIB_H
    "session management,"
#endif
#if defined(NO_RESOURCES)
    "NoResources"
#else
    "Resources"
#endif
    "\nUsage: ";	/* Usage */


#define INDENT 24


/*
 * Print usage and exit.
 */
/* EXTPROTO */
void
rxvt_usage(int type)
{
    unsigned int    i, col;

    write(STDOUT_FILENO, releasestring, sizeof(releasestring) - 1);
    write(STDOUT_FILENO, optionsstring, sizeof(optionsstring) - 1);
    write(STDOUT_FILENO, APL_NAME, sizeof(APL_NAME) - 1);

    switch (type)
    {
	case 0:		/* brief listing */
	    fprintf(stdout, " [-help] [--help]\n");
	    for (col = 1, i = 0; i < optList_size(); i++)
		if (NOT_NULL(optList[i].desc))
		{
		    int		 len = 0;

		    if (!optList_isBool(i))
		    {
			len = optList_STRLEN(i);
			if (len > 0)
			    len++;  /* account for space */
		    }
		    assert(NOT_NULL(optList[i].opt));

		    len += 4 + STRLEN(optList[i].opt) +
			(optList_isBool(i) ? 2: 0);
		    col += len;
		    if (col > 79)	    /* assume regular width */
		    {
			putc('\n', stdout);
			col = 1 + len;
		    }
		    fprintf(stdout, " [-%s%s", (optList_isBool(i) ?
			"/+" : ""), optList[i].opt);
		    if (optList_STRLEN(i))
			fprintf(stdout, " %s]", optList[i].arg);
		    else
			fprintf(stdout, "]");
		}
	    break;

	case 1:		/* full command-line listing */
	    fprintf(stdout, " [options] [-e command args]\n\n"
		"where options include:\n");
	    for (i = 0; i < optList_size(); i++)
		if (NOT_NULL(optList[i].desc))
		{
		    assert(NOT_NULL(optList[i].opt));

		    fprintf(stdout, "  %s%s %-*s%s%s\n",
			(optList_isBool(i) ? "-/+" : "-"), optList[i].opt,
			(INDENT - STRLEN(optList[i].opt)
			 + (optList_isBool(i) ? 0 : 2)),
			(optList[i].arg ? optList[i].arg : ""),
			(optList_isBool(i) ? "turn on/off " : ""),
			optList[i].desc);
		}
	    fprintf(stdout, "\n  --help to list long-options");
	    break;

	case 2:		/* full resource listing */
	    fprintf(stdout,
		" [options] [-e command args]\n\n"
		"where resources (long-options) include:\n");

#ifdef DEBUG
	    /*
	     * Print short options, long options and descriptions so we can
	     * generate man page.
	     */
	    for (i = 0; i < optList_size(); i++)
		fprintf( stdout, "%3d. %s   %s	%s  %s	%d\n", i,
			optList[i].kw ? optList[i].kw : "NullKW",
			optList[i].opt ? optList[i].opt : "NullOpt",
			!optList_isBool(i) ? optList[i].arg : "boolean",
			optList[i].desc ? optList[i].desc : "NullDsc",
			optList[i].multiple);

#else
	    for (i = 0; i < optList_size(); i++)
		if (NOT_NULL(optList[i].kw))
		    fprintf(stdout, "  %s: %*s%s\n",
			optList[i].kw,
			(INDENT - STRLEN(optList[i].kw)), "", /* XXX */
			(optList_isBool(i) ? "boolean" : optList[i].arg));
#endif
	    fprintf(stdout, "\n  -help to list options");
	    break;
    }	/* switch */

    fprintf(stdout, "\n\n");
    exit(EXIT_FAILURE);
    /* NOTREACHED */
}


/* INTPROTO */
int
num_tabs( int len )
{
    return (len >=0 && len < 40) ? (40 - (len - (len%8)) ) / 8 : 0;
}

/* EXTPROTO */
int
rxvt_save_options (rxvt_t* r, const char* filename)
{
    int		i;
    FILE*	pf = fopen (filename, "w");

    const char	*name = r->h->rs[Rs_name] ? r->h->rs[Rs_name] : APL_NAME;

    char	*tabs="\t\t\t\t\t";

    if (IS_NULL(pf))
	return 0;
    
    for (i = 0; i < optList_size(); i ++)
    {
	if(
	     IS_NULL( optList[i].kw ) ||	/* Options without a keyword
						   can't be written */
	     ( optList[i].doff == -1 && !optList_isBool(i) )
						/* Boolean options don't require
						 * a data offset. */
	  )
	    continue;

	if( optList[i].multiple)
	{
	    register int    j;

	    for (j = 0; j < MAX_PROFILES; j ++)
	    {
		if (r->h->rs[optList[i].doff + j])
		    fprintf( pf, "%s.profile%d.%s:%.*s%s\n", name,
			j, optList[i].kw,
			num_tabs( STRLEN(name) + sizeof(".profile") + 2
				    + STRLEN( optList[i].kw ) ), tabs,
			r->h->rs[optList[i].doff +j]);
	    }
	}
	else if (optList_isBool(i))
	{
	    int	    bval;
	    char*   OnOff[2] = {"False", "True"};

	    bval = ISSET_OPTION(r, optList[i].flag) ? 1 : 0;

	    if (optList_isReverse(i))
		bval = !bval;
	    fprintf( pf, "%s.%s:%.*s%s\n", name,
		optList[i].kw,
		num_tabs( STRLEN(name) + 1 + STRLEN(optList[i].kw) + 1 ), tabs,
		OnOff[bval] );
	}
	else if (r->h->rs[optList[i].doff])
	{
	    fprintf( pf, "%s.%s:%.*s%s\n", name,
		optList[i].kw,
		num_tabs( STRLEN(name) + 1 + STRLEN(optList[i].kw) + 1 ), tabs,
		r->h->rs[optList[i].doff] );
	}
    }

    fputs( "\n\n# vim: set ft=mrxvtrc :\n", pf );

    fclose (pf);
    return 1;
}


/*
 * Process command line options.
 */
/* EXTPROTO */
void
rxvt_get_options(rxvt_t *r, int argc, const char *const *argv)
{
    int		 i, bad_option = 0;
    static const char On[3] = "ON", Off[4] = "OFF";


    rxvt_dbgmsg ((DBG_DEBUG, DBG_RESOURCE, "rxvt_get_options()\n"));

    for (i = 1; i < argc; i++)
    {
	unsigned int	entry, longopt = 0;
	const char	*flag, *opt;
	int		profileNum;

	opt = argv[i];
	profileNum = 0;	/* initialize profileNum to 0 by default */

	rxvt_msg (DBG_INFO, DBG_RESOURCE, "argv[%d] = %s: \n", i, opt);
	if (*opt == '-')
	{
	    flag = On;
	    if (*++opt == '-')
		longopt = *opt++;   /* long option */
	}
	else if (*opt == '+')
	{
	    flag = Off;
	    if (*++opt == '+')
		longopt = *opt++;   /* long option */
	}
	else
	{
	    bad_option = 1;
	    rxvt_msg (DBG_ERROR, DBG_RESOURCE, "bad option \"%s\"\n", opt);
	    continue;
	}

	if (!STRCMP(opt, "help"))
	    rxvt_usage(longopt ? 2 : 1);
	if (!STRCMP(opt, "h"))
	    rxvt_usage(0);

	/* feature: always try to match long-options */
	for (entry = 0; entry < optList_size(); entry++)
	{
	    char	    buflong[256];
	    char	    bufshort[128];

	    /* initialize it to empty string */
	    buflong[0]	= '\0';
	    bufshort[0] = '\0';

	    /*
	     * Get the long option name in buflong, or the short option name in
	     * bufshort.
	     */
	    if ( optList[entry].multiple )
	    {
		int offset = 0;

		/*
		 * For backward compatibility, accept vt%d style options.
		 */
		offset = rxvt_str_match( opt, "vt" );
		if( offset == 0 )
		    offset = rxvt_str_match( opt, "profile" );

		/*
		 * Copy --profile%d.resource or --vt%d.resource into buflong and
		 * bufshort.
		 */
		if( offset )
		{
		    profileNum = atoi( opt + offset );
		    if( profileNum < 0 || profileNum >= MAX_PROFILES )
		    {
			entry = optList_size();
			break;	/* out of range, jump to bad option */
		    }

		    snprintf( buflong, sizeof(buflong)-1, "%.*s%d.%s",
			    offset, opt, profileNum, optList[entry].kw );
		    buflong[sizeof(buflong)-1] = '\0';

		    snprintf( bufshort, sizeof(bufshort)-1, "%.*s%d.%s",
			    offset, opt, profileNum, optList[entry].opt );
		    bufshort[sizeof(bufshort)-1] = '\0';

		    rxvt_dbgmsg ((DBG_DEBUG, DBG_RESOURCE, "Matched profile=%d buflong=%s bufshort=%s\n", profileNum, buflong, bufshort ));
		}

		/* If no profile number is specified, use 0 by default */
		else
		{
		    STRNCPY( buflong, optList[entry].kw, sizeof(buflong) - 1 );
		    buflong[ sizeof(buflong)-1 ] = '\0';

		    STRNCPY( bufshort, optList[entry].opt, sizeof(bufshort)-1 );
		    bufshort[ sizeof(bufshort)-1 ] = '\0';

		    profileNum = 0;

		    rxvt_dbgmsg ((DBG_DEBUG, DBG_RESOURCE, "Matched default buflong=%s bufshort=%s\n", buflong, bufshort ));
		}
	    }
	    else if (optList[entry].kw)
	    {
		STRNCPY (buflong, optList[entry].kw, sizeof(buflong)-1);
		buflong[sizeof(buflong)-1] = '\0';
		if (optList[entry].opt)
		{
		    STRNCPY (bufshort, optList[entry].opt,
			sizeof(bufshort)-1);
		    bufshort[sizeof(bufshort)-1] = '\0';
		}
	    }
	    else if (optList[entry].opt)
	    {
		/* here NULL == optList[entry].kw */
		STRNCPY (bufshort, optList[entry].opt,
		    sizeof(bufshort)-1);
		bufshort[sizeof(bufshort)-1] = '\0';
	    }


	    /* Check to see if option matches */
	    if(
		( optList[entry].kw && !STRCMP(opt, buflong) )
		|| (
		     !longopt && optList[entry].opt
		     && !STRCASECMP(opt, bufshort)
		   )
	      )
		break;
	} /* for */

	/* Found option */
	if (entry < optList_size())
	{
	    if (optList_isReverse(entry))
		flag = (flag == On) ? Off : On;

	    /* string value */
	    if( optList_STRLEN(entry) )
	    {
		const char   *str = argv[++i];

		rxvt_dbgmsg ((DBG_DEBUG, DBG_RESOURCE, "string (%s,%s) = ", optList[entry].opt ? optList[entry].opt : "nil", optList[entry].kw ? optList[entry].kw : "nil"));

		if (flag == On && str && (optList[entry].doff != -1))
		{
		    rxvt_dbgmsg ((DBG_DEBUG, DBG_RESOURCE, "\"%s\"\n", str));
		    r->h->rs[optList[entry].doff + profileNum] = str;
		    /*
		     * special cases are handled in main.c:main() to allow X
		     * resources to set these values before we settle for
		     * default values
		     */
		}
		else
		    rxvt_dbgmsg ((DBG_DEBUG, DBG_RESOURCE, "???\n"));
	    }

	    /* boolean value */
	    else
	    {
		rxvt_dbgmsg ((DBG_DEBUG, DBG_RESOURCE, "boolean (%s,%s) = %s\n", optList[entry].opt, optList[entry].kw, flag ));

		if( flag == On )
		    SET_OPTION( r, optList[entry].flag );
		else
		    UNSET_OPTION( r, optList[entry].flag );

		/*
		 * Remember that we've set this option, so it does not get reset
		 * later.
		 */
		SET_ARRAYOPT( pSetOpts, optList[entry].flag );
	    }
	}

	/* No option found */
	else
	{
	    if ( rxvt_str_match( opt, "macro."))
	    {
		const char  *str = argv[++i];

		if (NOT_NULL(str))
		    rxvt_parse_macros( r, opt + sizeof( "macro." ) - 1,
			    str, 0); /* priority 0 = highest. All previous
					macros get replaced. */
	    }
	    else
	    {
		/*
		 * various old-style options, just ignore
		 * Obsolete since about Jan 96,
		 * so they can probably eventually be removed
		 */
		const char   *msg = "bad";

		if (longopt)
		{
		    opt--;
		    bad_option = 1;
		}
		else if (!STRCMP(opt, "7") || !STRCMP(opt, "8")
#ifdef GREEK_SUPPORT
		   /* obsolete 12 May 1996 (v2.17) */
		   || rxvt_str_match(opt, "grk")
#endif
		)
		    msg = "obsolete";
		else
		    bad_option = 1;

		rxvt_msg (DBG_ERROR, DBG_RESOURCE, "%s option \"%s\"", msg, --opt);
	    }
	}
    }

    if (bad_option)
    {
	/* Printing the usage is too verbose ... */
#if 0
	rxvt_usage(0);
#endif

	rxvt_msg (DBG_ERROR, DBG_RESOURCE,
		"Use -h, -help or --help to get help\n" );
	exit( EXIT_FAILURE );
    }

    rxvt_dbgmsg ((DBG_DEBUG, DBG_RESOURCE, "rxvt_get_options() done.\n"));
}


#ifndef NO_RESOURCES
/*
 * Read resources from a file. "name" is the class name to use.
 */
/* INTPROTO */
void
rxvt_get_xdefaults(rxvt_t *r, FILE *stream, const char *name,
	macro_priority_t priority)
{
    unsigned int    len, lenstr;
    char TAINTED *  str;
    char	    buffer[256];

    rxvt_msg (DBG_INFO, DBG_RESOURCE, "rxvt_get_xdefaults (%s)\n", name);

    if (IS_NULL(stream))
	return;
    len = STRLEN(name);

    while (NOT_NULL(str = fgets(buffer, sizeof(buffer), stream)))
    {
	unsigned int	entry, n;

	while (*str && isspace((int) *str))
	    str++;	/* leading whitespace */

	if(
	    (str[len] != '*' && str[len] != '.')
	    || (len && STRNCMP(str, name, len))
	  )
	    continue;
	str += (len + 1);   /* skip `name*' or `name.' */
	lenstr = STRLEN( str );

	{
	    /*
	     * replace '*' with '.', but stop at ':'! bug reported by
	     * afo@zlug.org
	     */
	    char*   ptr = str;
	    while (*ptr && *ptr != ':')
	    {
		if ('*' == *ptr)
		    *ptr = '.';
		ptr ++;
	    }
	}

	if( !rxvt_parse_macros( r, str, NULL, priority ) )
	{
	    for (entry = 0; entry < optList_size(); entry++)
	    {
		/* const char*	kw = optList[entry].kw; */
		char	kw[256];
		int	profileNum = 0;	/* default is no offset */

		if (IS_NULL(optList[entry].kw))
		    continue;
		STRNCPY (kw, optList[entry].kw, sizeof(kw)-1);
		kw[sizeof(kw)-1] = (char) 0;

		if( optList[entry].multiple )
		{
		    int offset = 0;

		    /*
		     * For backward compatibility, accept vt%d style options.
		     */
		    offset = rxvt_str_match( str, "vt" );
		    if( offset == 0 )
			offset = rxvt_str_match( str, "profile" );

		    /*
		     * Copy profile%d.resource into kw.
		     */
		    if( offset )
		    {
			char	buf[256];

			profileNum = atoi( str + offset );
			if (profileNum < 0 || profileNum >= MAX_PROFILES)
			    continue;	/* out of range */

			snprintf( buf, sizeof(buf)-1,
				"%.*s%d.%s", offset, str, profileNum, kw );
			buf[sizeof(buf)-1] = '\0';

			STRNCPY (kw, buf, sizeof(kw)-1);
			kw[sizeof(kw)-1] = '\0';

			rxvt_dbgmsg ((DBG_DEBUG, DBG_RESOURCE, "Matched profile=%d kw=%s\n", profileNum, kw));
		    }

		    /*
		     * No profile%d specified. Match resource on it's own, and
		     * use profile 0.
		     */
		    else
		    {
			profileNum = 0;

			rxvt_dbgmsg ((DBG_DEBUG, DBG_RESOURCE, "Matched default kw=%s for option %s", kw, str));
		    }
		}

		n = STRLEN(kw);
		if( n < lenstr && str[n] == ':' && rxvt_str_match(str, kw) )
		{
		    /* skip `keyword:' */
		    str += (n + 1);
		    rxvt_str_trim(str);
		    n = STRLEN(str);

		    /*
		     * Only set the resource if it's not already set. We read
		     * config file items in reverse order (most significant to
		     * least significant). So if a resource is already set, then
		     * we should not over ride it.
		     */
		    if( optList_isBool( entry ) )
		    {
			/*
			 * Boolean options: We dont' need to duplicate the
			 * argument of the resource string. That only wastes
			 * memory.
			 */
			if( NOTSET_ARRAYOPT( pSetOpts, optList[entry].flag ) )
			{
			    /*
			     * Only set this option if we've not already set it.
			     */
			    int	s;

			    s = STRCASECMP(str, "true") == 0 ||
				STRCASECMP(str, "yes") == 0 ||
				STRCASECMP(str, "on") == 0 ||
				STRCASECMP(str, "1") == 0;
			    if (optList_isReverse(entry))
				s = !s;

			    if (s)
				SET_OPTION(r, optList[entry].flag);
			    else
				UNSET_OPTION(r, optList[entry].flag);

			    /* Remember we've already set this option */
			    SET_ARRAYOPT( pSetOpts, optList[entry].flag );
			}
		    }

		    else if( IS_NULL( r->h->rs[ optList[entry].doff
							    + profileNum ] ) )
		    {
			/*
			 * Regular option, that has not previously been set.
			 * The argument needs to be duplicated and stored.
			 */
			r->h->rs[optList[entry].doff+profileNum] =
			    n && !optList_isBool(entry) ?
				STRDUP( str ) : emptyResource;
		    }

		    break; /* out of for( entry=0 ... ) */

		}   /* if( str[n] =':' ...) */
	    }	/* for (entry = 0...) */
	}   /* if( !rxvt_parse_macros ... ) */
    }	/* while() */

    rewind( stream );
}
#endif		    /* NO_RESOURCES */


/*
 * Open resource files and call rxvt_get_xdefaults() for each one in turn.
 */
/* ARGSUSED */
/* EXTPROTO */
void
rxvt_extract_resources (
	rxvt_t	    *r,
	Display	    *display __attribute__((unused)),
	const char  *name )
{
#ifndef NO_RESOURCES
    /* get resources the hard way, but save lots of memory */
    FILE	   *fd = NULL;
    char	   *home;

    macro_priority_t    priority = 1;   /* As resources get more general, we
					   increment this value so that
					   resources with lower priority are not
					   overwritten */

# if defined XAPPLOADDIR
#  if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
    /* Compute the path of the possibly available localized Rxvt file */ 
    char	   *localepath = NULL;

    if (NOT_NULL(r->h->locale))
    {
	/* XXX: must limit length of string */

	localepath = rxvt_malloc(256); 
	sprintf( localepath, XAPPLOADDIRLOCALE "/" APL_SUBCLASS,
	    (int) ( 258 - sizeof(XAPPLOADDIRLOCALE) - sizeof(APL_SUBCLASS)),
	    r->h->locale);  /* 258 = 255 + 4 (-.*s) - 1 (/) */
    }
#  endif
# endif /* XAPPLOADDIR */


    /* open user supplied config file first */
    if (NOT_NULL(r->h->rs[Rs_confFile]))
	fd = fopen( r->h->rs[Rs_confFile], "r" );

    if (IS_NULL(fd) && NOT_NULL(home = getenv("HOME")))
    {
	int	i, len = STRLEN(home) + 2;
	char*	f = NULL;

	/* possible integer overflow? */
	assert (len > 0);
	for( i = 0; i < (sizeof(xnames) / sizeof(xnames[0])); i++ )
	{
	    /* possible integer overflow? */
	    assert (len + STRLEN(xnames[i]) >= len);
	    f = rxvt_realloc( f, ( len + STRLEN(xnames[i]) ) * sizeof(char) );

	    sprintf( f, "%s/%s", home, xnames[i] );

	    if (NOT_NULL(fd = fopen(f, "r")))
		break;
	}
	rxvt_free(f);
    }

    /*
     * The normal order to match resources is the following:
     *
     *	    @ global resources (partial match, ~/.Xdefaults)
     * 	    @ application file resources (XAPPLOADDIR/Rxvt)
     * 	    @ class resources (~/.Xdefaults)
     * 	    @ private resources (~/.Xdefaults)
     *
     * However, for the hand-rolled resources, the matching algorithm checks if
     * a resource string value has already been allocated and won't overwrite it
     * with (in this case) a less specific resource value.
     *
     * This avoids multiple allocation.  Also, when we've called this routine
     * command-line string options have already been applied so we needn't to
     * allocate for those resources.
     *
     * So, search in resources from most to least specific.
     *
     * Also, use a special sub-class so that we can use either or both of
     * "XTerm" and "Rxvt" as class names.
     */

    rxvt_get_xdefaults(r, fd, name, priority++);
    rxvt_get_xdefaults(r, fd, APL_SUBCLASS, priority++);
    rxvt_get_xdefaults(r, fd, APL_CLASS, priority++);
#if 0
    rxvt_get_xdefaults(r, fd, "", priority++);  /* partial match */
#endif
    if (NOT_NULL(fd))
	fclose(fd);

#  if defined(XAPPLOADDIR) && defined(USE_XAPPLOADDIR)
    {
	FILE*	ad = NULL;

#   if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
	if (IS_NULL(localepath) || IS_NULL(ad = fopen(localepath, "r")))
#   endif
	    ad = fopen(XAPPLOADDIR "/" APL_SUBCLASS, "r");
	if (NOT_NULL(ad))
	{
	    rxvt_get_xdefaults(r, ad, APL_SUBCLASS, priority++);
#if 0
	    /*
	     * 2006-05-23 gi1242: If we don't use the X resource database, we
	     * don't have to match the empty class.
	     */
	    rxvt_get_xdefaults(r, ad, "", priority++);
#endif
	    fclose(ad);
	}
    }
#  endif	    /* XAPPLOADDIR */


    /*
     * Now read config from system wide config file.
     */
    if (NOTSET_OPTION(r, Opt2_noSysConfig) &&
        NOT_NULL(fd = fopen( PKG_CONF_DIR "/mrxvtrc", "r")))
    {
	rxvt_get_xdefaults( r, fd, APL_SUBCLASS, priority++ );
	fclose(fd);
    }

    /*
     * Unset resources that point to emptyResource
     *
     * 2006-06-02 gi1242: On second thought, let's leave them pointing to
     * emptyResource. That way we will be able to tell which resources have been
     * cleared by the user, and which were never set in the first place.
     */
#if 0
    {
	int i;

	for( i=0; i < NUM_RESOURCES; i++)
	{
	    if( r->h->rs[i] == emptyResource )
	    {
		rxvt_dbgmsg ((DBG_DEBUG, DBG_RESOURCE, "Setting resource #%d to NULL\n", i));
		r->h->rs[i] = NULL;
	    }
	}
    }
#endif

# if defined XAPPLOADDIR
#  if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
    /* Free the path of the possibly available localized Rxvt file */ 
    rxvt_free(localepath);
#  endif
# endif

#endif		    /* NO_RESOURCES */


    /*
     * Clear the boolean and reverse flags from Options and Options2. Otherwise
     * this will cause trouble when we want to save the options. In that case,
     * the boolean flag is set for each boolean options. Then if we compare
     * Options(2) to the flag, we always get TRUE!
     */
    UNSET_OPTION(r, ( Opt_Reverse | IS_OPTION1 ) );
    UNSET_OPTION(r, ( Opt_Reverse | IS_OPTION2 ) );
    UNSET_OPTION(r, ( Opt_Reverse | IS_OPTION3 ) );
    UNSET_OPTION(r, ( Opt_Reverse | IS_OPTION4 ) );


    /*
     * even without resources, at least do this setup for command-line options
     * and command-line long options
     */
#ifdef MULTICHAR_SET
    if (r->h->rs[Rs_multichar_encoding])
	rxvt_set_multichar_encoding(r, r->h->rs[Rs_multichar_encoding]);
    else
    {
	char*    enc;

	if (NOT_NULL(enc = rxvt_get_encoding_from_locale (r)))
	    rxvt_set_multichar_encoding (r, enc);
	else
	    rxvt_set_multichar_encoding(r, MULTICHAR_ENCODING);
    }
#endif

#ifdef GREEK_SUPPORT
    /* this could be a function in grkelot.c */
    /* void set_greek_keyboard (const char * str); */
    if (r->h->rs[Rs_greek_keyboard])
    {
	if (!STRCMP(r->h->rs[Rs_greek_keyboard], "iso"))
	    greek_setmode(GREEK_ELOT928);   /* former -grk9 */
	else if (!STRCMP(r->h->rs[Rs_greek_keyboard], "ibm"))
	    greek_setmode(GREEK_IBM437);    /* former -grk4 */
    }
    {
	KeySym	      sym;

	if (r->h->rs[Rs_greektoggle_key] &&
	    ((sym = XStringToKeysym(r->h->rs[Rs_greektoggle_key]))!=0))
	    r->h->ks_greekmodeswith = sym;
    }
#endif		    /* GREEK_SUPPORT */
}
/*----------------------- end-of-file (C source) -----------------------*/
