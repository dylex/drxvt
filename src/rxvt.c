/*--------------------------------*-C-*---------------------------------*
 * File:    rxvt.c
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1997-2001   Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2004        Jingmin Zhou <jimmyzhou@users.sourceforge.net>
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



/*----------------------------------------------------------------------*/
/* main() */
/* INTPROTO */
int
main(int argc, const char *const *argv)
{
    rxvt_t*	rxvt_vars;

    /*
     * Save and then give up any super-user privileges immediately after program
     * starts. If we need privileges in any area then we must specifically
     * request it.
     *
     * We should only need to be root in these cases:
     *
     *	    1.  write utmp entries on some systems
     *	    2.  chown tty on some systems
     */
    rxvt_privileges(SAVE);
    rxvt_privileges(IGNORE);

    /*
     * Initialize debug level and debug mask. Must do it first in order
     * to debug memory management system.
     */
    rxvt_parse_dbg_arguments (argc, argv);

#ifdef OUR_MALLOC
    /* initialize our memory management system */
    rxvt_mem_init ();
#endif

    if ((rxvt_vars = rxvt_init(argc, argv)) == NULL)
	return EXIT_FAILURE;

    rxvt_main_loop(rxvt_vars);	/* main processing loop */

#ifdef OUR_MALLOC
    rxvt_mem_exit ();
#endif

    return EXIT_SUCCESS;
}
/*----------------------- end-of-file (C source) -----------------------*/
