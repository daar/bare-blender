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

/* Version: $Id: sector.h,v 1.7 2000/08/31 00:05:18 ton Exp $ */

#ifndef SECTOR_H
#define SECTOR_H

#include "game.h"

#define LF_MAXBUF	128
#define LF_MAXLIFE	128

#define TOLER 0.000076

/* dface->flag */
#define DF_HILITE		1

/* dface->edcode: (zie ook mface) ook voor vertices! */
#define DF_V1			1
#define DF_V2			2
#define DF_V3			4
#define DF_V4			8
#define DF_V1V2			16
#define DF_V2V3			32
#define DF_V3V1			64
#define DF_V3V4			64
#define DF_V4V1			128

/* sn->flag */
#define FH_SECTOR	1
#define FH_PROP		2
#define FH_DYNA		3


#define DTIME	0.02
#define IDTIME	50.0
#define TOTKEY 32

/* ipo->type: gedeeltelijk flags */
#define	IP_LOC		1
#define	IP_ROT		2
#define	IP_SIZE		4
#define IP_OBCOL	8
#define	IP_FROMOB	15

/* lf->type */
#define LF_LINK	1


/* lf->flag1: buttons, sgistuff */

#define LF_CALC_MATRIX	2
#define LF_GHOST_OTHER	8
#define LF_DRAWNEAR		16
#define LF_AERO_AXIS_Y	256
#define LF_DO_FH		4096

/* lf->dflag : starten altijd op nul */
#define LF_TEMPLIFE		1
#define LF_DYNACHANGED	2
#define LF_OMEGA		4
#define LF_DONTDRAW		8
#define LF_TRIP_POS		16
#define LF_TRIP_NEG		32
#define LF_NO_DAMAGE	64
#define LF_ROT_LOCAL	128

/* lf->state: lokale vars: namen willekeurig? */
#define LF_STATE	1
#define LF_DAMAGE	2
#define LF_ACTION	3
#define LF_FRAME	4


typedef struct DFace {
		/* alleen gedurende simulatie */
	float *v1, *v2, *v3, *v4;
	float dist;
	Material *ma;
	float no[3];
	short proj;
	char flag, edcode;
	ushort ocx, ocy, ocz, rt;
	
} DFace;

typedef struct pIpo {
	void *next, *prev;

	short type, nr_keys;	/* keys staan achter ipodata */
	short nr_elems, sta;
	short elemsize, rt;
	int offs[3];			/* zodat loc-ipo's shortjes blijven */
} pIpo;


typedef struct Life {
	ID id;

	Object *sector;
	Object *ob;
	
	float oldloc[3], loc[3], speed[3];		/* loc ook in object zetten */
	float oldloc1[3], loc1[3], speed1[3];	/* lokale sector co's */
	float startloc[3], startrot[3];
	float rot[3], rotspeed[3];
	float oldimat[4][4];
	float mass, frict, rotfrict, axsize, frictfac;
	float r, g, b;
	float aero, localaxsize;
	
	float omega[3], force[3];
	float dloc[3], drot[3];
	
	Material *contact;
	Object *collision, *from;
	DFace *floor;
	Mesh *oldmesh;

	char type, lay;
	short flag1, pad;
	short pad2;
	int flag;
	
	short timer, sfra, cfra, dflag;		/* voor init/afhandeling ipoos, dflag: zit niet aan buttons */
	short state[4];						/* lokale variables */

	float colloc[3];					/* collision loc */
	float floorloc[3];
	
	LBuf links;							/* tijdens simul: de kinderen, in volgorde */
	
	ListBase ipo;
	
} Life;



/* sector.c */

extern Object *find_sector(float *vec, float *local);
extern void add_to_lbuf(LBuf *lbuf, Object *ob);
extern void free_lbuf(LBuf *lbuf);
extern void del_from_lbuf(LBuf *lbuf, Object *ob);
extern Object *find_sector(float *vec, float *local);
extern void life_in_sector_test(Object *ob);
extern void init_dynamesh(Object *ob, Mesh *me);
extern void end_dynamesh(Mesh *me);
extern int test_visibility(float *lookat, float *from, Life *lf, Object *se);
extern void add_dyna_life(Object *ob);
extern void add_dupli_life(Object *ob, Object *from, int time);
extern void del_dupli_life(Object *ob);
extern int cliptest_sector(float *vec, float *size, float *mat);
extern void build_sectorlist(Object *cam);

/* life.c */

extern void life_to_local_sector_co(Life *lf);
extern void life_from_inv_sector_co(Life *lf);
extern void aerodynamics(Object *ob);
extern int sca_handling(Object *ob, Life *lf);




#endif /* SECTOR_H */

