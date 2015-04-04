/**
 * $Id:$
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * The contents of this file may be used under the terms of either the GNU
 * General Public License Version 2 or later (the "GPL", see
 * http://www.gnu.org/licenses/gpl.html ), or the Blender License 1.0 or
 * later (the "BL", see http://www.blender.org/BL/ ) which has to be
 * bought from the Blender Foundation to become active, in which case the
 * above mentioned GPL option does not apply.
 *
 * The Original Code is Copyright (C) 2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */



/* render.h
 *
 *
 * maart 95
 *
 * Version: $Id: render.h,v 1.7 2000/09/14 16:32:49 nzc Exp $
 */


#ifndef RENDER_H
#define RENDER_H "$Id: render.h,v 1.7 2000/09/14 16:32:49 nzc Exp $"

/* ------------------------------------------------------------------------- */
/* This little preamble might be moved to a separate include. It contains    */
/* some defines that should become functions, and some platform dependency   */
/* fixes. I think it is risky to always include it...                        */
/* ------------------------------------------------------------------------- */

/* For #undefs of stupid windows defines */
#ifdef WIN32
#include "winstuff.h"
#endif

#ifdef __BeOS

	/* dirty hack: pointers are negative, indices positive */
	/* pointers are not converted */
	
#define IS_A_POINTER_CODE(a)		((a)<0)
#define POINTER_FROM_CODE(a)		((void *)((a)))
#define POINTER_TO_CODE(a)			((long)(a))

#else

	/* dirty hack: pointers are negative, indices positive */
	/* pointers should be converted to positive numbers */
	
#define IS_A_POINTER_CODE(a)		((a)<0)
#define POINTER_FROM_CODE(a)		((void *)(-(a)))
#define POINTER_TO_CODE(a)			(-(long)(a))

#endif

#define BRICON		Tin= (Tin-0.5)*tex->contrast+tex->bright-0.5; \
					if(Tin<0.0) Tin= 0.0; else if(Tin>1.0) Tin= 1.0;

#define BRICONRGB	Tr= tex->rfac*((Tr-0.5)*tex->contrast+tex->bright-0.5); \
					if(Tr<0.0) Tr= 0.0; else if(Tr>1.0) Tr= 1.0; \
					Tg= tex->gfac*((Tg-0.5)*tex->contrast+tex->bright-0.5); \
					if(Tg<0.0) Tg= 0.0; else if(Tg>1.0) Tg= 1.0; \
					Tb= tex->bfac*((Tb-0.5)*tex->contrast+tex->bright-0.5); \
					if(Tb<0.0) Tb= 0.0; else if(Tb>1.0) Tb= 1.0;

/* ------------------------------------------------------------------------- */
/* Types                                                                     */
/* Both external and internal types can be placed here. Make sure there are  */
/* no dirty extras in the type files so they can be included without         */
/* problems. If possible, make a note why the include is needed.             */
/* ------------------------------------------------------------------------- */
#include "radio_types.h"  /* for RadView */
#include "render_types.h"

/* ------------------------------------------------------------------------- */
/* Global variables                                                          */
/* These variable are global to the render module, and also externally       */
/* visible. The file where they are defined must be added.                   */
/* ------------------------------------------------------------------------- */

extern Render    R;           /* rendercore.c */
extern Osa       O;           /* rendercore.c */
extern Material  defmaterial; /* initrender.c */
extern ushort   *igamtab1;    /* initrender.c */
extern ushort   *gamtab;      /* initrender.c */

/* ------------------------------------------------------------------------- */
/* Function definitions                                                      */
/*                                                                           */
/* All functions that need to be externally visible must be declared here.   */
/* Currently, this interface contains 41 functions.                          */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* initrender (17)                                                           */
/* ------------------------------------------------------------------------- */

/**
 * Guarded call to frame renderer? Tests several limits and boundary
 * conditions. 
 */
void    RE_initrender(void);

/**
 *
 */
void    RE_setwindowclip(int mode, int jmode);

void    RE_animrender(void);
void    RE_end_timer(int *real, int *cpu);
void    RE_exit_render_stuff(void);
void    RE_free_render_data(void);
void    RE_free_filt_mask(void);
void    RE_holoview(void);
void    RE_init_filt_mask(void);
void    RE_init_render_data(void);
void    RE_jitterate1(float *jit1, float *jit2, int num, float rad1);
void    RE_jitterate2(float *jit1, float *jit2, int num, float rad2);
void    RE_make_existing_file(char *name);
void    RE_make_stars(int wire);
void    RE_screendump(void);
void    RE_start_timer(void);
void    RE_write_image(char *name);

/* ------------------------------------------------------------------------- */
/* zbuf (2)                                                                  */
/* ------------------------------------------------------------------------- */

/**
 * Converts a world coordinate into a homogenous coordinate in view
 * coordinates. (WCS -> HCS)
 * Also called in: shadbuf.c render.c radfactors.c
 *                 initrender.c envmap.c editmesh.c
 * @param v1  [3 floats] the world coordinate
 * @param adr [4 floats] the homogenous view coordinate
 */
void    RE_projectverto(float *v1,float *adr);

/**
 * Something about doing radiosity z buffering?
 * (called in radfactors.c), hope the RadView is defined already... 
 * Also called in: radfactors.c
 * Note: Uses globals.
 * @param radview radiosity view definition
 */
void    RE_zbufferall_radio(RadView *vw);

/* ------------------------------------------------------------------------- */
/* renderfg (11)                                                                 */
/* ------------------------------------------------------------------------- */

/**
 * Draw a numbered cursor. This is used e.g. to display the number of
 * the frame currently being calculated. (It's the red counter
 * thingy when rendering pictures.).
 */
void    RE_set_timecursor(int nr);

/**
 * Clean up the active render context.
 */
void    RE_close_render_display();

/**
 * Draw a block of scanlines from the current displaybuffer (rect) in
 * the current rendercontext. Rendering starts at the current valid
 * raster position.
 * @param starty y coordinate of the first line to draw
 * @param endy y coordinate of the last line to draw
 */
void    RE_render_display(int starty, int endy);

void    RE_bgntimer(void);
void    RE_endtimer(void);
int     RE_do_renderfg(int anim);
int     RE_test_break(void);
void    RE_zoomwin(void);
void    RE_do_renderfg_seq(void);
void    RE_redraw_render_win(int val);
void    RE_toggle_render_display(void);

/* ------------------------------------------------------------------------- */
/* previewrender (3)                                                            */
/* ------------------------------------------------------------------------- */
void    RE_preview_changed(short win);

/* maybe not external? */
uint    RE_previewback(int type, int x, int y);

/* only used in space.c */
void    RE_previewrender(void);

/* ------------------------------------------------------------------------- */
/* envmap (6)                                                                   */
/* ------------------------------------------------------------------------- */
void    RE_save_envmap(EnvMap *env, char *str);
void    RE_free_envmapdata(EnvMap *env);
void    RE_free_envmap(EnvMap *env);
EnvMap *RE_add_envmap(void);
/* these two maybe not external */
EnvMap *RE_copy_envmap(EnvMap *env);
/* (used in texture.c) */
int     RE_envmaptex(Tex *tex, float *texvec, float *dxt, float *dyt);

/* ------------------------------------------------------------------------- */
/* rendercore (2)                                                            */
/* ------------------------------------------------------------------------- */
float   RE_Spec(float inp, int hard);

/* maybe not external */
void    RE_calc_R_ref(void);

/* ------------------------------------------------------------------------- */
/* texture.c (22)                                                            */
/* ------------------------------------------------------------------------- */
/* These names should keep their names for now. I think these are needed for */
/* the plugins.                                                              */
extern     ColorBand *add_colorband(void);
extern     MTex *add_mtex(void);
extern     PluginTex *add_plugin_tex(char *str);
extern     Tex *add_texture(char *name);
extern     void autotexname(Tex *tex);
extern     void default_mtex(MTex *mtex);
extern     Tex *copy_texture(Tex *tex);
extern     void default_tex(Tex *tex);
extern     void end_render_texture(Tex *tex);
extern     void end_render_textures(void);
extern     void externtex(MTex *mtex, float *vec);
extern     void externtexcol(MTex *mtex, float *orco, char *col);
extern     void free_plugin_tex(PluginTex *pit);
extern     void free_texture(Tex *tex);
extern     void init_render_texture(Tex *tex);
extern     void init_render_textures();
extern     void make_local_texture(Tex *tex);
extern     void tubemap(float x, float y, float z, float *adr1, float *adr2);
extern     void spheremap(float x, float y, float z, float *adr1, float *adr2);
extern     int do_colorband(ColorBand *coba);
extern     void open_plugin_tex(PluginTex *pit);
extern     int test_dlerr(const char *name,  const char *symbol);


#endif /* RENDER_H */

