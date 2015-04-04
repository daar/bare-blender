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

/**

 * These are functions from imbuf that are called externally. There
 * is as of yet no way of doing this neatly.... This header is for 
 * use with the imbuf/ module.
 *
 * imbuf_extern.h
 *
 * Version: $Id: imbuf_extern.h,v 1.1 2000/07/20 15:41:40 nzc Exp $
 */

#ifndef IMBUF_EXTERN_H
#define IMBUF_EXTERN_H "$Id: imbuf_extern.h,v 1.1 2000/07/20 15:41:40 nzc Exp $"

#include "blender.h"

/* rectop.c */
void rectxor(int* d, int * s, int x);
void rectmakepremul(uchar *drect, uchar *srect, uchar *x);
void rectop(struct ImBuf *dbuf, struct ImBuf *sbuf,
            int destx, int desty, int srcx, int srcy, int width,
            int height, 
            void (*operation)(uint *, uint*, int, int), 
            int value);
void rectcpy(uint *drect, uint *srect, int x, int dummy);

/* dither.c */
/**
 * Set the pointer to the function that performs the dithering.
 */
void setdither(void (*func)(struct ImBuf *, short, short));

#endif /* IMBUF_EXTERN_H */

