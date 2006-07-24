/*--------------------------------*-H-*---------------------------------*
 * File:	grkelot.h
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2006        Jingmin Zhou <jimmyzhou@users.sourceforge.net>
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

#ifndef __RXVTMEM_H__
#define __RXVTMEM_H__


#ifdef OUR_MALLOC

struct block_head_t;
struct block_head_t
{
#ifdef _DEBUG
    RUINT32T		    magic;/* magic number */
#endif
    struct block_head_t*    next; /* next block */
};


/*
 * Note: the name trunk_head_t is somewhat misleading. Actually it resides
 * at the END of each allocated trunk.
 */
struct trunk_head_t;
struct trunk_head_t
{
#ifdef _DEBUG
    RUINT32T	magic;  /* magic number */
#endif
    struct trunk_head_t*    prev; /* prev trunk with the same block size */
    struct trunk_head_t*    next; /* next trunk with the same block size */
    struct block_head_t*    fblock; /* first free block in this trunk */
    RUINT16T	bmax;   /* max # of free blocks in this trunk */
    RUINT16T	fcount; /* # of free blocks in this trunk */
    RUINT16T	bsize;  /* block size to allocate */
};


/*
 * Header for trunk list of each block size
 */
struct trunk_list_t;
struct trunk_list_t
{
    RUINT16T		    block_size;
    struct trunk_head_t*    first_trunk;
};


#endif /* OUR_MALLOC */

#endif	/* __RXVTMEM_H__ */
/*----------------------- end-of-file (H source) -----------------------*/
