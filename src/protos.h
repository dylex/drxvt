/*--------------------------------*-H-*---------------------------------*
 * File:    protos.h
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1997-2001   Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2004        Sergey Popov <p_sergey@jungo.com>
 * Copyright (c) 2004        Jingmin Zhou <jimmyzhou@users.sourceforge.net>
 * Copyright (C) 2008		  Jehan Hysseo <hysseo@users.sourceforge.net>
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

#ifndef __PROTOS_H__
#define __PROTOS_H__


/* Begin prototypes of command.c */
void	    rxvt_main_loop(rxvt_t *);
void		 rxvt_cmd_write                   __PROTO((rxvt_t* r, const unsigned char* str, unsigned int count));
void             rxvt_tt_printf                   __PROTO((rxvt_t* r, const char* fmt,...));
void             rxvt_tt_write                    __PROTO((rxvt_t* r, const unsigned char* d, int len));
void             rxvt_pointer_unblank             __PROTO((rxvt_t* r));
void           	 rxvt_resize_on_font           	  __PROTO((rxvt_t* r, char* fontname));
/* End prototypes of command.c */


/* Begin prototypes of encoding.c */
#ifdef MULTICHAR_SET
void             rxvt_set_multichar_encoding      __PROTO((rxvt_t* r, const char* str));
#endif
void             rxvt_decode_dummy                __PROTO((unsigned char* str, int len));
void             rxvt_set_default_locale          __PROTO((rxvt_t *r));
char*            rxvt_get_encoding_from_locale    __PROTO((rxvt_t *r));
void             rxvt_set_default_font_x11        __PROTO((rxvt_t *r));
char*            rxvt_fallback_mfont_x11          __PROTO((rxvt_t *r));
#ifdef XFT_SUPPORT
void             rxvt_set_default_font_xft        __PROTO((rxvt_t *r));
char*            rxvt_fallback_mfont_xft          __PROTO((rxvt_t *r));
#endif
char*            rxvt_encoding_name               __PROTO((rxvt_t *r));
/* End prototypes of defaultfont.c */


/* Begin prototypes of macros.c */
int		 rxvt_parse_macros		  __PROTO((rxvt_t*, const char *, const char *, macro_priority_t priority));
void		 rxvt_cleanup_macros		  __PROTO((rxvt_t*));
int		 rxvt_process_macros		  __PROTO((rxvt_t*, KeySym, XKeyEvent*));
void	 	 rxvt_free_macros		  __PROTO((rxvt_t*));
/* End prototypes of macros.c */

/* Begin prototypes of init.c */
int              rxvt_init_vars                   __PROTO((rxvt_t* r));
void             rxvt_init_secondary              __PROTO((rxvt_t* r));
void		 rxvt_set_jumpscroll		  __PROTO((rxvt_t* r));
const char    ** rxvt_init_resources              __PROTO((rxvt_t* r, int argc, const char* const *argv));
int		 rxvt_set_fgbg_colors		  __PROTO((rxvt_t* r));
int		 rxvt_set_vt_colors		  __PROTO((rxvt_t* r));
void		 rxvt_copy_color                  __PROTO((rxvt_t*, int, int));
void		 rxvt_set_color                   __PROTO((rxvt_t*, int, const XColor*));
void             rxvt_init_env                    __PROTO((rxvt_t *r));
void             rxvt_init_xlocale                __PROTO((rxvt_t *r));
void             rxvt_init_command                __PROTO((rxvt_t* r));
CARD32           rxvt_get_desktop                 __PROTO((rxvt_t* r));
void             rxvt_create_show_windows         __PROTO((rxvt_t* r, int argc, const char* const *argv));
void             rxvt_destroy_termwin             __PROTO((rxvt_t* r));
int             rxvt_create_termwin              __PROTO((rxvt_t* r));
const char*	 getProfileOption		  __PROTO(( rxvt_t *r, int resource ));
Status		 ewmh_message			  __PROTO(( Display *, Window, Window, Atom, long, long, long, long, long));
int		 rxvt_async_exec		  __PROTO((rxvt_t*, const char *));
int              rxvt_run_command                 __PROTO((rxvt_t* r, const char** argv));
termenv_t        rxvt_get_termenv                 __PROTO((const char* str));
char**		 rxvt_string_to_argv    	  __PROTO((const char*, int*));
/* End prototypes of init.c */


/* Begin prototypes of logging.c */
#ifdef UTMP_SUPPORT
void             rxvt_makeutent                   __PROTO((rxvt_t* r, const char* pty, const char* hostname));
void             rxvt_cleanutent                  __PROTO((rxvt_t* r));
#endif
/* End prototypes of logging.c */


/* Begin prototypes of rxvtmem.c */
#ifndef MTRACE
void*            rxvt_malloc                      __PROTO((size_t size)) __attribute__((malloc));
void*            rxvt_calloc                      __PROTO((size_t number, size_t size)) __attribute__((malloc));
void*            rxvt_realloc                     __PROTO((void *ptr, size_t size)) __attribute__((warn_unused_result));
void             rxvt_free                        __PROTO((void *ptr));
#else
#define rxvt_malloc malloc
#define rxvt_calloc calloc
#define rxvt_realloc realloc
#define rxvt_free free
#endif
/* End prototypes of rxvtmem.c */


/* Begin prototypes of main.c */
rxvt_t*	    rxvt_init (int, const char *const *);
void             rxvt_privileges                  __PROTO((int mode));
RETSIGTYPE       rxvt_Child_signal                __PROTO((int sig __attribute__((unused))));
RETSIGTYPE       rxvt_Exit_signal                 __PROTO((int sig));
void		 rxvt_exit_request                __PROTO((rxvt_t*));
void             rxvt_clean_exit                  __PROTO((rxvt_t* r));
void             rxvt_privileged_utmp             __PROTO((rxvt_t* r, char action));
void             rxvt_privileged_ttydev           __PROTO((rxvt_t* r, char action));
void             rxvt_tt_winsize                  __PROTO((int fd, unsigned short col, unsigned short row, pid_t pid));
void             rxvt_init_font_x11               __PROTO((rxvt_t *r));
int              rxvt_change_font_x11             __PROTO((rxvt_t* r, const char* fontname));
#ifdef XFT_SUPPORT
int              rxvt_init_font_xft               __PROTO((rxvt_t *r));
int              rxvt_change_font_xft             __PROTO((rxvt_t* r, const char* fontname));
#endif
void             rxvt_set_term_title              __PROTO((rxvt_t* r, const unsigned char* str));
void             rxvt_set_icon_name               __PROTO((rxvt_t* r, const unsigned char* str));
void             rxvt_set_window_color            __PROTO((rxvt_t* r, int idx, const char* color));
void             rxvt_recolour_cursor             __PROTO((rxvt_t *r));
#ifdef XFT_SUPPORT
int              rxvt_alloc_xft_color             __PROTO((rxvt_t *r, const XColor *xcol, XftColor* xftcolor));
#endif
int              rxvt_parse_alloc_color           __PROTO((rxvt_t* r, XColor *screen_in_out, const char* colour));
int              rxvt_alloc_color                 __PROTO((rxvt_t* r, XColor *screen_in_out, const char* colour));
void             rxvt_IM_send_spot                __PROTO((rxvt_t *r));
void             rxvt_IM_set_fontset              __PROTO((rxvt_t* r, int idx));
void             rxvt_IM_init_callback            __PROTO((Display *unused __attribute__((unused)), XPointer client_data __attribute__((unused)), XPointer call_data __attribute__((unused))));
void             rxvt_IM_resize                   __PROTO((rxvt_t *r));
rxvt_t         * rxvt_get_r                       __PROTO((void));
/* End prototypes of main.c */


/* Begin prototypes of misc.c */
char           * rxvt_r_basename                  __PROTO((const char* str));
int              rxvt_str_match                   __PROTO((const char* s1, const char* s2));
char*            rxvt_str_trim                    __PROTO((char* str));
int              rxvt_str_escaped                 __PROTO((char* str));
int		 rxvt_percent_interpolate         __PROTO((rxvt_t*, const char *, int, char *, int));
char**           rxvt_splitcommastring            __PROTO((const char* cs));
/* End prototypes of misc.c */


/* Begin prototypes of netdisp.c */
#ifdef NET_DISPLAY
char           * rxvt_network_display             __PROTO((const char* display));
#endif
/* End prototypes of netdisp.c */


/* Begin prototypes of ptytty.c */
int              rxvt_get_pty                     __PROTO((int *fd_tty, char** ttydev));
int              rxvt_get_tty                     __PROTO((const char* ttydev));
int              rxvt_control_tty                 __PROTO((int fd_tty, const char* ttydev));
/* End prototypes of ptytty.c */


/* Begin prototypes of screen.c */
void             rxvt_init_screen                 __PROTO((rxvt_t* r));
void             rxvt_scr_reset                   __PROTO((rxvt_t* r));
void             rxvt_scr_release                 __PROTO((rxvt_t* r));
void             rxvt_scr_poweron                 __PROTO((rxvt_t* r));
void             rxvt_scr_cursor                  __PROTO((rxvt_t* r, int mode));
int              rxvt_scr_change_screen           __PROTO((rxvt_t* r, int scrn));
void             rxvt_scr_color                   __PROTO((rxvt_t* r, unsigned int color, int fgbg));
void             rxvt_scr_rendition               __PROTO((rxvt_t* r, int set, rend_t style));
int              rxvt_scroll_text                 __PROTO((rxvt_t* r, int row1, int row2, int count, int spec));
void             rxvt_scr_add_lines               __PROTO((rxvt_t* r, const unsigned char* str, int nlines, int len));
void             rxvt_scr_backspace               __PROTO((rxvt_t* r));
void             rxvt_scr_tab                     __PROTO((rxvt_t* r, int count));
void             rxvt_scr_backindex               __PROTO((rxvt_t* r));
void             rxvt_scr_forwardindex            __PROTO((rxvt_t* r));
void             rxvt_scr_gotorc                  __PROTO((rxvt_t* r, int row, int col, int relative));
void             rxvt_scr_index                   __PROTO((rxvt_t* r, enum page_dirn direction));
void             rxvt_scr_erase_line              __PROTO((rxvt_t* r, int mode));
void             rxvt_scr_erase_screen            __PROTO((rxvt_t* r, int mode));
void             rxvt_scr_E                       __PROTO((rxvt_t* r));
void             rxvt_scr_insdel_lines            __PROTO((rxvt_t* r, int count, int insdel));
void             rxvt_scr_insdel_chars            __PROTO((rxvt_t* r, int count, int insdel));
void             rxvt_scr_scroll_region           __PROTO((rxvt_t* r, int top, int bot));
void             rxvt_scr_cursor_visible          __PROTO((rxvt_t* r, int mode));
void             rxvt_scr_autowrap                __PROTO((rxvt_t* r, int mode));
void             rxvt_scr_relative_origin         __PROTO((rxvt_t* r, int mode));
void             rxvt_scr_insert_mode             __PROTO((rxvt_t* r, int mode));
void             rxvt_scr_set_tab                 __PROTO((rxvt_t* r, int mode));
void             rxvt_scr_rvideo_mode             __PROTO((rxvt_t* r, int mode));
void             rxvt_scr_report_position         __PROTO((rxvt_t* r));
void             rxvt_scr_charset_choose          __PROTO((rxvt_t* r, int set));
void             rxvt_scr_charset_set             __PROTO((rxvt_t* r, int set, unsigned int ch));
int              rxvt_scr_get_fgcolor             __PROTO((rxvt_t *r));
int              rxvt_scr_get_bgcolor             __PROTO((rxvt_t *r));
void             rxvt_scr_expose                  __PROTO((rxvt_t* r, int x, int y, int width, int height, Bool refresh));
void             rxvt_scr_touch                   __PROTO((rxvt_t* r, Bool refresh));
int              rxvt_scr_move_to                 __PROTO((rxvt_t* r, int y, int len));
int              rxvt_scr_page                    __PROTO((rxvt_t* r, enum page_dirn direction, int nlines));
void             rxvt_scr_bell                    __PROTO((rxvt_t *r));
void             rxvt_scr_refresh                 __PROTO((rxvt_t* r, unsigned char refresh_type));
void             rxvt_scr_clear                   __PROTO((rxvt_t* r));
void             rxvt_scr_dump                    __PROTO((rxvt_t* r, int fd));
void             rxvt_selection_check             __PROTO((rxvt_t* r, int check_more));
int              rxvt_selection_paste             __PROTO((rxvt_t* r, Window win, Atom prop, Bool delete_prop));
void             rxvt_selection_property          __PROTO((rxvt_t* r, Window win, Atom prop));
void             rxvt_selection_request           __PROTO((rxvt_t* r, Time tm, int x, int y));
void             rxvt_selection_request_by_sel    __PROTO((rxvt_t* r, Time tm, int x, int y, int sel));
void             rxvt_process_selectionclear      __PROTO((rxvt_t* r));
void             rxvt_selection_make              __PROTO((rxvt_t* r, Time tm));
void             rxvt_selection_click             __PROTO((rxvt_t* r, int clicks, int x, int y));
void             rxvt_selection_extend            __PROTO((rxvt_t* r, int x, int y, int flag));
void             rxvt_selection_rotate            __PROTO((rxvt_t* r, int x, int y));
void             rxvt_paste_file                  __PROTO((rxvt_t* r, Time tm, int x, int y, char* filename));
void             rxvt_process_selectionrequest    __PROTO((rxvt_t* r, const XSelectionRequestEvent *rq));
void             rxvt_pixel_position              __PROTO((rxvt_t* r, int *x, int *y));
/* End prototypes of screen.c */


/* Begin prototypes of strings.c */
#ifdef HAVE_WCHAR_H
wchar_t*         rxvt_mbstowcs                    __PROTO((const char* str));
char*            rxvt_wcstoutf8                   __PROTO((const wchar_t* str));
#endif	/* HAVE_WCHAR_H */
/* End prototypes of strings.c */

/* Begin prototypes of transparent.c */
int		rxvt_set_opacity	     	 __PROTO((rxvt_t*));
void		rxvt_process_reparentnotify   	 __PROTO((rxvt_t*, XEvent*));
/* End prototypes of transparent.c */

/* Begin prototypes of xdefaults.c */
int              rxvt_save_options                __PROTO((rxvt_t* r, const char* filename));
void             rxvt_get_options                 __PROTO((rxvt_t* r, int argc, const char* const *argv));
void             rxvt_extract_resources           __PROTO((rxvt_t* r, Display *display __attribute__((unused)), const char* name));
/* End prototypes of xdefaults.c */


/* Begin prototypes of tabbar.c */
void             rxvt_append_page               __PROTO((rxvt_t*, const char *command));
void             rxvt_kill_page                 __PROTO((rxvt_t*));
void             rxvt_remove_page               __PROTO((rxvt_t*));
/* End prototypes of tabbar.c */

/* Begin prototypes of session.c */
#ifdef HAVE_X11_SM_SMLIB_H
void            rxvt_process_ice_msgs          __PROTO((rxvt_t* r));
void            rxvt_session_init              __PROTO((rxvt_t* r));
void            rxvt_session_exit              __PROTO((rxvt_t* r));
#endif	/* HAVE_X11_SM_SMLIB_H */
/* End prototypes of session.c */

/* Begin prototypes of debug.c */
int             rxvt_msg                       __PROTO((uint32_t, uint32_t, const char*, ...));
void            rxvt_parse_dbg_arguments       __PROTO((int argc, const char* const*));
/* End prototypes of debug.c */

#endif  /* __PROTOS_H__ */
/*----------------------- end-of-file (H source) -----------------------*/
