/*--------------------------------*-C-*----------------------------------------*
 * File:    macros.c (used to be hotkeys.c)
 *-----------------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 *
 *  Copyright (c) 2005-2006   Gautam Iyer <gi1242@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 *----------------------------------------------------------------------------*/

#include "../config.h"
#include "rxvt.h"


/*
 * Must sync these to macroFnNames in rxvt.h.
 */
static const char *const macroNames[] =
{
    "Dummy",		    /* Delete macro */
    "Esc",		    /* Escape sequence to send to mrxvt */
    "Str",		    /* String to send to child process */
    "Exec",		    /* Exec a program asynchronusly */
    "Close",		    /* Close tab(s) */
    "Scroll",		    /* Scroll up/down */
    "Copy",		    /* Copy selection */
    "Paste",		    /* Paste selection */
    "PasteFile",	    /* Paste the content of a file */
    "ResizeFont",	    /* Resize terminal font */
    "ToggleVeryBold",	    /* Toggle use of bold font for colored text */
    "ToggleBoldColors",	    /* Toggle option boldColors */
    "ToggleVeryBright",	    /* Toggle option veryBright */
    "ToggleHold",	    /* Toggle holding of completed tabs */
    "ToggleFullscreen",	    /* Toggle full screen mode */
    "Raise",		    /* Raise the terminal window */
    "SetTitle",		    /* Set title of active tab to selection */
    "SaveConfig",	    /* Save config to file */
    "ToggleMacros"	    /* Toggle using keyboard macros */
};


/******************************************************************************\
* 		       BEGIN INTERNAL ROUTINE PROTOTYPES		       *
\******************************************************************************/
static int		rxvt_add_macro	    ( rxvt_t*, KeySym, unsigned char, char*, Bool, macro_priority_t);
static Bool		 rxvt_set_action		  (action_t*, char*);
static int		 rxvt_dispatch_action		  (rxvt_t*, action_t*, XEvent *ev);
/******************************************************************************\
*			END INTERNAL ROUTINE PROTOTYPES			       *
\******************************************************************************/


/*
 * 2006-02-23 gi1242: New macro code. This should extend the hotkey aproach
 * without causing code bloat. The idea is that defining "macros" can also
 * enable the user to communicate with mrxvt using escape sequences!
 */

/*
 * Functions to parse macros (add them to our list), and exec actions.
 */
/* {{{1 macro_cmp()
 *
 * Used by bsearch and qsort for macro comparison.
 */
static int
macro_cmp( const void *p1, const void *p2)
{
    const macros_t  *macro1 = p1,
	     	    *macro2 = p2;

    /* First compare keysyms, then modflags. Ignore the "Primary" modifier */
    if( macro1->keysym == macro2->keysym )
    {
	if(
	     (macro1->modFlags & ~MACRO_PRIMARY & MACRO_MODMASK)
		== (macro2->modFlags & ~MACRO_PRIMARY & MACRO_MODMASK)
	  )
	    return MACRO_GET_NUMBER( macro1->modFlags )
			- MACRO_GET_NUMBER( macro2->modFlags );
	else
	    return (macro1->modFlags & ~MACRO_PRIMARY & MACRO_MODMASK)
			- (macro2->modFlags & ~MACRO_PRIMARY & MACRO_MODMASK);
    }
    else
	return macro1->keysym - macro2->keysym;
}

/* {{{1 macro_set_number( flag, num ) */
/* INTPROTO */
static unsigned char
macro_set_number( unsigned char flag, unsigned char num )
{
    flag &= MACRO_MODMASK;
    flag |= (num << MACRO_N_MOD_BITS);

    return flag;
}

/* {{{1 rxvt_parse_macros(str, arg): Parse macro from arguments
 *
 * str and arg can be as follows:
 *
 * 	1. str = keyname,		    arg = argument.
 * 	2. str = macro.keyname: argument,   arg = NULL
 *
 * If a valid macro is found, it is added to our list (r->macros) of macros.
 * Returns 1 if the macro is added to r->macros, 0 if it is not a macro (i.e.
 * does not begin with "macro."), and -1 if invalid syntax / error adding / etc.
 */
/* EXTPROTO */
#define NEWARGLIM   500	/* `reasonable' size */
int
rxvt_parse_macros( rxvt_t *r, const char *str, const char *arg,
	macro_priority_t priority)
{
    char	    newarg[NEWARGLIM],
		    keyname[ NEWARGLIM],
		    *keyname_nomods;
    unsigned char   modFlags = 0;
    KeySym	    keysym;

    Bool	    addmacro = False;

    if (IS_NULL(arg))
    {
	char *keyend;
	int	n;

	/*
	 * Need to split str into keyname and argument.
	 */
	if( (n = rxvt_str_match(str, "macro.")) == 0 )
	    return 0;
	str += n;	/* skip `macro.' */

	if (IS_NULL(keyend = STRCHR( str, ':' )))
	    return -1;

	n = min( keyend - str, NEWARGLIM - 1 );

	STRNCPY( keyname, str, n );
	keyname[n] = 0;

	STRNCPY( newarg, keyend + 1, NEWARGLIM - 1 );
    }
    else
    {
	/*
	 * Keyname is already in str. Copy arg into newarg.
	 */
	STRNCPY( keyname, str, NEWARGLIM - 1 );
	keyname[ NEWARGLIM - 1] = '\0';

	STRNCPY( newarg, arg, NEWARGLIM - 1 );
    }

    /* Null terminate and strip leading / trailing spaces */
    newarg[NEWARGLIM - 1] = '\0';
    rxvt_str_trim( newarg );

    rxvt_msg( DBG_INFO, DBG_MACROS,
	    "Got macro '%s' -- '%s'\n", keyname, newarg );

    /*
     * Breakup keyname into a keysym and modifier flags.
     */
    if (IS_NULL(keyname_nomods = STRRCHR( keyname, '+' )))
    {
	/* No modifiers specified */
#ifdef UNSHIFTED_MACROS
	keyname_nomods = keyname;
#else
	return -1;
#endif
    }
    else
    {
	*(keyname_nomods++) = 0;

	/*
	 * keyname is now a null terminated string containing only the
	 * modifiers, and keyname_nomods is a null terminated string
	 * containing only the key name.
	 */
	if( STRCASESTR( keyname, "ctrl" ) )
	    modFlags |= MACRO_CTRL;
	if( STRCASESTR( keyname, "meta" ) )
	    modFlags |= MACRO_META;
	if( STRCASESTR( keyname, "alt" ) )
	    modFlags |= MACRO_ALT;
	if( STRCASESTR( keyname, "shift") )
	    modFlags |= MACRO_SHIFT;
	if( STRCASESTR( keyname, "primary"))
	    modFlags |= MACRO_PRIMARY;
	if( STRCASESTR( keyname, "add" ) )
	    addmacro = True;
    }

    /*
     * Always store the keysym as lower case. That way we can treat shift
     * correctly even when Caps Lock is pressed.
     */
    {
        KeySym upper;
        XConvertCase(XStringToKeysym(keyname_nomods), &keysym, &upper);
    }

    if (NOT_KEYSYM(keysym))
    {
	rxvt_msg (DBG_ERROR, DBG_MACROS,  "Invalid keysym %s. Skipping macro.",
		keyname_nomods);
	return -1;
    }

    return rxvt_add_macro( r, keysym, modFlags, newarg, addmacro, priority)
		? 1 : -1;
}

/* {{{1 rxvt_add_macro( keysym, modFlags, astring, priority)
 *
 * Add a macro to our list of macros (r->macros) if astring describes a valid
 * macro.
 *
 * priority is the priority with which the macro should be added. A macro should
 * never overwrite a macro with lower priority (yes, I know that's backwards). A
 * macro should be added to a chain ONLY IF it's priority is the same as the
 * previous macro's priority.
 */
/* INTPROTO */
static int
rxvt_add_macro( rxvt_t *r, KeySym keysym, unsigned char modFlags, char *astring,
	Bool addmacro, macro_priority_t priority)
{
    const unsigned nmacros_increment = 64;  /* # extra macros to alloc space for
					       when we need to enlarge our list
					       of macros. A large number here is
					       not wasteful as we clean it up
					       after initialization */
    unsigned short  i,
		    replaceIndex = r->nmacros,
		    dummyIndex = r->nmacros;

    unsigned char   macroNum = 0;
    action_t	    action;


    rxvt_dbgmsg(( DBG_DEBUG, DBG_MACROS, "%s(%08lx, %2hhx, '%s', %d, %d)\n",
		__func__, keysym, modFlags, astring, addmacro, priority ));

    /*
     * Check to see if macro already exists.
     */
    for( i=0; i < r->nmacros; i++ )
    {
	if(
	    r->macros[i].keysym == keysym
	    && (r->macros[i].modFlags & MACRO_MODMASK & ~MACRO_PRIMARY)
		    == (modFlags & MACRO_MODMASK & ~MACRO_PRIMARY)
	  )
	{
	    if( addmacro && r->macros[i].priority == priority )
	    {
		/*
		 * Find the last macro in the macro chain (the macro with the
		 * largest number).
		 */
		unsigned char num;

		num	 = MACRO_GET_NUMBER( r->macros[i].modFlags );
		if( num > macroNum )
		    macroNum = num;

		if( macroNum == MACRO_MAX_CHAINLEN )
		{
		    rxvt_msg (DBG_ERROR, DBG_MACROS,  "Macro chain too long" );
		    return 0;
		}

		replaceIndex = i;
	    }

	    /*
	     * Macro for key already exists. Only replace it if we have a
	     * *lower* priority (which in theory should never happen).
	     */
	    else if( priority > r->macros[i].priority )
		return 1; /* Claim to have succeded so that caller will not
			     complain about "Failing to add a ... macro". */
	    
	    /*
	     * 2006-03-06 gi1242: Don't delete "Dummy" macros here. If we do
	     * that then the user will have no way to delete macros defined in
	     * the system /etc/mrxvt/mrxvtrc file. "Dummy" macros will be
	     * deleted after init.
	     */
	    else
	    {
		if( replaceIndex < r->nmacros )
		{
		    /*
		     * replaceIndex points to a macro with keysym == the keysym
		     * of the macro to be added. Set keysym to 0 so that it will
		     * be cleaned up by rxvt_cleanup_macros().
		     */
		    r->macros[replaceIndex].keysym = 0;
		}
		replaceIndex = i;
	    }
	}

	else if( r->macros[i].keysym == 0 )
	    /*
	     * Macros with keysym 0 are dummies, and can be safely replaced.
	     */
	    dummyIndex = i;
    } /* for */

    /*
     * Now dummyIndex will point to a macro that can safely be replaced.
     * replaceIndex (if less than r->nmacros) will be the index of a macro in
     * r->macros which has the same keysym / modflags as the macro we wish to
     * add. We will either replace this macro with the new one, or add to it if
     * the "Add" flag was specified.
     */
    if( addmacro )
    {
	if( replaceIndex == r->nmacros )
	{
	    rxvt_msg( DBG_ERROR, DBG_MACROS,
		    "No previous macro to add to (key %s%s%s%s%s)",
		    (modFlags & MACRO_CTRL) ? "Ctrl+" : "",
		    (modFlags & MACRO_META) ? "Meta+" : "",
		    (modFlags & MACRO_ALT) ? "Alt+" : "",
		    (modFlags & MACRO_SHIFT) ? "Shift+" : "",
		    XKeysymToString( keysym ) );
	    return 0;	/* Failure */
	}

	else if( r->macros[replaceIndex].priority != priority )
	{
	    rxvt_msg( DBG_ERROR, DBG_MACROS,
		    "Can not add to a macro defined at a different location "
		    "(key %s%s%s%s%s)",
		    (modFlags & MACRO_CTRL) ? "Ctrl+" : "",
		    (modFlags & MACRO_META) ? "Meta+" : "",
		    (modFlags & MACRO_ALT) ? "Alt+" : "",
		    (modFlags & MACRO_SHIFT) ? "Shift+" : "",
		    XKeysymToString( keysym ) );
	    return 0;	/* Failure */
	}

	else if( r->macros[replaceIndex].action.type == MacroFnDummy )
	{
	    /* Do not add to a dummy macro */
	    rxvt_msg (DBG_ERROR, DBG_MACROS,
		    "Can not add actions to a Dummy macro"
		    "(key %s%s%s%s%s)",
		    (modFlags & MACRO_CTRL) ? "Ctrl+" : "",
		    (modFlags & MACRO_META) ? "Meta+" : "",
		    (modFlags & MACRO_ALT) ? "Alt+" : "",
		    (modFlags & MACRO_SHIFT) ? "Shift+" : "",
		    XKeysymToString( keysym ) );
	    return 0;	/* Failure */
	}

	/*
	 * We're going to add to this macro chain, so don't replace this macro.
	 */
	replaceIndex = dummyIndex;

	/*
	 * Make the number of this macro one higher than the last in the macro
	 * chain.
	 */
	modFlags = macro_set_number( modFlags, macroNum+1 );
    }

    else
    {
	modFlags = macro_set_number( modFlags, 0 );

	/* Set replaceIndex to the index of a macro we can replace */
	if( dummyIndex < replaceIndex )
	    replaceIndex = dummyIndex;
    }



    /*
     * Add action to the list of macros (making it bigger if necessary).
     */
    if( replaceIndex == r->nmacros )
    {
	if( r->nmacros == r->maxMacros )
	{
	    /* Get space for more macros*/
	    r->maxMacros += nmacros_increment;
	    r->macros = (macros_t *) rxvt_realloc( r->macros,
					r->maxMacros * sizeof(macros_t));
	}

	r->nmacros++;
    }
    else
    {
	/* Macro action string needs to be freed (as it will be replaced) */
	if( r->macros[replaceIndex].action.str )
	    rxvt_free( r->macros[replaceIndex].action.str );
    }


    /*
     * Set the action string. This malloc's memory so any returns after this
     * should either save action in to a global variable, or free it.
     */
    assert( astring );
    SET_NULL(action.str);   /* Make sure rxvt_set_action won't free non-existent
			       memory */
    if( !rxvt_set_action( &action, astring) )
	return 0; /* Failure: Probably unrecognized action type */

    /*
     * Save macro values in our global macro list.
     */
    r->macros[replaceIndex].keysym	= keysym;
    r->macros[replaceIndex].modFlags	= modFlags;
    r->macros[replaceIndex].action	= action;
    r->macros[replaceIndex].priority	= priority;

    rxvt_dbgmsg(( DBG_DEBUG, DBG_MACROS,
		"Added macro %hu of %hu. Type %s, len %hu, args '%s'.\n",
		replaceIndex, r->maxMacros, macroNames[ action.type ],
		action.len,
		(action.type == MacroFnStr || action.type == MacroFnEsc) ?
		    "(escaped string)" :
		    (IS_NULL(action.str) ?
			"(nil)" : (char*) action.str)));

    return 1;	/* Success */
}

/* {{{1 rxvt_cleanup_macros()
 *
 * Delete all "Dummy" macros from our list of macros, and free space alloced for
 * extra macros
 */
void
rxvt_cleanup_macros( rxvt_t *r )
{
    unsigned i, nDummyMacros = 0;

    if( r->nmacros == 0 ) return; /* Nothing to be done */

    for( i = 0; i < r->nmacros; i++)
    {
	if(
	     r->macros[i].action.type == MacroFnDummy ||
	     NOT_KEYSYM(r->macros[i].keysym)
	  )
	{
	    /*
	     * Dummy macro needs to be deleted. Make sure this macro comes first
	     * in the macro list.
	     *
	     * 2006-03-06 gi1242: Would be more efficient if we made sure that
	     * this macro was last in the list, however that would involve
	     * knowing what the max keysym value is. Could be different on
	     * different architectures.
	     */
	    r->macros[i].keysym	    = 0;
	    r->macros[i].modFlags   = 0;

	    if (NOT_NULL(r->macros[i].action.str))
	    {
	        rxvt_free( r->macros[i].action.str );
	        SET_NULL(r->macros[i].action.str); /* Probably unnecessary */
	    }

	    nDummyMacros++;
	}
    } /* for */

    /*
     * The macro list now needs to be sorted on keysym. When we look for macros,
     * we assume the macro list is sorted, so we can use a binary search to
     * lookup macros quickly.
     */
    qsort( r->macros, r->nmacros, sizeof( macros_t ), (comparison_fn_t)&macro_cmp);

    /* Remove dummy macros from our list */
    MEMMOVE( r->macros, r->macros + nDummyMacros,
	    (r->nmacros - nDummyMacros) * sizeof( macros_t ) );
    r->nmacros -= nDummyMacros;

    /* Shrink our macros list */
    if( r->nmacros < r->maxMacros )
    {
	r->macros = rxvt_realloc( r->macros, r->nmacros * sizeof( macros_t ));
	r->maxMacros = r->nmacros;
    }

    rxvt_dbgmsg(( DBG_DEBUG, DBG_MACROS,
		"Read %d macros. (Have space for %d macros)\n",
		r->nmacros, r->maxMacros));
}

/* {{{1 rxvt_set_action( action, astring)
 *
 * Check what action is specified by astring, and assign respective values in
 * action.
 * 
 * The string astring might be modified, but can be freed immediately after
 * calling this function (regardless of wether it succeeds or not).
 */
static Bool
rxvt_set_action	    (action_t *action, char *astring)
{
    unsigned short type, len;

    rxvt_dbgmsg ((DBG_DEBUG, DBG_MACROS, "Setting action '%s'\n", astring));
    /*
     * Match head of "astring" to a name in macroNames to figure out the macro
     * type.
     */
    for( type = 0; type < NMACRO_FUNCS; type++)
    {
	if( (len = rxvt_str_match( astring, macroNames[type])) )
	{
	    /* Matched a macroName at the start of astring */
	    if( astring[len] && !isspace( astring[len] ) )
		/* Not delimited by a space */
		continue;

	    /* Skip macroName and delimiting spaces */
	    astring += len;
	    while( *astring && isspace( *astring ) ) astring++;

	    /* Exit for loop early */
	    break;
	}
    }

    if( type == NMACRO_FUNCS )
    {
	rxvt_msg (DBG_ERROR, DBG_MACROS,  "Action %s is not of known type", astring);
	return False; /* Failure: No matching macro name */
    }

    /*
     * Setup values in action
     */
    action->type	= type;

    /*
     * Interpolate escape sequences into action. XXX: Should we only do this for
     * MacroFnStr and MacroFnEsc?.
     */
    len	= rxvt_str_escaped( astring );

    /* All macros exept MacroFnStr and MacroFnEsc have null terminated string */
    if( type != MacroFnStr && type != MacroFnEsc && len > 0 && astring[len-1] )
	astring[ len++ ] = 0;	/* Since astring was null terminated,
				   astring[len] is certainly part of the memory
				   in astring. */

    action->len	    = len;

    /* Set action->str. If any data is previously there, realloc it. */
    if( len > 0 )
    {
	action->str = (unsigned char *) rxvt_realloc( action->str,
						len * sizeof(unsigned char));
	MEMCPY( action->str, astring, len);
    }
    else
    if (NOT_NULL(action->str))
    {
	rxvt_free( action->str );
	SET_NULL(action->str);
    }
    return True;
}

/* {{{1 rxvt_process_macros( keysym, ev)
 *
 * Check to see if a macro key was pressed. If yes, exec the action and return
 * 1. Else return 0.
 *
 * 2006-02-24 gi1242: Take both a keysym, and a XKeyEvent argument because, the
 * caller might have modified keysym based on XIM.
 */
/* EXTPROTO */
int
rxvt_process_macros( rxvt_t *r, KeySym keysym, XKeyEvent *ev)
{
    macros_t	ck,		/* Storing the keysym and mods of the current
				   key that's pressed. */
		*macro;		/* Macro we find in our saved list corresponding
				   to the current key press */
    int		status;

    if( r->nmacros == 0 )
	return 0;   /* No macro processed */

    /* Copy the modifier mask and keysym into ck */
    ck.modFlags = 0;
    if (ev->state & ShiftMask)		    ck.modFlags |= MACRO_SHIFT;
    if (ev->state & ControlMask)	    ck.modFlags |= MACRO_CTRL;
    if (ev->state & r->h.ModMetaMask)	    ck.modFlags |= MACRO_META;
    if (ev->state & r->h.ModAltMask)	    ck.modFlags |= MACRO_ALT;

    /* Use lowercase version so we can ignore caps lock */
    {
        KeySym upper;
        XConvertCase(keysym, &ck.keysym, &upper);
    }

    /* Check if macro ck is in our list of macros. */
    macro = bsearch( &ck, r->macros, r->nmacros, sizeof( macros_t ),
		(comparison_fn_t)&macro_cmp);
    if (
         /*
          * No macro found.
          */
         IS_NULL(macro)
         || (
              /*
               * Primary only macro in secondary screen.
               */
              (macro->modFlags & MACRO_PRIMARY)
              && PVTS(r)->current_screen != PRIMARY
            )
         || (
              /*
               * When macros are disabled, only the toggle macros macro should
               * work.
               */
              ISSET_OPTION(r, Opt2_disableMacros)
              && macro->action.type != MacroFnToggleMacros
            )
       )
	return 0;   /* No macro processed */

    do
      {
	rxvt_dbgmsg ((DBG_DEBUG, DBG_MACROS, "Processing macro #%d mods %02hhx\n", macro - r->macros, macro->modFlags));
	status = rxvt_dispatch_action( r, &(macro->action), (XEvent*) ev );
      }
    while(
	   status == 1
	   && (++macro - r->macros) < r->nmacros
	   && MACRO_GET_NUMBER( macro->modFlags ) 
	 );

    return status;
}

/* {{{1 rxvt_dispatch_action( action, ev)
 *
 * Exec the macro / menu action with type "type" and action "action". Returns 1
 * on success, -1 on failure.
 */
static int
rxvt_dispatch_action( rxvt_t *r, action_t *action, XEvent *ev)
{
    const int	maxLen = 1024;
    char	expstr[ maxLen ];
    char	*astr;
    int		alen,
		retval = 1;	    /* Succeed by default */

    if( IS_NULL( action->str ) )
    {
	SET_NULL( astr );
	alen = 0;
    }
    else
    {
	/* % interpolate the action string */
	astr = expstr;
	alen = rxvt_percent_interpolate( r, (char *) action->str,
		action->len, astr, maxLen );
    }


    switch( action->type )
    {
	case MacroFnEsc:
	    /* Send action to rxvt */
	    if( NOT_NULL( astr ) && alen > 1 )
		rxvt_cmd_write( r, (unsigned char*) astr, alen - 1);
	    else
	    {
		rxvt_msg (DBG_ERROR, DBG_MACROS,  "Macro %s requires argument.",
			macroNames[action->type] );
		retval = -1;
	    }
	    break;

	case MacroFnStr:
	    /* Send action to child process */
	    if( NOT_NULL( astr ) && alen > 1 )
		rxvt_tt_write( r, (unsigned char*) astr, alen - 1);
	    else
	    {
		rxvt_msg (DBG_ERROR, DBG_MACROS,  "Macro %s requires argument.",
			macroNames[action->type] );
		retval = -1;
	    }
	    break;

	case MacroFnExec:
	    if( NOT_NULL( astr ) )
		retval = rxvt_async_exec( r, astr ) ? 1 : -1;

	    else
	    {
		rxvt_msg (DBG_ERROR, DBG_MACROS,  "Macro %s requires argument.",
			macroNames[action->type] );
		retval = -1;
	    }

	    break;

	case MacroFnClose:
	    if( alen > 0 && *(astr) )
	    {
		/* Close tab specified by str */
		int tabno = atoi( (char*) astr) - 1;

		if ( tabno == -1 || tabno == 0 )
		{
		    rxvt_kill_page (r);
		}
		else
		    retval = -1;
	    }
	    else
		rxvt_exit_request( r );

	    break;

	case MacroFnScroll:
	    /* Scroll by an amount specified in astr */
	    if( alen > 1 )
	    {
		char	*suffix;
		int	amount = abs(strtol( astr, &suffix, 0 ));
		enum page_dirn	direction   = (*(astr) == '-' ? UP : DN);

		rxvt_dbgmsg ((DBG_DEBUG, DBG_MACROS, "astr: '%s', alen: %d\n", astr, alen));

		if( tolower( *suffix ) == 'p' )
		    /* scroll pages */
		    amount *=
#ifdef PAGING_CONTEXT_LINES
				r->TermWin.nrow - PAGING_CONTEXT_LINES
#else
				r->TermWin.nrow * 4 / 5
#endif
		    ;
		else if( tolower( *suffix ) == '%' )
		    /*
		     * 2008-08-04 Jim Diamond: Scroll as a perCent of the
		     * current window size.
		     */
		    amount = amount * r->TermWin.nrow / 100;

		rxvt_scr_page(r, direction, amount);

	    }
	    break;

#if 0
	case MacroFnCopy:
#endif
	case MacroFnPaste:
	{
	    int sel = XA_PRIMARY;

	    if (NOT_NULL(ev))
	    {
		if( NOT_NULL(astr) && *astr )
		{
		    if(strcmp ("PRIMARY", astr) == 0)
			sel=XA_PRIMARY;
		    else if (strcmp ("SECONDARY", astr) == 0)
		      sel=XA_SECONDARY;
		    else if (strcmp ("CLIPBOARD", astr) == 0)
		      sel=XA_CLIPBOARD;
		    else
		      break;
		    rxvt_selection_request_by_sel( r, ev->xkey.time,
			    0, 0, sel);
		}
		else
		    rxvt_selection_request (r, ev->xkey.time, 0, 0);
	    }
	    else
	    {
		retval = -1;
	    }
	    break;
	}

	case MacroFnPasteFile:
	{
	    if (NOT_NULL(ev))
	    {
	        if( NOT_NULL(astr) && *astr )
		    rxvt_paste_file (r, ev->xkey.time, 0, 0, astr);
	        else
		    break;
	    }
	    else
		retval = -1;

	   break;
	}

	case MacroFnFont:
	{
	    const int MaxFontLen = 8;	/* Only need space for "#+xx" */

	    char fontname[MaxFontLen];
	    if( alen >= MaxFontLen - 1 ) break;	/* Remember that
						   alen includes
						   the trailing null
						   char */

	    fontname[0] = FONT_CMD;		/* Internal prefix */
	    STRNCPY( fontname + 1, astr, MaxFontLen - 1);
	    fontname[MaxFontLen - 1] = '\0';	/* Null terminate */

	    rxvt_resize_on_font( r, fontname );
	    break;
	}

	case MacroFnToggleVeryBold:
	    TOGGLE_OPTION( r, Opt2_veryBold );

	    rxvt_scr_touch (r, True);
	    break;

	case MacroFnToggleBoldColors:
	    TOGGLE_OPTION( r, Opt2_boldColors );

	    rxvt_scr_touch (r, True);
	    break;

	case MacroFnToggleVeryBright:
	    TOGGLE_OPTION( r, Opt_veryBright );

	    rxvt_scr_touch (r, True);
	    break;

	case MacroFnToggleHold:
	    if( NOT_NULL(astr) && *astr )
	    {
		/* Set the hold option for this tab */
		char		op = *astr++;
		unsigned long	holdMask;
		
		holdMask = strtoul( astr, NULL, 0 );
		switch( op )
		{
		    case '+':
			PVTS(r)->holdOption |= holdMask;
			break;

		    case '-':
			PVTS(r)->holdOption &= ~holdMask;
			break;

		    case '!':
			PVTS(r)->holdOption ^= holdMask;
			break;

		    default:
			rxvt_msg (DBG_ERROR, DBG_MACROS,  "Badly formed argument '%s' to %s\n",
				astr, macroNames[action->type] );
			retval = -1;
			break;
		}

		/* Remove the ATAB if it no longer needs to be held */
		if(
		     PVTS(r)->dead && PVTS(r)->hold > 1 &&
		     !SHOULD_HOLD( r )
		  )
		    rxvt_remove_page( r );
	    }

	    else
	    {
		/*
		 * Behaviour almost compatible with mrxvt-0.5.1: Just get rid of
		 * all held tabs.
		 */
		    if (PVTS(r)->dead && PVTS(r)->hold > 1)
			rxvt_remove_page (r);
	    }

	    break;

	case MacroFnToggleFullscreen:
	    ewmh_message( r->Xdisplay, XROOT, r->TermWin.parent,
		XInternAtom( r->Xdisplay, "_NET_WM_STATE", True),
		_NET_WM_STATE_TOGGLE,
		XInternAtom( r->Xdisplay, "_NET_WM_STATE_FULLSCREEN", True),
		0, 0, 0 );
	    break;

	case MacroFnRaise:
	    ewmh_message( r->Xdisplay, XROOT, r->TermWin.parent,
		XInternAtom( r->Xdisplay, "_NET_ACTIVE_WINDOW", True),
		1, 0 /*timestamp?*/, r->TermWin.parent, 0, 0 );
	    break;


	case MacroFnSetTitle:
	    if (NOT_NULL(astr))
		rxvt_set_term_title( r,
			(unsigned char*) astr);
	    else if (NOT_NULL(r->selection.text))
		rxvt_set_term_title( r,
			(const unsigned char TAINTED*) r->selection.text);
	    else
		retval = -1;
	    break;

	case MacroFnSaveConfig:
	{
	    char    cfile[PATH_MAX] = "";

	    if (NOT_NULL(astr))
		STRNCPY( cfile, astr, PATH_MAX-1 );
	    else
	    {
		char*	home = getenv ("HOME");

		if (IS_NULL(home)) return -1; /* Failure */

		snprintf (cfile, PATH_MAX-1, "%s/%s", home,
			".drxvtrc.save");
	    }
	    cfile[PATH_MAX-1] = (char) 0;   /* Null terminate */

	    retval = rxvt_save_options (r, cfile) ? 1 : -1;
	    break;
	}

	case MacroFnToggleMacros:
	    TOGGLE_OPTION(r, Opt2_disableMacros);
	    break;

	default:
	    assert( action->type < sizeof( macroNames ) / sizeof( char ** ) );

	    rxvt_msg (DBG_ERROR, DBG_MACROS,  "Support for macro type '%s' not compiled.",
		    macroNames[action->type]);
	    retval = -1;
    }

    return retval;
}
/* }}} */

void
rxvt_free_macros( rxvt_t *r )
{
    unsigned i;
    for (i = 0; i < r->nmacros; i ++)
    {
	rxvt_free(r->macros[i].action.str);
    }
    rxvt_free(r->macros);
    r->macros = NULL;
}

/* vim: set fdm=marker: */
/*----------------------- end-of-file (C source) -----------------------*/
