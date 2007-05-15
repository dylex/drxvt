/*--------------------------------*-H-*---------------------------------*
 * File:    debug.h
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2007        Jingmin Zhou <jimmyzhou@users.sourceforge.net>
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

#ifndef __DEBUG_H__
#define __DEBUG_H__

/* Debug level definition.
 *   fatal  : fatal error, mrxvt must abort immediately
 *   error  : serious error, mrxvt should gracefully exit
 *   warn   : some problems cause certain function of mrxvt not working
 *   info   : informative messages about mrxvt execution
 *   verbose: verbose information about mrxvt that is useful for developers
 *   debug  : more information about mrxvt that is useful for developers
 */
typedef enum {
    DBG_FATAL = 0,
    DBG_ERROR,
    DBG_WARN,
    DBG_INFO,
    DBG_VERBOSE,    /* used by rxvt_dbgmsg macro */
    DBG_DEBUG,	    /* used by rxvt_dbgmsg macro */
    DBG_LEVEL_MAX,
} dbglevel_t;

typedef enum {
    DBG_COMMAND	    = 0x00000001,
    DBG_SCREEN	    = 0x00000002,
    DBG_PTYTTY	    = 0x00000004,
    DBG_INIT	    = 0x00000008,
    DBG_MAIN	    = 0x00000010,
    DBG_LOGGING	    = 0x00000020,
    DBG_MACROS	    = 0x00000040,
    DBG_MENUBAR	    = 0x00000080,
    DBG_TABBAR	    = 0x00000100,
    DBG_SCROLLBAR   = 0x00000200,
    DBG_IMAGES	    = 0x00000400,
    DBG_PIXMAP	    = 0x00000800,
    DBG_TRANSPARENT = 0x00001000,
    DBG_ENCODING    = 0x00002000,
    DBG_GKRELOT	    = 0x00004000,
    DBG_MEMORY	    = 0x00008000,
    DBG_SESSION	    = 0x00010000,
    DBG_STRING	    = 0x00020000,
    DBG_RESOURCE    = 0x00040000,
    DBG_XFTACS	    = 0x00080000,
    DBG_MISC	    = 0x00100000,
} dbgmask_t;

/* !!! Sync the list with the above definition of dbgmask_t !!! */
typedef enum {
    DBG_IDX_COMMAND = 0,
    DBG_IDX_SCREEN,
    DBG_IDX_PTYTTY,
    DBG_IDX_INIT,
    DBG_IDX_MAIN,
    DBG_IDX_LOGGING,
    DBG_IDX_MACROS,
    DBG_IDX_MENUBAR,
    DBG_IDX_TABBAR,
    DBG_IDX_SCROLLBAR,
    DBG_IDX_IMAGES,
    DBG_IDX_PIXMAP,
    DBG_IDX_TRANSPARENT,
    DBG_IDX_ENCODING,
    DBG_IDX_GKRELOT,
    DBG_IDX_MEMORY,
    DBG_IDX_SESSION,
    DBG_IDX_STRING,
    DBG_IDX_RESOURCE,
    DBG_IDX_XFTACS,
    DBG_IDX_MISC,
    DBG_IDX_MAX,
} dbgmask_index_t;


/*
 * rxvt_dbgmsg macro is used for DBG_DEBUG and DBG_VERBOSE level only.
 * for other level of information, you should call rxvt_msg directly.
 *
 * Note: the call of rxvt_dbgmsg is different to the call of rxvt_msg!
 *     rxvt_dbgmsg ((argument list));
 *     rxvt_msg (argument list);
 */
#ifdef DEBUG
# define rxvt_dbgmsg(x)	rxvt_msg x
#else
# define rxvt_dbgmsg(x)
#endif

/*
 * For temporary debugging (so that only certain messages can be enabled).
 */
#define rxvt_dbgtmsg(x)	rxvt_msg x

#endif	/* __DEBUG_H__ */
/*----------------------- end-of-file (H source) -----------------------*/
