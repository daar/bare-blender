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

 *
 * Some vector operations.
 *
 * Always use
 * - vector with x components :   float x[3], int x[3], etc
 *
 * Version: $Id: vectorops.c,v 1.2 2000/09/14 09:11:41 nzc Exp $
 */

/* ------------------------------------------------------------------------- */
/* General format: op(a, b, c): a = b op c                                   */
/* Copying is done cp <from, to>                                             */
/* ------------------------------------------------------------------------- */

#include "vectorops.h"

void MTC_diff3Int(int v1[3], int v2[3], int v3[3])
{
	v1[0] = v2[0] - v3[0];
	v1[1] = v2[1] - v3[1];
	v1[2] = v2[2] - v3[2];
}

/* ------------------------------------------------------------------------- */
void MTC_diff3Float(float v1[3], float v2[3], float v3[3])
{
	v1[0] = v2[0] - v3[0];
	v1[1] = v2[1] - v3[1];
	v1[2] = v2[2] - v3[2];
}

/* ------------------------------------------------------------------------- */

void MTC_cross3Int(int v1[3], int v2[3], int v3[3])
{
	v1[0] = v2[1]*v3[2] - v2[2]*v3[1];
	v1[1] = v2[2]*v3[0] - v2[0]*v3[2];
	v1[2] = v2[0]*v3[1] - v2[1]*v3[0];
}

/* ------------------------------------------------------------------------- */

void MTC_cross3Float(float v1[3], float v2[3], float v3[3])
{
	v1[0] = v2[1]*v3[2] - v2[2]*v3[1];
	v1[1] = v2[2]*v3[0] - v2[0]*v3[2];
	v1[2] = v2[0]*v3[1] - v2[1]*v3[0];
}
/* ------------------------------------------------------------------------- */

void MTC_cross3Double(double v1[3], double v2[3], double v3[3])
{
	v1[0] = v2[1]*v3[2] - v2[2]*v3[1];
	v1[1] = v2[2]*v3[0] - v2[0]*v3[2];
	v1[2] = v2[0]*v3[1] - v2[1]*v3[0];
}

/* ------------------------------------------------------------------------- */

int MTC_dot3Int(int v1[3], int v2[3])
{
	return (v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]);
}

/* ------------------------------------------------------------------------- */

float MTC_dot3Float(float v1[3], float v2[3])
{
	return (v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]);
}

/* ------------------------------------------------------------------------- */

void MTC_cp3Float(float v1[3], float v2[3])
{
	v2[0] = v1[0];
	v2[1] = v1[1];
	v2[2] = v1[2];
}

/* ------------------------------------------------------------------------- */

void MTC_cp3FloatInv(float v1[3], float v2[3])
{
	v2[0] = -v1[0];
	v2[1] = -v1[1];
	v2[2] = -v1[2];
}

/* ------------------------------------------------------------------------- */

void MTC_swapInt(int *i1, int *i2)
{
	int swap;
	swap = *i1;
	*i1 = *i2;
	*i2 = swap;
}

/* ------------------------------------------------------------------------- */

void  MTC_diff3DFF(double v1[3], float v2[3], float v3[3])
{
	v1[0] = v2[0] - v3[0];
	v1[1] = v2[1] - v3[1];
	v1[2] = v2[2] - v3[2];
}

/* ------------------------------------------------------------------------- */

/* eof */

