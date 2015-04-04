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

 * matrixops.h
 *
 * Version: $Id: matrixops.h,v 1.3 2000/09/21 13:46:22 nzc Exp $
 */

#ifndef MATRIXOPS_H
#define MATRIXOPS_H "$Id: matrixops.h,v 1.3 2000/09/21 13:46:22 nzc Exp $"

/* ------------------------------------------------------------------------- */
/* need rewriting: */
/**
 * copy the left upp3 3 by 3 of m2 to m1
 */
void MTC_Mat3CpyMat4(float m1[][3], float m2[][4]);

/* ------------------------------------------------------------------------- */
/* operations based on 4 by 4 matrices */

/**
 * Copy m1 to m2
 */
void MTC_Mat4CpyMat4(float m1[][4], float m2[][4]);

/**
 * Multiply all matrices after the first, leave the result in the
 * first argument
 */
void MTC_Mat4MulSerie(float answ[][4],
					  float m1[][4], float m2[][4], float m3[][4],
					  float m4[][4], float m5[][4], float m6[][4],
					  float m7[][4], float m8[][4]);

/**
 * m1 = m2 matprod m3
 */
void MTC_Mat4MulMat4(float m1[][4], float m2[][4], float m3[][4]);

/**
 * Do vec^t prod mat, result in vec. Ignore vec[3] (vec is a
 * float[3])
 */
void MTC_Mat4MulVecfl(float mat[][4], float *vec);

/**
 * Invert mat, result in inverse. Always returns 1
 */
int MTC_Mat4Invert(float inverse[][4], float mat[][4]);

/**
 * Make the set of mat orthonormal (mat should already be orthogonal)?
 * (doesn't appear to normalise properly?)
 */
void MTC_Mat4Ortho(float mat[][4]);

/**
 * vec = vec prod mat, result in vec, ignore fourth component entirely
 */
void MTC_Mat4Mul3Vecfl(float mat[][4], float *vec);

/**
 * Set <m> to the 4-D unity matrix
 */
void MTC_Mat4One(float m[][4]);
	
/* ------------------------------------------------------------------------- */
/* Operations based on 3 by 3 matrices */
/**
 * Do vec^t prod mat, result in vec.(vex is 3d)
 */
void MTC_Mat3MulVecfl(float mat[][3], float *vec);

/**
 * Copy m1 to m2
 */
void MTC_Mat3CpyMat3(float m1[][3], float m2[][3]);

/**
 * m1 = m2 prod m3
 */
void MTC_Mat3MulMat3(float m1[][3], float m3[][3], float m2[][3]);

/**
 * vec = vec prod mat
 */
void MTC_Mat3MulVecd(float mat[][3], double *vec);

/**
 * Guess: invert matrix
 * result goes to m1
 */
void MTC_Mat3Inv(float m1[][3], float m2[][3]);

/**
 * Sort of a determinant matrix? Doesn't seem very adjoint to me...
 * result goes to m1
 */
void MTC_Mat3Adj(float m1[][3], float m[][3]);

/**
 * Set <m> to the 3D unity matrix
 */
void MTC_Mat3One(float m[][3]);

/* ------------------------------------------------------------------------- */

#endif /* MATRIXOPS_H */

