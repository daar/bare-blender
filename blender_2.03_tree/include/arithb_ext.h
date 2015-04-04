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

 * arithb_ext.h
 *
 * This c lib needs to be replaced by Geno and Erwin's c++ lib ...
 *
 * Version: $Id: arithb_ext.h,v 1.7 2000/07/20 19:14:57 ton Exp $
 */

#ifndef ARITHB_EXT_H
#define ARITHB_EXT_H "$Id: arithb_ext.h,v 1.7 2000/07/20 19:14:57 ton Exp $"

#ifdef __WIN32
extern void srand48(uint );
extern float drand48(void);
#endif


/* matrix operations */
/* void Mat4MulMat4(float m1[][4], float m2[][4], float m3[][4]); */
/* void Mat3MulVecfl(float mat[][3], float *vec);  */
/* or **mat, but it's the same */
/*void Mat3MulVecd(float mat[][3], double *vec); */

/* void Mat4MulVecfl(float mat[][4], float *vec); */
/* void Mat4MulSerie(float answ[][4], float m1[][4], float m2[][4],  */
/*                   float m3[][4], float m4[][4], float m5[][4],  */
/*                   float m6[][4], float m7[][4], float m8[][4]); */
/* int Mat4Invert(float inverse[][4], float mat[][4]); */

/* m2 to m1 */
/*  void Mat3CpyMat4(float m1p[][3], float m2p[][4]); */
/* void Mat3CpyMat4(float *m1p, float *m2p); */

/* m1 to m2 */
/*  void Mat3CpyMat3(float m1p[][3], float m2p[][3]); */
/* void Mat3CpyMat3(float *m1p, float *m2p); */

/* m2 to m1 */
/* void Mat4CpyMat3(float m1p[][4], float m2p[][3]); */

/* M1 = M3*M2 */
/*  void Mat3MulMat3(float m1[][3], float m2[][3], float m3[][3]); */
/*void Mat3MulMat3(float *m1, float *m3, float *m2); */

/* m1 = m2 * m3, ignore the elements on the 4th row/column of m3 */
/*void Mat3IsMat3MulMat4(float m1[][3], float m2[][3], float m3[][4]); */

/* m1 to m2 */
/*  void Mat4CpyMat4(float m1[][4], float m2[][4]); */
/* void Mat4CpyMat4(float *m1, float *m2); */


/* void Mat4Ortho(float mat[][4]); */
/* void Mat4Mul3Vecfl(float mat[][4], float *vec); */
/* void Mat4MulVec4fl(float mat[][4], float *vec); */
/* void Mat4SwapMat4(float *m1, float *m2); */

/* void Mat3Inv(float m1[][3], float m2[][3]); */
/* void Mat4One(float m[][4]); */
/* void Mat3One(float m[][3]); */


extern void CalcCent3f(float *cent, float *v1, float *v2, float *v3);
extern void CalcCent4f(float *cent, float *v1, float *v2, float *v3, float *v4);
extern void Crossf(float *c, float *a, float *b);


extern void EulToMat3(float *eul, float mat[][3]);
void EulToMat4(float* eul, float mat[][4]);


extern void QuatToEul(float *quat, float *eul);
extern void QuatOne(float *);
extern void QuatMul(float *, float *, float *);
extern void NormalQuat(float *);
extern void VecRotToQuat(float *vec, float phi, float *quat);
extern void QuatSub(float *q, float *q1, float *q2);


/* extern void Mat3Inv(float m1[][3], float m2[][3]); */
/* extern int Mat4Invert(float inverse[][4], float mat[][4]); */
/* extern void Mat3CpyMat4(float *m1, float *m2); */
extern void Mat3ToEul(float tmat[][3], float *eul);
/* extern void Mat3MulMat3(float *m1, float *m3, float *m2); */
/* extern void Mat3MulVecfl(float mat[][3], float *vec); */
/* extern void Mat4MulVecfl(float mat[][4], float *vec); */
extern void Mat3MulFloat(float *m, float f);
extern void Mat4MulFloat(float *m, float f);
extern void Mat4MulFloat3(float *m, float f);
extern int FloatCompare(float *v1, float *v2, float limit);
extern float Normalise(float *n);
extern float CalcNormFloat(float *v1,float *v2,float *v3,float *n);
extern float CalcNormFloat4(float *v1,float *v2,float *v3,float *v4,float *n);
extern float VecLenf(float *v1, float *v2);
extern void VecMulf(float *v1, float f);
extern float Sqrt3f(float f);
extern double Sqrt3d(double d);
extern void euler_rot(float *beul, float ang, char axis);
extern    float safacos(float fac);
extern    float safsqrt(float fac);
extern float Inpf(float *v1, float *v2);
extern void VecSubf(float *v, float *v1, float *v2);
extern void VecAddf(float *v, float *v1, float *v2);
extern void VecUpMat3(float *vec, float mat[][3], short axis);

extern float DistVL2Dfl(float *v1,float *v2,float *v3);
extern float PdistVL2Dfl(float *v1,float *v2,float *v3);
extern float AreaF2Dfl(float *v1, float *v2, float *v3);
extern float AreaQ3Dfl(float *v1, float *v2, float *v3, float *v4);
extern float AreaT3Dfl(float *v1, float *v2, float *v3);
extern float AreaPoly3Dfl(int nr, float *verts, float *normal);
extern void VecRotToMat3(float *vec, float phi, float mat[][3]);
extern float Spec(float inp, int hard);
extern float *vectoquat(float *vec, short axis, short upflag);

extern void i_lookat(float vx, float vy, float vz, float px, float py, float pz, float twist, float mat[][4]);
extern void i_window(float left, float right, float bottom, float top, float near, float far, float mat[][4]);

extern void hsv_to_rgb(float h, float s, float v, float *r, float *g, float *b);
extern void rgb_to_hsv(float r, float g, float b, float *lh, float *ls, float *lv);
extern uint hsv_to_cpack(float h, float s, float v);
extern uint rgb_to_cpack(float r, float g, float b);
extern void cpack_to_rgb(uint col, float *r, float *g, float *b);


#endif /* ARITHB_EXT_H */

