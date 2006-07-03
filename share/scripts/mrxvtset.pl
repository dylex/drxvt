#!/usr/bin/perl -w
#
# All portions of code are copyright by their respective author/s.
# Copyright (c) 2004     Avi Yagodnick  <aviy@bezeqint.net>
# Copyright (c) 2006     mark olesen
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# ---------------------------------------------------------------------------
#
# Mark Olesen: 2006-01-10
# Complete code re-write (uses warnings/strict) as all perl code should.
# The entire program is a set of callbacks for Getopt::Long
#
# ---------------------------------------------------------------------------
use strict;
use Getopt::Long;
( my $Script = $0 ) =~ s{^.*/}{};

our ( $VERSION, $DATE ) = do {
    my ( $ver, $date ) =
      q{$Id: mrxvtset.pl,v 1.2 2006/02/24 23:51:44 jimmyzhou Exp $} =~
      m{\s+(\d+\.\d+).*?\s+(\d{4}[-/]\d{2}[-/]\d{2})};
    $date =~ s{/}{-}g;    # ISO-formatted
    ( $ver, $date );
};

my %config = (
    ## set it to your favourite location you wish
    pixmap => "/tmp/rxvtpixmap-" . getpwuid($<) . ".xpm",
);

#
# conversion of file formats
#
sub convert_pixmap {
    my ($val) = @_;
    -f $val or return undef;

    return $val if $val =~ /\.(xpm)$/;
    if ( $val =~ /\.(gif|jpe?g|png)$/i ) {
	system( convert => $val => $config{pixmap} );
	chmod 0666 => $config{pixmap};
	return $config{pixmap};
    }

    return undef;
}

#
# lookup table with the xterm codes
#
my %opcode = (
    title => 0,    # change title and iconName
    ##  XTerm_name		(0)
    ##  XTerm_iconName		(1)
    ##  XTerm_title		(2)
    ##  XTerm_Color		(4)  /* change colors */
    cursor    => 12,    # Xterm_Color_cursor: change actual 'Cursor' color
    pointer   => 13,    # Xterm_Color_pointer: change actual 'Pointer' color
    highlight => 17,    # XTerm_Color_RV: change actual 'Highlight' color
    bold      => 18,    # Xterm_Color_BD: change actual 'Bold' color
    underline => 19,    # Xterm_Color_UL: change actual 'Underline' color
    ##  XTerm_logfile		(46) /* not implemented */
    fn => 50,           # Xterm_font: restrict our use to font numbers
    ##  XTerm_Menu		(10) /* set menu item */
    pixmap => 20,       # new bg pixmap
    fg     => 39,       # Xterm_restoreFG: change default fg color
    bg     => 49,       # Xterm_restoreBG: change default bg color
    ##  XTerm_dumpscreen	(55) /* dump scrollback and all screen */
    tabtitle => 60,     # Xterm_tab: change tab title
    ##  Xterm_tabterm		(62) /* change tab and terminal title */
    newtab => 63,       # create a new tab with title
    ##  Xterm_prevtab		(64) /* switch to previous tab */
    ##  Xterm_nexttab		(65) /* switch to next tab */
    ##  Xterm_tint		(66) /* change tinting color */
    ##  Xterm_shade		(67) /* change shade level */
    ##  Xterm_encode		(68) /* change encoding */
    tb => 69,           # Xterm_hide: hide/show tabbar
    ##  Xterm_opacity		(70) /* set opacity level */
    tabbtn => 71,       # hide/show tabbar buttons
    tabfg  => 72,       # change active tab fg
    tabbg  => 73,       # change tabbar/active tab bg
    itabfg => 74,       # change inactive tab fg
    itabbg => 75,       # change inactive tab bg
    trans  => 76,       # toggle transparency
    left   => 77,       # Xterm_moveleft: move active tab to left
    right  => 78,       # Xterm_moveright: move active tab to right
    ##  Xterm_verybold		(79) /* toggle bold font for color text */
    ##  Xterm_hotkeys		(80) /* toggle hotkeys */
    ##  Xterm_saveconfig	(81) /* save configuration */
    ##  Xterm_bgfade		(82) /* set bgfade degree */
    ##  Xterm_termenv		(83) /* set TERMENV type */
);

#
# process high/low/toggle commands
#
sub high_low_toggle {
    my ( $type, $val, $opcode ) = @_;

    print "\033[?${opcode}$val\007"
      if $val =~ s/^(1|y|t).*$/h/i    # high
      or $val =~ s/^(0|n|f).*$/l/i    # low
      or $val =~ s/^$/t/              # toggle
}

# XTerm OSCs:
# ESC ] Ps;Pt (ST|BEL)
# process xterm toggle commands
sub xterm_osc {
    my ( $type, $val, $opcode ) = @_;

    # warn "$type <$val>\n";
    my $op = $opcode || $opcode{$type};
    if ( defined $op ) {
        print "\033]${op};$val\007";
    }
    else {
        warn "programming error for xterm_osc '$type'\n";
    }
}

# ---------------------------------------------------------------------------
sub usage {
    $! = 0;    # clean exit
    warn @_, "\n" if @_;
    die << "USAGE";
usage: $Script  { SETTINGS }

SETTINGS:
  -fg        COLOR     change foreground color
  -fg        0-15      change foreground to a particular color index
  -bg        COLOR     change background color
  -bg        0-15      change background to a particular color index
  -cursor    COLOR     change cursor color    (alias: -cr)
  -pointer   COLOR     change pointer color   (alias: -pr)
  -bold      COLOR     change bold color      (alias: -bd)
  -underline COLOR     change underline color (alias: -ul)
  -highlight COLOR     change highlight (reverse video) color (alias: -rv)
  -tabfg     COLOR     change active    tab foreground
  -tabbg     COLOR     change active    tab background
  -itabfg    COLOR     change inactive  tab foreground (alias: -tabifg)
  -itabbg    COLOR     change inactive  tab background (alias: -tabibg)
  -pixmap    FILE      if file is not xpm, uses 'convert'.
                       Note special names: "none", "full", "tiled"
                       (NB: full/tiled seem to be broken)
  -fn        [INDEX]   font number 0-6 or +/-, empty for default
  -title     STRING    change terminal title and icon name
  -tabtitle  STRING    change tab title (*)
  -newtab    STRING    create new tab with title (*)
  -left                move active tab to left
  -right               move active tab to right
  -tb                  toggle tabbar visibility (*)
  -tbb                 toggle tabbar button visibility (alias: tabbtn) (*)
  -tr                  toggle transparency (alias: trans) (*)
  -mb [Y|N|empty]      set, unset or toggle menubar   visibility
  -sb [Y|N|empty]      set, unset or toggle scrollbar visibility

Change (m)rxvt parameters on the fly.
NB: 'COLOR' can be a color name or specification
NB: aterm only supports some options (does not support fg)
(*) - mrxvt only

version $VERSION ($DATE)
copyright (c) 2006 Mark Olesen <Mark.Olesen\@ArvinMeritor.com>

USAGE
}

# ---------------------------------------------------------------------------
my %opt;
@ARGV or usage("missing settings");
GetOptions(
    \%opt,
    "help|h" => sub { die "\n"; },    # reverts to usage

    # select fonts by number or relative (-/+)
    # ---------------
    "fn:s" => sub {
	my ( $type, $val ) = @_;
	if ( $val =~ /^([-+]|\d+)$/ or $val =~ s/^(default|normal)?$// ) {
	    xterm_osc( $type => "#$val" );
	}
    },

    # foreground color - ensure reset screen works
    # ---------------
    "fg=s" => sub {
	my ( $type, $val ) = @_;
	xterm_osc( fg => $val );
    },

    # background color - problematic with / without pixmap
    # ---------------
    "bg=s" => sub {
	my ( $type, $val ) = @_;

	## use the direct method if an index was specified
	if ( $val =~ /^\d+$/ ) {
	    xterm_osc( bg => $val );
	}
	else {
	    ## let us sneak in pixmaps here too!
	    my $xpm = convert_pixmap($val);

	    unless ( defined $xpm ) {
		$xpm = $config{pixmap};

		## cheat - set background color indirectly (via a pixmap)
		local *FILE;
		open FILE, ">$xpm"
		  or warn "Cannot write to pixmap '$xpm'\n"
		  and return;

		print FILE << "PIXMAP";
/* XPM */
static char *rxvtbg[] = {
/* columns rows colors chars-per-pixel */
"1 1 1 1",
"x c $val",
/* pixels */
"x"
};
PIXMAP
		chmod 0666 => $xpm;
		close FILE;

	    }
	    xterm_osc( pixmap => $xpm );
	}
    },

    # background pixmap control
    # ---------------
    "pixmap=s" => sub {
	my ( $type, $val ) = @_;
	if ( $val eq "none" ) {
	    xterm_osc( pixmap => "" );
	}
	## this part seems to be very broken !
	elsif ( $val eq "full" ) {
	    print "\033[m";
	    xterm_osc( pixmap => ";100x100+50+50;?" );
	}
	elsif ( $val eq "tiled" ) {
	    print "\033[m";
	    xterm_osc( pixmap => ";;0" );
	}
	elsif ( -f $val ) {
	    my $xpm = convert_pixmap($val);
	    defined $xpm
	      or warn "skipping unknown file format\n"
	      and return;

	    xterm_osc( pixmap => $val ) if -f $val;
	}
	else {
	    warn "skipping non-existent file\n";
	}
    },

    #
    # high/low/toggle
    #
    "sb:s" => sub { high_low_toggle( @_, 30 ) },    # scrollbar visibility
    "mb:s" => sub { high_low_toggle( @_, 10 ) },    # menubar visibility

    #
    # the following use xterm_osc() directly
    #
    (
	map { ( $_ => \&xterm_osc ) } (
	    "cursor|cr=s",        # change cursor color
	    "pointer|pr=s",       # change pointer color
	    "bold|bd=s",          # change bold color
	    "underline|ul=s",     # change underline color
	    "highlight|rv=s",     # change highlight color
	    "tb",                 # toggle tabbar visibility
	    "tabbtn|tbb",         # toggle tabbar button visibility
	    "tabfg=s",            # change active tab fg
	    "tabbg=s",            # change active tab bg
	    "itabfg|tabifg=s",    # change inactive tab fg
	    "itabbg|tabibg=s",    # change inactive tab bg
	    "trans|tr",           # toggle transparency
	    "left",               # move active tab to left
	    "right",              # move active tab to right
	    "title=s",            # change terminal title and icon name
	    "tabtitle=s",         # change tab title
	    "newtab=s",           # create new tab with title
	)
    ),
  )
  or usage();

# use Data::Dumper;
# warn Dumper(\%opt);

# --------------------------------------------------------------- end-of-file
