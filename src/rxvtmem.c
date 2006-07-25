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
 *                      MEMORY ALLOCATION WRAPPERS						*
 * -------------------------------------------------------------------- */

#ifdef OUR_MALLOC

/* Use use our malloc function only if we bypass the memory check */
static int   use_our_malloc = 0;

/* Determine the optimal trunk size we want to use */
static size_t g_trunk_size;

/* EXTPROTO */
void
rxvt_init_trunk_size(size_t col, size_t row, size_t scroll)
{
    /* we should change this later based on col/row/scroll */
    g_trunk_size = (64UL << 10);
}



static struct trunk_list_t	g_trunk_list[] =
{
    {16,   (struct trunk_head_t*) NULL},
    {32,   (struct trunk_head_t*) NULL},
    {64,   (struct trunk_head_t*) NULL},
    {128,  (struct trunk_head_t*) NULL},
    {256,  (struct trunk_head_t*) NULL},
    {512,  (struct trunk_head_t*) NULL},
    {1024, (struct trunk_head_t*) NULL},
    {0,    (struct trunk_head_t*) NULL},
};
static unsigned int	g_trunk_list_num;


/* INTPROTO */
struct trunk_head_t*
get_trunk()
{
    struct trunk_head_t*    tk_head;
    void*		    ptr;


    if (IS_NULL(ptr = malloc (g_trunk_size + sizeof (struct trunk_head_t))))    
	{
		fprintf(stderr, APL_NAME ": memory allocation failure.  Aborting");
		exit(EXIT_FAILURE);
    }

    tk_head = (struct trunk_head_t*) ((RUINT32T) ptr + g_trunk_size);
    return tk_head;
}


/* INTPROTO */
void
init_trunk(struct trunk_head_t* tk_head, RUINT16T block_size)
{
    struct block_head_t*    block;
    RUINT16T		    i;
    RUINT32T		    bmax;


    assert (NOT_NULL(tk_head));
    assert (0 != block_size);


#ifdef _DEBUG
    tk_head->magic = TRUNK_MAGIC;
#endif
    bmax = g_trunk_size / ((size_t) block_size + BHEAD_OFFSET);
    assert (bmax <= 0x0000ffff); /* in case of overflow */
    tk_head->bmax = (RUINT16T) bmax;
    block = (struct block_head_t*) ((RUINT32T) tk_head - g_trunk_size);
    tk_head->fblock = block;
    tk_head->fcount = tk_head->bmax;
    tk_head->bsize = block_size;

    /* initialize free block link list inside the trunk */
    for (i = 0; i < tk_head->bmax - 1; i ++)    
	{
#ifdef _DEBUG
		block->magic_f = BLOCK_MAGIC;
		block->magic_e = BLOCK_MAGIC;
#endif
		block->u.next = (struct block_head_t*) ((RUINT32T) block + block_size + BHEAD_OFFSET);
		block = block->u.next;
    }
    block->u.next = NULL; /* not necessary if we use tk_head->fcount */
}


/* INTPROTO */
void
free_trunk(struct trunk_head_t* tk_head)
{
    void*   ptr;


    assert (NOT_NULL(tk_head));
#ifdef _DEBUG
    assert (TRUNK_MAGIC == tk_head->magic);
#endif

    ptr = (void*) ((RUINT32T) tk_head - g_trunk_size);
    free (ptr);
}


/* EXTPROTO */
void
rxvt_mem_init(void)
{
    register unsigned int   i;
    struct trunk_list_t*    tklist;


    /* check the behavior of memory management systems */
    rxvt_init_trunk_size (0, 0, 0);

    /* use system malloc */
    if (!use_our_malloc)
		return;

    assert (g_trunk_size);

    /* allocate and initialize trunk memory */
    for (i = 0, tklist = g_trunk_list;
		0 != tklist->block_size;
		tklist ++, i++)   
    {
		tklist->first_trunk = get_trunk ();
		init_trunk (tklist->first_trunk, tklist->block_size);
		SET_NULL(tklist->first_trunk->next);
		SET_NULL(tklist->first_trunk->prev);
    }
    g_trunk_list_num = i;
}


/* EXTPROTO */
void
rxvt_mem_exit (void)
{
    /* use system malloc */
    if (!use_our_malloc)
	return;

    {
		struct trunk_list_t*	tklist;

		assert (g_trunk_size);

		for (tklist = g_trunk_list; 0 != tklist->block_size; tklist ++)	
		{
			struct trunk_head_t*    tk_head;

		    while (NOT_NULL(tk_head = tklist->first_trunk))  
		    {
#ifdef _DEBUG
				assert (TRUNK_MAGIC == tk_head->magic);
#endif
				tklist->first_trunk = tk_head->next; /* new first trunk */
				free_trunk (tk_head);
			}
		}
    }
}


/* EXTPROTO */
void*
rxvt_malloc(size_t size)
{
	struct block_head_t*	block;

    /* use system malloc */
    if (!use_our_malloc)	{
		if (0 == size)
			size = 1;

		if (IS_NULL(block = malloc (size + BHEAD_OFFSET)))   {
		    fprintf(stderr, APL_NAME ": memory allocation failure.  Aborting");
		    exit(EXIT_FAILURE);
		}

#ifdef _DEBUG
		block->magic_f = BLOCK_MAGIC;
		block->magic_e = BLOCK_MAGIC;
#endif
		/* set special trunk address */
		block->u.trunk = (struct trunk_head_t*) SYS_MALLOC_PTR;
    }
	else
    {
		struct trunk_list_t*	tklist;


		assert (size <= (size_t) g_trunk_list[g_trunk_list_num - 1].block_size);

		/* look for appropriate block_info entry */
		for (tklist = &g_trunk_list[g_trunk_list_num - 2];
			(tklist > g_trunk_list) && (size > tklist->block_size);
			tklist --)
			;

		assert (tklist->first_trunk);
#ifdef _DEBUG
		assert (TRUNK_MAGIC == tklist->first_trunk->magic);
#endif

		/* find the free block */
		block = tklist->first_trunk->fblock;
		/* set the address of trunk head so that we can find it in free */
		block->u.trunk = tklist->first_trunk;
#ifdef _DEBUG
		assert (BLOCK_MAGIC == block->magic_f);
		assert (BLOCK_MAGIC == block->magic_e);
#endif
		/* adjust information in trunk_head */
		tklist->first_trunk->fblock = block->u.next;
		tklist->first_trunk->fcount --;

		/* if no free block left in the trunk */
		if (0 == tklist->first_trunk->fcount)	
		{
			if (IS_NULL(tklist->first_trunk->next))	
			{
				/* no free trunk in this trunk list, allocate a new trunk */
				struct trunk_head_t*	new_trunk = get_trunk ();
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
#ifdef _DEBUG
				assert (TRUNK_MAGIC == tklist->first_trunk->magic);
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
	/* possible overflow? */
	assert (number * size >= number);
	assert (number * size >= size);
    return rxvt_malloc (number * size);
}


/* EXTPROTO */
void*
rxvt_realloc(void* ptr, size_t size)
{
	struct block_head_t*	block;
	struct trunk_head_t*	tk_head;


	if (IS_NULL(ptr))
		return (rxvt_malloc (size));

	/* find the real block head structure */
	block = (struct block_head_t*) ptr;
	block --;
#ifdef _DEBUG
	assert (BLOCK_MAGIC == block->magic_f);
	assert (BLOCK_MAGIC == block->magic_e);
#endif
	/* find trunk_head_t structure for ptr */
	tk_head = block->u.trunk;


	if ((struct trunk_head_t*) SYS_MALLOC_PTR == tk_head)
	{
		/* ptr was allocated useing system malloc here */
		block = realloc (block, size + BHEAD_OFFSET);

		if (IS_NULL(block))   {
			fprintf(stderr, APL_NAME ": memory allocation failure.  Aborting");
			exit(EXIT_FAILURE);
		}
#ifdef _DEBUG
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
#ifdef _DEBUG
		assert (TRUNK_MAGIC == tk_head->magic);
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


    if (IS_NULL(ptr))	{
        fprintf(stderr, APL_NAME ": free NULL pointer.  Aborting");
        exit(EXIT_FAILURE);
    }

	block = (struct block_head_t*) ptr;
	/* find the block_head_t structure */
	block --;

#ifdef _DEBUG
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
#ifdef _DEBUG
		assert (TRUNK_MAGIC == tk_head->magic);
#endif

		/* insert the block of ptr to the head of free block link list */
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
			tklist = &g_trunk_list[tk_head->bsize];

			assert (IS_NULL(tk_head->prev));
			assert (IS_NULL(tk_head->next));

			assert (NOT_NULL(tklist->first_trunk));
#ifdef _DEBUG
			assert (TRUNK_MAGIC == tklist->first_trunk->magic);
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
			tklist = &g_trunk_list[tk_head->bsize];

			if (tklist->first_trunk == tk_head)	
			{
				/* this trunk is the first in trunk list */
				if (NOT_NULL(tk_head->next))    {
#ifdef _DEBUG
					assert (TRUNK_MAGIC == tk_head->next->magic);
#endif
					tklist->first_trunk = tk_head->next;
					tklist->first_trunk->prev = NULL;
					free_trunk (tk_head);
				}
			}
			else    
			{
				/* this trunk is not the first in trunk list */
#ifdef _DEBUG
				assert (TRUNK_MAGIC == tk_head->prev->magic);
#endif
				tk_head->prev->next = tk_head->next;
				if (NOT_NULL(tk_head->next))	{
#ifdef _DEBUG
					assert (TRUNK_MAGIC == tk_head->next->magic);
#endif
					tk_head->next->prev = tk_head->prev;
				}
				free_trunk (tk_head);
			}
		}
	}
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
    free (ptr);
}


#endif	/* OUR_MALLOC */

/*----------------------- end-of-file (C source) -----------------------*/
