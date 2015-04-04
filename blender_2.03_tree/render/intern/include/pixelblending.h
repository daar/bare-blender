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

 *    pixelblending_ext.h
 * external interface for pixelblending 
 *
 * Version: $Id: pixelblending.h,v 1.2 2000/09/08 16:41:20 nzc Exp $
 */

#ifndef PIXELBLENDING_EXT_H
#define PIXELBLENDING_EXT_H "$Id: pixelblending.h,v 1.2 2000/09/08 16:41:20 nzc Exp $"

/* global includes */
#include "blender.h"

/* local includes */
#include "vanillaRenderPipe_types.h"

/* own include */
#include "pixelblending_types.h"

/**
 * Samples pixel, depending on R.osa setting
 */
int addtosampcol(ushort *sampcol, ushort *shortcol, int mask);

/**
 * Samples pixel, bring your own R.osa setting
 */
int addToSampCol(ushort *sampcol, ushort *shortcol, int mask, int osaNr);

/**
 * Halo-add pixel, bring your own R.osa setting, and add factor
 */
void addAddSampColF(float *s, float *d, int m, int osa, uchar add);

/**
 * Alpha undersamples pixel, bring your own R.osa setting
 */
int addUnderSampColF(float *sampcol, float *dest, int mask, int osaNr);

/**
 * Alpha oversample pixel, bring your own R.osa setting
 */
void addOverSampColF(float *sampcol, float *dest, int mask, int osaNr);

/**
 * Convert a series of oversampled pixels into a single pixel. 
 * (float vecs to float vec)
 */
void sampleFloatColV2FloatColV(float *sample, float *dest, int osaNr);

/**
 * Convert a series of oversampled pixels into a single pixel. Uses R.osa to
 * count the length! (short vecs to short vec)
 */
void sampleShortColV2ShortColV(ushort *sample, ushort *dest, int osaNr);

/**
 * Take colour <bron>, and apply it to <doel> using the alpha value of
 * <bron>. 
 * @param doel
 * @param bron
 */
void addAlphaOverShort(ushort *doel, ushort *bron);   

/**
 * Take colour <bron>, and apply it to <doel> using the alpha value of
 * <doel>. 
 * @param doel
 * @param bron
 */
void addAlphaUnderShort(ushort *doel, ushort *bron);  

/**
 * Alpha-over blending for floats.
 */
void addAlphaOverFloat(float *dest, float *source);  

/**
 * Alpha-under blending for floats.
 */
void addAlphaUnderFloat(float *dest, float *source);  

/**
 * Write a 16-bit-colour colour vector to a 8-bit-colour colour vector. 
 */
void cpShortColV2CharColV(ushort *source, uchar *dest);

/**
 * Write a 8-bit-colour colour vector to a 16-bit-colour colour vector. 
 */
void cpCharColV2ShortColV(uchar *source, ushort *dest);

/**
 * Write a 32-bit-colour colour vector to a 8-bit-colour colour vector. 
 */
void cpIntColV2CharColV(uint *source, uchar *dest);

/**
 * Write a floating-point-colour colour vector to a 8-bit-colour colour 
 * vector. Clip colours to [0, 1].
 */
void cpFloatColV2CharColV(float *source, uchar *dest);

/**
 * Cpoy a 8-bit-colour vector to floating point colour vector.
 */
void cpCharColV2FloatColV(uchar *source, float *dest);
/**
 * Cpoy a 16-bit-colour vector to floating point colour vector.
 */
void cpShortColV2FloatColV(ushort *source, float *dest);

/**
 * Copy a float-colour colour vector.
 */
void cpFloatColV(float *source, float *dest);

/**
 * Copy a 16-bit-colour colour vector.
 */
void cpShortColV(ushort *source, ushort *dest);

/**
 * Copy an 8-bit-colour colour vector.
 */
void cpCharColV(uchar *source, uchar *dest);

/**
 * Add a fraction of <source> to <dest>. Result ends up in <dest>.
 * The internal calculation is done with floats.
 * 
 * col(dest)   = (1 - alpha(source)*(1 - addfac)) * dest + source
 * alpha(dest) = alpha(source) + alpha (dest)
 */
void addalphaAddfacShort(ushort *dest, ushort *source, uchar addfac);

/**
 * Same for floats
 */
void addalphaAddfacFloat(float *dest, float *source, uchar addfac);

/**
 * Add two halos. Result ends up in <dest>. This should be the 
 * addition of two light sources. So far, I use normal alpha-under blending here.
 * The internal calculation is done with floats. The add-factors have to be 
 * compensated outside this routine.
 * col(dest)   = s + (1 - alpha(s))d
 * alpha(dest) = alpha(s) + (1 - alpha(s))alpha (d)
 */
void addHaloToHaloShort(ushort *dest, ushort *source);

/**
 * Initialise the gamma lookup tables
 */
void makeGammaTables(float gamma);

/**
 * Returns true if the table is initialised, false otherwise
 */
int gammaTableIsInitialised(void);

/**
 * Apply gamma correction on col
 */
float gammaCorrect(float col);

/**
 * Apply inverse gamma correction on col
 */
float invGammaCorrect(float col);

#endif /* PIXELBLENDING_EXT_H */

