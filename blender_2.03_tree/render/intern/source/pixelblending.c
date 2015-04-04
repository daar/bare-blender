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

 * pixelblending.c
 *
 * Functions to blend pixels with or without alpha, in various formats
 * nzc - June 2000
 *
 * Version: $Id: pixelblending.c,v 1.3 2000/09/11 13:13:00 nzc Exp $
 */

/* global includes */
#include "blender.h"
#include "render.h"

/* local includes */
#include "vanillaRenderPipe_types.h"

/* own includes */
#include "pixelblending_types.h"
#include "pixelblending.h"

/* externals */

/* ------------------------------------------------------------------------- */
/* Debug/behaviour defines                                                   */
/* if defined: alpha blending with floats clips colour, as with shorts       */
/* #define RE_FLOAT_COLOUR_CLIPPING  */
/* if defined: alpha values are clipped                                      */
/* For now, we just keep alpha clipping. We run into thresholding and        */
/* blending difficulties otherwise. Be careful here.                         */
#define RE_ALPHA_CLIPPING

/* One of these three MUST be set.                                           */
/* if defined: do 'expensive' gamma correction, ie. without lookup table.    */
/*  #define RE_PRIMITIVE_GAMMA_CORRECTION  */
/* if defined: lookup-table and interpolation gamma correction.              */
#define RE_INTERPOLATED_GAMMA_CORRECTION
/* if defined: no gamma correction.                                          */
/*  #define RE_NO_GAMMA_CORRECTION  */


/* These indicate the status of the gamma lookup table --------------------- */

static float gamma_range_table[RE_GAMMA_TABLE_SIZE + 1];
static float gamfactor_table[RE_GAMMA_TABLE_SIZE];
static float inv_gamma_range_table[RE_GAMMA_TABLE_SIZE + 1];
static float inv_gamfactor_table[RE_GAMMA_TABLE_SIZE];
static float colour_domain_table[RE_GAMMA_TABLE_SIZE + 1];
static float colour_step;
static float inv_colour_step;
static float valid_gamma;
static float valid_inv_gamma;
static int   gamma_table_initialised = 0;

/* functions --------------------------------------------------------------- */

void addAddSampColF(float *sampvec, float *source, int mask, int osaNr, 
                    uchar addfac)
{
	int a;
	
	for(a=0; a < osaNr; a++) {
		if(mask & (1<<a)) addalphaAddfacFloat(sampvec, source, addfac);
		sampvec+= 4;
	}
} /* end of void addAddSampColF(float, float, int, int) */

/* ------------------------------------------------------------------------- */

void addOverSampColF(float *sampvec, float *source, int mask, int osaNr)
{
	int a;
	
	for(a=0; a < osaNr; a++) {
		if(mask & (1<<a)) addAlphaOverFloat(sampvec, source);
		sampvec+= 4;
	}
} /* end of void addOverSampColF(float, float, int, int) */

/* ------------------------------------------------------------------------- */

int addUnderSampColF(float *sampvec, float *source, int mask, int osaNr)
{
	int a, retval = osaNr;
	
	for(a=0; a < osaNr; a++) {
		if(mask & (1<<a)) addAlphaUnderFloat(sampvec, source);
		if(sampvec[3] > RE_FULL_COLOUR_FLOAT) retval--;
		sampvec+= 4;
	}
	return retval;
} /* end of int addToSampColF(float, float, int, int) */

/* ------------------------------------------------------------------------- */

int addToSampCol(ushort *sampcol, ushort *shortcol, int mask, int osaNr)
{
	int a, retval = osaNr;
	
	for(a=0; a < osaNr; a++) {
		if(mask & (1<<a)) addAlphaUnderShort(sampcol, shortcol);
		if(sampcol[3]>0xFFF0) retval--;
		sampcol+= 4;
	}
	return retval;
} /* end of int addToSampCol(ushort, uhost, int, int) */

/* ------------------------------------------------------------------------- */

int addtosampcol(ushort *sampcol, ushort *shortcol, int mask)
{
	int a, retval = R.osa;
	
	for(a=0; a < R.osa; a++) {
		if(mask & (1<<a)) addAlphaUnderShort(sampcol, shortcol);
		if(sampcol[3]>0xFFF0) retval--;
		sampcol+= 4;
	}
	return retval;
} /* end of int addtosampcol(ushort *sampcol, ushort *shortcol, int mask) */

/* ------------------------------------------------------------------------- */

void addAlphaOverShort(ushort *doel, ushort *bron)   /* vult bron over doel in met alpha van bron */
{
	uint c;
	uint mul;

	if( doel[3]==0 || bron[3]>=0xFFF0) {	/* is getest, scheelt veel */
		*((uint *)doel)= *((uint *)bron);
		*((uint *)(doel+2))= *((uint *)(bron+2));
		return;
	}

	mul= 0xFFFF-bron[3];

	c= ((mul*doel[0])>>16)+bron[0];
	if(c>=0xFFF0) doel[0]=0xFFF0; 
	else doel[0]= c;
	c= ((mul*doel[1])>>16)+bron[1];
	if(c>=0xFFF0) doel[1]=0xFFF0; 
	else doel[1]= c;
	c= ((mul*doel[2])>>16)+bron[2];
	if(c>=0xFFF0) doel[2]=0xFFF0; 
	else doel[2]= c;
	c= ((mul*doel[3])>>16)+bron[3];
	if(c>=0xFFF0) doel[3]=0xFFF0; 
	else doel[3]= c;

} /* end of void addAlphaOverShort(ushort *doel, ushort *bron) */

/* ------------------------------------------------------------------------- */

void addAlphaUnderShort(ushort *doel, ushort *bron)   /* vult bron onder doel in met alpha van doel */
{
	register uint c;
	register uint mul;

	if(doel[3]>=0xFFF0) return;
	if( doel[3]==0 ) {	/* is getest, scheelt veel */
		*((uint *)doel)= *((uint *)bron);
		*((uint *)(doel+2))= *((uint *)(bron+2));
		return;
	}

	mul= 0xFFFF-doel[3];

	c= ((mul*bron[0])>>16)+doel[0];
	if(c>=0xFFF0) doel[0]=0xFFF0; 
	else doel[0]= c;
	c= ((mul*bron[1])>>16)+doel[1];
	if(c>=0xFFF0) doel[1]=0xFFF0; 
	else doel[1]= c;
	c= ((mul*bron[2])>>16)+doel[2];
	if(c>=0xFFF0) doel[2]=0xFFF0;
	else doel[2]= c;
	c= ((mul*bron[3])>>16)+doel[3];
	if(c>=0xFFF0) doel[3]=0xFFF0;
	else doel[3]= c;

} /* end of void addAlphaUnderShort(ushort *doel, ushort *bron) */
  
/* ------------------------------------------------------------------------- */

void addAlphaOverFloat(float *dest, float *source)
{
    /* d = s + (1-alpha_s)d*/
    float c;
    float mul;
    
    /* I may want to disable this clipping */
#ifdef RE_FLOAT_COLOUR_CLIPPING
    if( /*  (-RE_FULL_COLOUR_FLOAT < source[3]) */
/*          && */ (source[3] >  RE_FULL_COLOUR_FLOAT) ) {	/* is getest, scheelt veel */
        dest[0] = source[0];
        dest[1] = source[1];
        dest[2] = source[2];
        dest[3] = source[3];
        return;
    }
#endif

	mul= 1.0 - source[3];

	c= (mul*dest[0]) + source[0];
#ifdef RE_FLOAT_COLOUR_CLIPPING
	if(c >= RE_FULL_COLOUR_FLOAT) dest[0] = RE_UNITY_COLOUR_FLOAT; 
	else 
#endif
       dest[0]= c;
   
	c= (mul*dest[1]) + source[1];
#ifdef RE_FLOAT_COLOUR_CLIPPING
	if(c >= RE_FULL_COLOUR_FLOAT) dest[1] = RE_UNITY_COLOUR_FLOAT; 
	else 
#endif
       dest[1]= c;

	c= (mul*dest[2]) + source[2];
#ifdef RE_FLOAT_COLOUR_CLIPPING
	if(c >= RE_FULL_COLOUR_FLOAT) dest[2] = RE_UNITY_COLOUR_FLOAT; 
	else 
#endif
       dest[2]= c;

	c= (mul*dest[3]) + source[3];
#ifdef RE_ALPHA_CLIPPING
	if(c >= RE_FULL_COLOUR_FLOAT) dest[3] = RE_UNITY_COLOUR_FLOAT; 
	else 
#endif
       dest[3]= c;

} /* end of void addAlphaOverFloat(float *doel, float *bron) */



/* ------------------------------------------------------------------------- */

void addAlphaUnderFloat(float *dest, float *source)
{
    float c;
    float mul;
    
    /* I may want to disable this clipping */
#ifdef RE_FLOAT_COLOUR_CLIPPING
    if( dest[3] >= RE_FULL_COLOUR_FLOAT) return;
#endif
    if( (-RE_EMPTY_COLOUR_FLOAT < dest[3])
        && (dest[3] <  RE_EMPTY_COLOUR_FLOAT) ) {	/* is getest, scheelt veel */
        dest[0] = source[0];
        dest[1] = source[1];
        dest[2] = source[2];
        dest[3] = source[3];
        return;
    }

	mul= 1.0 - dest[3];

	c= (mul*source[0]) + dest[0];
#ifdef RE_FLOAT_COLOUR_CLIPPING
	if(c >= RE_FULL_COLOUR_FLOAT) dest[0] = RE_UNITY_COLOUR_FLOAT; 
	else 
#endif
       dest[0]= c;
   
	c= (mul*source[1]) + dest[1];
#ifdef RE_FLOAT_COLOUR_CLIPPING
	if(c >= RE_FULL_COLOUR_FLOAT) dest[1] = RE_UNITY_COLOUR_FLOAT; 
	else 
#endif
       dest[1]= c;

	c= (mul*source[2]) + dest[2];
#ifdef RE_FLOAT_COLOUR_CLIPPING
	if(c >= RE_FULL_COLOUR_FLOAT) dest[2] = RE_UNITY_COLOUR_FLOAT; 
	else 
#endif
       dest[2]= c;

	c= (mul*source[3]) + dest[3];
#ifdef RE_ALPHA_CLIPPING
	if(c >= RE_FULL_COLOUR_FLOAT) dest[3] = RE_UNITY_COLOUR_FLOAT; 
	else 
#endif
       dest[3]= c;

} /* end of void addAlphaUnderFloat(float *doel, float *bron) */

/* ------------------------------------------------------------------------- */

void cpShortColV2CharColV(ushort *source, uchar *dest)
{
    dest[0] = source[0]>>8;
    dest[1] = source[1]>>8;
    dest[2] = source[2]>>8;
    dest[3] = source[3]>>8;
} /* end of void cpShortColV2CharColV(ushort *source, uchar *dest) */
/* ------------------------------------------------------------------------- */

void cpCharColV2ShortColV(uchar *source, ushort *dest)
{
    dest[0] = source[0]<<8;
    dest[1] = source[1]<<8;
    dest[2] = source[2]<<8;
    dest[3] = source[3]<<8;
} /* end of void cpShortColV2CharColV(uchar *source, ushort *dest) */

/* ------------------------------------------------------------------------- */

void cpIntColV2CharColV(uint *source, uchar *dest)
{
    dest[0] = source[0]>>24;
    dest[1] = source[1]>>24;
    dest[2] = source[2]>>24;
    dest[3] = source[3]>>24;
} /* end of void cpIntColV2CharColV(uint *source, uchar *dest) */

/* ------------------------------------------------------------------------- */

void cpCharColV2FloatColV(uchar *source, float *dest)
{
    dest[0] = source[0]/255.0;  
    dest[1] = source[1]/255.0;  
    dest[2] = source[2]/255.0;
    dest[3] = source[3]/255.0;
} /* end of void cpCharColV2FloatColV(char *source, float *dest) */
/* ------------------------------------------------------------------------- */

void cpShortColV2FloatColV(ushort *source, float *dest)
{
    dest[0] = source[0]/65535.0;  
    dest[1] = source[1]/65535.0;  
    dest[2] = source[2]/65535.0;
    dest[3] = source[3]/65535.0;
} /* end of void cpShortColV2FloatColV(char *source, float *dest) */

/* ------------------------------------------------------------------------- */

void cpFloatColV2CharColV(float* source, uchar *dest)
{
  /* can't this be done more efficient? hope the conversions are correct... */
  if (source[0] < 0.0)      dest[0] = 0;
  else if (source[0] > 1.0) dest[0] = 255;
  else dest[0] = (uchar) (source[0] * 255.0);

  if (source[1] < 0.0)      dest[1] = 0;
  else if (source[1] > 1.0) dest[1] = 255;
  else dest[1] = (uchar) (source[1] * 255.0);

  if (source[2] < 0.0)      dest[2] = 0;
  else if (source[2] > 1.0) dest[2] = 255;
  else dest[2] = (uchar) (source[2] * 255.0);

  if (source[3] < 0.0)      dest[3] = 0;
  else if (source[3] > 1.0) dest[3] = 255;
  else dest[3] = (uchar) (source[3] * 255.0);

} /* end of void cpFloatColV2CharColV(float* source, uchar *dest) */

/* ------------------------------------------------------------------------- */

void cpShortColV(ushort *source, ushort *dest)
{
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
    dest[3] = source[3];
} /* end of void cpShortColV(ushort *source, ushort *dest) */

/* ------------------------------------------------------------------------- */
void cpFloatColV(float *source, float *dest)
{
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
    dest[3] = source[3];
} /* end of void cpFloatColV(float *source, float *dest) */

/* ------------------------------------------------------------------------- */

void cpCharColV(uchar *source, uchar *dest)
{
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
    dest[3] = source[3];
} /* end of void cpCharColV(uchar *source, uchar *dest) */

/* ------------------------------------------------------------------------- */
void addalphaAddfacFloat(float *dest, float *source, uchar addfac)
  /* doel= bron over doel  */
{
    float m; /* weiging factor of destination */
    float c; /* intermediate colour           */

    /* 1. copy source straight away if dest has zero alpha */
	/* 2. copy dest straight away if dest has full alpha   */
	/* I am not sure whether (2) is correct. It seems to   */
	/* me that this should not happen if float colours     */
	/* aren't clipped at 1.0 .                             */
	/* I'll keep the code, but disabled....                */
    if ( (dest[3] < RE_EMPTY_COLOUR_FLOAT) 
		/*   || source[3] > RE_FULL_COLOUR_FLOAT */ ) {
        dest[0] = source[0];
        dest[1] = source[1];
        dest[2] = source[2];
        dest[3] = source[3];
        return;
    }

    /* Addfac is a number between 0 and 1: rescale */
    /* final target is to diminish the influence of dest when addfac rises */
    m = 1.0 - ( source[3] * ((255.0 - addfac) / 255.0));

    /* blend colours*/
    c= (m * dest[0]) + source[0];
#ifdef RE_FLOAT_COLOUR_CLIPPING
    if(c >= RE_FULL_COLOUR_FLOAT) dest[0] = RE_FULL_COLOUR_FLOAT; 
    else 
#endif
        dest[0]= c;
   
    c= (m * dest[1]) + source[1];
#ifdef RE_FLOAT_COLOUR_CLIPPING
    if(c >= RE_FULL_COLOUR_FLOAT) dest[1] = RE_FULL_COLOUR_FLOAT; 
    else 
#endif
        dest[1]= c;
    
    c= (m * dest[2]) + source[2];
#ifdef RE_FLOAT_COLOUR_CLIPPING
    if(c >= RE_FULL_COLOUR_FLOAT) dest[2] = RE_FULL_COLOUR_FLOAT; 
    else 
#endif
        dest[2]= c;

	c= dest[3] + source[3];
#ifdef RE_ALPHA_CLIPPING
	if(c >= RE_FULL_COLOUR_FLOAT) dest[3] = RE_FULL_COLOUR_FLOAT; 
	else 
#endif
       dest[3]= c;

} /* end of void addalphaAddfacFloat(ushort *doel, ushort *bron, uchar addfac_help) */

/* ------------------------------------------------------------------------- */

void addalphaAddfacShort(ushort *doel, ushort *bron, uchar addfac)
  /* doel= bron over doel  */
{
    float m; /* weiging factor of destination */
    float c; /* intermediate colour           */

    /* 1. copy bron straight away if doel has zero alpha */
    if( doel[3] == 0) {
        *((uint *)doel)     = *((uint *)bron);
        *((uint *)(doel+2)) = *((uint *)(bron+2));
        return;
    }
    
    /* Addfac is a number between 0 and 1: rescale */
    /* final target is to diminish the influence of dest when addfac rises */
    m = 1.0 - ( bron[3] * ((255.0 - addfac) / 255.0));

    /* blend colours*/
    c = (m * doel[0]) + bron[0];
    if( c > 65535.0 ) doel[0]=65535; 
    else doel[0] = ffloor(c);
    c = (m * doel[1]) + bron[1];
    if( c > 65535.0 ) doel[1]=65535; 
    else doel[1] = ffloor(c);
    c = (m * doel[2]) + bron[2];
    if( c > 65535.0 ) doel[2]=65535; 
    else doel[2] = ffloor(c);

    c = doel[3] + bron[3];
    if(c > 65535.0) doel[3] = 65535; 
    else doel[3]=  ffloor(c);

} /* end of void addalphaAddfacShort(ushort *doel, ushort *bron, uchar addfac_help) */

/* ------------------------------------------------------------------------- */

void addHaloToHaloShort(ushort *d, ushort *s)
{
    /*  float m; */ /* weiging factor of destination */
    float c[4]; /* intermediate colour           */
    float rescale = 1.0;

    /* 1. copy <s> straight away if <d> has zero alpha */
    if( d[3] == 0) {
        *((uint *) d)      = *((uint *) s);
        *((uint *)(d + 2)) = *((uint *)(s + 2));
        return;
    }

    /* 2. halo blending  */
    /* no blending, just add */
    c[0] = s[0] + d[0];
    c[1] = s[1] + d[1];
    c[2] = s[2] + d[2];
    c[3] = s[3] + d[3];
    /* One thing that may happen is that this pixel is over-saturated with light - */
    /* i.e. too much light comes out, and the pixel is clipped. Currently, this    */
    /* leads to artifacts such as overproportional undersampling of background     */
    /* colours.                                                                    */
    /* Compensating for over-saturation:                                           */
    /* - increase alpha                                                            */
    /* - increase alpha and rescale colours                                        */

    /* let's try alpha increase and clipping */

    /* calculate how much rescaling we need */
    if( c[0] > 65535.0 ) { 
      rescale *= c[0] /65535.0;
      d[0] = 65535; 
    } else d[0] = ffloor(c[0]);
    if( c[1] > 65535.0 ) { 
      rescale *= c[1] /65535.0;
      d[1] = 65535; 
    } else d[1] = ffloor(c[1]);
    if( c[2] > 65535.0 ) { 
      rescale *= c[2] /65535.0;
      d[2] = 65535; 
    } else d[2] = ffloor(c[2]);

    /* a bit too hefty I think */
    c[3] *= rescale;

    if( c[3] > 65535.0 ) d[3] = 65535; else d[3]=  ffloor(c[3]);

} /* end of void addHaloToHaloShort(ushort *dest, ushort *source, char addfac) */

/* ------------------------------------------------------------------------- */

void sampleShortColV2ShortColV(ushort *sample, ushort *dest, int osaNr)
{
    uint intcol[4] = {0};
    ushort *scol = sample; 
    int a = 0;
    
    for(a=0; a < osaNr; a++, scol+=4) {
        intcol[0]+= scol[0]; intcol[1]+= scol[1];
        intcol[2]+= scol[2]; intcol[3]+= scol[3];
    }
    
    /* Now normalise the integrated colour. It is guaranteed */
    /* to be correctly bounded.                              */
    dest[0]= intcol[0]/osaNr;
    dest[1]= intcol[1]/osaNr;
    dest[2]= intcol[2]/osaNr;
    dest[3]= intcol[3]/osaNr;
    
} /* end of void sampleShortColVToShortColV(ushort *sample, ushort *dest) */


/* ------------------------------------------------------------------------- */

void sampleFloatColV2FloatColV(float *sample, float *dest, int osaNr)
{
    float intcol[4] = {0};
    float *scol = sample; 
    int   a = 0;
#ifdef RE_PRIMITIVE_GAMMA_CORRECTION
	/* Needs to become something with lookup tables.*/
	/* Go to intensities, and integrate those. Don't touch alpha  */
	float invgamma = 1.0 / RE_DEFAULT_GAMMA;
    for(a=0; a < osaNr; a++, scol+=4) {
		intcol[0] += powf(scol[0], RE_DEFAULT_GAMMA);
		intcol[1] += powf(scol[1], RE_DEFAULT_GAMMA);
		intcol[2] += powf(scol[2], RE_DEFAULT_GAMMA);
		intcol[3] += scol[3];
    }

	/* renormalise */
	intcol[0] /= osaNr;
	intcol[1] /= osaNr;
	intcol[2] /= osaNr;
	intcol[3] /= osaNr;

	/* back to pixel values */
	dest[0] = powf(intcol[0], invgamma);
	dest[1] = powf(intcol[1], invgamma);
	dest[2] = powf(intcol[2], invgamma);
	dest[3] = intcol[3];

	return;		
#endif

#ifdef RE_INTERPOLATED_GAMMA_CORRECTION
	/* use a LUT and interpolation to do the gamma correction */
	for(a=0; a < osaNr; a++, scol+=4) {
  		intcol[0] += gammaCorrect(scol[0]); 
  		intcol[1] += gammaCorrect(scol[1]); 
  		intcol[2] += gammaCorrect(scol[2]); 
		intcol[3] += scol[3];
    }

	/* renormalise */
	intcol[0] /= osaNr;
	intcol[1] /= osaNr;
	intcol[2] /= osaNr;
	intcol[3] /= osaNr;

	/* back to pixel values */
	dest[0] = invGammaCorrect(intcol[0]);
	dest[1] = invGammaCorrect(intcol[1]);
	dest[2] = invGammaCorrect(intcol[2]);
	dest[3] = intcol[3];

	return;		
#endif

#ifdef RE_NO_GAMMA_CORRECTION
	/* Each colour should be weighted by alpha? We need gamma correction     */
	/* badly */
    for(a=0; a < osaNr; a++, scol+=4) {
		intcol[0] += scol[0]; intcol[1] += scol[1];
		intcol[2] += scol[2]; intcol[3] += scol[3];
    }
    
    dest[0]= intcol[0]/osaNr;
    dest[1]= intcol[1]/osaNr;
    dest[2]= intcol[2]/osaNr;
    dest[3]= intcol[3]/osaNr;
#endif
	
} /* end of void sampleFloatColVToFloatColV(ushort *sample, ushort *dest) */


/* ------------------------------------------------------------------------- */

float gammaCorrect(float c)
{
	int i;
	float res = 0.0;
	
	i = ffloor(c * inv_colour_step);

	/* Clip to range [0,1]: outside, just do the complete calculation.       */
	/* We may have some performance problems here. Stretching up the LUT     */
	/* may help solve that, by exchanging LUT size for the interpolation.    */
	if (i < 0) res = powf(c, valid_gamma);
	else if (i >= RE_GAMMA_TABLE_SIZE ) res = powf(c, valid_gamma);
	else res = gamma_range_table[i] + 
  			 ( (c - colour_domain_table[i]) * gamfactor_table[i]); 

	
	
	return res;
} /* end of float gammaCorrect(float col) */

/* ------------------------------------------------------------------------- */

float invGammaCorrect(float col)
{
	int i;
	float res = 0.0;

	i = ffloor(col*inv_colour_step);
	if (i < 0) res = powf(col, valid_inv_gamma);
	else if (i >= RE_GAMMA_TABLE_SIZE) res = powf(col, valid_inv_gamma);
	else res = inv_gamma_range_table[i] + 
  			 ( (col - colour_domain_table[i]) * inv_gamfactor_table[i]);
			   
	return res;
} /* end of float invGammaCorrect(float col) */


/* ------------------------------------------------------------------------- */

void makeGammaTables(float gamma)
{
	/* we need two tables: one forward, one backward */
	int i;

	valid_gamma        = gamma;
	valid_inv_gamma    = 1.0 / gamma;
	colour_step        = 1.0 / RE_GAMMA_TABLE_SIZE;
	inv_colour_step    = (float) RE_GAMMA_TABLE_SIZE; 

	/* We could squeeze out the two range tables to gain some memory.        */	
	for (i = 0; i < RE_GAMMA_TABLE_SIZE; i++) {
		colour_domain_table[i]   = i * colour_step;
		gamma_range_table[i]     = powf(colour_domain_table[i],
										valid_gamma);
		inv_gamma_range_table[i] = powf(colour_domain_table[i],
										valid_inv_gamma);
	}

	/* The end of the table should match 1.0 carefully. In order to avoid    */
	/* rounding errors, we just set this explicitly. The last segment may    */
	/* have a different lenght than the other segments, but our              */
	/* interpolation is insensitive to that.                                 */
	colour_domain_table[RE_GAMMA_TABLE_SIZE]   = 1.0;
	gamma_range_table[RE_GAMMA_TABLE_SIZE]     = 1.0;
	inv_gamma_range_table[RE_GAMMA_TABLE_SIZE] = 1.0;

	/* To speed up calculations, we make these calc factor tables. They are  */
	/* multiplication factors used in scaling the interpolation.             */
	for (i = 0; i < RE_GAMMA_TABLE_SIZE; i++ ) {
		gamfactor_table[i] = inv_colour_step
			* (gamma_range_table[i + 1] - gamma_range_table[i]) ;
		inv_gamfactor_table[i] = inv_colour_step
			* (inv_gamma_range_table[i + 1] - inv_gamma_range_table[i]) ;
	}

	gamma_table_initialised = 1;
} /* end of void makeGammaTables(float gamma) */

/* ------------------------------------------------------------------------- */

int gammaTableIsInitialised(void)
{
	return gamma_table_initialised;
}

/* ------------------------------------------------------------------------- */

/* eof pixelblending.c */



