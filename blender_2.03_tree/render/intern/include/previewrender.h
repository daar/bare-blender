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

 * previewrender_ext.h
 *
 * Version: $Id: previewrender.h,v 1.3 2000/08/28 12:23:44 nzc Exp $
 */

#ifndef PREVIEWRENDER_EXT_H
#define PREVIEWRENDER_EXT_H "$Id: previewrender.h,v 1.3 2000/08/28 12:23:44 nzc Exp $"

/* no types!!! */
/*  #include "previewrender_types.h" */

void display_pr_scanline(uint *rect, int recty);
void draw_tex_crop(Tex *tex);
void halo_preview_pixel(HaloRen *har, int startx, int endx, int y, char *rect);
void init_previewhalo(HaloRen *har, Material *mat);
void lamp_preview_pixel(LampRen *la, int x, int y, char *rect);
void previewdraw(void);
void previewflare(HaloRen *har, uint *rect);
void set_previewrect(int xmin, int ymin, int xmax, int ymax);
void shade_preview_pixel(float *vec, int x, int y, char *rect, int smooth);
void sky_preview_pixel(float lens, int x, int y, char *rect);
void texture_preview_pixel(Tex *tex, int x, int y, char *rect);

#endif /* PREVIEWRENDER_EXT_H */

