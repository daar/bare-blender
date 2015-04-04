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

 * shadbuf_ext.h
 *
 * Version: $Id: shadbuf.h,v 1.3 2000/09/18 09:05:08 nzc Exp $
 */

#ifndef SHADBUF_EXT_H
#define SHADBUF_EXT_H "$Id: shadbuf.h,v 1.3 2000/09/18 09:05:08 nzc Exp $"

/* no types!!! */
/*  #include "shadbuf_types.h" */

void initshadowbuf(LampRen *lar, Lamp *la, float mat[][4]);

/**
 * Calculates shadowbuffers for a vector of shadow-giving lamps
 * @param lar The vector of lamps
 */
void makeshadowbuf(LampRen *lar);

float testshadowbuf(struct ShadBuf *shb, float inp);	

/**
 * Determines the shadow factor for lamp <lar>, between <p1>
 * and <p2>. (Which CS?)
 */
float shadow_halo(LampRen *lar, float *p1, float *p2);

#endif /* SHADBUF_EXT_H */

