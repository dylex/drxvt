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
struct trunk_head_t;
struct trunk_list_t;


#ifdef _DEBUG
#define TRUNK_MAGIC		(0x41414141)
#define BLOCK_MAGIC		(0x43434343)
#endif


/*
 * This structure has two states: (1) when it records a free block, u.next
 * stores the address of next block's head; (2) when it records a block
 * in use, u.trunk stores the address of the trunk it belonds to.
 */
struct block_head_t
{
#ifdef _DEBUG
    RUINT32T	magic_f;	/* magic number */
#endif
    union
	{
		struct block_head_t*	next;	/* next block */
		struct trunk_head_t*	trunk;	/* address of trunk_head */
	} u;
#ifdef _DEBUG
    RUINT32T    magic_e;	/* magic number */
#endif
};

#define BHEAD_OFFSET	(sizeof (struct block_head_t))
#define SYS_MALLOC_PTR	(-1)


/*
 * Note: the name trunk_head_t is somewhat misleading. Actually it resides
 * at the END of each allocated trunk.
 */
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
struct trunk_list_t
{
    RUINT16T		    block_size;
    struct trunk_head_t*    first_trunk;
};


#endif /* OUR_MALLOC */

#endif	/* __RXVTMEM_H__ */
/*----------------------- end-of-file (H source) -----------------------*/
