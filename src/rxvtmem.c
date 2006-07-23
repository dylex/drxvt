/*--------------------------------*-C-*---------------------------------*
 * File:	rxvt.c
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

#include "rxvtmem.h"


#ifdef DEBUG_VERBOSE
#define DEBUG_LEVEL 1
#else 
#define DEBUG_LEVEL 0
#endif

#if DEBUG_LEVEL
#define DBG_MSG(d,x) if(d <= DEBUG_LEVEL) fprintf x
#else
#define DBG_MSG(d,x)
#endif


/* -------------------------------------------------------------------- *
 * MEMORY ALLOCATION WRAPPERS						*
 * -------------------------------------------------------------------- */

#ifdef OUR_MALLOC

/* INTPROTO */
void
rxvt_mem_check (void)
{
    fprintf(stderr, APL_NAME ": unimplemented feature.  Aborting");
    exit(EXIT_FAILURE);
}


/* EXTPROTO */
void
rxvt_mem_init (void)
{
    fprintf(stderr, APL_NAME ": unimplemented feature.  Aborting");
    exit(EXIT_FAILURE);
}


/* EXTPROTO */
void
rxvt_mem_exit (void)
{
    fprintf(stderr, APL_NAME ": unimplemented feature.  Aborting");
    exit(EXIT_FAILURE);
}


/* EXTPROTO */
void*
rxvt_malloc(size_t size)
{
    fprintf(stderr, APL_NAME ": unimplemented feature.  Aborting");
    exit(EXIT_FAILURE);
}


/* EXTPROTO */
void*
rxvt_calloc(size_t number, size_t size)
{
    fprintf(stderr, APL_NAME ": unimplemented feature.  Aborting");
    exit(EXIT_FAILURE);
}


/* EXTPROTO */
void*
rxvt_realloc(void* ptr, size_t len)
{
    fprintf(stderr, APL_NAME ": unimplemented feature.  Aborting");
    exit(EXIT_FAILURE);
}


/* EXTPROTO */
void*
rxvt_free(void* ptr)
{
    fprintf(stderr, APL_NAME ": unimplemented feature.  Aborting");
    exit(EXIT_FAILURE);
}


#else	/* !OUR_MALLOC */


/* EXTPROTO */
void*
rxvt_malloc(size_t size)
{
	void*		p;

	/* see AC_FUNC_MALLOC macro in autoconf documentation */
	if (0 == size)
		size = 1;

	p = malloc(size);
	if (p)
		return p;

	fprintf(stderr, APL_NAME ": memory allocation failure.  Aborting");
	exit(EXIT_FAILURE);
	/* NOTREACHED */
}


/* EXTPROTO */
void*
rxvt_calloc(size_t number, size_t size)
{
	void*		p;

	p = calloc(number, size);
	if (p)
		return p;

	fprintf(stderr, APL_NAME ": memory allocation failure.  Aborting");
	exit(EXIT_FAILURE);
	 /* NOTREACHED */
}


/* EXTPROTO */
void*
rxvt_realloc(void *ptr, size_t size)
{
	void*		p;

	if (ptr)
		p = realloc(ptr, size);
	else
		p = malloc(size);
	if (p)
		return p;

	fprintf(stderr, APL_NAME ": memory allocation failure.  Aborting");
	exit(EXIT_FAILURE);
	/* NOTREACHED */
}


/* EXTPROTO */
void
rxvt_free(void* ptr)
{
    if (IS_NULL(ptr))	{
	fprintf(stderr, APL_NAME ": free NULL pointer.  Aborting");
	exit(EXIT_FAILURE);
    }

    free (ptr);
}


#endif	/* OUR_MALLOC */

/*----------------------- end-of-file (C source) -----------------------*/
