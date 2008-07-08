/*--------------------------------*-H-*---------------------------------*
 * File:	rxvtmem.h
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


#ifdef DEBUG
#define TRUNK_MAGIC		(0x41414141)
#define BLOCK_MAGIC		(0x43434343)
#define MEMORY_MAGIC		(0x47474747)
#endif

/* special pointer to indicate that the block is allocated using system
 * wide malloc instead of our trunk/block system. It means that we will
 * waste some memory on tracking these memory blocks before initializing
 * our track/block system.
 */
#define SYS_MALLOC_PTR	(-1)


/*
 * This structure has two states: (1) when it records a free block, u.next
 * stores the address of next block's head; (2) when it records a block
 * in use, u.trunk stores the address of the trunk it belonds to.
 */
struct block_head_t
{
#ifdef DEBUG
    uint32_t	magic_f;	/* magic number */
    size_t	bbyte;		/* allocated bytes */
#endif
    union
    {
        struct block_head_t*	next;	/* next block */
	struct trunk_head_t*	trunk;	/* address of trunk_head */
    } u;
#ifdef DEBUG
    uint32_t    magic_e;	/* magic number */
#endif
};

/*
 * Note: the name trunk_head_t is somewhat misleading. Actually it resides
 * at the END of each allocated trunk.
 */
struct trunk_head_t
{
#ifdef DEBUG
    uint32_t	magic_f;  /* magic number */
    size_t	tbyte;  /* total allocated bytes in this trunk */
#endif
    struct block_head_t*    begin;/* begin address of the trunk. this is
				   * ONLY used by get_trunk, init_trunk
				   * and free trunk, and NOTHING ELSE!!!
				   */

    struct trunk_head_t*    prev; /* prev trunk with the same block size */
    struct trunk_head_t*    next; /* next trunk with the same block size */
    struct trunk_list_t*    list; /* entry in the trunk list of this trunk*/
    struct block_head_t*    fblock; /* first free block in this trunk */
    uint16_t	bmax;   /* max # of free blocks in this trunk */
    uint16_t	fcount; /* # of free blocks in this trunk */
    uint16_t	bsize;  /* block size to allocate (exclude head) */
#ifdef DEBUG
    uint32_t	magic_e;  /* magic number */
#endif
};


/*
 * Header for trunk list of each block size
 */
struct trunk_list_t
{
    uint16_t	    block_size;	/* block size for this trunk list */
    uint16_t	    trunk_count;/* how many trunks allocated */
    union
    {
	size_t	    bnum;	/* number of blocks to allocate, used
				 * before rxvt_mem_init as hint
				 */
	size_t	    tsize;	/* actual trunk size (exclude trunk head)
				 * after rxvt_mem_init
				 */
    } u;
    struct trunk_head_t*    first_trunk;
};


/* size of trunk_head_t and block_head_t */
#define THEAD_OFFSET	(sizeof (struct trunk_head_t))
#define BHEAD_OFFSET	(sizeof (struct block_head_t))

/* 4KB is the default trunk size */
#define DEFAULT_TRUNK_SIZE	(1UL << 12)
/* 1KB is the minimal trunk size */
#define MINIMAL_TRUNK_SIZE	(1UL << 10)
/* 64KB is the optimal trunk size */
#define OPTIMAL_TRUNK_SIZE	(1UL << 16)
/* 1MB is the maximal trunk size, are you crazy?! */
#define MAXIMAL_TRUNK_SIZE	(1UL << 20)

/* stage threshold to increase trunk size based on trunk count in a
 * trunk list.
 */
#define TRUNK_INCREASE_STAGE	(3)


#endif /* OUR_MALLOC */

#endif	/* __RXVTMEM_H__ */
/*----------------------- end-of-file (H source) -----------------------*/
