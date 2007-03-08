/*--------------------------------*-C-*---------------------------------*
 * File:	rxvtmem.c
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


static const char*  abort_msg = APL_NAME ": memory allocation failure.  Aborting";

/* -------------------------------------------------------------------- *
 *                      MEMORY ALLOCATION WRAPPERS						*
 * -------------------------------------------------------------------- */

#ifdef OUR_MALLOC

#ifdef DEBUG
static int  memory_initialized = 0;
#endif

/* Use use our malloc function only if we bypass the memory check */
static int  use_our_malloc = 1;



/* Several notes:
 * . Block size must be aligned to 8 (64bit)!!! Or we will enforce it.
 * . Block size must be in increasing order!!! Or there will be crash.
 *   no kidding ;)
 * . The last entry must have block size 0. Or we are going to access
 *   all the memory available, and of course crash in the end ;-)
 */
static struct trunk_list_t	g_trunk_list[] =
{
    /* block_size, trunk_count, block_num/trunk_size, first_trunk */
    { 16,  0, {125}, (struct trunk_head_t*) NULL},
    { 32,  0, {150}, (struct trunk_head_t*) NULL},
    { 48,  0, { 25}, (struct trunk_head_t*) NULL},
    { 80,  0, {  0}, (struct trunk_head_t*) NULL},
    {128,  0, {  0}, (struct trunk_head_t*) NULL},
    {320,  0, {  0}, (struct trunk_head_t*) NULL},
    {512,  0, {  4}, (struct trunk_head_t*) NULL},
    {  0,  0, {  0}, (struct trunk_head_t*) NULL},
};
static unsigned int	g_trunk_list_num;


/* INTPROTO */
struct trunk_head_t*
get_trunk(size_t trunk_size)
{
    struct trunk_head_t*    tk_head;
    void*		    ptr;


    if (IS_NULL(ptr = malloc (trunk_size + THEAD_OFFSET)))    
    {
	rxvt_msg (DBG_FATAL, DBG_MEMORY, abort_msg);
	exit(EXIT_FAILURE);
    }

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MEMORY, "--Trunk allocated %d bytes\n", (int) trunk_size));
    tk_head = (struct trunk_head_t*) ((char*) ptr + trunk_size);
    /* set the real beginning of trunk. this should only be used by
     * init_trunk and free_trunk
     */
    tk_head->begin = (struct block_head_t*) ptr;
    return tk_head;
}


/* INTPROTO */
void
init_trunk(struct trunk_head_t* tk_head, uint16_t block_size)
{
    struct block_head_t*    block;
    uint16_t		    i;
    size_t		    bmax;


    assert (NOT_NULL(tk_head));
    assert (0 != block_size);


#ifdef DEBUG
    tk_head->magic_f = tk_head->magic_e = TRUNK_MAGIC;
    tk_head->tbyte = 0;
#endif
    bmax = ((char*) tk_head - (char*) tk_head->begin) /
	    ((size_t) block_size + BHEAD_OFFSET);
    assert (bmax <= 0x0000ffff); /* in case of overflow */
    tk_head->bmax = (uint16_t) bmax;
    block = tk_head->begin;
    tk_head->fblock = block;
    tk_head->fcount = tk_head->bmax;
    tk_head->bsize = block_size;

    /* initialize free block link list inside the trunk */
    for (i = 0; i < tk_head->bmax; i ++)    
    {
#ifdef DEBUG
	block->magic_f = BLOCK_MAGIC;
	block->magic_e = BLOCK_MAGIC;
# ifdef DEBUG_MEMORY
	MEMSET(block + 1, MEMORY_MAGIC, block_size);
# endif
#endif
	block->u.next = (struct block_head_t*) ((char*) block + block_size + BHEAD_OFFSET);
	block = block->u.next;
    }
}


/* INTPROTO */
void
free_trunk(struct trunk_head_t* tk_head)
{
    assert (NOT_NULL(tk_head));
#ifdef DEBUG
    assert (TRUNK_MAGIC == tk_head->magic_f);
    assert (TRUNK_MAGIC == tk_head->magic_e);
#endif

    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MEMORY, "--Trunk freed %d bytes\n", (int) ((char*) tk_head - (char*) tk_head->begin)));
    free ((void*) tk_head->begin);
}


/*
 * This function increase the trunk size for a trunk list if we decide
 * that there are many memory allocation for this trunk list. Whether
 * it really improve the performance is a question to answer. ;-)  It
 * just shows something that we can abuse with our own memory system.
 */
/* INTPROTO */
void
optimize_trunk_size(struct trunk_list_t* tklist)
{
    register size_t	tsize;

    assert (NOT_NULL(tklist));
    assert (NOT_NULL(tklist->first_trunk));

    /* already optimal trunk size, nothing to do */
    if (tklist->u.tsize >= OPTIMAL_TRUNK_SIZE)
	return;

    /* optimize trunk size by increasing it!)
     * . obtain a trunk size based on DEFAULT_TRUNK_SIZE
     * . decide whether we should increase trunk size
     * . do not use the size obtained based on DEFAULT_TRUNK_SIZE
     *   in order to avoid fragmentation.
     */
    tsize = DEFAULT_TRUNK_SIZE <<
	    (tklist->trunk_count / TRUNK_INCREASE_STAGE);
    if (tsize >= (tklist->u.tsize << 1))
	tklist->u.tsize <<= 1;
}


/*
 * This function decrease the trunk size for a trunk list if we decide
 * that there are less memory allocation for this trunk list.
 */
/* INTPROTO */
void
shrink_trunk_size(struct trunk_list_t* tklist)
{
    register size_t	tsize;

    assert (NOT_NULL(tklist));
    assert (NOT_NULL(tklist->first_trunk));

    /* already optimal trunk size, nothing to do */
    if (tklist->u.tsize <= DEFAULT_TRUNK_SIZE)
	return;

    /* shrink trunk size it!)
     * . obtain a trunk size based on DEFAULT_TRUNK_SIZE
     * . decide whether we should decrease trunk size
     */
    tsize = DEFAULT_TRUNK_SIZE <<
	    (tklist->trunk_count / TRUNK_INCREASE_STAGE);
    if (tsize <= (tklist->u.tsize >> 1))
	tklist->u.tsize >>= 1;
}


/* EXTPROTO */
void
rxvt_mem_init(void)
{
    register unsigned int   i;
    struct trunk_list_t*    tklist;


#ifdef DEBUG
    memory_initialized = 1;
#endif


    /* use system malloc */
    if (!use_our_malloc)
	return;

    /* allocate and initialize trunk memory */
    for (i = 0, tklist = g_trunk_list;
	0 != tklist->block_size;
	tklist ++, i++)   
    {
	size_t		tsize;

	if (tklist->block_size & 0x07)	
	{
	    /* block is not aligned to 8, we'll align it */
	    tklist->block_size &= ~(0x07);
	    tklist->block_size += 8;
	}
	/* really bad size, want to overflow us? ;-) */
	if (tklist->block_size > MINIMAL_TRUNK_SIZE)
	    tklist->block_size = MINIMAL_TRUNK_SIZE;


	if (0 == tklist->u.bnum)
	    tsize = DEFAULT_TRUNK_SIZE;
	else	
	{
	    /* really bad number, want to overflow us? ;-) */
	    if (tklist->u.bnum > 0x0000ffff)
		tklist->u.bnum = 0x0000ffff;

	    /* get *optimal*  real trunk size */
	    tsize = (tklist->block_size + BHEAD_OFFSET) * tklist->u.bnum;
	    if (tsize > MAXIMAL_TRUNK_SIZE)
	    	tsize = MAXIMAL_TRUNK_SIZE;
	    if (tsize < MINIMAL_TRUNK_SIZE)
	    	tsize = MINIMAL_TRUNK_SIZE;
	}

	/* OK, this supposes to be the optimal size */
	tklist->u.tsize = tsize;

	tklist->first_trunk = get_trunk (tsize),
	tklist->first_trunk->list = tklist,
	tklist->trunk_count ++; /* increase trunk counter */

	init_trunk (tklist->first_trunk, tklist->block_size);
	SET_NULL(tklist->first_trunk->next);
	SET_NULL(tklist->first_trunk->prev);
    }	/* for */

    g_trunk_list_num = i;

}


/* EXTPROTO */
void
rxvt_mem_exit (void)
{
    /* use system malloc */
    if (!use_our_malloc)
	return;

#ifdef DEBUG
    assert (memory_initialized);
#endif

    {
	struct trunk_list_t*	tklist;

	for (tklist = g_trunk_list; 0 != tklist->block_size; tklist ++)	
	{
	    struct trunk_head_t*    tk_head;

	    while (NOT_NULL(tk_head = tklist->first_trunk))  
	    {
#ifdef DEBUG
		assert (TRUNK_MAGIC == tk_head->magic_f);
		assert (TRUNK_MAGIC == tk_head->magic_e);
#endif
		tklist->first_trunk = tk_head->next; /* new first trunk */

		free_trunk (tk_head),
		tklist->trunk_count --; /* decrease trunk counter */
	    }	/* while */
	}   /* for */
    }
}


/* EXTPROTO */
void*
rxvt_malloc(size_t size)
{
    struct block_head_t*	block;


#ifdef DEBUG
    assert (memory_initialized);
    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MEMORY, "rxvt_malloc (%d)\n", (int) size)));
#endif

    if (!use_our_malloc || /* use system malloc, or request size is big */
	(size > g_trunk_list[g_trunk_list_num - 1].block_size))
    {
	if (0 == size)
	    size = 1;

	if (IS_NULL(block = malloc (size + BHEAD_OFFSET)))   {
	    rxvt_msg (DBG_FATAL, DBG_MEMORY, abort_msg);
	    exit(EXIT_FAILURE);
	}

#ifdef DEBUG
	block->magic_f = BLOCK_MAGIC;
	block->magic_e = BLOCK_MAGIC;
#endif
	/* set special trunk address */
	block->u.trunk = (struct trunk_head_t*) SYS_MALLOC_PTR;
    }
    else
    {
	struct trunk_list_t*	tklist;
	register int		idx;

	assert (size <= (size_t) g_trunk_list[g_trunk_list_num - 1].block_size);

	/* look for appropriate block_info entry */
	for (idx = 0;
	     (idx < g_trunk_list_num) && (size > g_trunk_list[idx].block_size);
	     idx ++)
	    ;
	tklist = &g_trunk_list[idx];
	assert (tklist->first_trunk);
#ifdef DEBUG
	assert (TRUNK_MAGIC == tklist->first_trunk->magic_f);
	assert (TRUNK_MAGIC == tklist->first_trunk->magic_e);
#endif

	/* find the free block */
	block = tklist->first_trunk->fblock;
#ifdef DEBUG
	assert (BLOCK_MAGIC == block->magic_f);
	assert (BLOCK_MAGIC == block->magic_e);
	block->bbyte = size;
	tklist->first_trunk->tbyte += size;
#endif
	/* adjust information in trunk_head */
	tklist->first_trunk->fblock = block->u.next;
	tklist->first_trunk->fcount --;

	/* Set the address of trunk head so that we can find it in free.
	 * Notice that u.trunk is the placehold for u.next when block is
	 * free!!!
	 */
	block->u.trunk = tklist->first_trunk;

	/* if no free block left in the trunk */
	if (0 == tklist->first_trunk->fcount)	
	{
#ifdef DEBUG
	    /* print out statistics for the trunk */
	    rxvt_dbgmsg ((DBG_VERBOSE, DBG_MEMORY, "--Trunk of block size %d: %d bytes used (%d%%)\n",
		tklist->block_size, (int) tklist->first_trunk->tbyte,
		(int) (tklist->first_trunk->tbyte * 100 /
		      (tklist->block_size * tklist->first_trunk->bmax))));
#endif


	    if (IS_NULL(tklist->first_trunk->next))	
	    {
		/* no free trunk in this trunk list, allocate a new trunk */
		struct trunk_head_t*	new_trunk;
		
		new_trunk = get_trunk (tklist->u.tsize),
		new_trunk->list = tklist,
		tklist->trunk_count ++, /* increase trunk counter */
		optimize_trunk_size (tklist);

		init_trunk (new_trunk, tklist->block_size);
		SET_NULL(new_trunk->next);
		SET_NULL(new_trunk->prev);

		/* clean up the old trunk */
		SET_NULL(tklist->first_trunk->next);
		SET_NULL(tklist->first_trunk->prev);

		/* insert new trunk into trunk list */
		tklist->first_trunk = new_trunk;
	    }
	    else    
	    {
		/* there is some free trunk in this trunk list, remove the
		 * first trunk from the trunk list
		 */
		struct trunk_head_t*	remove = tklist->first_trunk;
		tklist->first_trunk = remove->next;
#ifdef DEBUG
		assert (TRUNK_MAGIC == tklist->first_trunk->magic_f);
		assert (TRUNK_MAGIC == tklist->first_trunk->magic_e);
#endif
		SET_NULL(tklist->first_trunk->prev);

		/* clean up the old trunk */
		SET_NULL(remove->next);
		SET_NULL(remove->prev);
	    }
	}

    }

    /* now return the free block, BUT skip the block_head_t first */
    block ++;
    return block;
}


/* EXTPROTO */
void*
rxvt_calloc(size_t number, size_t size)
{
    void*	ptr;
    size_t	total = number * size;

#ifdef DEBUG
    assert (memory_initialized);
#endif

    /* possible overflow? */
    assert (total >= 0 && size > 0);
    assert ((total / size) == number);

    ptr = rxvt_malloc (total);
    /* calloc requires us to zero out the memory, slow call */
    MEMSET(ptr, 0, total);

    return (ptr);
}


/* EXTPROTO */
void*
rxvt_realloc(void* ptr, size_t size)
{
    struct block_head_t*	block;
    struct trunk_head_t*	tk_head;


#ifdef DEBUG
    assert (memory_initialized);
#endif

    if (IS_NULL(ptr))
	return (rxvt_malloc (size));

    /* find the real block head structure */
    block = (struct block_head_t*) ptr;
    block --;
#ifdef DEBUG
    assert (BLOCK_MAGIC == block->magic_f);
    assert (BLOCK_MAGIC == block->magic_e);
#endif
    /* find trunk_head_t structure for ptr */
    tk_head = block->u.trunk;


    if ((struct trunk_head_t*) SYS_MALLOC_PTR == tk_head)
    {
	/* ptr was allocated useing system malloc here */
	block = realloc (block, size + BHEAD_OFFSET);

	if (IS_NULL(block))   
	{
	    rxvt_msg (DBG_FATAL, DBG_MEMORY, abort_msg);
	    exit(EXIT_FAILURE);
	}
#ifdef DEBUG
	block->magic_f = BLOCK_MAGIC;
	block->magic_e = BLOCK_MAGIC; 
#endif
	/* set special trunk address */
	block->u.trunk = (struct trunk_head_t*) SYS_MALLOC_PTR;
	/* skip block structure */
	block ++;
    }
    else
    {
	/* now do some serious business about reallocating this ptr */
#ifdef DEBUG
	assert (TRUNK_MAGIC == tk_head->magic_f);
	assert (TRUNK_MAGIC == tk_head->magic_e);
#endif
	/* nothing to do if this block is actually big enough */
	if (size <= (size_t) tk_head->bsize)
		return ptr;

	block = rxvt_malloc (size);
	MEMCPY (block, ptr, tk_head->bsize); /* a bit waste, though */
	rxvt_free (ptr);
    }

    return (block);
}


/* EXTPROTO */
void
rxvt_free(void* ptr)
{
    struct block_head_t*	block;
    struct trunk_head_t*	tk_head;


#ifdef DEBUG
    assert (memory_initialized);
#endif
#if 0
    assert (NOT_NULL(ptr)); /* generate core dump */
#else
    /*
     * 2006-11-19 gi1242: glibc accepts free(NULL), and has been used in a few
     * places throughout the mrxvt code base. It's better to fail gracefully
     * here, as opposed to code bloat by first testing if a pointer is not null
     * before calling rxvt_free().
     */
    if( IS_NULL(ptr) )
	return;
#endif


    block = (struct block_head_t*) ptr;
    /* find the block_head_t structure */
    block --;

#ifdef DEBUG
    assert (BLOCK_MAGIC == block->magic_f);
    assert (BLOCK_MAGIC == block->magic_e);
#endif
    /* find trunk_head_t structure for ptr */
    tk_head = block->u.trunk;


    /* use system malloc */
    if ((struct trunk_head_t*) SYS_MALLOC_PTR == tk_head)
    {
	free (block);
    }
    else
    {
#ifdef DEBUG
	assert (TRUNK_MAGIC == tk_head->magic_f);
	assert (TRUNK_MAGIC == tk_head->magic_e);
	tk_head->tbyte -= block->bbyte;
# ifdef DEBUG_MEMORY
	MEMSET (ptr, MEMORY_MAGIC, tk_head->list->block_size);
# endif
#endif

	/* Insert the block of ptr to the head of free block link list.
	 * Notice that u.next is the placeholder for u.trunk when block
	 * is in use!!!
	 */
	block->u.next = tk_head->fblock;
	tk_head->fblock = block;
	/* increase free block counter */
	tk_head->fcount ++;

	if (1 == tk_head->fcount)	
	{
	    /* link the trunk back to the trunk list if all blocks were 
	     * allocated previously
	     */
	    struct trunk_list_t*    tklist;
	    tklist = tk_head->list;

	    assert (IS_NULL(tk_head->prev));
	    assert (IS_NULL(tk_head->next));

	    assert (NOT_NULL(tklist->first_trunk));
#ifdef DEBUG
	    assert (TRUNK_MAGIC == tklist->first_trunk->magic_f);
	    assert (TRUNK_MAGIC == tklist->first_trunk->magic_e);
#endif
	    tk_head->next = tklist->first_trunk;
	    tklist->first_trunk->prev = tk_head;

	    /* insert this trunk into the beginnig of trunk list */
	    tklist->first_trunk = tk_head;
	}
	else
	if (tk_head->fcount == tk_head->bmax)	
	{
	    /* we can free this trunk now since all blocks in it are free.
	     * but we do not free it if it is the only trunk in its trunk
	     * list.
	     */
	    struct trunk_list_t*    tklist;
	    tklist = tk_head->list;

	    if (tklist->first_trunk == tk_head)	
	    {
		/* this trunk is the first in trunk list */
		if (NOT_NULL(tk_head->next))    {
#ifdef DEBUG
		    assert (TRUNK_MAGIC == tk_head->next->magic_f);
		    assert (TRUNK_MAGIC == tk_head->next->magic_e);
#endif
		    tklist->first_trunk = tk_head->next;
		    tklist->first_trunk->prev = NULL;

		    free_trunk (tk_head),
		    tklist->trunk_count --, /* decrease trunk counter */
		    shrink_trunk_size (tklist);
		}
	    }
	    else    
	    {
		/* this trunk is not the first in trunk list */
#ifdef DEBUG
		assert (TRUNK_MAGIC == tk_head->prev->magic_f);
		assert (TRUNK_MAGIC == tk_head->prev->magic_e);
#endif
		tk_head->prev->next = tk_head->next;
		if (NOT_NULL(tk_head->next))	
		{
#ifdef DEBUG
		    assert (TRUNK_MAGIC == tk_head->next->magic_f);
		    assert (TRUNK_MAGIC == tk_head->next->magic_e);
#endif
		    tk_head->next->prev = tk_head->prev;
		}

		free_trunk (tk_head),
		tklist->trunk_count --, /* decrease trunk counter */
		shrink_trunk_size (tklist);
	    }
	}
    }
}


#else	/* !OUR_MALLOC */


/* EXTPROTO */
void*
rxvt_malloc(size_t size)
{
    void*   p;

    /* see AC_FUNC_MALLOC macro in autoconf documentation */
    if (0 == size)
	size = 1;

    p = malloc(size);
    if (p)
	return p;

    rxvt_msg (DBG_FATAL, DBG_MEMORY, abort_msg);
    exit(EXIT_FAILURE);
    /* NOTREACHED */
}


/* EXTPROTO */
void*
rxvt_calloc(size_t number, size_t size)
{
    void*   p;

    p = calloc(number, size);
    if (p)
	return p;

    rxvt_msg (DBG_FATAL, DBG_MEMORY, abort_msg);
    exit(EXIT_FAILURE);
    /* NOTREACHED */
}


/* EXTPROTO */
void*
rxvt_realloc(void *ptr, size_t size)
{
    void*   p;

    if (ptr)
	p = realloc(ptr, size);
    else
	p = malloc(size);
    if (p)
	return p;

    rxvt_msg (DBG_FATAL, DBG_MEMORY, abort_msg);
    exit(EXIT_FAILURE);
    /* NOT REACHED */
}


/* EXTPROTO */
void
rxvt_free(void* ptr)
{
#if 0
    assert (NOT_NULL(ptr)); /* generate core dump */
#endif
    free (ptr);
}

#endif	/* OUR_MALLOC */

/*----------------------- end-of-file (C source) -----------------------*/
