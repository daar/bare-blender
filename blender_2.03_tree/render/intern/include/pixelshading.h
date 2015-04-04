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

 * pixelshading.h
 *
 * These functions determine what actual colour a pixel will have.
 *
 * Version: $Id: pixelshading.h,v 1.1 2000/09/14 09:11:40 nzc Exp $
 */

#ifndef PIXELSHADING_H
#define PIXELSHADING_H "$Id: pixelshading.h,v 1.1 2000/09/14 09:11:40 nzc Exp $"

/* ------------------------------------------------------------------------- */

#include "render.h"

/* ------------------------------------------------------------------------- */

/**
 * Render the pixel at (x,y) for object ap. Apply the jitter mask. 
 * Output is given in shortcol. The type vector:
 * t[0] - min. distance
 * t[1] - face/halo index
 * t[2] - jitter mask                     
 * t[3] - type ZB_POLY or ZB_HALO
 * t[4] - max. distance
 * @return pointer to the object
 */
void *renderPixel(float x, float y, int *t);

/**
 * Spothalos on otherwise empty pixels.
 */
void renderSpotHaloPixel(float x, float y, float* colbuf);

/* ------------------------------------------------------------------------- */
/* All these are supposed to be internal. I should move these to a separate  */
/* header.                                                                   */

/**
 * Determine colour for pixel at SCS x,y for face <vlaknr>. Result end up in
 * <collector>
 * @return pointer to this object's VlakRen
 */
void *renderFacePixel(float x, float y, int vlaknr);

/**
 * Render this pixel for halo haloNr. Leave result in <collector>.
 * @return pointer to this object's HaloRen
 */
void *renderHaloPixel(float x, float y, int haloNr);

/**
 * Shade the halo at the given location
 */
void shadeHaloFloat(HaloRen *har, float *col, uint zz, 
					float dist, float xn, float yn, short flarec);

void shadeSpotHaloPixelFloat(float *col);
void spotHaloFloat(struct LampRen *lar, float *view, float *intens);
void shadeLampLusFloat(void);

/* this should be replaced by shadeSpotHaloPixelFloat(), but there's         */
/* something completely fucked up here with the arith.                       */
void renderspothaloFix(ushort *col);

/* ------------------------------------------------------------------------- */

#endif

