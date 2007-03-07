# All portions of code are copyright by their respective author/s.
#
#   Copyright (c) 2002   Alexis <materm@tuxfamily.org>
#   Copyright (c) 2004   Jingmin Zhou <jimmyzhou@users.sourceforge.net>
#   Copyright (c) 2006	 Gautam Iyer <gi1242@users.sourceforge.net>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 675 Mass
# Ave, Cambridge, MA 02139, USA.

DEFS = -DPKG_CONF_DIR=\"$(sysconfdir)/$(PACKAGE_NAME)\"

bin_PROGRAMS = mrxvt
mrxvt_SOURCES =	    command.h		\
	       	    debug.h		\
	       	    encoding.h		\
	       	    feature.h		\
	       	    init.h		\
	       	    menubar.h		\
	       	    protos.h		\
	       	    rxvt.h		\
	       	    rxvtlib.h		\
		    rxvtmem.h


mrxvt_SOURCES +=    command.c		\
		    debug.c		\
		    encoding.c		\
		    init.c		\
		    macros.c		\
		    main.c		\
		    menubar.c		\
		    misc.c		\
		    netdisp.c		\
		    pixmap.c		\
		    ptytty.c		\
		    rxvt.c		\
		    rxvtmem.c		\
		    screen.c		\
		    scrollbar.c		\
		    strings.c		\
		    tabbar.c		\
		    transparent.c	\
		    xdefaults.c

mrxvt_SOURCES +=    close_term.xbm	\
		    left.xbm		\
		    right.xbm		\
		    term.xbm

mrxvt_SOURCES +=    close_term.xpm	\
		    close_term_d.xpm	\
		    left.xpm		\
		    left_d.xpm		\
		    right.xpm		\
		    right_d.xpm		\
		    term.xpm		\
		    term_d.xpm

#install-exec-hook:
#   -chown root $(DESTDIR)$(bindir)/mrxvt
#   -chmod go-r $(DESTDIR)$(bindir)/mrxvt
#   -chmod u+s $(DESTDIR)$(bindir)/mrxvt

if USE_JPEG
JPEG_SRC = mjpg.c
else
JPEG_SRC = 
endif

if USE_PNG
PNG_SRC = mpng.c
else
PNG_SRC = 
endif

if PLAIN_SCROLLBAR
SCROLLBAR_PLAIN_SRC = scrollbar-plain.c
else
SCROLLBAR_PLAIN_SRC = 
endif

if XTERM_SCROLLBAR
SCROLLBAR_XTERM_SRC = scrollbar-xterm.c
else
SCROLLBAR_XTERM_SRC = 
endif

if NEXT_SCROLLBAR
SCROLLBAR_NEXT_SRC = scrollbar-next.c
else
SCROLLBAR_NEXT_SRC = 
endif

if RXVT_SCROLLBAR
SCROLLBAR_RXVT_SRC = scrollbar-rxvt.c
else
SCROLLBAR_RXVT_SRC = 
endif

if SGI_SCROLLBAR
SCROLLBAR_SGI_SRC = scrollbar-sgi.c
else
SCROLLBAR_SGI_SRC = 
endif

if UTMP_SUPPORT
UTMP_SRC = logging.c
else
UTMP_SRC =
endif

if GREEK
GREEK_SRC = grkelot.c grkelot.h
else
GREEK_SRC = 
endif

if SESSION_SUPPORT
SESSION_SRC = session.c
else
SESSION_SRC = 
endif

if XFT_SUPPORT
XFTACS_SRC = xftacs.c xftacs.h
XFTACS_H   = xftacs.h
else
XFTACS_SRC =
XFTACS_H   =
endif


mrxvt_SOURCES +=    $(SCROLLBAR_PLAIN_SRC)	\
		    $(SCROLLBAR_XTERM_SRC)	\
		    $(SCROLLBAR_NEXT_SRC)	\
		    $(SCROLLBAR_RXVT_SRC)	\
		    $(SCROLLBAR_SGI_SRC)	\
		    $(UTMP_SRC)			\
		    $(GREEK_SRC)		\
		    $(JPEG_SRC)			\
		    $(PNG_SRC)			\
		    $(SESSION_SRC)		\
		    $(XFTACS_SRC)

MRXVT_H =	    ../config.h			\
		    command.h			\
		    debug.h			\
		    feature.h			\
		    menubar.h			\
		    protos.h			\
		    rxvt.h			\
		    rxvtlib.h

MRXVT_PIXMAPS =	    left.xbm			\
		    right.xbm			\
		    close_term.xbm		\
		    term.xbm			\
		    left.xpm			\
		    left_d.xpm			\
		    right.xpm			\
		    right_d.xpm			\
		    close_term.xpm		\
		    close_term_d.xpm		\
		    term.xpm			\
		    term_d.xpm



AUTOMAKE_OPTIONS = no-dependencies

# set dependencies
$(bin_PROGRAMS) : 
command.o:	    $(MRXVT_H)				command.c
encoding.o:	    $(MRXVT_H) encoding.h		encoding.c
grkelot.o:	    $(MRXVT_H) grkelot.h		grkelot.c
macros.o:	    $(MRXVT_H)				macros.c
init.o:		    $(MRXVT_H) $(XFTACS_H) init.h	init.c
logging.o:	    $(MRXVT_H)				logging.c
main.o:		    $(MRXVT_H) $(XFTACS_H)		main.c
menubar.o:	    $(MRXVT_H) 				menubar.c
misc.o:		    $(MRXVT_H) 				misc.c
mjpg.o:		    $(MRXVT_H) 				mjpg.c
mpng.o:		    $(MRXVT_H) 				mpng.c
netdisp.o:	    $(MRXVT_H) 				netdisp.c
pixmap.o:	    $(MRXVT_H) 				pixmap.c
ptytty.o:	    $(MRXVT_H) 				ptytty.c
rxvt.o:		    $(MRXVT_H) 				rxvt.c
rxvtmem.o:	    $(MRXVT_H) 				rxvtmem.h rxvtmem.c
screen.o:	    $(MRXVT_H) $(XFTACS_H)		screen.c
scrollbar-next.o:   $(MRXVT_H) 				scrollbar-next.c
scrollbar-plain.o:  $(MRXVT_H) 				scrollbar-plain.c
scrollbar-rxvt.o:   $(MRXVT_H) 				scrollbar-rxvt.c
scrollbar-sgi.o:    $(MRXVT_H) 				scrollbar-sgi.c
scrollbar-xterm.o:  $(MRXVT_H) 				scrollbar-xterm.c
scrollbar.o:	    $(MRXVT_H) 				scrollbar.c
session.o:	    $(MRXVT_H) 				session.c
strings.o:	    $(MRXVT_H) 				strings.c
tabbar.o:	    $(MRXVT_H) $(MRXVT_PIXMAPS)		tabbar.c
xdefaults.o:	    $(MRXVT_H) 				xdefaults.c
transparent.o:	    $(MRXVT_H) 				transparent.c
xftacs.o:			xftacs.h		xftacs.c
