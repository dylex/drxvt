# All portions of code are copyright by their respective author/s.
#   Copyright (c) 2006	   Gautam Iyer <gi1242@users.sourceforge.net>
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

docdir = $(datadir)/doc/$(PACKAGE_NAME)

dist_conf_DATA = default.menu submenus.menu mrxvtrc mrxvtrc.sample
confdir = $(sysconfdir)/$(PACKAGE_NAME)

# Distribute scripts with documentation. Package maintaners can clean them up
# and install them if necessary.
dist_scripts_DATA = scripts/mrxvt.vbs scripts/mrxvtset.pl scripts/settitle.c
scriptsdir = $(docdir)/scripts

dist_pixmap_DATA =  pixmaps/mrxvt-csh.png	\
		    pixmaps/mrxvt-root.png	\
		    pixmaps/mrxvt.png		\
		    pixmaps/mrxvt-csh.xpm	\
		    pixmaps/mrxvt-root.xpm	\
		    pixmaps/mrxvt.xpm
pixmapdir = $(datadir)/pixmaps

# builds_DATA =	builds/mrxvt.SlackBuild		\
#		builds/mrxvt.slack-desc		\
#		builds/mrxvt.spec
# buildsdir = $(pkgdatadir)/builds

dist_etc_DATA  =    etc/XTerm.ad		\
		    etc/mrxvt.terminfo		\
		    etc/rxvt.termcap		\
      		    etc/rxvt.terminfo
etcdir = $(docdir)/etc
