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
 * Some matrix operations.
 *
 * Always use
 * - vector with x components :   float x[3], int x[3], etc
 *
 * Version: $Id: matrixops.c,v 1.3 2000/09/21 13:46:22 nzc Exp $
 */

/* ------------------------------------------------------------------------- */
#include "blender.h"
#include "matrixops.h"
/* ------------------------------------------------------------------------- */

/* even if the types don't say so, use 4 by 4 matrices here */
/* copy from m1 to m2                                       */
void MTC_Mat4CpyMat4(float m1[][4], float m2[][4])
{
/*      int i = 0; */
/*      while (i<4) { */
/*          m2[i][0] = m1[i][0]; */
/*          m2[i][1] = m1[i][1]; */
/*          m2[i][2] = m1[i][2]; */
/*          m2[i][3] = m1[i][3]; */
/*          i++; */
/*      } */
  	bcopy(m2, m1, 64); 
}

/* ------------------------------------------------------------------------- */

void MTC_Mat4MulSerie(float answ[][4],
				  float m1[][4], float m2[][4], float m3[][4],
				  float m4[][4], float m5[][4], float m6[][4],
				  float m7[][4], float m8[][4])
{
	float temp[4][4];
	
	if(m1==0 || m2==0) return;
	
	MTC_Mat4MulMat4(answ, m2, m1);
	if(m3) {
		MTC_Mat4MulMat4(temp, m3, answ);
		if(m4) {
			MTC_Mat4MulMat4(answ, m4, temp);
			if(m5) {
				MTC_Mat4MulMat4(temp, m5, answ);
				if(m6) {
					MTC_Mat4MulMat4(answ, m6, temp);
					if(m7) {
						MTC_Mat4MulMat4(temp, m7, answ);
						if(m8) {
							MTC_Mat4MulMat4(answ, m8, temp);
						}
						else MTC_Mat4CpyMat4(answ, temp);
					}
				}
				else MTC_Mat4CpyMat4(answ, temp);
			}
		}
		else MTC_Mat4CpyMat4(answ, temp);
	}
}

/* ------------------------------------------------------------------------- */
void MTC_Mat4MulMat4(float m1[][4], float m2[][4], float m3[][4])
{
  /* matrix product: c[j][k] = a[j][i].b[i][k] */

	m1[0][0] = m2[0][0]*m3[0][0] + m2[0][1]*m3[1][0] + m2[0][2]*m3[2][0] + m2[0][3]*m3[3][0];
	m1[0][1] = m2[0][0]*m3[0][1] + m2[0][1]*m3[1][1] + m2[0][2]*m3[2][1] + m2[0][3]*m3[3][1];
	m1[0][2] = m2[0][0]*m3[0][2] + m2[0][1]*m3[1][2] + m2[0][2]*m3[2][2] + m2[0][3]*m3[3][2];
	m1[0][3] = m2[0][0]*m3[0][3] + m2[0][1]*m3[1][3] + m2[0][2]*m3[2][3] + m2[0][3]*m3[3][3];

	m1[1][0] = m2[1][0]*m3[0][0] + m2[1][1]*m3[1][0] + m2[1][2]*m3[2][0] + m2[1][3]*m3[3][0];
	m1[1][1] = m2[1][0]*m3[0][1] + m2[1][1]*m3[1][1] + m2[1][2]*m3[2][1] + m2[1][3]*m3[3][1];
	m1[1][2] = m2[1][0]*m3[0][2] + m2[1][1]*m3[1][2] + m2[1][2]*m3[2][2] + m2[1][3]*m3[3][2];
	m1[1][3] = m2[1][0]*m3[0][3] + m2[1][1]*m3[1][3] + m2[1][2]*m3[2][3] + m2[1][3]*m3[3][3];

	m1[2][0] = m2[2][0]*m3[0][0] + m2[2][1]*m3[1][0] + m2[2][2]*m3[2][0] + m2[2][3]*m3[3][0];
	m1[2][1] = m2[2][0]*m3[0][1] + m2[2][1]*m3[1][1] + m2[2][2]*m3[2][1] + m2[2][3]*m3[3][1];
	m1[2][2] = m2[2][0]*m3[0][2] + m2[2][1]*m3[1][2] + m2[2][2]*m3[2][2] + m2[2][3]*m3[3][2];
	m1[2][3] = m2[2][0]*m3[0][3] + m2[2][1]*m3[1][3] + m2[2][2]*m3[2][3] + m2[2][3]*m3[3][3];

	m1[3][0] = m2[3][0]*m3[0][0] + m2[3][1]*m3[1][0] + m2[3][2]*m3[2][0] + m2[3][3]*m3[3][0];
	m1[3][1] = m2[3][0]*m3[0][1] + m2[3][1]*m3[1][1] + m2[3][2]*m3[2][1] + m2[3][3]*m3[3][1];
	m1[3][2] = m2[3][0]*m3[0][2] + m2[3][1]*m3[1][2] + m2[3][2]*m3[2][2] + m2[3][3]*m3[3][2];
	m1[3][3] = m2[3][0]*m3[0][3] + m2[3][1]*m3[1][3] + m2[3][2]*m3[2][3] + m2[3][3]*m3[3][3];

}
/* ------------------------------------------------------------------------- */

void MTC_Mat4MulVecfl(float mat[][4], float *vec)
{
	float x,y;

	x=vec[0]; 
	y=vec[1];
	vec[0]=x*mat[0][0] + y*mat[1][0] + mat[2][0]*vec[2] + mat[3][0];
	vec[1]=x*mat[0][1] + y*mat[1][1] + mat[2][1]*vec[2] + mat[3][1];
	vec[2]=x*mat[0][2] + y*mat[1][2] + mat[2][2]*vec[2] + mat[3][2];
}

/* ------------------------------------------------------------------------- */
void MTC_Mat3MulVecfl(float mat[][3], float *vec)
{
	float x,y;

	x=vec[0]; 
	y=vec[1];
	vec[0]= x*mat[0][0] + y*mat[1][0] + mat[2][0]*vec[2];
	vec[1]= x*mat[0][1] + y*mat[1][1] + mat[2][1]*vec[2];
	vec[2]= x*mat[0][2] + y*mat[1][2] + mat[2][2]*vec[2];
}

/* ------------------------------------------------------------------------- */

int MTC_Mat4Invert(float inverse[][4], float mat[][4])
{
	int i, j, k;
	double temp;
	float tempmat[4][4];
	float max;
	int maxj;

	/* Set inverse to identity */
	for (i=0; i<4; i++)
		for (j=0; j<4; j++)
			inverse[i][j] = 0;
	for (i=0; i<4; i++)
		inverse[i][i] = 1;

	/* Copy original matrix so we don't mess it up */
	for(i = 0; i < 4; i++)
		for(j = 0; j <4; j++)
			tempmat[i][j] = mat[i][j];

	for(i = 0; i < 4; i++) {
		/* Look for row with max pivot */
		max = ABS(tempmat[i][i]);
		maxj = i;
		for(j = i + 1; j < 4; j++) {
			if(ABS(tempmat[j][i]) > max) {
				max = ABS(tempmat[j][i]);
				maxj = j;
			}
		}
		/* Swap rows if necessary */
		if (maxj != i) {
			for( k = 0; k < 4; k++) {
				SWAP(float, tempmat[i][k], tempmat[maxj][k]);
				SWAP(float, inverse[i][k], inverse[maxj][k]);
			}
		}

		temp = tempmat[i][i];
		if (temp == 0)
			return 0;  /* No non-zero pivot */
		for(k = 0; k < 4; k++) {
			tempmat[i][k] /= temp;
			inverse[i][k] /= temp;
		}
		for(j = 0; j < 4; j++) {
			if(j != i) {
				temp = tempmat[j][i];
				for(k = 0; k < 4; k++) {
					tempmat[j][k] -= tempmat[i][k]*temp;
					inverse[j][k] -= inverse[i][k]*temp;
				}
			}
		}
	}
	return 1;
}

/* ------------------------------------------------------------------------- */
void MTC_Mat3CpyMat4(float m1[][3], float m2[][4])
{
	
	m1[0][0]= m2[0][0];
	m1[0][1]= m2[0][1];
	m1[0][2]= m2[0][2];

	m1[1][0]= m2[1][0];
	m1[1][1]= m2[1][1];
	m1[1][2]= m2[1][2];

	m1[2][0]= m2[2][0];
	m1[2][1]= m2[2][1];
	m1[2][2]= m2[2][2];
}

/* ------------------------------------------------------------------------- */

/*  void Mat3CpyMat3(float m1[][3], float m2[][3]) */
void MTC_Mat3CpyMat3(float m1[][3], float m2[][3])
{	
/*      int i = 0; */
/*      while (i < 3) { */
/*          m2[i][0] = m1[i][0]; */
/*          m2[i][1] = m1[i][1]; */
/*          m2[i][2] = m1[i][2]; */
/*      } */
    bcopy(m2, m1, 9*sizeof(float));
}

/* ------------------------------------------------------------------------- */
/*  void Mat3MulMat3(float m1[][3], float m3[][3], float m2[][3]) */
void MTC_Mat3MulMat3(float m1[][3], float m3[][3], float m2[][3])
{
	/* be careful about this rewrite... */
	    /* m1[i][j] = m2[i][k]*m3[k][j], args are flipped! */
	m1[0][0]= m2[0][0]*m3[0][0] + m2[0][1]*m3[1][0] + m2[0][2]*m3[2][0];
	m1[0][1]= m2[0][0]*m3[0][1] + m2[0][1]*m3[1][1] + m2[0][2]*m3[2][1];
	m1[0][2]= m2[0][0]*m3[0][2] + m2[0][1]*m3[1][2] + m2[0][2]*m3[2][2];

	m1[1][0]= m2[1][0]*m3[0][0] + m2[1][1]*m3[1][0] + m2[1][2]*m3[2][0];
	m1[1][1]= m2[1][0]*m3[0][1] + m2[1][1]*m3[1][1] + m2[1][2]*m3[2][1];
	m1[1][2]= m2[1][0]*m3[0][2] + m2[1][1]*m3[1][2] + m2[1][2]*m3[2][2];

	m1[2][0]= m2[2][0]*m3[0][0] + m2[2][1]*m3[1][0] + m2[2][2]*m3[2][0];
	m1[2][1]= m2[2][0]*m3[0][1] + m2[2][1]*m3[1][1] + m2[2][2]*m3[2][1];
	m1[2][2]= m2[2][0]*m3[0][2] + m2[2][1]*m3[1][2] + m2[2][2]*m3[2][2];

/*  	m1[0]= m2[0]*m3[0] + m2[1]*m3[3] + m2[2]*m3[6]; */
/*  	m1[1]= m2[0]*m3[1] + m2[1]*m3[4] + m2[2]*m3[7]; */
/*  	m1[2]= m2[0]*m3[2] + m2[1]*m3[5] + m2[2]*m3[8]; */
/*  	m1+=3; */
/*  	m2+=3; */
/*  	m1[0]= m2[0]*m3[0] + m2[1]*m3[3] + m2[2]*m3[6]; */
/*  	m1[1]= m2[0]*m3[1] + m2[1]*m3[4] + m2[2]*m3[7]; */
/*  	m1[2]= m2[0]*m3[2] + m2[1]*m3[5] + m2[2]*m3[8]; */
/*  	m1+=3; */
/*  	m2+=3; */
/*  	m1[0]= m2[0]*m3[0] + m2[1]*m3[3] + m2[2]*m3[6]; */
/*  	m1[1]= m2[0]*m3[1] + m2[1]*m3[4] + m2[2]*m3[7]; */
/*  	m1[2]= m2[0]*m3[2] + m2[1]*m3[5] + m2[2]*m3[8]; */
} /* end of void Mat3MulMat3(float m1[][3], float m3[][3], float m2[][3]) */

/* ------------------------------------------------------------------------- */

void MTC_Mat4Ortho(float mat[][4])
{
	float len;
	
	len= Normalise(mat[0]);
	if(len!=0.0) mat[0][3]/= len;
	len= Normalise(mat[1]);
	if(len!=0.0) mat[1][3]/= len;
	len= Normalise(mat[2]);
	if(len!=0.0) mat[2][3]/= len;
}

/* ------------------------------------------------------------------------- */

void MTC_Mat4Mul3Vecfl(float mat[][4], float *vec)
{
	float x,y;
	/* vec = mat^T dot vec !!! or vec a row, then vec = vec dot mat*/

	x= vec[0]; 
	y= vec[1];
	vec[0]= x*mat[0][0] + y*mat[1][0] + mat[2][0]*vec[2];
	vec[1]= x*mat[0][1] + y*mat[1][1] + mat[2][1]*vec[2];
	vec[2]= x*mat[0][2] + y*mat[1][2] + mat[2][2]*vec[2];
}

/* ------------------------------------------------------------------------- */

void MTC_Mat4One(float m[][4])
{

	m[0][0]= m[1][1]= m[2][2]= m[3][3]= 1.0;
	m[0][1]= m[0][2]= m[0][3]= 0.0;
	m[1][0]= m[1][2]= m[1][3]= 0.0;
	m[2][0]= m[2][1]= m[2][3]= 0.0;
	m[3][0]= m[3][1]= m[3][2]= 0.0;
}


/* ------------------------------------------------------------------------- */
/* Result is a 3-vector!*/
void MTC_Mat3MulVecd(float mat[][3], double *vec)
{
	double x,y;

	/* vec = mat^T dot vec !!! or vec a row, then vec = vec dot mat*/
	x=vec[0]; 
	y=vec[1];
	vec[0]= x * mat[0][0] + y * mat[1][0] + mat[2][0] * vec[2];
	vec[1]= x * mat[0][1] + y * mat[1][1] + mat[2][1] * vec[2];
	vec[2]= x * mat[0][2] + y * mat[1][2] + mat[2][2] * vec[2];
}

/* ------------------------------------------------------------------------- */

void MTC_Mat3Inv(float m1[][3], float m2[][3])
{
	short a,b;
	float det;

	/* eerst adjoint */
	MTC_Mat3Adj(m1,m2);

	/* dan det oude mat! */
	det= m2[0][0]* (m2[1][1]*m2[2][2] - m2[1][2]*m2[2][1])
	    -m2[1][0]* (m2[0][1]*m2[2][2] - m2[0][2]*m2[2][1])
	    +m2[2][0]* (m2[0][1]*m2[1][2] - m2[0][2]*m2[1][1]);

	if(det==0) det=1;
	det= 1/det;
	for(a=0;a<3;a++) {
		for(b=0;b<3;b++) {
			m1[a][b]*=det;
		}
	}
}

/* ------------------------------------------------------------------------- */

void MTC_Mat3Adj(float m1[][3], float m[][3])
{
	m1[0][0]=m[1][1]*m[2][2]-m[1][2]*m[2][1];
	m1[0][1]= -m[0][1]*m[2][2]+m[0][2]*m[2][1];
	m1[0][2]=m[0][1]*m[1][2]-m[0][2]*m[1][1];

	m1[1][0]= -m[1][0]*m[2][2]+m[1][2]*m[2][0];
	m1[1][1]=m[0][0]*m[2][2]-m[0][2]*m[2][0];
	m1[1][2]= -m[0][0]*m[1][2]+m[0][2]*m[1][0];

	m1[2][0]=m[1][0]*m[2][1]-m[1][1]*m[2][0];
	m1[2][1]= -m[0][0]*m[2][1]+m[0][1]*m[2][0];
	m1[2][2]=m[0][0]*m[1][1]-m[0][1]*m[1][0];
}

/* ------------------------------------------------------------------------- */
void MTC_Mat3One(float m[][3])
{

	m[0][0]= m[1][1]= m[2][2]= 1.0;
	m[0][1]= m[0][2]= 0.0;
	m[1][0]= m[1][2]= 0.0;
	m[2][0]= m[2][1]= 0.0;
}

/* eof */

