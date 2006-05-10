/*-----------------------------------*-H-*-------------------------------------*
 * File		: xftacs.h
 * Created	: Tue 27 Dec 2005 09:59:55 PM CST
 * Modified	: Wed 12 Apr 2006 01:42:08 AM CDT
 * Author	: Gautam Iyer <gi1242@users.sourceforge.net>
 * Description	: Header file for Xft ACS graphics chars. See comments in
 *		  xftacs.c for details.
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
 *----------------------------------------------------------------------------*/

#ifndef XFTACS_H
#define XFTACS_H

void xftInitACS ( Display *dpy, Drawable d, unsigned depth);
void xftCloseACS ( Display *dpy);

void xftDrawACSString (
	Display			*dpy,
	Drawable		d,
	GC			gc,
	void			(*xftdraw_string)(),
	XftDraw			*draw,
	const XftColor		*color,
	XftFont			*pub,
	int			x,
	int			y,
	const unsigned char	*str,
	int			len
    );

#endif
/*-------------------------- end-of-file (H source) --------------------------*/
