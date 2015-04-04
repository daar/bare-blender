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

/*

 * initrender_ext.h
 *
 * Version: $Id: initrender.h,v 1.3 2000/08/28 12:23:44 nzc Exp $
 */

#ifndef INITRENDER_EXT_H
#define INITRENDER_EXT_H "$Id: initrender.h,v 1.3 2000/08/28 12:23:44 nzc Exp $"

/* type includes */

#include "effect_types.h"        /* for PartEff type */
#include "render_types.h"

/* Functions */

/**
 * Retrieve number of this halo, or add one. This function is currently overloaded!
 * It should be split...
 * @param nr   The halo index
 * @return     Pointer to the indexed halo, or a new one
 */
HaloRen *addhalo(int nr);

/**
 * Retrieve number of this vertex, or add one. This function is currently overloaded!
 * It should be split...
 * @param nr   The vertex index
 * @return     Pointer to the indexed vertex, or a new one
 */
VertRen *addvert(int nr);

/**
 * Retrieve number of this face, or add one. This function is currently overloaded!
 * It should be split... (note: 'vlak' is dutch for 'face')
 * @param nr   The face index
 * @return     Pointer to the indexed face, or a new one
 */
VlakRen *addvlak(int nr);


/* Here are all functions from initrender. Needs to be sorted still  */
void add_cmapcode(void);
void add_render_lamp(Object *ob, int doshadbuf);
void add_to_blurbuf(int blur);
void addparttorect(short nr, Part *part);
float  calc_weight(float *weight, int i, int j);
int contrpuntnormr(float *n, float *puno);
void defaultlamp(void);
void freeroteerscene(void);
Material *give_render_material(Object *ob, int nr);
void info_calc_drot(Object *ob, float *co);
int info_calc_schermco(float *vec, float *sco);
void info_file(Object *ob);
void init_def_material(void);
void init_render_curve(Object *ob);
void init_render_jit(int nr);
void init_render_mball(Object *ob);
void init_render_mesh(Object *ob);
void init_render_object(Object *ob);
void init_render_surf(Object *ob);
HaloRen *inithalo(Material *ma, float *vec, float *vec1, float *orco, float hasize, float vectsize);
void initjit(float *jit, int num);
void initparts(void);


HaloRen *initstar(float *vec, float hasize);
void make_info_file_string(char *string, int frame);
void make_render_halos(Object *ob, Mesh *me, Material *ma, float *extverts);
void normalenrender(int startvert, int startvlak);
int panotestclip(float *v);
void render(void);  /* hierbinnen de PART en FIELD lussen */
void render_particle_system(Object *ob, PartEff *paf);

/** 
 * Transform all objects in this scene from world to view coordinates.
 */ 
void roteerscene(void);

void schrijfplaatje(char *name);
void setpanorot(int part);
short setpart(short nr); /* return 0 als geen goede part */
/**
 * Test visibility of vertices, and sets flags accordingly. The Z
 * buffer uses the flags to see whether a vertex need rendering? Is
 * there clipping going on inside this routine?
 */
void setzbufvlaggen( void (*projectfunc)() );

/**
 * Determine normal vectors on faces?
 */
void set_normalflags();

void sort_halos(void);
void split_u_renderfaces(int startvlak, int startvert, int usize, int plek, int cyclu);
void split_v_renderfaces(int startvlak, int startvert, int usize, int vsize, int plek, int cyclu, int cyclv);
int verghalo(const void *x1, const void *x2);

#endif /* INITRENDER_EXT_H */


