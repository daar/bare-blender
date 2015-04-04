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

/** image_ext.h

 *
 * Version: $Id: image_ext.h,v 1.2 2000/07/20 19:14:58 ton Exp $
 */

#ifndef IMAGE_EXT_H
#define IMAGE_EXT_H "$Id: image_ext.h,v 1.2 2000/07/20 19:14:58 ton Exp $"

/* #include "blender.h" */
#include "render_types.h"

void free_image_buffers(Image *ima);
void free_unused_animimages(void);

#endif /* IMAGE_EXT_H */

