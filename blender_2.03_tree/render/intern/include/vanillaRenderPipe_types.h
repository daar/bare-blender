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

 * vanillaRenderPipe_types.h
 *
 * Version: $Id: vanillaRenderPipe_types.h,v 1.1 2000/08/24 16:58:57 nzc Exp $
 */

#ifndef VANILLARENDERPIPE_TYPES_H
#define VANILLARENDERPIPE_TYPES_H "$Id: vanillaRenderPipe_types.h,v 1.1 2000/08/24 16:58:57 nzc Exp $"

#include "blender.h"


/* Render defines */
#define  RE_MAX_OSA_COUNT 16 /* The max. number of possible oversamples     */
#define  RE_MAX_FACES_PER_PIXEL 500 /* max. nr of faces rendered behind one */
                             /* pixel                                       */
/* Render typedefs */
typedef float RE_COLBUFTYPE; /* datatype for the colour buffer              */


/**
 * Threshold for add-blending for faces
 */
#define RE_FACE_ADD_THRESHOLD 0.001

/**
 * Data about the rendering process
 */
typedef struct RE_RenderStatistics {
    float cmax[4]; /* max. colours (overall) */
} RE_RenderStatistics;

#endif /* VANILLARENDERPIPE_TYPES_H */



