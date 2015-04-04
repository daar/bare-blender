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

 * zbuf_types.h
 * type definitions used (and maybe exported) by zbuf.c.
 *
 * Version: $Id: zbuf_types.h,v 1.1 2000/08/24 16:58:57 nzc Exp $
 */

#ifndef ZBUF_TYPES_H
#define ZBUF_TYPES_H "$Id: zbuf_types.h,v 1.1 2000/08/24 16:58:57 nzc Exp $"

#define ABUFPART 64

/**
 * Primitive data structure for zbuffering. One struct 
 * stores data for 4 entries.
 */
typedef struct APixstr {
    ushort mask[4]; /* jitter mask */
    int z[4];       /* distance    */
    int p[4];       /* index       */
    struct APixstr *next;
} APixstr;


typedef struct APixstrMain
{
	struct APixstr *ps;
	struct APixstrMain *next;
} APixstrMain;


typedef struct {
	float *vert;
	float hoco[4];
	int clip;
} VertBucket;

#endif /* ZBUF_TYPES_H */





