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

 * texture_ext.h
 *
 * Version: $Id: texture.h,v 1.4 2000/09/13 12:33:50 nzc Exp $
 */

#ifndef TEXTURE_EXT_H
#define TEXTURE_EXT_H "$Id: texture.h,v 1.4 2000/09/13 12:33:50 nzc Exp $"

/* no types!!! */
/*  #include "texture_types.h" */

/**
 * Takes uv coordinates (R.uv[], O.dxuv, O.dyuv), find texture colour
 * at that spot (using imagewrap()). 
 * Result is kept in R.vcol (float vector 3)
 */
void render_realtime_texture();

/**
 * Do texture mapping for materials. Communicates with R.... variables.
 */
void do_material_tex(void);

/* unsorted */
int blend(Tex *tex, float *texvec);
int clouds(Tex *tex, float *texvec);
int cubemap(MTex *mtex, float x, float y, float z, float *adr1, float *adr2);
int cubemap_glob(MTex *mtex, float x, float y, float z, float *adr1, float *adr2);
int cubemap_ob(MTex *mtex, float x, float y, float z, float *adr1, float *adr2);
void do_2d_mapping(MTex *mtex, float *t, float *dxt, float *dyt);
void do_halo_tex(HaloRen *har, float xn, float yn, float *colf);
void do_lamp_tex(LampRen *la, float *lavec);
void do_sky_tex(void);
int magic(Tex *tex, float *texvec);
int marble(Tex *tex, float *texvec);
int multitex(Tex *tex, float *texvec, float *dxt, float *dyt);
int plugintex(Tex *tex, float *texvec, float *dxt, float *dyt);
int stucci(Tex *tex, float *texvec);
int texnoise(Tex *tex);
int wood(Tex *tex, float *texvec);

#endif /* TEXTURE_EXT_H */











