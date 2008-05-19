/*--------------------------------*-C-*---------------------------------*
 * File:    menubar.c
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1997,1998   mj olesen <olesen@me.QueensU.CA>
 * Copyright (c) 2004        Jingmin Zhou <jimmyzhou@users.sourceforge.net>
 * Copyright (c) 2005-6      Gautam Iyer <gi1242@users.sourceforge.net>
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

#define NEWARGLIM 500


#ifdef HAVE_MENUBAR


#define CHOOSE_GC_FG(R, PIXCOL)	\
    XSetForeground ((R)->Xdisplay, (R)->menuBar.gc, (PIXCOL))

#define TRIANGLE_WIDTH	(HEIGHT_TEXT * 3 / 5 - 2 * SHADOW)


/*--------------------------------------------------------------------*
 *         BEGIN `INTERNAL' ROUTINE PROTOTYPES                        *
 *--------------------------------------------------------------------*/
menuitem_t*   rxvt_menuitem_find          (const menu_t*, const unsigned char*);
void          rxvt_menuitem_free          (rxvt_t*, menu_t*, menuitem_t*);
/* int           rxvt_action_type            (action_t*, unsigned char*); */
/* int           rxvt_action_dispatch        (rxvt_t*, action_t*); */
int           rxvt_menuarrow_find         (char);
void          rxvt_menuarrow_free         (rxvt_t*, unsigned char);
void          rxvt_menuarrow_add          (rxvt_t*, unsigned char*);
menuitem_t*   rxvt_menuitem_add           (rxvt_t*, menu_t*, const unsigned char*, const unsigned char*, const unsigned char*);
unsigned char* rxvt_menu_find_base         (rxvt_t*, menu_t**, unsigned char*);
menu_t*       rxvt_menu_delete            (rxvt_t*, menu_t*);
menu_t*       rxvt_menu_add               (rxvt_t*, menu_t*, unsigned char*);
void          rxvt_drawbox_menubar        (rxvt_t*, int, int, int);
void          rxvt_menubar_draw_triangle  (rxvt_t*, int, int, int);
void          rxvt_drawbox_menuitem       (rxvt_t*, int, int);
void	      rxvt_build_tablist	  (rxvt_t*, menu_t *);
void          rxvt_menu_display           (rxvt_t*, void (*update)(rxvt_t*));
void          rxvt_menu_hide_all          (rxvt_t*);
void          rxvt_menu_hide              (rxvt_t*);
void          rxvt_menu_clear             (rxvt_t*, menu_t*);
void          rxvt_menubar_clear          (rxvt_t*);
void          rxvt_draw_arrows            (rxvt_t*, int, int);
void          rxvt_menubar_select         (rxvt_t*, XButtonEvent*);
void	      rxvt_menubar_draw_labels	  (rxvt_t*);
void	      resizeSubMenus		  (rxvt_t*, menu_t*);
#ifdef DEBUG
void          rxvt_print_menu_ancestors   (menu_t*);
void          rxvt_print_menu_descendants (menu_t*);
#endif
/*--------------------------------------------------------------------*
 *         END   `INTERNAL' ROUTINE PROTOTYPES                        *
 *--------------------------------------------------------------------*/


static const struct {
    const char	  name;	/* (l)eft, (u)p, (d)own, (r)ight */
    const unsigned char str[5];	/* str[0] = STRLEN (str+1) */
} Arrows[NARROWS] = {
    { 'l', "\003\033[D" },
    { 'u', "\003\033[A" },
    { 'd', "\003\033[B" },
    { 'r', "\003\033[C" }
};

#ifdef XFT_SUPPORT
#define PTEXTWIDTH( R, s, len)				    \
    ( (ISSET_OPTION(R, Opt_xft) && r->TermWin.xftpfont ) ?  \
	( xftPfontTextWidth( (R), (s), (len)) )	:	    \
	Width2Pixel(len))

int
xftPfontTextWidth( rxvt_t *r, const unsigned char *s, int len)
{
    if( s && len )
    {
	XGlyphInfo ginfo;

	XftTextExtents8( r->Xdisplay, r->TermWin.xftpfont, s, len, &ginfo);
	return ginfo.width;
    }
    else
	return 0;
}

# else
# define PTEXTWIDTH( r, s, len) (Width2Pixel((len)))
#endif

#define MENUWIDTH( menu )			    \
    ( MENU_ITEM_WIDTH( (menu)->lwidth, (menu)->rwidth ) )

#define MENU_ITEM_WIDTH( item_lwidth, item_rwidth )		    \
    ((item_lwidth) + (item_rwidth) + 2*SHADOW + 3*HSPACE_PIXEL)


/*
 * find an item called NAME in MENU
 */
/* INTPROTO */
menuitem_t*
rxvt_menuitem_find(const menu_t *menu, const unsigned char *name)
{
    menuitem_t	 *item;

#ifdef DEBUG
    assert(NOT_NULL(name));
    assert(NOT_NULL(menu));
#endif

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menuitem_find(%s) ... ", name));

    /* find the last item in the menu, this is good for separators */
    for (item = menu->tail; NOT_NULL(item); item = item->prev)
    {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "%d", item->entry.itemType));

	if (item->entry.itemType == MenuSubMenu)
	{
	    if (!STRCMP(name, (item->entry.submenu.menu)->name))
		break;
	}
	else if ((isSeparator(name) && isSeparator(item->name))
	       || !STRCMP(name, item->name))
	    break;

	rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, ","));
    }

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "Returning %p\n", item));
    return item;
}

/*
 * unlink ITEM from its MENU and free its memory
 */
/* INTPROTO */
void
rxvt_menuitem_free(rxvt_t *r, menu_t *menu, menuitem_t *item)
{
    /* disconnect */
    menuitem_t	 *prev, *next;

#ifdef DEBUG
    assert(NOT_NULL(menu));
#endif
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menuitem_free()\n"));

    prev = item->prev;
    next = item->next;
    if (NOT_NULL(prev)) prev->next = next;
    if (NOT_NULL(next)) next->prev = prev;

    /* new head, tail */
    if (menu->tail == item) menu->tail = prev;
    if (menu->head == item) menu->head = next;

    switch (item->entry.itemType)
    {
	case MenuSubMenu:
	    rxvt_menu_delete(r, item->entry.submenu.menu);
	    SET_NULL(item->entry.submenu.menu);
	    break;

	case MenuItem:
	    rxvt_free(item->entry.action.str);
	    SET_NULL(item->entry.action.str);
	    break;
    }

    if (NOT_NULL(item->name)) rxvt_free(item->name);
    if (NOT_NULL(item->name2)) rxvt_free(item->name2);
    rxvt_free(item);
}


/* return the arrow index corresponding to NAME */
/* INTPROTO */
int
rxvt_menuarrow_find(char name)
{
    int		 i;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menuarrow_find()\n"));

    for (i = 0; i < NARROWS; i++)
    if (name == Arrows[i].name)
	return i;
    return -1;
}

/* free the memory associated with arrow NAME of the current menubar */
/* INTPROTO */
void
rxvt_menuarrow_free(rxvt_t *r, unsigned char name)
{
    int		 i;
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menuarrow_free()\n"));

    if (name)
    {
	i = rxvt_menuarrow_find(name);
	if (i >= 0)
	{
	    action_t	   *act = &(r->h->MenuBar.arrows[i]);

	    if (act->len)
	    {
		assert (NOT_NULL(act->str));
		rxvt_free(act->str);
		SET_NULL(act->str);
		act->len = 0;
	    }

	    act->type = MenuLabel;
	}
    }
    else
    {
	/* Free all arrows */
	/*
	 * 2006-02-22 gi1242 XXX: If Arrows[i].name is NULL this will lead to an
	 * infinite loop. WTF.
	 */
	for (i = 0; i < NARROWS; i++)
	    rxvt_menuarrow_free(r, Arrows[i].name);
    }
}

/* INTPROTO */
void
rxvt_menuarrow_add(rxvt_t *r, unsigned char *string)
{
    int		 	i;
    unsigned short	xtra_len;
    unsigned char	*p;

    struct
    {
	unsigned char	*str;
	unsigned short	len;
    }
	    beg = { NULL, 0 },
	    end = { NULL, 0 },
	    *cur,
	    parse[NARROWS];

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menuarrow_add()\n"));
    MEMSET(parse, 0, sizeof(parse));

    for (p = string; NOT_NULL(p) && *p; string = p)
    {
    p = (string + 3);
    switch (string[1])
    {
	case 'b':
	    cur = &beg;
	    break;
	case 'e':
	    cur = &end;
	    break;

	default:
	    i = rxvt_menuarrow_find(string[1]);
	    if (i >= 0)
	    cur = &(parse[i]);
	    else
	    continue;	/* not found */
	    break;
    }

    string = p;
    cur->str = string;
    cur->len = 0;

    if (cur == &end)
    {
	p = (unsigned char*) STRCHR(string, '\0');
    }
    else
    {
	unsigned char	*next = string;

	while (1)
	{
	    p = (unsigned char*) STRCHR(next, '<');
	    if (NOT_NULL(p))
	    {
		if (p[1] && p[2] == '>')
		break;
		/* parsed */
	    }
	    else
	    {
		if (IS_NULL(beg.str))	/* no end needed */
		p = (unsigned char*) STRCHR(next, '\0');
		break;
	    }
	    next = (p + 1);
	}
    }

    if (IS_NULL(p))
	return;
    cur->len = (p - string);
    }

#ifdef DEBUG
    cur = &beg;
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "<b>(len %d) = %.*s\n", cur->len, (cur->str ? (char*) cur->str : "")));
    for (i = 0; i < NARROWS; i++)
    {
	cur = &(parse[i]);
	rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "<%c>(len %d) = %.*s\n", Arrows[i].name, cur->len, (cur->str ? (char*) cur->str : "")));
    }
    cur = &end;
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "<e>(len %d) = %.*s\n", cur->len, (cur->str ? (char*) cur->str : "")));
#endif

    xtra_len = (beg.len + end.len);
    for (i = 0; i < NARROWS; i++)
    {
	if (xtra_len || parse[i].len)
	    rxvt_menuarrow_free(r, Arrows[i].name);
    }

    for (i = 0; i < NARROWS; i++)
    {
	char  		str[NEWARGLIM];
	unsigned short	len;

	if (!parse[i].len)
	    continue;

	/* possible integer overflow? */
	assert (parse[i].len <= 0xffff - xtra_len - 1);
	/* str = rxvt_malloc(parse[i].len + xtra_len + 1); */

	len = 0;
	if (beg.len)
	{
	    STRNCPY(str + len, beg.str, beg.len);
	    len += beg.len;
	}
	STRNCPY(str + len, parse[i].str, parse[i].len);
	len += parse[i].len;

	if (end.len)
	{
	    STRNCPY(str + len, end.str, end.len);
	    len += end.len;
	}
	str[len] = '\0';

#ifdef DEBUG
	rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "<%c>(len %d) = %s\n", Arrows[i].name, len, str));
#endif
	rxvt_set_action( &(r->h->MenuBar.arrows[i]), str);
    }
}

/* INTPROTO */
menuitem_t   *
rxvt_menuitem_add(rxvt_t *r, menu_t *menu,
	const unsigned char *name, const unsigned char *name2,
	const unsigned char *action)
{
    menuitem_t	 *item;
    unsigned int    len;
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menuitem_add()\n"));

#ifdef DEBUG
    assert(NOT_NULL(name));
    assert(NOT_NULL(action));
#endif

    if (IS_NULL(menu)) return NULL;

    if (isSeparator(name))
    {
	/* add separator, no action */
	name	= (unsigned char *) "";
	action	= (unsigned char *) "";
    }
    else
    {
	/*
	 * add/replace existing menu item
	 */
	item = rxvt_menuitem_find(menu, name);
	if (NOT_NULL(item))
	{
	    if (NOT_NULL(item->name2) && NOT_NULL(name2))
	    {
		rxvt_free(item->name2);
		item->len2 = 0;
		SET_NULL(item->name2);
	    }
	    switch (item->entry.itemType)
	    {
		case MenuSubMenu:
		    rxvt_menu_delete(r, item->entry.submenu.menu);
		    SET_NULL(item->entry.submenu.menu);
		    break;
		    
		case MenuItem:
		    rxvt_free(item->entry.action.str);
		    SET_NULL(item->entry.action.str);
		break;
	    }
	    goto Item_Found;
	}
    }

    /* allocate a new itemect */
    item = (menuitem_t *) rxvt_malloc( sizeof( menuitem_t) );

    SET_NULL(item->entry.action.str); /* If not null will be freed by
				     rxvt_action_set */

    item->len2 = 0;
    SET_NULL(item->name2);

    len = STRLEN(name);
    /* possible integer overflow? */
    assert (len >= 0 && len + 1 >= 0);
    item->name = rxvt_malloc(len + 1);
    STRCPY(item->name, name);
    if (name[0] == '.' && name[1] != '.')
	len = 0;	/* hidden menu name */
    item->len = len;

    /* add to tail of list */
    item->prev = menu->tail;
    SET_NULL(item->next);

    if (NOT_NULL(menu->tail))
	(menu->tail)->next = item;
    menu->tail = item;

    /* fix head */
    if (IS_NULL(menu->head))
	menu->head = item;

    /*
     * add action
     */
Item_Found:
    if (NOT_NULL(name2) && IS_NULL(item->name2))
    {
	len = STRLEN(name2);
	if (len == 0)
	    SET_NULL(item->name2);
	else
	{
	    /* possible integer overflow? */
	    assert (len > 0 && len + 1 > 0);
	    item->name2 = rxvt_malloc(len + 1);
	    STRCPY(item->name2, name2);
	}
	item->len2 = len;
    }
    item->entry.itemType = MenuLabel;
    len = STRLEN(action);

    if (len == 0 && NOT_NULL(item->name2))
    {
	action = item->name2;
	len = item->len2;
    }
    if (len)
    {
	char	str[NEWARGLIM];
	/* possible integer overflow? */
	assert (len > 0 && len + 1 > 0);
	/* str = rxvt_malloc(len + 1); */

	STRCPY(str, action);
	SET_NULL(item->entry.action.str);

	if( rxvt_set_action( &(item->entry.action), str) )
	    item->entry.itemType = MenuItem;
    }

    /* new item and a possible increase in width */
    len = PTEXTWIDTH( r, item->name, item->len );   /* Left text width */
    if( menu->lwidth < len ) menu->lwidth = len;

    len = PTEXTWIDTH( r, item->name2, item->len2);  /* Right text width */
    if( menu->rwidth < len ) menu->rwidth = len;

    return item;
}

/*
 * search for the base starting menu for NAME.
 * return a pointer to the portion of NAME that remains
 */
/* INTPROTO */
unsigned char*
rxvt_menu_find_base(rxvt_t *r, menu_t **menu, unsigned char *path)
{
    menu_t	 *m = NULL;
    menuitem_t	 *item;
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menu_find_base()\n"));

#ifdef DEBUG
    assert(NOT_NULL(menu));
#endif

    if (path[0] == '\0')
    return path;

    if (NOT_NULL(STRCHR(path, '/')))
    {
	unsigned char	       *p = path;

	while (NOT_NULL(p = (unsigned char*) STRCHR(p, '/')))
	{
	    p++;
	    if (*p == '/')
	    path = p;
	}
	if (path[0] == '/')
	{
	    path++;
	    SET_NULL(*menu);
	}
	while (NOT_NULL(p = (unsigned char*) STRCHR(path, '/')))
	{
	    p[0] = '\0';
	    if (path[0] == '\0')
	    return NULL;
	    if (!STRCMP(path, DOT))
	    {
		/* nothing to do */
	    }
	    else if (!STRCMP(path, DOTS))
	    {
		if (NOT_NULL(*menu))
		    *menu = (*menu)->parent;
	    }
	    else
	    {
		path = rxvt_menu_find_base(r, menu, path);
		if (path[0] != '\0')		/* not found */
		{
		    p[0] = '/';	/* fix-up name again */
		    return path;
		}
	    }

	    path = (p + 1);
	}
    }

    if (!STRCMP(path, DOTS))
    {
	path += STRLEN(DOTS);
	if (NOT_NULL(*menu))
	    *menu = (*menu)->parent;
	return path;
    }

    /* find this menu */
    if (IS_NULL(*menu))
    {
	for (m = r->h->MenuBar.tail; NOT_NULL(m); m = m->prev)
	{
	    if (!STRCMP(path, m->name))
	    break;
	}
    }
    else
    {
	/* find this menu */
	for (item = (*menu)->tail; NOT_NULL(item); item = item->prev)
	{
	    if (
		    item->entry.itemType == MenuSubMenu
		    && !STRCMP(path, (item->entry.submenu.menu)->name)
	       )
	    {
		m = (item->entry.submenu.menu);
		break;
	    }
	}
    }
    if (NOT_NULL(m))
    {
	*menu = m;
	path += STRLEN(path);
    }
    return path;
}

/*
 * delete this entire menu
 */
/* INTPROTO */
menu_t*
rxvt_menu_delete(rxvt_t *r, menu_t *menu)
{
    menu_t	 *parent = NULL, *prev, *next;
    menuitem_t	 *item;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menu_delete()\n"));
    assert( menu != r->h->ActiveMenu ); /* Shouldn't delete current menu */

    /* delete the entire menu */
    if (IS_NULL(menu)) return NULL;

    parent = menu->parent;

    /* unlink MENU */
    prev = menu->prev;
    next = menu->next;
    if (NOT_NULL(prev)) prev->next = next;
    if (NOT_NULL(next)) next->prev = prev;

    if (IS_NULL(parent))
    {
	/*
	 * If we got here, then this menu is either a popup menu, or a pulldown
	 * submenu of the main menubar.
	 */
	const int width = 2 * HSPACE_PIXEL +
	    PTEXTWIDTH( r, menu->name, menu->len);

	/*
	 * Reset start / end of menus in the menubar if we deleted the first /
	 * last pulldown menu.
	 */
	if (r->h->MenuBar.tail == menu) r->h->MenuBar.tail = prev;
	if (r->h->MenuBar.head == menu) r->h->MenuBar.head = next;

	/*
	 * Move all subsequent entries of the menubar to the left. Popups don't
	 * have next or prev set, so no worries doing this
	 */
	for (next = menu->next; NOT_NULL(next); next = next->next)
	    next->x -= width;
    }
    else
    {
	for (item = parent->tail; NOT_NULL(item); item = item->prev)
	{
	    if (item->entry.itemType == MenuSubMenu
		    && item->entry.submenu.menu == menu)
	    {
		/* Unlink from the parent menu */
		SET_NULL(item->entry.submenu.menu);
		rxvt_menuitem_free(r, menu->parent, item);
		break;
	    }
	}
    }

    item = menu->tail;
    while (NOT_NULL(item))
    {
	menuitem_t   *p = item->prev;

	rxvt_menuitem_free(r, menu, item);
	item = p;
    }

    rxvt_free(menu->name);
    rxvt_free(menu);

    return parent;
}

/* INTPROTO */
menu_t*
rxvt_menu_add(rxvt_t *r, menu_t *parent, unsigned char *path)
{
    menu_t	 *menu;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menu_add(): Adding menu %s to parent %s\n", path, NOT_NULL(parent) ? (char *)parent->name : "(nil)"));

    if (NOT_NULL(STRCHR(path, '/')))
    {
	unsigned char	*p;

	if (path[0] == '/')
	{
	    /* shouldn't happen */
	    path++;
	    SET_NULL(parent);
	}
	while (NOT_NULL(p = (unsigned char*) STRCHR(path, '/')))
	{
	    p[0] = '\0';
	    if (path[0] == '\0')
	    return NULL;

	    parent = rxvt_menu_add(r, parent, path);
	    path = (p + 1);
	}
    }
    if (!STRCMP(path, DOTS))
	return (NOT_NULL(parent) ? parent->parent : parent);

    if (!STRCMP(path, DOT) || path[0] == '\0')
	return parent;

    /* allocate a new menu */
    menu = (menu_t *) rxvt_malloc(sizeof(menu_t));

    menu->lwidth = menu->rwidth = 0;
    menu->parent = parent;
    menu->len = STRLEN(path);
    /* possible integer overflow? */
    assert (menu->len > 0 && menu->len+1 > 0);
    menu->name = rxvt_malloc((menu->len + 1));
    STRCPY(menu->name, path);

    /* initialize head/tail */
    SET_NULL(menu->head);
    SET_NULL(menu->tail);
    SET_NULL(menu->prev);
    SET_NULL(menu->next);

    UNSET_WIN(menu->win);
    menu->x = menu->y = menu->height = 0;
    SET_NULL(menu->item);

    if (IS_NULL(parent))
    {
	/* Add menus named PopupButton%d to popupMenu[%d] */
	if(
	    !STRNCASECMP("popupbutton", (char*) path, (sizeof("popupbutton")-1)) &&
	    path[sizeof("popupbutton")-1] >= '1' &&
	    path[sizeof("popupbutton")-1] <= '3' &&
	    path[sizeof("popupbutton")] == '\0'
	  )
	{
	    int i = path[sizeof("popupbutton")-1] - '1';

	    /* Free the menu */
	    if( r->h->popupMenu[i] )
		rxvt_menu_delete( r, r->h->popupMenu[i] );

	    r->h->popupMenu[i] = menu;
	}
	else
	{
	    /*
	     * Add this menu as a the last pulldown menu in the menubar.
	     */
	    menu->prev = r->h->MenuBar.tail;
	    if (NOT_NULL(r->h->MenuBar.tail))
		r->h->MenuBar.tail->next = menu;
	    r->h->MenuBar.tail = menu;

	    /* fix head */
	    if (IS_NULL(r->h->MenuBar.head))
		r->h->MenuBar.head = menu;

	    if (menu->prev)
		menu->x = menu->prev->x + 2 * HSPACE_PIXEL
		    + PTEXTWIDTH( r, menu->prev->name, menu->prev->len);
	}
    }
    else
    {
	menuitem_t   *item;

	item = rxvt_menuitem_add(r, parent, path, (unsigned char*) "",
		(unsigned char*) "");
	
	if (IS_NULL(item))
	{
	    if (NOT_NULL(menu->name))
		rxvt_free( menu->name );
	    rxvt_free(menu);
	    return parent;
	}
#ifdef DEBUG
	assert(item->entry.itemType == MenuLabel);
#endif
	item->entry.itemType = MenuSubMenu;
	item->entry.submenu.menu = menu;
    }

    return menu;
}

/* INTPROTO */
void
rxvt_drawbox_menubar(rxvt_t *r, int x, int len, int state)
{
    unsigned long   top, bot;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_drawbox_menubar()\n"));

    /* XXXMENU */
    len += HSPACE_PIXEL + SHADOW;

    if (x >= TWIN_WIDTH(r))
	return;
    else if (x + len >= TWIN_WIDTH(r))
	len = (TWIN_WIDTH(r) - x);

#ifdef MENUBAR_SHADOW_IN
    state = -state;
#endif
    switch (state)
    {
	case +1:
	    top = r->menuBar.topshadow;
	    bot = r->menuBar.botshadow;
	    break;	    /* SHADOW_OUT */
	case -1:
	    top = r->menuBar.botshadow;
	    bot = r->menuBar.topshadow;
	    break;	    /* SHADOW_IN */
	default:
	    top = bot = r->menuBar.bg;
	    break;	    /* neutral */
    }

    rxvt_draw_shadow(r->Xdisplay, r->menuBar.win, r->menuBar.gc,
	top, bot, x, 0, len, rxvt_menubar_height(r));
}


/* INTPROTO */
void
rxvt_menubar_draw_triangle(rxvt_t *r, int x, int y, int state)
{
    unsigned long   top, bot;
    int		    w;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_draw_triangle()\n"));

#ifdef MENU_SHADOW_IN
    state = -state;
#endif
    switch (state)
    {
	case +1:
	    top = r->menuBar.topshadow;
	    bot = r->menuBar.botshadow;
	    break;	    /* SHADOW_OUT */
	case -1:
	    top = r->menuBar.botshadow;
	    bot = r->menuBar.topshadow;
	    break;	    /* SHADOW_IN */
	default:
	    top = bot = r->menuBar.bg;
	    break;	    /* neutral */
    }

    w = TRIANGLE_WIDTH;

    x -= SHADOW + (3 * w / 2);
    y += SHADOW*2 + (HEIGHT_TEXT - TRIANGLE_WIDTH)/2;

    rxvt_draw_triangle (r->Xdisplay, r->h->ActiveMenu->win, r->menuBar.gc, top, bot, x, y, w, 'r');
}


/* INTPROTO */
void
rxvt_drawbox_menuitem(rxvt_t *r, int y, int state)
{
    unsigned long   top, bot;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_drawbox_menuitem()\n"));

#ifdef MENU_SHADOW_IN
    state = -state;
#endif
    switch (state)
    {
	case +1:
	    top = r->menuBar.topshadow;
	    bot = r->menuBar.botshadow;
	    break;	    /* SHADOW_OUT */
	case -1:
	    top = r->menuBar.botshadow;
	    bot = r->menuBar.topshadow;
	    break;	    /* SHADOW_IN */
	default:
	    top = bot = r->menuBar.bg;
	    break;	    /* neutral */
    }

    rxvt_draw_shadow (r->Xdisplay, r->h->ActiveMenu->win, r->menuBar.gc,
	top, bot,
	SHADOW + 0, SHADOW + y,
	MENUWIDTH( r->h->ActiveMenu ) - 2 * (SHADOW),
	HEIGHT_TEXT + 2 * SHADOW);
    XFlush(r->Xdisplay);
}

#ifdef DEBUG
/* INTPROTO */
void
rxvt_print_menu_ancestors(menu_t *menu)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_print_menu_ancestors()\n"));

    if (IS_NULL(menu))
    {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "Top Level menu\n"));
	return;
    }
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "menu %s ", menu->name));
    if (NOT_NULL(menu->parent))
    {
	menuitem_t   *item;

	for (item = menu->parent->head; NOT_NULL(item); item = item->next)
	{
	    if (item->entry.itemType == MenuSubMenu &&
		item->entry.submenu.menu == menu)
	    {
		break;
	    }
	}
	if (IS_NULL(item))
	{
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "is an orphan!\n"));
	    return;
	}
    }
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "\n"));
    rxvt_print_menu_ancestors(menu->parent);
}

/* INTPROTO */
void
rxvt_print_menu_descendants(menu_t *menu)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_print_menu_descendants()\n"));

    menuitem_t	 *item;
    menu_t	 *parent;
    int		 i, level = 0;

    parent = menu;
    do
      {
	level++;
	parent = parent->parent;
      }
    while (NOT_NULL(parent));

    for (i = 0; i < level; i++)
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, ">"));
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "%s\n", menu->name));

    for (item = menu->head; NOT_NULL(item); item = item->next)
    {
	if (item->entry.itemType == MenuSubMenu)
	{
	    if (IS_NULL(item->entry.submenu.menu))
		rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "> %s == NULL\n", item->name));
	    else
		rxvt_print_menu_descendants(item->entry.submenu.menu);
	}
	else
	{
	    for (i = 0; i < level; i++)
		rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "+"));
	    if (item->entry.itemType == MenuLabel)
		rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "label: "));
	    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "%s\n", item->name));
	}
    }

    for (i = 0; i < level; i++)
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "<"));
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "\n"));
}
#endif	/* DEBUG */

/*
 * Build a menu with all the titles of current tabs.
 */
/* INTPROTO */
void
rxvt_build_tablist(rxvt_t *r, menu_t *menu)
{
    int i;
    unsigned char   action[16],
		    title[MAX_DISPLAY_TAB_TXT + 4];

    assert( menu);
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_build_tablist()\n"));

    /*
     * Empty menus should get renamed to "Switch to tab".
     */
    if( !menu->head && STRCMP( menu->name, "Switch to tab"))
    {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "Renaming menu to Switch to tab\n"));

	menu->name = rxvt_realloc( menu->name, 14);
	STRCPY( menu->name, "Switch to tab");
    }

    rxvt_menu_clear( r, menu);
    for( i=0; i <= LTAB(r); i++)
    {
	sprintf( (char*) title, "%2d. %.*s", i+1, MAX_DISPLAY_TAB_TXT-1,
		PVTS(r, i)->tab_title);
	/* sprintf( action, "]\e]%d;%d\a", Xterm_switchtab, i+1); */
	sprintf( (char*) action, "GotoTab %d", i + 1);

	rxvt_menuitem_add( r, menu, title, NULL, action);

	rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "Added menuentry for %s\n", PVTS(r, i)->tab_title));
    }
}

/* pop up/down the current menu and redraw the menuBar button */
/* EXTPROTO */
void
rxvt_menu_show(rxvt_t *r)
{
    int		 y, xright;
    menu_t	 *ActiveMenu = r->h->ActiveMenu;
    menuitem_t	 *item;

    unsigned short amenu_width;


    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menu_show()\n"));

    if (IS_NULL(ActiveMenu))
	return;

    /*
     * Popup tablist for menus named tablist, or empty popupbutton menus.
     */
    if(
	 !STRCASECMP( (char*) ActiveMenu->name, "Switch to tab")
	 || (
	      !STRNCASECMP( (char*) ActiveMenu->name, "popupbutton", 11 )
	      && ActiveMenu->head == NULL
	    )
      )
	rxvt_build_tablist( r, ActiveMenu );

    if (IS_NULL(ActiveMenu->parent))
    {
	register int	h;

	/* find the height */
	for (h = 0, item = ActiveMenu->head; NOT_NULL(item); item = item->next)
	    h += isSeparator(item->name) ? HEIGHT_SEPARATOR
		     : HEIGHT_TEXT + 2 * SHADOW;
	ActiveMenu->height = h + 2 * SHADOW;

	if( !( r->h->showingMenu & POPUP_MENU ))
	{
	    rxvt_drawbox_menubar(r, ActiveMenu->x,
		    PTEXTWIDTH( r, ActiveMenu->name, ActiveMenu->len), -1);

	    ActiveMenu->y = rxvt_menubar_height( r );
	}
    }

    amenu_width = MENUWIDTH( ActiveMenu );

    /*
     * Move popup menus, or submenus back on screen if they are not completely
     * on screen.
     *
     * XXX 2006-02-02 gi1242: It would be a LOT nicer if we could move all
     * parents of the menu too. We should only do this for popup menus. This is
     * what fvwm does :)
     *
     * BUG: If a moved menu obscures a grandparent menu, then that menu is not
     * redrawn when the child is removed.
     */
    if( (r->h->showingMenu & POPUP_MENU) || ActiveMenu->parent != NULL)
    {
	int	 unused_x, unused_y;
	unsigned rootWidth, rootHeight, unused_depth, unused_border_width;
	Window	 unused_win;

	XGetGeometry( r->Xdisplay, XROOT, &unused_win, &unused_x, &unused_y,
		&rootWidth, &rootHeight,
		&unused_border_width, &unused_depth);

	if( r->szHint.x + ActiveMenu->x + amenu_width >= rootWidth
		&& amenu_width < rootWidth)
	    ActiveMenu->x = rootWidth - amenu_width - r->szHint.x;

	if( r->szHint.y + ActiveMenu->y + ActiveMenu->height >= rootHeight
		&& ActiveMenu->height < rootHeight)
	    ActiveMenu->y = rootHeight - ActiveMenu->height - r->szHint.y;
    }

    /*
     * Create the menu window.
     */
    if (NOT_WIN(ActiveMenu->win))
    {
#if 0
	ActiveMenu->win = XCreateSimpleWindow(r->Xdisplay,
			    r->TermWin.parent,
			    x, ActiveMenu->y,
			    ActiveMenu->width, ActiveMenu->height,
			    0,
			    r->pixColorsFocus[Color_fg],
			    r->pixColorsFocus[Color_scroll]);
#endif
	/*
	 * 2006-02-05 gi1242: We should make menu windows childs of the root
	 * window. That way they can go outside the terminal window's
	 * boundaries.
	 */
	XSetWindowAttributes attrs;

	attrs.override_redirect = True;
	attrs.save_under = True;
	attrs.cursor = r->h->bar_pointer;
	attrs.background_pixel = r->pixColorsFocus[ Color_scroll ];
	attrs.border_pixel = r->pixColorsFocus[Color_fg];

	ActiveMenu->win = XCreateWindow( r->Xdisplay, XROOT,
		r->szHint.x + ActiveMenu->x, r->szHint.y + ActiveMenu->y,
		amenu_width, ActiveMenu->height, 0,
		CopyFromParent, CopyFromParent, CopyFromParent,
		CWOverrideRedirect | CWSaveUnder | CWCursor
		    | CWBackPixel | CWBorderPixel, &attrs);


	XMapWindow(r->Xdisplay, ActiveMenu->win);
    }
#ifdef XFT_SUPPORT
    else if (ISSET_OPTION(r, Opt_xft))
	XClearWindow( r->Xdisplay, ActiveMenu->win );
#endif

    rxvt_draw_shadow (r->Xdisplay, ActiveMenu->win, r->menuBar.gc,
	r->menuBar.topshadow, r->menuBar.botshadow,
	0, 0, amenu_width, ActiveMenu->height);

    /* determine the correct right-alignment */
    for (xright = 0, item = ActiveMenu->head; NOT_NULL(item); item = item->next)
	if( item->name2 && item->len )
	{
	    int width = PTEXTWIDTH( r, item->name2, item->len2);

	    if( xright < width ) xright = width;
	}

    for (y = 0, item = ActiveMenu->head; NOT_NULL(item); item = item->next)
    {
	const int	xoff = (SHADOW + HSPACE_PIXEL/2);
	register int	h;

	if (isSeparator(item->name))
	{
	    rxvt_draw_shadow (r->Xdisplay, ActiveMenu->win,
		r->menuBar.gc,
		r->menuBar.topshadow, r->menuBar.botshadow,
		SHADOW, y + SHADOW + 1,
		amenu_width - 2 * SHADOW, 0);
	    h = HEIGHT_SEPARATOR;
	}
	else
	{
	    unsigned char	*name	= item->name;
	    unsigned short	len	= item->len;

	    if (item->entry.itemType == MenuSubMenu)
	    {
		int	     x1, y1;
		menuitem_t   *it;
		menu_t	     *menu = item->entry.submenu.menu;

		rxvt_menubar_draw_triangle(r, amenu_width, y, +1);

		name = menu->name;
		len = menu->len;

		if( name && !STRCMP( menu->name, "Switch to tab"))
		    rxvt_build_tablist( r, menu);

		y1 = ActiveMenu->y + y;
#if 0
		/* place sub-menu at midpoint of parent menu */
		x1 = ActiveMenu->width / 2;

		if (x1 > menu->width)	/* right-flush menu if too small */
		    x1 += (x1 - menu->width);
		x1 += x;
#endif
		/*
		 * 2006-01-28 gi1242: Oh my god. This is so ugly. Place
		 * sub-menus near the triangle. That'll look a *little* better.
		 */
		x1 = ActiveMenu->x + amenu_width - TRIANGLE_WIDTH - xoff;

		/* find the height of this submenu */
		for (h = 0, it = menu->head; NOT_NULL(it); it = it->next)
		    h += isSeparator(it->name) ? HEIGHT_SEPARATOR
				   : HEIGHT_TEXT + 2 * SHADOW;
		menu->height = h + 2 * SHADOW;

		/*
		 * XXX 2006-02-02 gi1242: Should we really move the menu back
		 * over the terminal window? It's a better idea to do what fvwm
		 * does.
		 */
#if 0
		/* ensure menu is in window limits */
		if ((x1 + menu->width) >= TWIN_WIDTH(r))
		    x1 = (TWIN_WIDTH(r) - menu->width);

		if ((y1 + menu->height) >= TWIN_HEIGHT(r))
		    y1 = (TWIN_HEIGHT(r) - menu->height);
#endif

		menu->x = (x1 < 0 ? 0 : x1);
		menu->y = (y1 < 0 ? 0 : y1);
	    }
	    else if (item->name2 && !STRCMP(name, item->name2))
		SET_NULL(name);


	    if (item->entry.itemType == MenuLabel)
	    {
		CHOOSE_GC_FG (r, r->menuBar.botshadow);
	    }
	    else
	    {
		CHOOSE_GC_FG (r, r->menuBar.fg);
	    }


	    if (len && name)
	    {
# ifdef XFT_SUPPORT
		/*
		 * TODO: Add multichar support.
		 */
		if (ISSET_OPTION(r, Opt_xft))
		{
		    XftFont *font = r->TermWin.xftpfont ?
			r->TermWin.xftpfont : r->TermWin.xftfont;

		    XftDrawChange( r->menuBar.xftDraw, ActiveMenu->win);
		    XftDrawString8( r->menuBar.xftDraw, &r->menuBar.xftFore,
			    font, xoff, 2 * SHADOW + y + font->ascent + 1,
			    name, len);
		}
		else
#endif
#ifdef USE_XIM
		if (r->TermWin.fontset)
		{
		    XmbDrawString(r->Xdisplay,
			  ActiveMenu->win, r->TermWin.fontset,
			  r->menuBar.gc, xoff,
			  2 * SHADOW + y + r->TermWin.font->ascent + 1,
			  (char*) name, len);
		}
		else
#endif
		XDrawString(r->Xdisplay, ActiveMenu->win,
		    r->menuBar.gc, xoff,
		    2 * SHADOW + y + r->TermWin.font->ascent + 1,
		    (char*) name, len);
	    }

	    len = item->len2;
	    name = item->name2;
	    if (len && name)
	    {
# ifdef XFT_SUPPORT
		/*
		 * XXX Add multichar support.
		 */
		if (ISSET_OPTION(r, Opt_xft))
		{
		    XftFont *font = r->TermWin.xftpfont ?
			r->TermWin.xftpfont : r->TermWin.xftfont;

		    XftDrawChange( r->menuBar.xftDraw, ActiveMenu->win);
		    XftDrawString8( r->menuBar.xftDraw, &r->menuBar.xftFore,
			    font,
			    amenu_width - (xoff + xright),
			    2 * SHADOW + y + font->ascent + 1,
			    name, len);
		}
		else
#endif
#ifdef USE_XIM
		if (r->TermWin.fontset)
		    XmbDrawString(r->Xdisplay,
			ActiveMenu->win, r->TermWin.fontset,
			r->menuBar.gc,
			amenu_width - (xoff + xright),
			2 * SHADOW + y + r->TermWin.font->ascent + 1,
			(char*) name, len);
		else
#endif
		XDrawString(r->Xdisplay, ActiveMenu->win,
		    r->menuBar.gc,
		    amenu_width - (xoff + xright),
		    2 * SHADOW + y + r->TermWin.font->ascent + 1,
		    (char*) name, len);
	    }
	    h = HEIGHT_TEXT + 2 * SHADOW;
	}
	y += h;
    }
#ifdef XFT_SUPPORT
    if (ISSET_OPTION(r, Opt_xft))
	/*
	 * 2006-01-29 gi1242: For some reason if we leave xftDraw
	 * with drawable ActiveMenu->win, we get a RenderBadPicture
	 * error (I'm not sure on what function call). Thus we reset
	 * it.
	 */
	XftDrawChange( r->menuBar.xftDraw, r->menuBar.win);
#endif
}


/*
 * Destroy the ActiveMenu window if any, and redisplay (using update) the
 * parent. Setting updated to recursively call this function will hide all menus
 * (as in rxvt_menu_hide_all)
 */
/* INTPROTO */
void
rxvt_menu_display(rxvt_t *r, void (*update)(rxvt_t *))
{
    menu_t	 *ActiveMenu = r->h->ActiveMenu;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menu_display()\n"));

    if (IS_NULL(ActiveMenu))
	return;

    if (IS_WIN(ActiveMenu->win))
    {
	XDestroyWindow(r->Xdisplay, ActiveMenu->win);
	UNSET_WIN(ActiveMenu->win);
    }
    SET_NULL(ActiveMenu->item);

    if (IS_NULL(ActiveMenu->parent) &&
	    !(r->h->showingMenu & POPUP_MENU) )
	/*
	 * 2006-01-30 gi1242: Just clearing the shadow will be enough. But with
	 * transparency, drawbox_menubar(0) is not enough.
	 */
#if 0
	rxvt_drawbox_menubar(r, ActiveMenu->x, ActiveMenu->len, 0);
#endif
	rxvt_menubar_draw_labels(r);

    r->h->ActiveMenu = ActiveMenu->parent;
    update(r);
}

/* INTPROTO */
void
rxvt_menu_hide_all(rxvt_t *r)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menu_hide_all()\n"));

    rxvt_menu_display(r, rxvt_menu_hide_all);
    /* r->h->showingMenu &= ~PULLDOWN_MENU; */
}

/*
 * Hide current menu window, and redisplay the parent.
 */
/* INTPROTO */
void
rxvt_menu_hide(rxvt_t *r)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menu_hide()\n"));

    /*
     * rxvt_menu_display just destroys the current menu window, and reshow's the
     * parent (if rxvt_menu_show is passed as update function).
     */
    rxvt_menu_display(r, rxvt_menu_show);
}

/* INTPROTO */
void
rxvt_menu_clear(rxvt_t *r, menu_t *menu)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menu_clear()\n"));

    if (NOT_NULL(menu))
    {
	menuitem_t   *item = menu->tail;

	while (NOT_NULL(item))
	{
	    rxvt_menuitem_free(r, menu, item);
	    /* it didn't get freed ... why? */
	    if (item == menu->tail) return;

	    item = menu->tail;
	}
	menu->lwidth = menu->rwidth = 0;
    }
}

/* INTPROTO */
void
rxvt_menubar_clear(rxvt_t *r)
{
    menu_t	 *menu = r->h->MenuBar.tail;
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_clear()\n"));

    while (NOT_NULL(menu))
    {
	menu_t	     *prev = menu->prev;

	rxvt_menu_delete(r, menu);
	menu = prev;
    }
    SET_NULL(r->h->MenuBar.head);
    SET_NULL(r->h->MenuBar.tail);

    if (r->h->MenuBar.title)
    {
	rxvt_free(r->h->MenuBar.title);
	SET_NULL(r->h->MenuBar.title);
    }
    rxvt_menuarrow_free(r, 0);	/* remove all arrow functions */
}


/* INTPROTO */
void
rxvt_draw_arrows(rxvt_t *r, int name, int state)
{
    unsigned long   top, bot;

    int		 i;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_draw_arrows()\n"));

#ifdef MENU_SHADOW_IN
    state = -state;
#endif
    switch (state)
    {
	case +1:
	    top = r->menuBar.topshadow;
	    bot = r->menuBar.botshadow;
	    break;	    /* SHADOW_OUT */

	case -1:
	    top = r->menuBar.botshadow;
	    bot = r->menuBar.topshadow;
	    break;	    /* SHADOW_IN */

	default:
	    top = bot = r->menuBar.bg;
	    break;	    /* neutral */
    }

    if (!r->h->Arrows_x)
	return;

    for (i = 0; i < NARROWS; i++)
    {
	const int   w = MENUBAR_ARROW_WIDTH;
	const int   y = (rxvt_menubar_height(r) - w) / 2;

	int	    x = r->h->Arrows_x + (5 * MENUBAR_ARROW_WIDTH * i) / 4;

	if (!name || name == Arrows[i].name)
	    rxvt_draw_triangle (r->Xdisplay, r->menuBar.win,
		r->menuBar.gc, top, bot,
		x, y, w, Arrows[i].name);
    }
    XFlush(r->Xdisplay);
}


/* EXTPROTO */
int
rxvt_menu_select(rxvt_t *r, XButtonEvent *ev)
{
    menuitem_t	    *thisitem, *item = NULL;
    int		    this_y, y;
    menu_t	    *ActiveMenu = r->h->ActiveMenu;
    unsigned short  amenu_width = MENUWIDTH( ActiveMenu );

    Window	    unused_root, unused_child;
    int		    unused_root_x, unused_root_y;
    unsigned int    unused_mask;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menu_select() (%d)\n", ev->type));

    if (IS_NULL(ActiveMenu)) return 0;

    XQueryPointer(r->Xdisplay, ActiveMenu->win,
	  &unused_root, &unused_child,
	  &unused_root_x, &unused_root_y,
	  &(ev->x), &(ev->y), &unused_mask);

    if (NOT_NULL(ActiveMenu->parent) && (ev->x < 0 || ev->y < 0))
    {
	rxvt_menu_hide(r);
	return 1;
    }

    /* determine the menu item corresponding to the Y index */
    y = SHADOW;
    if (ev->x >= 0 && ev->x <= (amenu_width - SHADOW))
    {
	for (item = ActiveMenu->head; NOT_NULL(item); item = item->next)
	{
	    int		 h = HEIGHT_TEXT + 2 * SHADOW;

	    if (isSeparator(item->name)) h = HEIGHT_SEPARATOR;
	    else if (ev->y >= y && ev->y < (y + h))
		break;
	    y += h;
	}
    }
    if (IS_NULL(item) && ev->type == ButtonRelease)
    {
	rxvt_menu_hide_all(r);
	return 0;
    }

    thisitem = item;
    this_y = y - SHADOW;

    /* erase the last item */
    if (NOT_NULL(ActiveMenu->item) || ev->type == ButtonRelease)
    {
	if (ActiveMenu->item && ActiveMenu->item != thisitem)
	{
	    /*
	     * Erase shadow of old menuitem. We can not get here during a button
	     * release.
	     */
	    for (y = 0, item = ActiveMenu->head;
		    NOT_NULL(item);
		    item = item->next)
	    {
		int	     h;

		if (isSeparator(item->name))
		    h = HEIGHT_SEPARATOR;
		else if (item == ActiveMenu->item)
		{
		    /* erase old menuitem shadow */
		    rxvt_drawbox_menuitem(r, y, 0); /* No Shadow */
		    if (item->entry.itemType == MenuSubMenu)
			rxvt_menubar_draw_triangle(r, amenu_width, y, +1);
		    break;
		}
		else
		    h = HEIGHT_TEXT + 2 * SHADOW;
		y += h;
	    }
	}
	else
	{
	    switch (ev->type)
	    {
		case ButtonRelease:
		    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MENUBAR, "Menuitem released\n"));
		    switch (item->entry.itemType)
		    {
			case MenuLabel:
			case MenuSubMenu:
			    rxvt_menu_hide_all(r);
			    break;

			case MenuItem:
			    rxvt_drawbox_menuitem(r, this_y, -1);
			    {
#ifdef HAVE_NANOSLEEP
				struct timespec rqt;

				rqt.tv_sec = 0;
				rqt.tv_nsec = MENU_DELAY_USEC * 1000;
				nanosleep(&rqt, NULL);
#else
				/* use select for timing */
				struct timeval  tv;

				tv.tv_sec = 0;
				tv.tv_usec = MENU_DELAY_USEC;
				select(0, NULL, NULL, NULL, &tv);
#endif
			    }
			    /* remove menu before sending keys to the application */
			    rxvt_menu_hide_all(r);
			    rxvt_dispatch_action(r, &(item->entry.action),
				    (XEvent *) ev );
#ifdef DEBUG        /* DEBUG */
			    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MENUBAR, "%s: %s\n", item->name, item->entry.action.str));
#endif		    /* DEBUG */
			    break;
		    }
		    break;

		default:
		    if (item->entry.itemType == MenuSubMenu)
			goto DoMenu;
		    break;
	    }
	    return 0;
	}
    }

DoMenu:
    ActiveMenu->item = thisitem;
    y = this_y;
    if (NOT_NULL(thisitem))
    {
	item = ActiveMenu->item;
	if (item->entry.itemType != MenuLabel)
	    rxvt_drawbox_menuitem(r, y, +1);
	if (item->entry.itemType == MenuSubMenu)
	{
	    int		 x;

	    rxvt_menubar_draw_triangle(r, amenu_width, y, -1);

	    x = ev->x + ActiveMenu->x;

	    if (x >= item->entry.submenu.menu->x)
	    {
		r->h->ActiveMenu = item->entry.submenu.menu;
		rxvt_menu_show(r);
		return 1;
	    }
	}
    }
    return 0;
}

/* INTPROTO */
void
rxvt_menubar_select(rxvt_t *r, XButtonEvent *ev)
{
    menu_t	 *menu = NULL;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_select()\n"));

    /* determine the pulldown menu corresponding to the X index */
    if ( ev->y >= 0 && ev->y <= (rxvt_menubar_height(r)-MENUBAR_MARGIN) )
    {
	for (menu = r->h->MenuBar.head; NOT_NULL(menu); menu = menu->next)
	{
	    int w = HSPACE_PIXEL + PTEXTWIDTH( r, menu->name, menu->len);

	    if ((ev->x >= menu->x && ev->x < menu->x + w)) break;
	}
    }
    switch (ev->type)
    {
	case ButtonRelease:
	    rxvt_menu_hide_all(r);
	    break;

	case ButtonPress:
	    if (IS_NULL(menu) && r->h->Arrows_x && ev->x >= r->h->Arrows_x)
	    {
		int	     i;

		for (i = 0; i < NARROWS; i++)
		{
		    if (ev->x >= (r->h->Arrows_x + (Width2Pixel(4 * i + i)) / 4)
			&& ev->x < (r->h->Arrows_x
				+ (Width2Pixel(4 * i + i + 4)) / 4))
		    {
			rxvt_draw_arrows(r, Arrows[i].name, -1);
			{
#ifdef HAVE_NANOSLEEP
			    struct timespec rqt;

			    rqt.tv_sec = 0;
			    rqt.tv_nsec = MENU_DELAY_USEC * 1000;
			    nanosleep(&rqt, NULL);
#else
			    /* use select for timing */
			    struct timeval  tv;

			    tv.tv_sec = 0;
			    tv.tv_usec = MENU_DELAY_USEC;
			    select(0, NULL, NULL, NULL, &tv);
#endif
			}
			rxvt_draw_arrows(r, Arrows[i].name, +1);

			rxvt_dispatch_action(r, &(r->h->MenuBar.arrows[i]),
				(XEvent *) ev);
			return;
		    }
		}
	    }
	    /* FALLTHROUGH */

	default:
	/*
	 * press menubar or move to a new entry
	 */
	if (NOT_NULL(menu) && menu != r->h->ActiveMenu)
	{
	    rxvt_menu_hide_all(r);  /* pop down old menu */
	    r->h->ActiveMenu = menu;
	    rxvt_menu_show(r);	/* pop up new menu */
	}
	break;
    }
}



/* EXTPROTO */
void
rxvt_menubar_create (rxvt_t* r)
{
    XGCValues	    gcvalue;
    unsigned long   gcmask;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR,"rxvt_menubar_create\n"));
    /*
     * Only create a menubar if it's non-empty.
     */

    r->menuBar.state = 0;

    r->menuBar.win = XCreateSimpleWindow(r->Xdisplay,
		    r->TermWin.parent,
		    0, 0,
		    TWIN_WIDTH(r), rxvt_menubar_rheight (r),
		    0, r->pixColorsFocus[Color_fg],
		    r->pixColorsFocus[Color_scroll]);
    assert (IS_WIN(r->menuBar.win));

#  ifdef DEBUG_X
    rxvt_set_win_title (r, r->menuBar.win, "menubar");
#  endif

    XDefineCursor(r->Xdisplay, r->menuBar.win, r->h->bar_pointer);
    XSelectInput(r->Xdisplay, r->menuBar.win,
	      (ExposureMask | ButtonPressMask | ButtonReleaseMask
	      | Button1MotionMask));

#  ifdef BACKGROUND_IMAGE
    UNSET_PIXMAP(r->menuBar.pixmap);	/* Initialize it to None */
#   ifdef TRANSPARENT
    if (!(ISSET_OPTION(r, Opt_transparent) &&
	  ISSET_OPTION(r, Opt_transparent_menubar)
	))
#   endif
    if (r->h->rs[Rs_menubarPixmap])
    {
	long	w = 0, h = 0;
	r->menuBar.pixmap = rxvt_load_pixmap (r,
				r->h->rs[Rs_menubarPixmap], &w, &h);
	if (IS_PIXMAP(r->menuBar.pixmap))
	    XSetWindowBackgroundPixmap (r->Xdisplay, r->menuBar.win,
		r->menuBar.pixmap);
    }
#  endif

#  ifdef TRANSPARENT
    if (ISSET_OPTION(r, Opt_transparent) &&
	ISSET_OPTION(r, Opt_transparent_menubar))
    {
	XSetWindowBackgroundPixmap (r->Xdisplay, r->menuBar.win,
	    ParentRelative);
    }
#  endif

    /*
     * Initialize the colors. TODO: Add a sperate resource for this, instead of
     * mooching of the scroll bar resources.
     */
    if( XDEPTH > 2 )
    {
	/*
	 *  If Color_scroll is too dark, then we should use White for the menu
	 *  foreground.
	 */
	XColor xcol;

	xcol.pixel = r->pixColorsFocus[Color_scroll];
	XQueryColor( r->Xdisplay, XCMAP, &xcol);

	r->menuBar.fg = r->pixColorsFocus[
		(xcol.red <= 0x60 && xcol.green <= 0x60 && xcol.blue <= 0x60 )
		    ? Color_White : Color_Black ];
    }
    else
	r->menuBar.fg = r->pixColorsFocus[ Color_bg ]; /* Reverse video */

    r->menuBar.bg = r->pixColorsFocus[XDEPTH <= 2 ? Color_fg : Color_scroll];
    r->menuBar.topshadow = r->pixColorsFocus[Color_topShadow];
    r->menuBar.botshadow = r->pixColorsFocus[Color_bottomShadow];


    gcvalue.foreground = r->menuBar.fg;
#  ifdef TRANSPARENT
    if (!(ISSET_OPTION(r, Opt_transparent) &&
	  ISSET_OPTION(r, Opt_transparent_menubar)
	))
#  endif
#  ifdef BACKGROUND_IMAGE
    if (NOT_PIXMAP(r->menuBar.pixmap))
#  endif
    gcvalue.background = r->menuBar.bg;
    gcmask = GCForeground;

#  ifdef TRANSPARENT
    if (!(ISSET_OPTION(r, Opt_transparent) &&
	  ISSET_OPTION(r, Opt_transparent_menubar)
	))
#  endif
#  ifdef BACKGROUND_IMAGE
    if (NOT_PIXMAP(r->menuBar.pixmap))
#  endif
    gcmask |= GCBackground;
    r->menuBar.gc = XCreateGC (r->Xdisplay, r->menuBar.win,
			gcmask, &gcvalue);
    assert (IS_GC(r->menuBar.gc));

#  ifdef XFT_SUPPORT
    if (ISSET_OPTION(r, Opt_xft))
    {
	/*
	 * Set up Xft stuff here.
	 */
	XColor	xcol;

	r->menuBar.xftDraw = XftDrawCreate( r->Xdisplay, r->menuBar.win,
		XVISUAL, XCMAP);

	xcol.pixel = r->menuBar.fg;
	XQueryColor( r->Xdisplay, XCMAP, &xcol );
	rxvt_alloc_xft_color( r, &xcol, &r->menuBar.xftFore);
    }
    else
#  endif
    {
	XSetFont(r->Xdisplay, r->menuBar.gc, r->TermWin.font->fid);
    }
}


/* EXTPROTO */
void
rxvt_menubar_clean_exit (rxvt_t* r)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_clean_exit()\n"));

#  ifdef XFT_SUPPORT
    /*
     * Sometimes gives a RenderBadPicture error, so don't destroy it.
     */
#   if 0
    if (ISSET_OPTION(r, Opt_xft))
    {
	XftDrawDestroy( r->menuBar.xftDraw);
    }
#   endif
#  endif

    UNSET_WIN(r->menuBar.win);	/* Destroyed by XDestroySubwindows */

    if (IS_GC(r->menuBar.gc))
    {
	XFreeGC (r->Xdisplay, r->menuBar.gc);
	UNSET_GC(r->menuBar.gc);
    }
#  ifdef BACKGROUND_IMAGE
    if (IS_PIXMAP(r->menuBar.pixmap))
    {
	XFreePixmap (r->Xdisplay, r->menuBar.pixmap);
	UNSET_PIXMAP(r->menuBar.pixmap);
    }
#  endif
}


/*
** Is the menubar visible
*/
/* EXTPROTO */
int
rxvt_menubar_visible (rxvt_t* r)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_visible()\n"));

    return (IS_WIN(r->menuBar.win) && r->menuBar.state);
}


/*
** Hide the menubar
*/
/* EXTPROTO */
int
rxvt_menubar_hide (rxvt_t* r)
{
    int	    changed = 0;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_hide()\n"));

    assert (IS_WIN(r->menuBar.win));
    changed = r->menuBar.state;
    XUnmapWindow(r->Xdisplay, r->menuBar.win);
    r->menuBar.state = 0;

    return (changed);
}


/*
** Show the menubar
*/
/* EXTPROTO */
int
rxvt_menubar_show (rxvt_t* r)
{
    int	    changed = 0;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_show()\n"));
    assert (IS_WIN(r->menuBar.win));

    changed = !r->menuBar.state;
    XMapWindow(r->Xdisplay, r->menuBar.win);
    r->menuBar.state = 1;

    return (changed);
}


/*
 * Menubar expose handler
 */
/* EXTPROTO */
void
rxvt_menubar_expose(rxvt_t *r)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_expose()\n"));

    if (!r->menuBar.state || NOT_WIN(r->menuBar.win))
	return;

    rxvt_menu_hide_all(r);
    rxvt_menubar_draw_labels(r);
}


/* INTPROTO */
void
rxvt_menubar_draw_labels( rxvt_t *r)
{
    menu_t	 *menu;
    int		 x;

    XClearWindow(r->Xdisplay, r->menuBar.win);

    x = 0;
    for (menu = r->h->MenuBar.head; NOT_NULL(menu); menu = menu->next)
    {
	unsigned short	 len = menu->len;

	x = (menu->x + menu->len + HSPACE);

# ifdef DEBUG
	rxvt_print_menu_descendants(menu);
# endif

	/* if (x >= r->TermWin.ncol)
	    len = (r->TermWin.ncol - (menu->x + HSPACE)); */

	/* 2006-01-29 gi1242: Boxes in the menubar are ugly */
#if 0
	rxvt_drawbox_menubar(r, menu->x, len, +1);
#endif

	CHOOSE_GC_FG (r, r->menuBar.fg);
# ifdef XFT_SUPPORT
	/*
	 * XXX Add multichar support.
	 */
	if (ISSET_OPTION(r, Opt_xft))
	{
	    XftFont *font = r->TermWin.xftpfont ?
		r->TermWin.xftpfont : r->TermWin.xftfont;

	    XftDrawString8( r->menuBar.xftDraw, &r->menuBar.xftFore, font,
		  (menu->x + HSPACE_PIXEL / 2),
		  rxvt_menubar_height(r) - SHADOW - MENUBAR_MARGIN
			- font->descent,
		  menu->name, len);
	}
	else
# endif
# ifdef USE_XIM
	if (r->TermWin.fontset)
	    XmbDrawString(r->Xdisplay,
		  r->menuBar.win, r->TermWin.fontset, r->menuBar.gc,
		  (menu->x + HSPACE_PIXEL / 2),
		  rxvt_menubar_height(r) - SHADOW - MENUBAR_MARGIN
			- r->TermWin.font->descent,
		  (char*) menu->name, len);
	else
# endif	/* USE_XIM */
	XDrawString(r->Xdisplay, r->menuBar.win, r->menuBar.gc,
		  (menu->x + HSPACE_PIXEL / 2),
		  rxvt_menubar_height(r) - SHADOW - MENUBAR_MARGIN
			- r->TermWin.font->descent,
		  (char*) menu->name, len);

	if (x >= TWIN_WIDTH(r) ) break;
    }
#if 0
    rxvt_drawbox_menubar(r, x, r->TermWin.ncol, (r->h->CurrentBar ? +1 : -1));
#endif

    /* add the menuBar title, if it exists and there's plenty of room */
    r->h->Arrows_x = 0;
    if (x < TWIN_WIDTH(r) )
    {
	const unsigned char  	*str;
	int	     	npixels;    /* used to be ncol */
	unsigned short	len;
	unsigned char	title[256];

	npixels = TWIN_WIDTH(r);
	if (x < (npixels - NARROWS * MENUBAR_ARROW_WIDTH - HSPACE_PIXEL))
	{
	    npixels -= NARROWS * MENUBAR_ARROW_WIDTH + HSPACE_PIXEL;
	    r->h->Arrows_x = npixels;
	}
	rxvt_draw_arrows(r, 0, +1);

	str = ( r->h->MenuBar.title) ?
	    r->h->MenuBar.title : (unsigned char*) "%n-%v";
	for (len = 0; str[0] && len < sizeof(title) - 1; str++)
	{
	    const char	 *s = NULL;

	    switch (str[0])
	    {
		case '%':
		    str++;

		    switch (str[0])
		    {
			case 'n':
			    s = r->h->rs[Rs_name];
			    break;  /* resource name */

			case 'v':
			    s = VERSION;
			    break;  /* version number */

			case '%':
			    s = "%";
			    break;  /* literal '%' */
		    }

		    if (NOT_NULL(s))
			while (*s && len < sizeof(title) - 1)
			title[len++] = *s++;
		    break;

		    default:
			title[len++] = str[0];
			break;
	    }
	}
	title[len] = '\0';

	npixels -= x + HSPACE_PIXEL + PTEXTWIDTH( r, title, len);

	if (len > 0 && npixels >= 0)
	{
	    CHOOSE_GC_FG (r, r->menuBar.fg);
# ifdef XFT_SUPPORT
	    /*
	     * XXX Add multichar support.
	     */
	    if (ISSET_OPTION(r, Opt_xft))
	    {
		XftFont *font = r->TermWin.xftpfont ?
		    r->TermWin.xftpfont : r->TermWin.xftfont;

		XftDrawString8( r->menuBar.xftDraw, &r->menuBar.xftFore, font,
		      x + (npixels + HSPACE_PIXEL) / 2,
		      rxvt_menubar_height(r) - SHADOW - MENUBAR_MARGIN
				- font->descent,
		      title, len);
	    }
	    else
# endif
# ifdef USE_XIM
	    if (r->TermWin.fontset)
	    XmbDrawString(r->Xdisplay,
		      r->menuBar.win, r->TermWin.fontset, r->menuBar.gc,
		      x + (npixels + HSPACE_PIXEL) / 2,
		      rxvt_menubar_height(r) - SHADOW - MENUBAR_MARGIN
				- r->TermWin.font->descent,
		      (char*) title, len);
	    else
# endif	/* USE_XIM */
	    XDrawString(r->Xdisplay, r->menuBar.win, r->menuBar.gc,
		      x + (npixels + HSPACE_PIXEL) / 2,
		      rxvt_menubar_height(r) - SHADOW - MENUBAR_MARGIN
				- r->TermWin.font->descent,
		      (char*) title, len);
	    }
    }
}


/*
** user interface for building/deleting and otherwise managing menus
*/
/* EXTPROTO */
void
rxvt_menubar_dispatcher(rxvt_t *r, unsigned char *str)
{
    int		    n, cmd;
    unsigned char   *path, *name, *name2;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_dispatcher()\n"));

#if 0
    if (rxvt_menubar_visible(r) && NOT_NULL(r->h->ActiveMenu))
	rxvt_menubar_expose(r);
    else
	SET_NULL(r->h->ActiveMenu);
#endif

    cmd = *str;
    switch (cmd)
    {
	case '.':
	case '/':		/* absolute & relative path */
	case MENUITEM_BEG:	/* menuitem */
	    /* add `+' prefix for these cases */
	    cmd = '+';
	    break;

	case '+':
	case '-':
	    str++;	    /* skip cmd character */
	    break;

	case '<':
	    if (str[1] && str[2] == '>')    /* arrow commands */
		rxvt_menuarrow_add(r, str);
	    break;

	case '[':	    /* extended command */
	    while (str[0] == '[')
	    {
		unsigned char	       *next = (++str);	/* skip leading '[' */

		if (str[0] == ':')	/* [:command:] */
		{
		    do
		      {
			next++;
			if (IS_NULL(next = (unsigned char*) STRCHR(next, ':')))
			    return; /* parse error */
		      }
		    while (next[1] != ']');

		    /* remove and skip ':]' */
		    *next = '\0';
		    next += 2;
		}
		else
		{
		    if (IS_NULL(next = (unsigned char*) STRCHR(next, ']')))
			return;	/* parse error */
		    /* remove and skip ']' */
		    *next = '\0';
		    next++;
		}

		if (str[0] == ':')
		    rxvt_menubar_dispatcher(r, str + 1);

		else if (!STRCMP(str, "clear"))
		    rxvt_menubar_clear(r);

		else if (
			    !STRCMP((char*) str, "done")
			    || rxvt_str_match( (char*) str, "done:")
			)
		{
		    /* We shouldn't ever get here */
		    assert(0);
		}
		/*
		 * 2006-02-02 gi1242: Using this command in config files will
		 * override the users --showmenu option. Since there are other
		 * escape sequences and shortcuts to do the same, we don't need
		 * this here.
		 */
#if 0
		else if (!STRCMP(str, "show"))
		{
		    if (rxvt_menubar_show(r))
			rxvt_resize_on_subwin (r, SHOW_MENUBAR);
		}
		else if (!STRCMP(str, "hide"))
		{
		    if (rxvt_menubar_hide(r))
			rxvt_resize_on_subwin (r, HIDE_MENUBAR);
		}
#endif
		else if ((n = rxvt_str_match( (char*) str, "read:")) != 0)
		{
		    /* read in a menu from a file */
		    str += n;
		    rxvt_menubar_load_file(r, str);
		}
		else if ((n = rxvt_str_match( (char*) str, "title:")) != 0)
		{
		    str += n;
		    if (*str)
		    {
			name = rxvt_realloc(r->h->MenuBar.title,
			    STRLEN(str) + 1);

			if (NOT_NULL(name))
			{
			    STRCPY(name, str);
			    r->h->MenuBar.title = name;
			}
		    }
		    else
		    {
			rxvt_free(r->h->MenuBar.title);
			SET_NULL(r->h->MenuBar.title);
		    }
		}
		else if ((n = rxvt_str_match( (char*) str, "pixmap:")) != 0)
		{
		    str += n;
		    rxvt_xterm_seq(r, ATAB(r), XTerm_Pixmap,
			    (char*) str, CHAR_ST);
		}
		str = next;

		/*
		 * 2006-02-04 gi1242: Don't clear the menu currently bieng
		 * built. This will enable [read:...] commands to add to the
		 * current menu, instead of the menubar.
		 */
#if 0
		SET_NULL(r->h->BuildMenu);
		SET_NULL(r->h->ActiveMenu);
		rxvt_menubar_expose(r);
#endif
	    }
	    return;
	    break;
    }

    switch (cmd)
    {
	case '+':
	case '-':
	    path = name = str;

	    SET_NULL(name2);
	    /* parse STR, allow spaces inside (name)  */
	    if (path[0] != '\0')
	    {
		name = (unsigned char*) STRCHR(path, MENUITEM_BEG);
		str = (unsigned char*) STRCHR(path, MENUITEM_END);
		if (NOT_NULL(name) || NOT_NULL(str))
		{
		    if (IS_NULL(name) || IS_NULL(str) || str <= (name + 1) ||
			(name > path && name[-1] != '/')
		       )
		    {
			rxvt_msg (DBG_ERROR, DBG_MENUBAR, "menu error <%s>\n", path);
			break;
		    }
		    if (str[1] == MENUITEM_BEG)
		    {
			name2 = (str + 2);
			str = (unsigned char*) STRCHR(name2, MENUITEM_END);

			if (IS_NULL(str))
			{
			    rxvt_msg (DBG_ERROR, DBG_MENUBAR, "menu error <%s>\n", path);
			    break;
			}
			name2[-2] = '\0';   /* remove prev MENUITEM_END */
		    }
		    if (name > path && name[-1] == '/')
			name[-1] = '\0';

		    *name++ = '\0'; /* delimit */
		    *str++ = '\0';  /* delimit */

		    while (isspace((int) *str))
			str++;	/* skip space */
		}

		rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "`%c' path = <%s>, name = <%s>, name2 = <%s>, action = <%s>\n", cmd, (path ? (char*) path : "(nil)"), (name ? (char*) name : "(nil)"), (name2 ? (char*) name2 : "(nil)"), (str ? (char*) str : "(nil)")));
	    }

	    /* process the different commands */
	    switch (cmd)
	    {
		case '+':	/* add/replace existing menu or menuitem */
		    if (path[0] != '\0')
		    {
			int	     len;

			rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "Current menu %s\n", NOT_NULL(r->h->BuildMenu) ? (char *)r->h->BuildMenu->name : "(nil)"));

			path = rxvt_menu_find_base(r, &(r->h->BuildMenu), path);
			len = STRLEN(path);

			/* don't allow menus called `*' */
			if (path[0] == '*')
			{
			    rxvt_menu_clear(r, r->h->BuildMenu);
			    break;
			}
			else if (len >= 2 && !STRCMP((path + len - 2), "/*"))
			{
			    path[len - 2] = '\0';
			}

			if (path[0] != '\0')
			    r->h->BuildMenu =
				rxvt_menu_add(r, r->h->BuildMenu, path);
		    }

		    if (NOT_NULL(name) && name[0] != '\0')
			rxvt_menuitem_add(r, r->h->BuildMenu,
			      (STRCMP(name, SEPARATOR_NAME) ?
				    name : (unsigned char *) ""),
			      name2, str);
		    break;

		case '-':	/* delete menu entry */
		    if ( !STRCMP(path, "/*") &&
			(IS_NULL(name) || name[0] == '\0')
		       )
		    {
			rxvt_menubar_clear(r);
			SET_NULL(r->h->BuildMenu);
			break;
		    }
		    else if (path[0] != '\0')
		    {
			int	     len;
			menu_t	     *menu = r->h->BuildMenu;

			path = rxvt_menu_find_base(r, &menu, path);
			len = STRLEN(path);

			/* submenu called `*' clears all menu items */
			if (path[0] == '*')
			{
			    rxvt_menu_clear(r, menu);
			    break;  /* done */
			}
			else if (len >= 2 && !STRCMP(&path[len - 2], "/*"))
			{
			    /* done */
			    break;
			}
			else if (path[0] != '\0')
			{
			    SET_NULL(r->h->BuildMenu);
			    break;
			}
			else
			    r->h->BuildMenu = menu;
		    }

		    if (NOT_NULL(r->h->BuildMenu))
		    {
			if (IS_NULL(name) || name[0] == '\0')
			    r->h->BuildMenu =
				rxvt_menu_delete(r, r->h->BuildMenu);
			else
			{
			    const unsigned char	 *n1;
			    menuitem_t	 	 *item;
			    menu_t	 	 *BuildMenu = r->h->BuildMenu;

			    n1 = STRCMP(name, SEPARATOR_NAME)
				    ? name : (unsigned char*) "";
			    item = rxvt_menuitem_find(BuildMenu, n1);
			    if (NOT_NULL(item) &&
				item->entry.itemType != MenuSubMenu
			       )
			    {
				rxvt_menuitem_free(r, BuildMenu, item);

				/* fix up the width */
				BuildMenu->lwidth = BuildMenu->rwidth = 0;
				for (item = BuildMenu->head; NOT_NULL(item);
					item = item->next)
				{
				    unsigned short l;
				    
				    l = PTEXTWIDTH( r, item->name, item->len);
				    MAX_IT(BuildMenu->lwidth, l);

				    l = PTEXTWIDTH( r, item->name2, item->len2);
				    MAX_IT(BuildMenu->rwidth, l);

				}
			    }
			}
		    }
		    break;
	    }
	    break;
    }
}


/*
** general dispatch routine,
** it would be nice to have `sticky' menus
*/
/* EXTPROTO */
void
rxvt_menubar_control(rxvt_t *r, XButtonEvent *ev)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_control()\n"));

    switch (ev->type)
    {
	case ButtonPress:
	    if (ev->button == Button1)
		rxvt_menubar_select(r, ev);
	    break;

	case ButtonRelease:
	    if (ev->button == Button1 && r->h->ActiveMenu)
		rxvt_menu_select(r, ev);
	    break;

	case MotionNotify:
	    while (XCheckTypedWindowEvent(r->Xdisplay, r->TermWin.parent,
			      MotionNotify, (XEvent *) ev)) ;

	    if (r->h->ActiveMenu)
		while (rxvt_menu_select(r, ev)) ;
	    else
		ev->y = -1;

	    if (ev->y < 0)
	    {
		Window	      unused_root, unused_child;
		int	     unused_root_x, unused_root_y;
		unsigned int	unused_mask;

		XQueryPointer(r->Xdisplay, r->menuBar.win,
		      &unused_root, &unused_child,
		      &unused_root_x, &unused_root_y,
		      &(ev->x), &(ev->y), &unused_mask);
		rxvt_menubar_select(r, ev);
	    }
	    break;
    }
}


/*
 * read in menubar commands from FILENAME
 * ignore all input before the tag line [menu] or [menu:???]
 *
 * Note that since File_find () is used, FILENAME can be semi-colon
 * delimited such that the second part can refer to a tag
 * so that a large `database' of menus can be collected together
 *
 * FILENAME = "file"
 * FILENAME = "file;"
 *    read `file' starting with first [menu] or [menu:???] line
 *
 * FILENAME = "file;tag"
 *    read `file' starting with [menu:tag]
 */
/* EXTPROTO */
void
rxvt_menubar_load_file(rxvt_t *r, const unsigned char *filename)
{
/* read in a menu from a file */
    FILE	   *fp;
    unsigned char   buffer[256];
    unsigned char  *p, *file, *tag = NULL;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_load_file()\n"));

    file = (unsigned char*) rxvt_File_find( (char*) filename,
	    ".menu", r->h->rs[Rs_path]);
    if (IS_NULL(file))
    {
	rxvt_msg (DBG_ERROR, DBG_MENUBAR,  "Could not open file %s\n",
		filename);
	return;
    }

    fp = fopen( (char*) file, "rb");
    rxvt_free(file);
    if (IS_NULL(fp))
	return;

    /* semi-colon delimited */
    if (NOT_NULL(tag = (unsigned char*) STRCHR(filename, ';')))
    {
	tag++;
	if (*tag == '\0')
	    SET_NULL(tag);
    }

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "[read:%s]\n", filename));
    if (tag)
	rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "looking for [menu:%s]\n", tag));

    while (NOT_NULL(p = (unsigned char*) fgets( (char*) buffer, sizeof(buffer), fp)))
    {
	int	     n;

	if ((n = rxvt_str_match( (char*) p, "[menu")) != 0)
	{
	    if (tag)
	    {
		/* looking for [menu:tag] */
		if (p[n] == ':' && p[n + 1] != ']')
		{
		    n++;
		    n += rxvt_str_match( (char*) p + n, (char*) tag);
		    if (p[n] == ']')
		    {
			rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "[menu:%s]\n", tag));
			break;
		    }
		}
	    }
	    else if (p[n] == ':' || p[n] == ']')
		break;
	}
    }


    /* found [menu], [menu:???] tag */
    while (NOT_NULL(p))
    {
	int	     n;

	rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "read line = %s\n", p));

	/* looking for [done:tag] or [done:] */
	if ((n = rxvt_str_match( (char*) p, "[done")) != 0)
	{
	    if (p[n] == ']')
	    {
		break;
	    }
	    else if (p[n] == ':')
	    {
		n++;
		if (p[n] == ']')
		{
		    break;
		}
		else if (tag)
		{
		    n += rxvt_str_match( (char*) p + n, (char*) tag);
		    if (p[n] == ']')
		    {
			rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "[done:%s]\n", tag));
			break;
		    }
		}
		else
		{
		    /* what? ... skip this line */
		    p[0] = COMMENT_CHAR;
		}
	    }
	}

	/*
	 * remove leading/trailing space and strip-off leading/trailing quotes
	 * skip blank or comment lines
	 */
	rxvt_str_trim( (char*) p);
	if (*p && *p != '#') rxvt_menubar_dispatcher(r, p);

	/* get another line */
	p = (unsigned char*) fgets( (char*) buffer, sizeof(buffer), fp);
    }

    fclose(fp);
}



/* EXTPROTO */
unsigned short
rxvt_menubar_height(rxvt_t *r)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_height()\n"));

    /* If menubar is not created or not mapped, return 0 */
    return  (NOT_WIN(r->menuBar.win) || !r->menuBar.state) ?
	    0 : rxvt_menubar_rheight(r);
}


/* EXTPROTO */
unsigned short
rxvt_menubar_rheight(rxvt_t *r)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_rheight()\n"));

# ifdef XFT_SUPPORT
    if (ISSET_OPTION(r, Opt_xft))
	return (r->TermWin.pheight + 2*SHADOW + 2*MENUBAR_MARGIN);
    else
# endif
	return (r->TermWin.fheight + 2*SHADOW + 2*MENUBAR_MARGIN);
}


/* EXTPROTO */
int
rxvt_is_menubar_win(rxvt_t *r, Window w)
{
    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_is_menubar_win()\n"));

    return (w == r->menuBar.win);
}

/* EXTPROTO */
void
rxvt_menubar_resize(rxvt_t *r)
{
    menu_t  *menu;
    int	    i;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MENUBAR, "rxvt_menubar_resize()\n"));

    if (IS_WIN(r->menuBar.win) && r->menuBar.state)
	XMoveResizeWindow(r->Xdisplay, r->menuBar.win,
	    0, 0, TWIN_WIDTH(r), rxvt_menubar_rheight(r));

    /*
     * All submenus now need to be traversed and resized.
     */
    for (menu = r->h->MenuBar.head; NOT_NULL(menu); menu = menu->next)
    {
	resizeSubMenus( r, menu );

	/* X coordinate of menu names in menubar need to be updated */
	if( menu->prev )
	    menu->x = menu->prev->x + 2 * HSPACE_PIXEL
		+ PTEXTWIDTH( r, menu->prev->name, menu->prev->len);
    }

    /*
     * Resize the popup menus if any.
     */
    for( i=0; i < 3; i++)
	if( r->h->popupMenu[i] ) resizeSubMenus( r, r->h->popupMenu[i]);
}

/*
 * Update menu->width for all submenus.
 */
/* INTPROTO */
void
resizeSubMenus( rxvt_t *r, menu_t *menu)
{
    menuitem_t *item;

    menu->lwidth = menu->rwidth = 0;

    for( item = menu->head; NOT_NULL(item); item = item->next)
    {
	unsigned short width;
	
	width = PTEXTWIDTH( r, item->name, item->len);
	if( menu->lwidth < width ) menu->lwidth = width;

	width = PTEXTWIDTH( r, item->name2, item->len2);
	if( menu->rwidth < width ) menu->rwidth = width;

	if( item->entry.itemType == MenuSubMenu && item->entry.submenu.menu)
	    resizeSubMenus( r, item->entry.submenu.menu );
    }
}

#endif	/* HAVE_MENUBAR */
/*----------------------- end-of-file (C source) -----------------------*/
