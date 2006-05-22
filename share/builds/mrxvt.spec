#
# RPM spec file for mrxvt-0.4.0
#
# Copyright  (c)  2004-2005  Jingyu Zhou <jzhou@cs.ucsb.edu>
# Copyright  (c)  2005-2006  Jingmin Zhou <jimmyzhou@users.sourceforge.net>
#

Name:      mrxvt
Summary:   A lightweight multi-tabbed X terminal
Version:   0.5.0
Release:   1
License:   GPL
Vendor:    The mrxvt team <http://materm.sourceforge.net/>
Packager:  Jingyu Zhou <jzhou@cs.ucsb.edu>
Group:     User Interface/X
Source0:   /home/jzhou/tmp/t/mrxvt-%version.tar.gz
BuildRoot: %_tmppath/%name-%version-%release-root

%description
Mrxvt (previously named as materm) is a lightweight and powerful
multi-tabbed X terminal emulator based on the popular rxvt and aterm.
It implements many useful features seen in some modern X terminal
emulators, like gnome-terminal and konsole, but keep to be lightweight
and independent from the GNOME and KDE desktop environment.

%prep
%setup -q

%build
./configure --prefix=/usr --sysconfdir=/etc --enable-everything --with-save-lines=600 --with-encoding=noenc --disable-ourstrings --disable-debug
make

%install
#make install
mkdir -p %buildroot/usr/share/doc/mrxvt
mkdir -p %buildroot/usr/share/pixmaps
mkdir -p %buildroot/usr/bin
mkdir -p %buildroot/etc/mrxvt
make DESTDIR=%buildroot install

%clean
rm -rf %buildroot

%post

%postun

%files

%dir /usr/share/doc/mrxvt
#/usr/share/doc/mrxvt

/usr/bin/mrxvt
/etc/mrxvt/default.menu
/etc/mrxvt/mrxvtrc
/etc/mrxvt/mrxvtrc.sample
/etc/mrxvt/submenus.menu
/usr/man/man1/mrxvt.1
/usr/share/doc/mrxvt/behind_mrxvt.txt.bz2
/usr/share/doc/mrxvt/hack-mrxvt.txt.bz2
/usr/share/doc/mrxvt/mrxvt_seq.txt.bz2
/usr/share/doc/mrxvt/README.greek.bz2
/usr/share/doc/mrxvt/README.menu.bz2
/usr/share/doc/mrxvt/README.xvt.bz2
/usr/share/doc/mrxvt/rxvtRef.txt.bz2
/usr/share/doc/mrxvt/scripts/mrxvt.vbs
/usr/share/doc/mrxvt/scripts/mrxvtset.pl
/usr/share/doc/mrxvt/scripts/settitle.c
/usr/share/doc/mrxvt/xterm-ctlseqs.txt.bz2
/usr/share/pixmaps/mrxvt-csh.png
/usr/share/pixmaps/mrxvt-csh.xpm
/usr/share/pixmaps/mrxvt-root.png
/usr/share/pixmaps/mrxvt-root.xpm
/usr/share/pixmaps/mrxvt.png
/usr/share/pixmaps/mrxvt.xpm



%changelog
* Sun May 21 2005 Jingmin Zhou <jimmyzhou@users.sourceforge.net>
- Fix configure script for 0.5.x branch

* Sun Feb 27 2005 Jingmin Zhou <jimmyzhou@users.sourceforge.net>
- Update configure script

* Sat Feb 26 2005 Jingyu Zhou <jzhou@cs.ucsb.edu>
- Version 0.4.0 update

* Sun Dec 12 2004 Jingyu Zhou <jzhou@cs.ucsb.edu>
- Version 0.3.11 update

* Mon Dec 06 2004 Jingyu Zhou <jzhou@cs.ucsb.edu>
- Version 0.3.10 update

* Tue Nov 23 2004 Jingyu Zhou <jzhou@cs.ucsb.edu>
- created this spec file
