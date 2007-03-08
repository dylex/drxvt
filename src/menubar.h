/*--------------------------------*-H-*---------------------------------*
 * File:    menubar.h
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1997,1998   mj olesen <olesen@me.QueensU.CA>
 * Copyright (c) 1997-2001   Geoff Wing <gcw@pobox.com>
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

#ifndef __MENUBAR_H__
#define __MENUBAR_H__

typedef struct {
    /* short           menuType; */ /* must not be changed; first element */
    struct menu_t  *menu;	    /* sub-menu */
} submenu_t;

typedef struct menuitem_t {
    struct menuitem_t	*prev;	/* prev menu-item */
    struct menuitem_t	*next;	/* next menu-item */
    unsigned char	*name;	/* character string displayed */
    unsigned char	*name2;	/* character string displayed (right) */
    unsigned short	len;	/* strlen (name) */
    unsigned short	len2;	/* strlen (name) */
    struct {
	short           itemType;   /* must not be changed; first element */
	union {
	    action_t        action;
	    submenu_t       submenu;
	};
    } entry;
} menuitem_t;

enum menuitem_t_action {
    MenuLabel=0,
    MenuSubMenu,
    MenuItem
};

typedef struct menu_t {
    struct menu_t   *parent;/* parent menu */
    struct menu_t   *prev;  /* prev menu */
    struct menu_t   *next;  /* next menu */
    menuitem_t	    *head;  /* double-linked list */
    menuitem_t	    *tail;  /* double-linked list */
    menuitem_t	    *item;  /* current item */
    unsigned char   *name;  /* menu name */
    unsigned short  len;    /* strlen (name) */
    Window          win;    /* window of the menu */
    short           x;	    /* x location [pixels] relative to terminal win */
    short           y;	    /* y location [pixels] relative to terminal win */
    unsigned short  lwidth, /* Max width of the left text items */
		    rwidth, /* Max width of the right text items */
		    height; /* Window height (pixels) */
} menu_t;

typedef struct bar_t {
    menu_t         *head, *tail;    /* double-linked list of menus */
    unsigned char  *title;	    /* title to put in the empty menuBar */
#define NARROWS	4
    action_t        arrows[NARROWS];
} bar_t;


#define HSPACE	    1	/* one space */
#define isSeparator(name)   ((name)[0] == '\0')
#define HEIGHT_SEPARATOR    (SHADOW + 1)

#ifdef XFT_SUPPORT
# define HSPACE_PIXEL		(r->TermWin.pwidth)
# define HEIGHT_TEXT		(r->TermWin.pheight + 2)
#else
# define HSPACE_PIXEL		(r->TermWin.fwidth)
# define HEIGHT_TEXT		(r->TermWin.fheight + 2)
#endif

#define MENUBAR_ARROW_WIDTH HSPACE_PIXEL

#define MENU_DELAY_USEC	250000	/* 1/4 sec */


#define SEPARATOR_NAME	    "-"
#define MENUITEM_BEG	    '{'
#define MENUITEM_END	    '}'
#define COMMENT_CHAR	    '#'

#define DOT "."
#define DOTS	".."

/*
 * Values for r->h->showingMenu
 */
#define NOMENU		(0)
#define POPUP_MENU	(1)
#define PULLDOWN_MENU	(1 << 1)

#endif	/* __MENUBAR_H__ */
/*----------------------- end-of-file (H source) -----------------------*/
