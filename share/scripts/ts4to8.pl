#! /usr/bin/perl -w
# Created	: Mon 24 Jul 2006 08:23:22 PM CDT
# Modified  	: Mon 24 Jul 2006 11:01:48 PM CDT
# Author    	: Gautam Iyer <gautam@math.uchicago.edu>
#
# Description
#
#   Converts files with ts=4 to ts=8 without changing their appearance.
#

use strict;
use integer;

my $tabLine = "\t\t\t\t\t\t\t\t\t\t";

my $line;

while( $line = <> )
{
    my ( $i, $col ) = ( 0, 1 );

    chomp( $line );
    while( $i < length( $line ) )
    {
	if( substr( $line, $i, 1 ) eq "\t" )
	{
	    my ($ntabs, $lastcol, $repl);
	    $col--;	    # To represent the last non-tab

	    substr( $line, $i ) =~ m/^\t+/;
	    $ntabs = length( $& );

	    $lastcol = $col - ($col % 4) + $ntabs * 4;
	    $repl = substr( $tabLine, 1, $lastcol / 8 - $col / 8 );
	    if( $lastcol % 8 )
	    {
		$repl .= substr( "    ", 0,
				    $ntabs == 1 ? ( 4 - ($col % 4) ) : 4 );
	    }

	    substr( $line, $i, $ntabs ) = $repl;

	    $i += length( $repl );
	    $col = $lastcol + 1;
	}

	else
	{
	    $i++;
	    $col++;
	}
    }

    print( "$line\n" );
}
