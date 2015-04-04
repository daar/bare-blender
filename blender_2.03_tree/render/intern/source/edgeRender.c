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

/**

 * edgeRender.c
 *
 * 01-09-2000 nzc
 *
 * $Id: edgeRender.c,v 1.4 2000/09/22 09:23:55 nzc Exp $
 *
 */

/*
 * Edge rendering: use a mask to weigh the depth of neighbouring
 * pixels, and do a colour correction.
 *
 * We need:
 * - a buffer to store the depths (ints)
 * - a function that alters the colours in R.rectot (copy edge_enhance?)
 *   The max. z buffer depth is 0x7FFF.FFFF (7 F's)
 *
 * - We 'ignore' the pixels falling outside the regular buffer (we fill)
 *   these with the max depth. This causes artefacts when rendering in
 *   parts.
 */

/* ------------------------------------------------------------------------- */

/* enable extra bounds checking and tracing                                  */
/*  #define RE_EDGERENDERSAFE */
/* disable the actual edge correction                                        */
/*  #define RE_EDGERENDER_NO_CORRECTION */

/*  #include "edgeRender_types.h" */
#include "blender.h"
#include <limits.h>       /* INT_MIN,MAX are used here                       */ 
#include "edgeRender.h"
#include "render.h"
#include "zbuf.h"
#include "vectorops.h"
/*  #include "filtering.h" */ /* would like to use generic filtering... */

#ifdef RE_EDGERENDERSAFE
char edgeRender_h[] = EDGERENDER_H;
char edgeRender_c[] = "$Id: edgeRender.c,v 1.4 2000/09/22 09:23:55 nzc Exp $";
#include "errorHandler.h"
#endif

/* ------------------------------------------------------------------------- */

extern void (*zbuffunc)(float *, float *, float *); /* These function        */
extern void (*zbuflinefunc)(); /* pointers are used for z buffer filling.    */
extern float jit[64][2];     /* Table with jitter offsets                    */
extern float Zmulx, Zmuly;   /* Some kind of scale?                          */
extern float Zjitx,Zjity;    /* The x,y values for jitter offset             */
extern uint Zvlnr;           /* Face rendering pointer and counter: these    */
extern VlakRen *Zvlr;        /* are used for 'caching' render results.       */

/* ------------------------------------------------------------------------- */

static char* colBuffer;      /* buffer with colour correction                */
static int *edgeBuffer;      /* buffer with distances                        */
static int  bufWidth;        /* x-dimension of the buffer                    */
static int  bufHeight;       /* y-dimension of the buffer                    */
static int  imWidth;         /* x-dimension of the image                     */
static int  imHeight;        /* y-dimension of the image                     */
static int  osaCount;        /* oversample count                             */
static int  maskBorder;      /* size of the mask border                      */
static short int intensity;  /* edge intensity                               */
static int  compatible_mode; /* edge positioning compatible with old rederer */
static int  selectmode;      /* 0: only solid faces, 1: also transparent f's */

static int  Aminy;           /* y value of first line in the accu buffer     */
static int  Amaxy;           /* y value of last line in the accu buffer      */
                             /* -also used to clip when zbuffering           */


/* Local functions --------------------------------------------------------- */
/**
 * Initialise the edge render buffer memory.
 */
void initEdgeRenderBuffer();
/**
 * Release buffer memory.
 */
void freeEdgeRenderBuffer(void);

/**
 * Set all distances in the distance buffer to the maximum distance.
 */
void resetDistanceBuffer(void);

/**
 * Insert this distance at these pixel coordinates.
 */
void insertInEdgeBuffer(int x, int y, int dist);

/**
 * Renders enhanced edges. Distances from distRect are used to
 * determine a correction on colourRect
 */
void renderEdges(char * colourRect);

/**
 * Buffer an edge between these two vertices in the e.r. distance buffer.
 */
void fillEdgeRenderEdge(float *vec1, float *vec2);

/**
 * Buffer a face between these two vertices in the e.r. distance buffer.
 */
void fillEdgeRenderFace(float *v1, float *v2, float *v3);

/**
 * Compose the edge render colour buffer.
 */
void calcEdgeRenderColBuf(char * tarbuf);

/**
 * Loop over all objects that need to be edge rendered. This loop determines
 * which objects get to be elected for edge rendering.
 */
int  zBufferEdgeRenderObjects(void);

/* ------------------------------------------------------------------------- */

void addEdges(char * targetbuf, int iw, int ih,
			  int osanr, short int intens, int compat, int mode)
{
	/* render parameters */
	selectmode = mode;
	imWidth    = iw;
	imHeight   = ih;
	compatible_mode = compat;
	osaCount   = osanr;
	intensity  = intens;

	/* Go! */
	initEdgeRenderBuffer();
	calcEdgeRenderColBuf(targetbuf);
	freeEdgeRenderBuffer();
	
} /* end of void addEdges(char *, int, int, int, short int , int) */

/* ------------------------------------------------------------------------- */

void initEdgeRenderBuffer()
{
	maskBorder = 1; /* for 3 by 3 mask*/
		
	bufWidth   = imWidth + (2 * maskBorder);
	bufHeight  = imHeight + (2 * maskBorder);
	
	edgeBuffer = callocN(sizeof(int) * bufWidth * bufHeight, "edgeBuffer");
	colBuffer  = callocN(sizeof(char) * 4 * imWidth * imHeight, "colBuffer");
	
#ifdef RE_EDGERENDERSAFE
	if (!edgeBuffer || !colBuffer) {
		char *fname = "initEdgeRenderBuffer";
		RE_error(RE_CANNOT_ALLOCATE_MEMORY, fname);
	}
#endif
} /* end of void initEdgeRenderBuffer(void) */

/* ------------------------------------------------------------------------- */
void freeEdgeRenderBuffer(void)
{
	if(edgeBuffer) freeN(edgeBuffer);
	if(colBuffer)  freeN(colBuffer);
} /* end of void freeEdgeRenderBuffer(void) */

/* ------------------------------------------------------------------------- */

void resetDistanceBuffer(void)
{
	int i;
	for(i = 0; i < bufWidth * bufHeight; i++) edgeBuffer[i] = 0x7FFFFFFF;
} /* end of void resetDistanceBuffer(void) */

/* ------------------------------------------------------------------------- */

void insertInEdgeBuffer(int x, int y, int dist)
{
	int index;
#ifdef RE_EDGERENDERSAFE
	char *fname = "insertInEdgeBuffer";
	if ((x < 0) || (x > imWidth ) ||
		(y < 0) || (y > (imHeight-1) ) ) {
		RE_error(RE_EDGERENDER_WRITE_OUTSIDE_BUFFER, fname);
		return;
	}
#endif

	index = (y * bufWidth) + x + maskBorder;

	if (edgeBuffer[index] >dist ) edgeBuffer[index] = dist;

} /* end of void insertInEdgeBuffer(int x, int y, int dist) */

/* ------------------------------------------------------------------------- */
/* Modelled after rendercore.c/edge_enhance()                                */
void renderEdges(char *colourRect)
{
	/* use zbuffer to define edges, add it to the image */
	int val, y, x, col, *rz, *rz1, *rz2, *rz3;
	char *cp;
	int targetoffset, heightoffset;
	int i;

#ifdef RE_EDGERENDER_NO_CORRECTION
	return; /* no edge correction */
#endif
	
#ifdef RE_EDGERENDERSAFE
    fprintf(stderr, "\n*** Activated full error trace on "
            "edge rendering  using:\n\t%s\n\t%s"
			"\n*** Rendering edges at %d intensity", 
            edgeRender_c, edgeRender_h, intensity);
#endif

	
	/* Old renderer uses wrong positions! With the compat switch on, the po- */
	/* sitions will be corrected to be offset in the same way.               */
	if (compatible_mode) {
		targetoffset = 4 * (imWidth - 1);
		heightoffset = -1;
	} else {
		targetoffset = 0;
		heightoffset = 0;
	}
	
	/* Fill edges with some default values. We just copy what is in the edge */
	/* This looks messy, but it appears to be ok.                            */
	edgeBuffer[0]                          = edgeBuffer[bufWidth + 1];
	edgeBuffer[bufWidth - 1]               = edgeBuffer[(2 * bufWidth) - 2];
	edgeBuffer[bufWidth * (bufHeight - 1)] =
		edgeBuffer[bufWidth * (bufHeight - 2) + 1];
	edgeBuffer[(bufWidth * bufHeight) - 1] =
		edgeBuffer[(bufWidth * (bufHeight - 1)) - 2];

	for (i = 1; i < bufWidth - 1; i++) { /* lieing edges */
		edgeBuffer[i] = edgeBuffer[bufWidth + i]; /* bottom*/
		edgeBuffer[((bufHeight - 1)*bufWidth) + i]
			= edgeBuffer[((bufHeight - 2)*bufWidth) + i]; /* top */
	}

	for (i = 1; i < bufHeight - 2; i++) { /* standing edges */
		edgeBuffer[i * bufWidth] = edgeBuffer[(i * bufWidth) + 1]; /* left */
		edgeBuffer[((i + 1) * bufWidth) - 1] =
			edgeBuffer[((i + 1) * bufWidth) - 2]; /* right */
	}
	
	/* alle getallen in zbuffer 3 naar rechts shiften */
  	rz = edgeBuffer;
	if(rz==0) return;
	
	for(y=0; y < bufHeight * bufWidth; y++, rz++) {
		(*rz)>>= 3;
	}
	
	/* Distance pointers */
	rz1= edgeBuffer;
	rz2= rz1 + bufWidth;
	rz3= rz2 + bufWidth;
	if (osaCount == 1) {
		cp = colourRect + targetoffset;
	} else {
		cp = colBuffer + targetoffset;
	}

	for(y = 0; y < (imHeight + heightoffset) ; y++) {

		
		for(x = 0; x < imWidth; x++, rz1++, rz2++, rz3++, cp+=4) {

			col= abs( -   rz1[0] -  2*rz1[1] -   rz1[2]
					  - 2*rz2[0] + 12*rz2[1] - 2*rz2[2]
					  -   rz3[0] -  2*rz3[1] -   rz3[2]) / 3;

			col= (intensity * col)>>14;
			if(col>255) col= 255;
			
			if(col>0) {
				if(osaCount > 1) {
					/* This does not work properly.... Why?             */
					/* the range over which the col value run is wrong. */
					/* I can't get proper contracts.                    */
					col/= osaCount;
					
					val= cp[3]+col;
					if(val>255) cp[3]= 255; else cp[3]= val;
				}
				else {
					/* the pixel is blackened when col is too big */
					val= cp[0] - col;
					if(val<0) cp[0]= 0; else cp[0]= val;
					val= cp[1] - col;
					if(val<0) cp[1]= 0; else cp[1]= val;
					val= cp[2] - col;
					if(val<0) cp[2]= 0; else cp[2]= val;
				}
			}
		}
		rz1+= 2;
		rz2+= 2;
		rz3+= 2;
	}

} /* end of void renderEdges() */

/* ------------------------------------------------------------------------- */


void calcEdgeRenderColBuf(char* colTargetBuffer)
{
    extern float jit[64][2];  /* For jittered z buffering */

/*      int part; */
    int keepLooping = 1;
	int sample;
	
	/* zbuffer fix: here? */
	Zmulx= ((float) imWidth)/2.0;
  	Zmuly= ((float) imHeight)/2.0;
	
	/* use these buffer fill functions */    
	zbuffunc     = fillEdgeRenderFace;
	zbuflinefunc = fillEdgeRenderEdge;

	/* always buffer the max. extent */
	Aminy = 0;
	Amaxy = imHeight;
	        
	sample = 0; /* Zsample is used internally !                         */
	while ( (sample < osaCount) && keepLooping ) {
		/* jitter */
		Zjitx= -jit[sample][0];
		Zjity= -jit[sample][1];

		/* should reset dis buffer here */
		resetDistanceBuffer();
		
		/* kick all into a z buffer */
		keepLooping = zBufferEdgeRenderObjects();

		/* do filtering */
		renderEdges(colTargetBuffer);

  		if(RE_test_break()) keepLooping = 0; 
  		sample++; 
	}

	/* correction for osa-sampling...*/
	if( osaCount != 1) {
		char *rp, *rt;
		int a;
		
		rt= colTargetBuffer;
		rp= colBuffer;
		for(a = imWidth * imHeight; a>0; a--, rt+=4, rp+=4) {
			addalphaOver(rt, rp);
		}
	}
	
} /*End of void calcEdgeRenderZBuf(void) */

/* ------------------------------------------------------------------------- */
/* Clip flags etc. should still be set. When called in the span of 'normal'  */
/* rendering, this should be ok.                                             */
int zBufferEdgeRenderObjects(void)
{
    int keepLooping; 
    int faceCounter; /* counter for face number */
    Material *ma;
	
	keepLooping = 1;
    ma          = NULL;
    faceCounter = 0;
			
    while ( (faceCounter < R.totvlak) && keepLooping) {
        if((faceCounter & 255)==0) { Zvlr= R.blovl[faceCounter>>8]; }
        else Zvlr++;
        
        ma= Zvlr->mat;
        
        /* face number is used in the fill functions */
		Zvlnr = faceCounter + 1; 
        
        if(Zvlr->flag & R_VISIBLE) {

			/* here we cull all transparent faces if mode == 0 */
			if (selectmode || !(ma->mode & MA_ZTRA)) {
				/* here we can add all kinds of extra selection criteria */
				if(ma->mode & (MA_WIRE)) zbufclipwire(Zvlr);
				else {
					zbufclip(Zvlr->v1->ho,   Zvlr->v2->ho,   Zvlr->v3->ho, 
							 Zvlr->v1->clip, Zvlr->v2->clip, Zvlr->v3->clip);
					if(Zvlr->v4) {
						Zvlnr+= 0x800000; /* in a sense, the 'adjoint' face */
						zbufclip(Zvlr->v1->ho,   Zvlr->v3->ho,   Zvlr->v4->ho, 
								 Zvlr->v1->clip, Zvlr->v3->clip, Zvlr->v4->clip);
					}
				}
			}
        };
        if(RE_test_break()) keepLooping = 0; 
        faceCounter++;
    }
    return keepLooping;
} /* End of int zBufferEdgeRenderObjects(void) */

/* ------------------------------------------------------------------------- */

void fillEdgeRenderFace(float *v1, float *v2, float *v3)  
{
	/* Coordinates of the vertices are specified in ZCS */
	double z0; /* used as temp var*/
	double xx1;
	double zxd,zyd,zy0, tmp;
	float *minv,*maxv,*midv;
	register int zverg,zvlak,x;
	int my0,my2,sn1,sn2,rectx,zd;
	int y,omsl,xs0,xs1,xs2,xs3, dx0,dx1,dx2/*  , mask */;
	int linex, liney, xoffset, yoffset; /* pointers to the pixel number */

	/* These used to be doubles.  We may want to change them back if the     */
	/* loss of accuracy proves to be a problem? There does not seem to be    */
	/* any performance issues here, so I'll just keep the doubles.           */
	/*  	float vec0[3], vec1[3], vec2[3]; */
	double vec0[3], vec1[3], vec2[3];

	/* MIN MAX */
	/* sort vertices for min mid max y value */
	if(v1[1]<v2[1]) {
		if(v2[1]<v3[1])      { minv=v1; midv=v2; maxv=v3;}
		else if(v1[1]<v3[1]) { minv=v1; midv=v3; maxv=v2;}
		else	             { minv=v3; midv=v1; maxv=v2;}
	}
	else {
		if(v1[1]<v3[1]) 	 { minv=v2; midv=v1; maxv=v3;}
		else if(v2[1]<v3[1]) { minv=v2; midv=v3; maxv=v1;}
		else	             { minv=v3; midv=v2; maxv=v1;}
	}

	if(minv[1] == maxv[1]) return;	/* beveiliging 'nul' grote vlakken */

	my0  = fceil(minv[1]);
	my2  = ffloor(maxv[1]);
	omsl = ffloor(midv[1]);

	/* outside the current z buffer slice: clip whole face */
	if( (my2 < Aminy) || (my0 > Amaxy)) return;

	if(my0<Aminy) my0= Aminy;

	/* EDGES : DE LANGSTE */
	xx1= maxv[1]-minv[1];
	if(xx1>2.0/65536.0) {
		z0= (maxv[0]-minv[0])/xx1;
		
		tmp= (-65536.0*z0);
		dx0= CLAMPIS(tmp, INT_MIN, INT_MAX);
		
		tmp= 65536.0*(z0*(my2-minv[1])+minv[0]);
		xs0= CLAMPIS(tmp, INT_MIN, INT_MAX);
	}
	else {
		dx0= 0;
		xs0= 65536.0*(MIN2(minv[0],maxv[0]));
	}
	/* EDGES : DE BOVENSTE */
	xx1= maxv[1]-midv[1];
	if(xx1>2.0/65536.0) {
		z0= (maxv[0]-midv[0])/xx1;
		
		tmp= (-65536.0*z0);
		dx1= CLAMPIS(tmp, INT_MIN, INT_MAX);
		
		tmp= 65536.0*(z0*(my2-midv[1])+midv[0]);
		xs1= CLAMPIS(tmp, INT_MIN, INT_MAX);
	}
	else {
		dx1= 0;
		xs1= 65536.0*(MIN2(midv[0],maxv[0]));
	}
	/* EDGES : DE ONDERSTE */
	xx1= midv[1]-minv[1];
	if(xx1>2.0/65536.0) {
		z0= (midv[0]-minv[0])/xx1;
		
		tmp= (-65536.0*z0);
		dx2= CLAMPIS(tmp, INT_MIN, INT_MAX);
		
		tmp= 65536.0*(z0*(omsl-minv[1])+minv[0]);
		xs2= CLAMPIS(tmp, INT_MIN, INT_MAX);
	}
	else {
		dx2= 0;
		xs2= 65536.0*(MIN2(minv[0],midv[0]));
	}

	/* ZBUF DX DY */
	MTC_diff3DFF(vec1, v1, v2);
	MTC_diff3DFF(vec2, v2, v3);
	MTC_cross3Double(vec0, vec1, vec2);

	/* cross product of two of the sides is 0 => this face is too small */
	if(vec0[2]==0.0) return;

	if(midv[1] == maxv[1]) omsl= my2;
	if(omsl < Aminy) omsl= Aminy-1;  /* dan neemt ie de eerste lus helemaal */

	while (my2 > Amaxy) {  /* my2 kan groter zijn */
		xs0+=dx0;
		if (my2<=omsl) {
			xs2+= dx2;
		}
		else{
			xs1+= dx1;
		}
		my2--;
	}

	xx1= (vec0[0]*v1[0]+vec0[1]*v1[1])/vec0[2]+v1[2];

	zxd= -vec0[0]/vec0[2];
	zyd= -vec0[1]/vec0[2];
	zy0= my2*zyd+xx1;
	zd= (int)CLAMPIS(zxd, INT_MIN, INT_MAX);

	/* start-ofset in rect */
	/*    	rectx= R.rectx;  */
	/* I suspect this var needs very careful setting... When edge rendering  */
	/* is on, this is strange */
  	rectx   = imWidth;
	yoffset = my2;
	xoffset = 0;
	
	zvlak= Zvlnr;

	xs3= 0;		/* flag */
	if(dx0>dx1) {
		MTC_swapInt(&xs0, &xs1);
		MTC_swapInt(&dx0, &dx1);
		xs3= 1;	/* flag */

	}

	liney = yoffset;
	for(y=my2;y>omsl;y--) {

		sn1= xs0>>16;
		xs0+= dx0;

		sn2= xs1>>16;
		xs1+= dx1;

		sn1++;

		if(sn2>=rectx) sn2= rectx-1;
		if(sn1<0) sn1= 0;
		zverg= (int) CLAMPIS((sn1*zxd+zy0), INT_MIN, INT_MAX);

		if ((sn1 < 0) || (sn1>imWidth) ) printf("\n sn1 exceeds line");
		linex = xoffset + sn1;
		liney = yoffset;
		
		x= sn2-sn1;
		
		while(x>=0) {
			insertInEdgeBuffer(linex , liney, zverg); /* line y not needed here */
			zverg+= zd;
			linex++;
			x--;
		}
		zy0-= zyd;
		yoffset--;
	}

	if(xs3) {
		xs0= xs1;
		dx0= dx1;
	}
	if(xs0>xs2) {
		xs3= xs0;
		xs0= xs2;
		xs2= xs3;
		xs3= dx0;
		dx0= dx2;
		dx2= xs3;
	}

	for(; y>=my0; y--) {

		sn1= xs0>>16;
		xs0+= dx0;

		sn2= xs2>>16;
		xs2+= dx2;

		sn1++;

		if(sn2>=rectx) sn2= rectx-1;
		if(sn1<0) sn1= 0;
		zverg= (int) CLAMPIS((sn1*zxd+zy0), INT_MIN, INT_MAX);
		
		linex = sn1;
		liney = yoffset;
				
		x= sn2-sn1;
      
		while(x>=0) {
			insertInEdgeBuffer(linex, liney, zverg); /* line y not needed here */
			zverg+= zd;
			linex++;
			x--;
		}
		zy0-=zyd;
		yoffset--;
	}
} /* end of void fillEdgeRenderFace(float *v1, float *v2, float *v3) */

/* ------------------------------------------------------------------------- */

void fillEdgeRenderEdge(float *vec1, float *vec2)
{
/*  	RE_APixstrExt *ap; */
	int start, end, x, y, oldx, oldy, ofs;
	int dz, vergz/*  , mask */;
	float dx, dy;
	float v1[3], v2[3];
	int linex, liney;
	int xoffset, yoffset;

	
	dx= vec2[0]-vec1[0];
	dy= vec2[1]-vec1[1];
	
	if(fabs(dx) > fabs(dy)) {

		/* alle lijnen van links naar rechts */
		if(vec1[0]<vec2[0]) {
			VECCOPY(v1, vec1);
			VECCOPY(v2, vec2);
		}
		else {
			VECCOPY(v2, vec1);
			VECCOPY(v1, vec2);
			dx= -dx; dy= -dy;
		}

		start= ffloor(v1[0]);
		end= start+ffloor(dx);
		if(end >= imWidth) end = imWidth - 1;
		
		oldy= ffloor(v1[1]);
		dy/= dx;
		
		vergz= v1[2];
		dz= (v2[2]-v1[2])/dx;
		
		yoffset = oldy;
		xoffset = start;
		
		if(dy<0) ofs= -imWidth;
		else ofs= imWidth;

		liney = yoffset;
		linex = xoffset;
		
		for(x= start; x<=end; x++, xoffset++) {
			
			y= ffloor(v1[1]);
			if(y!=oldy) {
				oldy= y;
				liney++;
			}
			
			if(x>=0 && y>=Aminy && y<=Amaxy) {
				insertInEdgeBuffer(linex , liney, vergz);
			}
			
			v1[1]+= dy;
			vergz+= dz;
		}
	}
	else {
	
		/* alle lijnen van onder naar boven */
		if(vec1[1]<vec2[1]) {
			VECCOPY(v1, vec1);
			VECCOPY(v2, vec2);
		}
		else {
			VECCOPY(v2, vec1);
			VECCOPY(v1, vec2);
			dx= -dx; dy= -dy;
		}

		start= ffloor(v1[1]);
		end= start+ffloor(dy);
		
		if(start>Amaxy || end<Aminy) return;
		
		if(end>Amaxy) end= Amaxy;
		
		oldx= ffloor(v1[0]);
		dx/= dy;
		
		vergz= v1[2];
		dz= (v2[2]-v1[2])/dy;

		yoffset = start;
		xoffset = oldx;
				
		if(dx<0) ofs= -1;
		else ofs= 1;

		linex = xoffset;
		liney = yoffset;
		
		for(y= start; y<=end; y++, liney++) {
			
			x= ffloor(v1[0]);
			if(x!=oldx) {
				oldx= x;
				linex += ofs;
			}
			
			if(x>=0 && y>=Aminy && (x < imWidth)) {
				insertInEdgeBuffer(linex, liney, vergz);
			}
			
			v1[0]+= dx;
			vergz+= dz;
		}
	}
} /* End of void fillEdgeRenderEdge(float *vec1, float *vec2) */

/* ------------------------------------------------------------------------- */

/* eof edgeRender.c */

