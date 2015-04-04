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




/* effect.h 
 * 
 * dec 95
 * jan feb 96
 * 
 * Version: $Id: effect.h,v 1.4 2000/08/25 15:27:33 nzc Exp $
 */

#ifndef EFFECT_H
#define EFFECT_H

/* DENK ERAAN: NIEUWE EFFECTEN OOK IN DE WRITEFILE.C IVM DNA!!! */
#include "effect_types.h"

	/* effect.c */
extern Effect *add_effect(int type);
extern PartEff *give_parteff(Object *ob);
extern void where_is_particle(PartEff *paf, Particle *pa, float ctime, float *vec);
extern void free_effect(Effect *eff);
extern void free_effects(ListBase *lb);
extern void copy_effects(ListBase *lbn, ListBase *lb);
extern void build_particle_system(Object *ob);
/* used externally */
void set_buildvars(Object *ob, int *start, int *end);


#endif

