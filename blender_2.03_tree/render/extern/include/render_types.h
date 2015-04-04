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

 * render_types.h
 *
 * Version: $Id: render_types.h,v 1.2 2000/08/25 15:27:33 nzc Exp $
 */

#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H "$Id: render_types.h,v 1.2 2000/08/25 15:27:33 nzc Exp $"

#include "blender.h"

/* ------------------------------------------------------------------------- */

#define MAXVERT (2<<20)
#define MAXVLAK (2<<20)

/* ------------------------------------------------------------------------- */

typedef struct Branch
{
	struct Branch *b[8];
} Branch;

/* ------------------------------------------------------------------------- */

typedef struct HaloRen
{	
    float alfa, xs, ys, rad, radsq, sin, cos, co[3], no[3];
    uint zs, zd;
    uint zBufDist;/* depth in the z-buffer coordinate system */
    short miny, maxy;
    short hard, b, g, r;
    char starpoints, add, type, tex;
    char linec, ringc, seed;
	short flarec; /* used to be a char. why ?*/
    float hasize;
    int pixels;
    uint lay;
    Material *mat;
} HaloRen;

/* ------------------------------------------------------------------------- */

typedef struct LampRen
{
	float xs, ys, dist;
	float co[3];
	short type, mode;
	float r, g, b;
	float energy, haint;
	int lay;
	float spotsi,spotbl;
	float vec[3];
	float xsp, ysp, distkw, inpr;
	float halokw, halo;
	float ld1,ld2;
	struct ShadBuf *shb;
	float imat[3][3];
	float spottexfac;
	float sh_invcampos[3], sh_zfac;	/* sh_= spothalo */
	
	struct LampRen *org;
	MTex *mtex[8];
	
} LampRen;

/* ------------------------------------------------------------------------- */

typedef struct Osa
{
	float dxco[3], dyco[3];
	float dxlo[3], dylo[3], dxgl[3], dygl[3], dxuv[3], dyuv[3];
	float dxref[3], dyref[3], dxorn[3], dyorn[3];
	float dxno[3], dyno[3], dxview, dyview;
	float dxlv[3], dylv[3];
	float dxwin[3], dywin[3];
	float dxsticky[3], dysticky[3];
} Osa;

/* ------------------------------------------------------------------------- */

typedef struct Part
{
	struct Part *next, *prev;
	uint *rect;
	short x, y;
} Part;

/* ------------------------------------------------------------------------- */

typedef struct PixStr
{
	struct PixStr *next;
	int vlak0, vlak;
	uint z;
	uint mask;
	short aantal, ronde;
} PixStr;

/* ------------------------------------------------------------------------- */

typedef struct PixStrMain
{
	struct PixStr *ps;
	struct PixStrMain *next;
} PixStrMain;

/* ------------------------------------------------------------------------- */
	
/* Pixstrs for the advanced render pipe */
typedef struct PixStrExt
{
    struct PixStrExt *next;
    int vlak0, vlak;
    int type0, type;
    uint z;
    uint mask;
    short aantal, ronde;
} PixStrExt;

/* ------------------------------------------------------------------------- */

typedef struct PixStrExtMain
{
    struct PixStrExt *ps;
    struct PixStrExtMain *next;
} PixStrExtMain;

/* ------------------------------------------------------------------------- */

typedef struct ShadBuf {
	short samp, shadhalostep;
	float persmat[4][4];
	float viewmat[4][4];
	float winmat[4][4];
	float jit[25][2];
	float d,far,pixsize,soft;
	int co[3];
	int size,bias;
	ulong *zbuf;
	char *cbuf;
} ShadBuf;

/* ------------------------------------------------------------------------- */

typedef struct VertRen
{
	float co[3];
	float n[3];
	float ho[4];
	float *orco;
	float *sticky;
	void *svert;			/* smooth vert, only used during initrender */
	short clip, texofs;		/* texofs= flag */
} VertRen;

/* ------------------------------------------------------------------------- */

typedef struct VlakRen
{
	struct VertRen *v1, *v2, *v3, *v4;
	float n[3], len;
	Material *mat;
	MFace *mface;
	TFace *tface;
	uint *vcol;
	char snproj, puno;
	char flag, ec;
	uint lay;
} VlakRen;

/* ------------------------------------------------------------------------- */

typedef struct Render
{
	float co[3];
	float lo[3], gl[3], uv[3], ref[3], orn[3], winco[3], sticky[3], vcol[3];
	float itot, i, ic, rgb, norm;
	float vn[3], view[3], *vno, refcol[4];

	float grvec[3], inprz, inprh;
	float imat[3][3];

	float viewmat[4][4], viewinv[4][4];
	float persmat[4][4], persinv[4][4];
	float winmat[4][4];
	
	short flag, osatex, osa, rt;
   /**
    * Screen sizes and positions, in pixels
    */
	short xstart, xend, ystart, yend, afmx, afmy;
   short rectx;  /* Picture width - 1, normally xend - xstart. */  
   short recty;  /* picture height - 1, normally yend - ystart. */

  /**
   * Distances and sizes in world coordinates
   */
  float near;    /* near clip distance */
  float far;     /* far clip distance */
  float ycor, zcor, pixsize, viewfac;
	
	RenderData r;
	World wrld;
	ListBase parts;
	
	int totvlak, totvert, tothalo, totlamp;
	
	VlakRen *vlr;
	int vlaknr;
	
	Material *mat, *matren;
	LampRen **la;
	VlakRen **blovl;
	VertRen **blove;
	HaloRen **bloha;
	
	uint *rectaccu;
   uint *rectz; /* z buffer: distance buffer */
   uint *rectf1, *rectf2;
   uint *rectot; /* z buffer: face index buffer, recycled as colour buffer! */
   uint *rectspare; /*  */
	/* for 8 byte systems! */
	long *rectdaps;
	
	short win, winpos, winx, winy, winxof, winyof;
	short winpop, displaymode, sparex, sparey;
	
	Image *backbuf, *frontbuf;
	
} Render;

/* ------------------------------------------------------------------------- */

struct halosort {
	HaloRen *har;
	uint z;
};

/* ------------------------------------------------------------------------- */

#endif /* RENDER_TYPES_H */

