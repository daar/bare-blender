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

#include <math.h>

#include "matrix.h"
#include "../include/iff.h"

#define YB_yrb_start	0
#define YR_yrb_start	105
#define B_yrb_start		210
#define R_yrb_start		218
#define GREY_yrb_start	226
#define Y_yrb_start		243


#define YH_tan_start		0		/* + 15 * 8 */
#define YS_tan_start		120		/* + 15 * 4 */
#define Y_tan_start			180		/* + 32 */
#define H_tan_start			212		/* + 16 */
#define S_tan_start			228		/* + 7 */
#define HS_tan_start		235		/* + 16 ?? */
#define FREE_tan_start		251		/* + 4 ?? */


/*

zipfork "cc makeyuvx.c ../util.o -limbuf -lm -o makeyuvx >/dev/console"                                                     
zipfork "makeyuvx | tee yuvx.h >/dev/null"                                       

*/

/* volgorde is YBR */

main()
{
	int i, y, b, r, h, s, cy, cb, cr, ch, cs, col;
	double t;
	
	printf("short yuv_and_or[] = {\n");
	
	printf("/* YB */\n");
	for (i = YB_yrb_start; i < YR_yrb_start; i++) {
		printf("0x000F,\n");
	}
	
	printf("/* YR */\n");
	for (i = YR_yrb_start; i < B_yrb_start; i++) {
		printf("0x00F0,\n");
	}
	
	printf("/* B */\n");
	for (i = B_yrb_start; i < R_yrb_start; i++) {
		printf("0x0F0F,\n");
	}
	
	printf("/* R */\n");
	for (i = R_yrb_start; i < GREY_yrb_start; i++) {
		printf("0x0FF0,\n");
	}
	
	printf("/* GREY */\n");
	for (i = GREY_yrb_start; i < Y_yrb_start; i++) {
		printf("0x0000,\n");
	}
	
	printf("/* Y */\n");
	for (i = Y_yrb_start; i < 256; i++) {
		printf("0x00FF,\n");
	}	
	
	printf("/* YB */\n");
	for (i = YB_yrb_start; i < YR_yrb_start; i++) {
		col = i - YB_yrb_start;
		y = (col / 7) + 1;
		b = 2 * (col % 7) + 2;
		r = 0;
		col = (y << 8) + (b << 4) + r;
		printf("0x%0.4X,\n", col);
	}
	
	printf("/* YR */\n");
	for (i = YR_yrb_start; i < B_yrb_start; i++) {
		col = i - YR_yrb_start;
		y = (col / 7) + 1;
		b = 0;
		r = 2 * (col % 7) + 2;
		col = (y << 8) + (b << 4) + r;
		printf("0x%0.4X,\n", col);
	}
	
	printf("/* B */\n");
	for (i = B_yrb_start; i < R_yrb_start; i++) {
		col = i - B_yrb_start;
		y = 0;
		b = 2 * col + 1;
		r = 0;
		col = (y << 8) + (b << 4) + r;
		printf("0x%0.4X,\n", col);
	}
	
	printf("/* R */\n");
	for (i = R_yrb_start; i < GREY_yrb_start; i++) {
		col = i - R_yrb_start;
		y = 0;
		b = 0;
		r = 2 * col + 1;
		col = (y << 8) + (b << 4) + r;
		printf("0x%0.4X,\n", col);
	}
	
	printf("/* GREY */\n");
	
	for (i = GREY_yrb_start; i < Y_yrb_start; i++) {
		col = i - GREY_yrb_start;
		/* uitzonderingen */
		
		if (col == 0) {
			y = 1;
			b = 0;
			r = 8;
		} else if (col == 16) {
			y = 15;
			b = 8;
			r = 0;
		} else {
			y = col;
			b = 8;
			r = 8;
		}
		col = (y << 8) + (b << 4) + r;
		printf("0x%0.4X,\n", col);
	}
	
	printf("/* Y */\n");
	for (i = Y_yrb_start; i < 256; i++) {
		col = i - Y_yrb_start;
		/* uitzonderingen */
		
		y = col + 2;
		b = 0;
		r = 0;

		col = (y << 8) + (b << 4) + r;
		printf("0x%0.4X,\n", col);
	}	

	printf("};\n\n");

	printf("uint yuv_cmap[] = {\n");
	
	/* eerste 256 overslaan */
	
	for (i = 0; i < 256; i++) {
		printf("0x%0.6X,\n", 0);
	}
	
	for (y = 1; y < 16; y++) {
		for (b = 0; b < 16; b++) {
			for (r = 0; r < 16; r++) {
				if (b == 0) {
					if (y == 1) cy = 0;
					else cy = y;

					cb = 7;					
					if (r != 0) cr = r - 1;
					else cr = 7;
					
				} else if (r == 0) {
					if (y == 15) cy = 16;
					else cy = y;
					
					cb = b - 1;
					cr = 7;
				} else {
					cy = y;
					cb = b - 1;
					cr = r - 1;
				}
				
				cy = ((cy * 255.0) / 16.0) + 0.5;
				
/*				t = (cr - 7) / 7.0;
				if (t < 0) t = -t * t;
				else t = t * t;
				t = (7.0 * t) + 7.0;
				t = (t + cr) / 2.0;
				cr = (t * 255.0 / 14.0) + 0.5;

				t = (cb - 7) / 7.0;
				if (t < 0) t = -t * t;
				else t = t * t;
				t = (7.0 * t) + 7.0;
				t = (t + cb) / 2.0;
				cb = (t * 255.0 / 14.0) + 0.5;
*/

				cr = (cr * 255.0 / 14.0) + 0.5;
				cb = (cb * 255.0 / 14.0) + 0.5;
				
				col = (cb << 16) + (cy << 8) + cr;
				printf("0x%0.6X,\n", colcspace(col, yuvrgb));
			}
		}
	}
	
	printf("};\n\n");
	
	/* volgorde is: 3 bits S, 5 bits Y, 4 bits H */
	
	printf("short tan_and_or[] = {\n");
	
	printf("/* YH */\n");
	for (i = YH_tan_start; i < YS_tan_start; i++) {
		printf("0x0E00,\n");
	}

	printf("/* YS */\n");
	for (i = YS_tan_start; i < Y_tan_start; i++) {
		printf("0x000F,\n");
	}

	printf("/* Y */\n");
	for (i = Y_tan_start; i < H_tan_start; i++) {
		printf("0x0E0F,\n");
	}

	printf("/* H */\n");
	for (i = H_tan_start; i < S_tan_start; i++) {
		printf("0x0FF0,\n");
	}

	printf("/* S */\n");
	for (i = S_tan_start; i < HS_tan_start; i++) {
		printf("0x01FF,\n");
	}

	printf("/* HS */\n");
	for (i = HS_tan_start; i < FREE_tan_start; i++) {
		printf("0x01F0,\n");
	}

	printf("/* FREE */\n");
	for (i = FREE_tan_start; i < 256; i++) {
		printf("0x0FFF,\n");
	}
	
	
	/*********************/
	
	printf("/* YH */\n");
	for (i = YH_tan_start; i < YS_tan_start; i++) {
		col = i - YH_tan_start;
		y = col >> 3;
		y = 2 + 2 * y;
		h = (col & 0x7) << 1;
		col = (y << 4) + h;
		printf("0x%0.4X,\n", col);
	}

	printf("/* YS */\n");
	for (i = YS_tan_start; i < Y_tan_start; i++) {
		col = i - YS_tan_start;
		y = col >> 2;
		y = 2 + 2 * y;
		s = (col & 0x3) << 1;
		col = ((s + 1) << 9) + (y << 4);
		printf("0x%0.4X,\n", col);
	}

	printf("/* Y */\n");
	for (i = Y_tan_start; i < H_tan_start; i++) {
		y = i - Y_tan_start;
		col = (y << 4);
		printf("0x%0.4X,\n", col);
	}

	printf("/* H */\n");
	for (i = H_tan_start; i < S_tan_start; i++) {
		h = i - H_tan_start;
		col = h;
		printf("0x%0.4X,\n", col);
	}

	printf("/* S */\n");
	for (i = S_tan_start; i < HS_tan_start; i++) {
		s = i - S_tan_start;
		col = ((s + 1) << 9);
		printf("0x%0.4X,\n", col);
	}

	printf("/* HS */\n");
	for (i = HS_tan_start; i < FREE_tan_start; i++) {
		col = i - HS_tan_start;
		h = (col & 0x7) << 1;
		if (col >= 8) s = 4;
		else s = 2;
		col = ((s + 1) << 9) + h;
		printf("0x%0.4X,\n", col);
	}

	printf("/* FREE */\n");
	for (i = FREE_tan_start; i < 256; i++) {
		printf("0x0020,\n");
	}
	
	printf("};\n\n");
	
	printf("uint tan_cmap[] = {\n");
	
	/* eerste 512 overslaan */
	
	for (i = 0; i < 512; i++) {
		printf("0x%0.6X,\n", 0);
	}
	
	/* colormap wordt on the fly berekent ivm gamma & distort */
	
	for (s = 0; s < 7; s++) {
		for (y = 0; y < 32; y++) {
			for (h = 0; h < 16; h++) {
				ch = (h * 16.0) + 0.5;
				cy = ((y * 255.0) / 31.0) + 0.5;
				cs = (s * 16.0) + 0.5 + 80.0;

				/*cs = ffloor(((s * s) / 7.0) + 0.5) * 16.0 + 0.5 + 80.0;*/

				col = (ch << 16) + (cy << 8) + cs;
				printf("0x%0.6X,\n", col);
			}
		}
	}
	
	printf("};\n\n");

}

