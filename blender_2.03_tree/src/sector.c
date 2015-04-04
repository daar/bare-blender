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



/*  sector.c      MIXED MODEL
 * 
 *  maart 96
 *  
 * 
 * Version: $Id: sector.c,v 1.34 2000/09/25 22:02:54 ton Exp $
 */

#include "blender.h"
#include "graphics.h"
#include "group.h"
#include "sector.h"

/* ******************** NOTES ************************** 

	- de interne klok loopt op 50 Hz.
	- variable klok: erg lelijk en instabiel: botsingen/krachten/ipoos hangen nauw samen (ook met events)
	- klok op 25 Hz: zou kunnen. Is dan lastig naar 17 Hz terug te brengen (alsie framerate niet haalt)
  

   ***************************************************** */

Object *Glifebuf[LF_MAXLIFE];
Object *Gsectorbuf[256], *Gcursector;
int Gtotsect, Gtotlife, Gmaxsect;
int Gdfra, Gdfras, Gdfrao;

/* debug */
int sectcount;
int matcount, spherecount, cylcount, facecount;

void del_dupli_life(Object *ob);
int (*intersect_func)();

Object *main_actor;
char *simul_load_str;

typedef struct Snijp {
	float labda, minlabda, inpspeed;
	float radius, radiusq, slen;/* genormaliseerd, lengte */
	float fhdist;				/* last used fhdist */
	DFace *face;				/* alleen fh */
	Material *ma;				
	float *obmat;				/* als hier iets staat: ook doen! */
	float no[3];				/* als flag==1, staat hier normaal */
	float speedn[3];			
	ushort ocx, ocy, ocz;		/* van de 'ray' of de life */
	char flag;					/* 0: vlak gesneden, 1: cylinder/sphere, 2:life */
	char lay;
} Snijp;


void ApplyMatrix(void *mat, float *old, float *new)
{
	
	/* if(sectcount<100) matcount++; */
	VecMat4MulVecfl(new, mat, old);
}


/* ******************** LBUF ************************** */


#define LBUFSTEP	4

void add_to_lbuf(LBuf *lbuf, Object *ob)
{
	
	if(lbuf->max==0) {
		lbuf->ob= callocN(sizeof(void *)*LBUFSTEP, "firstlbuf");
		lbuf->max= LBUFSTEP;
		lbuf->tot= 1;
		lbuf->ob[0]= ob;
	}
	else {
		Object **obar;
		
		/* dubbels: gewoon toestaan */
		
		if(lbuf->tot==lbuf->max) {
			lbuf->max+= LBUFSTEP;
			obar= callocN(sizeof(void *)*lbuf->max, "nlbuf");
			memcpy(obar, lbuf->ob, lbuf->max * sizeof(void *));
			freeN(lbuf->ob);
			lbuf->ob= obar;
		}
		lbuf->ob[ lbuf->tot ]= ob;
		lbuf->tot++;
		
	}
}

void free_lbuf(LBuf *lbuf)
{
	if(lbuf->ob) freeN(lbuf->ob);
	lbuf->ob= 0;
	lbuf->max= lbuf->tot= 0;
}

void del_from_lbuf(LBuf *lbuf, Object *ob)
{
	int a;
	
	if(lbuf->tot==0 || lbuf->max==0) return;
	
	for(a=0; a<lbuf->tot; a++) {
		
		/* dubbels: wel afvangen */
		
		if(lbuf->ob[a] == ob) {
			lbuf->tot--;
			if(lbuf->tot>a) {
				lbuf->ob[a] = lbuf->ob[ lbuf->tot ];
				lbuf->ob[ lbuf->tot ]= 0;				/* hoeft niet? */
			}
			else lbuf->ob[a]= 0;
		}
		
	}
	
}

/* ****************** OCTREE x:16 y:16 z:16 *************************** */

void init_snijp(Mesh *me, Snijp *sn, float *oldloc, float *speed)
{
	float vec[3], *dvec, *size;
	short a, b, min[3], max[3];
	
	/* optimalisering sphere en cyl */
	VECCOPY(sn->speedn, speed);
	sn->slen= Normalise(sn->speedn);

	sn->ocx= 0;		/* voldoende om hierop te testen */
					/* ivm. returns wel pas op 't eind berekenen */

	
	if(me==0 || me->oc==0 || sn->slen==0.0) return;
	
	dvec= me->oc->dvec;
	size= me->oc->size;
	
	vec[0]= oldloc[0]-dvec[0];
	vec[1]= oldloc[1]-dvec[1];
	vec[2]= oldloc[2]-dvec[2];
					
	if(size[0]!=0.0) {
		a= ffloor(16.0*(vec[0]-sn->radius)/size[0]);
		b= ffloor(16.0*(vec[0]-sn->radius+speed[0])/size[0]);
		min[0]= MIN2(a, b);

		a= ffloor(16.0*(vec[0]+sn->radius)/size[0]);
		b= ffloor(16.0*(vec[0]+sn->radius+speed[0])/size[0]);
		max[0]= MAX2(a, b);

		if(max[0]<0 || min[0]>15) return;
		if(max[0]>15) max[0]= 15;
		if(min[0]< 0) min[0]=  0;
	}
	else {min[0]=0; max[0]= 15;}
	
	if(size[1]!=0.0) {
		a= ffloor(16.0*(vec[1]-sn->radius)/size[1]);
		b= ffloor(16.0*(vec[1]-sn->radius+speed[1])/size[1]);
		min[1]= MIN2(a, b);

		a= ffloor(16.0*(vec[1]+sn->radius)/size[1]);
		b= ffloor(16.0*(vec[1]+sn->radius+speed[1])/size[1]);
		max[1]= MAX2(a, b);

		if(max[1]<0 || min[1]>15) return;
		if(max[1]>15) max[1]= 15;
		if(min[1]< 0) min[1]=  0;
	}
	else {min[1]=0; max[1]= 15;}
	
	if(size[2]!=0.0) {
		a= ffloor(16.0*(vec[2]-sn->radius)/size[2]);
		b= ffloor(16.0*(vec[2]-sn->radius+speed[2])/size[2]);
		min[2]= MIN2(a, b);

		a= ffloor(16.0*(vec[2]+sn->radius)/size[2]);
		b= ffloor(16.0*(vec[2]+sn->radius+speed[2])/size[2]);
		max[2]= MAX2(a, b);

		if(max[2]<0 || min[2]>15) return;
		if(max[2]>15) max[2]= 15;
		if(min[2]<0) min[2]= 0;
	}
	else {min[2]=0; max[2]= 15;}
	
	sn->ocx= BROW(min[0], max[0]);
	sn->ocy= BROW(min[1], max[1]);
	sn->ocz= BROW(min[2], max[2]);

}



/* ****************** INSIDE **************************** */

int sector_cliptest(Object *ob, float *vec)
{
	float centre[3], size[3];
	
	get_local_bounds(ob, centre, size);
	
	if(vec[0] < centre[0]-size[0]) return 1;
	if(vec[1] < centre[1]-size[1]) return 1;
	if(vec[2] < centre[2]-size[2]) return 1;
	if(vec[0] > centre[0]+size[0]) return 1;
	if(vec[1] > centre[1]+size[1]) return 1;
	if(vec[2] > centre[2]+size[2]) return 1;
	
	return 0;
}

int sector_cliptest_sphere(Object *ob, float *vec, float radius)
{
	float centre[3], size[3];
	
	get_local_bounds(ob, centre, size);
	
	if(vec[0] < centre[0]-size[0]-radius) return 1;
	if(vec[1] < centre[1]-size[1]-radius) return 1;
	if(vec[2] < centre[2]-size[2]-radius) return 1;
	if(vec[0] > centre[0]+size[0]-radius) return 1;
	if(vec[1] > centre[1]+size[1]-radius) return 1;
	if(vec[2] > centre[2]+size[2]-radius) return 1;
	
	return 0;
}

int sector_inside(Object *ob, float *vec, float *local)		/* vec is globaal */
{

	if(ob==0) return 0;
	ApplyMatrix(ob->imat, vec, local);
	if( sector_cliptest(ob, local)==0 ) return 1;
	return 0;
}


Object *find_sector(float *vec, float *local)			/* algemene find */
{
	Base *base;
	
	base= FIRSTBASE;
	while(base) {
		if(base->lay & G.scene->lay) {
			if(base->object->gameflag & OB_SECTOR) {
				
				if(sector_inside(base->object, vec, local)) {
					return base->object;
				}
			}
		}
		base= base->next;
	}
	return 0;
}


void life_in_sector_test(Object *ob)
{
	Life *lf;
	Object *se, *obp;
	int b, out=0;
	
	lf= ob->life;
	if(lf==NULL || (lf->flag & OB_DYNAMIC)==0) return;

	/* apart afhandelen voor PROPS? */
	
	/* nog steeds in de huidige sector? */
	if(lf->sector) {
		
		out= sector_cliptest(lf->sector, lf->loc1);

		if(out) {
			
			del_from_lbuf( &(lf->sector->lbuf), ob);
			
			b= lf->sector->port.tot;
			while(b--) {
				obp= lf->sector->port.ob[b];
				ApplyMatrix(obp->imat, lf->loc, lf->loc1);
				out= sector_cliptest(obp, lf->loc1);
				if(out==0) {
					lf->sector= obp;
					if((lf->flag & OB_MAINACTOR)==0) {
						if((lf->dflag & LF_TEMPLIFE)==0) {
							add_to_lbuf( &(lf->sector->lbuf), ob);
						}
					}
					break;
				}
			}
		}
	}
	
	if(out || lf->sector==NULL) {

		se= find_sector(lf->loc, lf->loc1);
		if(se) {
			lf->sector= se;
			if((lf->flag & OB_MAINACTOR)==0) {
				if((lf->dflag & LF_TEMPLIFE)==0) {
					add_to_lbuf( &(lf->sector->lbuf), ob);
				}
			}
			/* printf("out, but found one\n"); */
			return;
		}

		/* helaas: */
		
		if(lf->dflag & LF_TEMPLIFE) {
			lf->timer= 0;
		}
		else {
			/* printf("out\n"); */
			
			if(ob==main_actor) {
				lf->loc1[0]= lf->loc1[1]= 0;
				lf->loc1[2]= lf->axsize;
			}
			lf->speed[0]= lf->speed[1]= lf->speed[2]= 0;
			
			/*  misschien speed flippen? twijfelachtig */
		}
	}
}

void normal_rot_inv(mat, inv, no)
float mat[][4], inv[][4], *no;
{
	float x, y, z;
	
	x= no[0]*mat[0][0] + no[1]*mat[1][0] + no[2]*mat[2][0];
	y= no[0]*mat[0][1] + no[1]*mat[1][1] + no[2]*mat[2][1];
	z= no[0]*mat[0][2] + no[1]*mat[1][2] + no[2]*mat[2][2];
	
	no[0]= x*inv[0][0] + y*inv[1][0] + z*inv[2][0];
	no[1]= x*inv[0][1] + y*inv[1][1] + z*inv[2][1];
	no[2]= x*inv[0][2] + y*inv[1][2] + z*inv[2][2];
	
	/* geen normalise: inpspeed bevat scale-informatie */
	/* Normalise(no); */
}



/* ********************** DYNA ************************* */


#define UNSUREF	0.02

/*					  labdacor  labda			*/
/*                 -----x---------x-------		*/
/* 						.		  .				*/
/* geval 1         -----.---->    .				*/
/* geval 2            --.---------.-->			*/
/* geval 3              .     ----.------>		*/
/* geval 4              .  -----> .				*/


int intersect_dface(DFace *dface, Snijp *sn, float *oldloc, float *speed)
{
	float ndist, labda, labdacor, s, t, inploc, inpspeed;
	int cox=0, coy=1;


	/* als dface->v3==0 : dface->ocx==0 */
	if( (dface->ocx & sn->ocx)==0 ) return 0;
	if( (dface->ocy & sn->ocy)==0 ) return 0;
	if( (dface->ocz & sn->ocz)==0 ) return 0;

	/* if((sn->lay & dface->ma->lay)==0) return 0; */

	inpspeed= dface->no[0]*speed[0] + dface->no[1]*speed[1] + dface->no[2]*speed[2];

	/* single sided! */	
	if(inpspeed < -TOLER) {
	
		inploc= dface->no[0]*oldloc[0] + dface->no[1]*oldloc[1] + dface->no[2]*oldloc[2];
		ndist= dface->dist - inploc;	/* negatief! */
		
		if(ndist>0.0) return 0;
		
		labda= (ndist)/inpspeed;	/* voor s-t */
		labdacor= (ndist+UNSUREF)/inpspeed;

		/* labda is gegarandeerd > 0! (ndist<0.0) */
		
		/* dit is een soort interval-wiskunde */
		if(labdacor<=sn->minlabda) {
			if(labda<=sn->minlabda) return 0;
		}
		if(labdacor>=sn->labda) {
			if(labda>= sn->labda) return 0;
		}
		
		if(dface->proj==1) coy= 2;
		else if(dface->proj==2) {
			cox= 1; coy= 2;
		}

		s= oldloc[cox] + labda*speed[cox];
		t= oldloc[coy] + labda*speed[coy];

		if( (dface->v2[cox] - s)*(dface->v2[coy] - dface->v1[coy]) - 
			(dface->v2[coy] - t)*(dface->v2[cox] - dface->v1[cox]) < -TOLER ) 
			return 0;

		if( (dface->v3[cox] - s)*(dface->v3[coy] - dface->v2[coy]) -
			(dface->v3[coy] - t)*(dface->v3[cox] - dface->v2[cox])< -TOLER ) 
			return 0;
		
		if(dface->v4==0) {
			if( (dface->v1[cox] - s)*(dface->v1[coy] - dface->v3[coy]) -
				(dface->v1[coy] - t)*(dface->v1[cox] - dface->v3[cox])< -TOLER ) 
				return 0;
		}
		else {
			if( (dface->v4[cox] - s)*(dface->v4[coy] - dface->v3[coy]) -
				(dface->v4[coy] - t)*(dface->v4[cox] - dface->v3[cox])< -TOLER ) 
				return 0;

			if( (dface->v1[cox] - s)*(dface->v1[coy] - dface->v4[coy]) -
				(dface->v1[coy] - t)*(dface->v1[cox] - dface->v4[cox])< -TOLER )
				return 0;
		}

		sn->labda= labdacor;
		sn->inpspeed= inpspeed;
		sn->face= dface;
		sn->flag= 0;
		
		return 1;
	}
	
	return 0;
}

/* genormaliseerde speed en kwadratische straal in sn-struct? inclusief lengte speed */

typedef struct pSnijp {
	int labda, minlabda, inpspeed;
	int radius, radiusq, slen;	/* slen: lengte speed */
	short no[3];				/* als flag==1, staat hier normaal */
	short speedn[3];			/* genormaliseerd */
	ushort ocx, ocy, ocz;		/* van de 'ray' of de life */
	short flag;					/* 0: vlak gesneden, 1: cylinder/sphere */
} pSnijp;

#define FIX(fac)	((int)(4096.0*fac))
#define FIX14(fac)	((int)(16384.0*fac))
#define S14MUL(a, b)           ( ((a)*(b))>>14 )

void fix_coordvec(int *ip, float *fp)
{
	ip[0]= FIX(fp[0]);
	ip[1]= FIX(fp[1]);
	ip[2]= FIX(fp[2]);
	
}

void fix_coordvecs(short *ip, float *fp)
{
	ip[0]= FIX(fp[0]);
	ip[1]= FIX(fp[1]);
	ip[2]= FIX(fp[2]);
}

int SquareRoot0(int num)
{
	return (int)fsqrt( (float)num);
}

int Normalises(short *vec)	/* 12 bits */
{
	float fp[3], len;
	
	VECCOPY(fp, vec);
	len= Normalise(fp);
	vec[0]= FIX(fp[0]);
	vec[1]= FIX(fp[1]);
	vec[2]= FIX(fp[2]);
	
	return (int)(len);
}

void CrossS(c, a, b)
short *c, *a, *b;
{
	c[0] = (a[1] * b[2] - a[2] * b[1])>>12;
	c[1] = (a[2] * b[0] - a[0] * b[2])>>12;
	c[2] = (a[0] * b[1] - a[1] * b[0])>>12;
}


int sphere_sphere_intersect(float *v1, Snijp *sn, float *oldloc, float *speed)
{
	float labda, labdacor, bsq, u, disc, rc[3];
	float radius, radiusq;
	
	radius= v1[3]+sn->radius;
	radiusq= radius*radius;
				
	rc[0]= oldloc[0]-v1[0];
	rc[1]= oldloc[1]-v1[1];
	rc[2]= oldloc[2]-v1[2];
	bsq= rc[0]*sn->speedn[0] + rc[1]*sn->speedn[1] + rc[2]*sn->speedn[2]; 
	u= rc[0]*rc[0] + rc[1]*rc[1] + rc[2]*rc[2] - radiusq;

	disc= bsq*bsq - u;

	if(disc>=0.0) {
		disc= fsqrt(disc);
		labdacor= (-bsq - disc)/sn->slen;	/* intrede */
		labda= (-bsq + disc)/sn->slen;
	}
	else return 0;	


	/* twee gevallen waarbij geen snijpunt is */
	if(labdacor>=sn->labda && labda>=sn->labda) return 0;
	if(labdacor<=sn->minlabda && labda<=sn->minlabda) return 0;

	/* PATCH!!! */
	if(labdacor<0.0) labdacor/= 2.0;


	/* snijpunt en normaal */
	sn->no[0]= rc[0] + labdacor*speed[0] ;
	sn->no[1]= rc[1] + labdacor*speed[1] ;
	sn->no[2]= rc[2] + labdacor*speed[2] ;

	/* corrigeren: v1[3]= axsize */
	disc= v1[3]/radius;
	sn->no[0]*= disc;
	sn->no[1]*= disc;
	sn->no[2]*= disc;

	sn->labda= labdacor;
	
	/* inpspeed op lengte brengen: twee keer radius!!! (inpspeed wordt weer met normaal vermenigvuldigd)*/
	radiusq= v1[3]*v1[3];
	sn->inpspeed= (sn->no[0]*speed[0] + sn->no[1]*speed[1] + sn->no[2]*speed[2])/(radiusq);

	sn->flag= FH_DYNA;

	return 1;
}


int vertex_sphere_intersect(float *v1, Snijp *sn, float *oldloc, float *speed)
{
	float labda, labdacor, bsq, u, disc, rc[3];


	rc[0]= oldloc[0]-v1[0];
	rc[1]= oldloc[1]-v1[1];
	rc[2]= oldloc[2]-v1[2];
	bsq= rc[0]*sn->speedn[0] + rc[1]*sn->speedn[1] + rc[2]*sn->speedn[2]; 
	u= rc[0]*rc[0] + rc[1]*rc[1] + rc[2]*rc[2] - sn->radiusq;

	disc= bsq*bsq - u;

	if(disc>=0.0) {
		disc= fsqrt(disc);
		labdacor= (-bsq - disc)/sn->slen;	/* intrede */
		labda= (-bsq + disc)/sn->slen;
	}
	else return 0;	

	/* twee gevallen waarbij geen snijpunt is */
	if(labdacor>=sn->labda && labda>=sn->labda) return 0;
	if(labdacor<=sn->minlabda && labda<=sn->minlabda) return 0;

/* PRINT2(f, f, labdacor, labda); */

	/* snijpunt en normaal */
	sn->no[0]= rc[0] + labdacor*speed[0] ;
	sn->no[1]= rc[1] + labdacor*speed[1] ;
	sn->no[2]= rc[2] + labdacor*speed[2] ;

/* PRINT3(f, f, f, sn->no[0], sn->no[1], sn->no[2]); */

	sn->labda= labdacor;
	/* inpspeed op lengte brengen: twee keer radius!!! (inpspeed wordt weer met normaal vermenigvuldigd)*/
	sn->inpspeed= (sn->no[0]*speed[0] + sn->no[1]*speed[1] + sn->no[2]*speed[2])/(sn->radiusq);
	sn->flag= FH_SECTOR;

/* PRINT(f, sn->inpspeed); */
	

	if(FALSE) {	/* namaak refl */
		float fac, speed1[3];
		
		VECCOPY(speed1, speed);
		
		fac= (-2.0)*sn->inpspeed;
	
		speed1[0]+= fac*sn->no[0];
		speed1[1]+= fac*sn->no[1];
		speed1[2]+= fac*sn->no[2];
		
		PRINT3(f, f, f, speed1[0], speed1[1], speed1[2]);
	}

	return 1;
}

int cylinder_edge_intersect(float *base, float *v2, Snijp *sn, float *oldloc, float *speed)
{
	float s, t, u,dist, len, len2, labda, labdacor, axis[3], rc[3], n[3], o[3];
	
	axis[0]= v2[0]-base[0];
	axis[1]= v2[1]-base[1];
	axis[2]= v2[2]-base[2];
	len2= Normalise(axis);
	if(len2<TOLER) return 0;

	rc[0]= oldloc[0]-base[0];
	rc[1]= oldloc[1]-base[1];
	rc[2]= oldloc[2]-base[2];

	/* if(sectcount<100) cylcount++; */

	Crossf(n, speed, axis);
	len= Normalise(n);
	if(len<TOLER) return 0;

	dist= fabs( rc[0]*n[0] + rc[1]*n[1] + rc[2]*n[2] );

	/* PRINT2(f, f, dist, sn->radius); */
	
	if( dist>=sn->radius ) return 0;
	
	Crossf(o, rc, axis);
	t= -(o[0]*n[0] + o[1]*n[1] + o[2]*n[2])/len;
	
	Crossf(o, n, axis);
    
    u= (o[0]*speed[0] + o[1]*speed[1] + o[2]*speed[2]);
    if(u<-TOLER || u>TOLER)
		s=  fabs(safsqrt( sn->radiusq - dist*dist) / u);
    else 
    	return 0;

	/* PRINT2(f, f, s, t );	 */

	labdacor= (t-s);
	labda= (t+s);

/* PRINT2(f, f, labdacor, labda); */

	/* twee gevallen waarbij geen snijpunt is */
	if(labdacor>=sn->labda && labda>=sn->labda) return 0;
	if(labdacor<=sn->minlabda && labda<=sn->minlabda) return 0;
	
	/* normaalvector berekenen */
	/* snijpunt: */
	
	rc[0]= rc[0] + labdacor*speed[0] ;
	rc[1]= rc[1] + labdacor*speed[1] ;
	rc[2]= rc[2] + labdacor*speed[2] ;
	
	s= (rc[0]*axis[0] + rc[1]*axis[1] + rc[2]*axis[2]) ;
	
	if(s<0.0 || s>len2) return 0;
	
	/* tot aan de laatste return niets in de sn struct invullen! */	

	sn->no[0]= (rc[0] - s*axis[0]);
	sn->no[1]= (rc[1] - s*axis[1]);
	sn->no[2]= (rc[2] - s*axis[2]);
		
	sn->labda= labdacor;
	sn->inpspeed= (sn->no[0]*speed[0] + sn->no[1]*speed[1] + sn->no[2]*speed[2])/sn->radiusq;
	sn->flag= FH_SECTOR;

/* PRINT(f, sn->inpspeed); */

	if(FALSE) {	/* namaak refl */
		float fac, speed1[3];
		
		VECCOPY(speed1, speed);
		
		fac= (-2.0)*sn->inpspeed;
	
		speed1[0]+= fac*sn->no[0];
		speed1[1]+= fac*sn->no[1];
		speed1[2]+= fac*sn->no[2];
		
		PRINT3(f, f, f, speed1[0], speed1[1], speed1[2]);
	}

	return 1;
}

int intersect_dface_cyl(DFace *dface, Snijp *sn, float *oldloc, float *speed)
{
	float *v1, *v2, *v3, *v4;
	float ndist, labda, labdacor, s, t, inploc, inpspeed;
	float out;
	short cox=0, coy=1, ok, ed, cytest;
	
	/* als dface->v3==0 : dface->ocx==0 */
	if( (dface->ocx & sn->ocx)==0 ) return 0;
	if( (dface->ocy & sn->ocy)==0 ) return 0;
	if( (dface->ocz & sn->ocz)==0 ) return 0;
	

	/* if((sn->lay & dface->ma->lay)==0) return 0; */
/* dface->flag |= DF_HILITE;	 */

	inpspeed= dface->no[0]*speed[0] + dface->no[1]*speed[1] + dface->no[2]*speed[2];

	/* single sided! */	
	if(inpspeed < -TOLER) {
	
		inploc= dface->no[0]*oldloc[0] + dface->no[1]*oldloc[1] + dface->no[2]*oldloc[2];
		ndist= dface->dist - inploc;	/* negatief! */
		
		if(ndist>0.0) return 0;
		
		labda= (ndist)/inpspeed;	/* voor s-t */
		labdacor= (ndist+sn->radius)/inpspeed;

		/* labda is gegarandeerd > 0! (ndist<0.0) */
		
		/* twee gevallen waarbij geen snijpunt is */
		if(labdacor>=sn->labda && labda>=sn->labda) return 0;
		if(labdacor<=sn->minlabda && labda<=sn->minlabda) return 0;
		
		if(dface->proj==1) coy= 2;
		else if(dface->proj==2) {
			cox= 1; coy= 2;
		}
		
		/* testen oftie exact het vlak snijdt */
		if(labdacor<=0.0) {
			/* geen labda, wel met correctie zodat snijp binnen vlak valt */
			/* dit is in feite de loodrechte projektie */
			s= oldloc[cox] - sn->radius*dface->no[cox];
			t= oldloc[coy] - sn->radius*dface->no[coy];
			labdacor= 0.0;
		}
		else {
			/* met correctie zodat snijp binnen vlak valt */
			s= oldloc[cox] + labdacor*speed[cox] - sn->radius*dface->no[cox];
			t= oldloc[coy] + labdacor*speed[coy] - sn->radius*dface->no[coy];
		}

		/* if(sectcount<100) facecount++; */
		
		v1= dface->v1;
		v2= dface->v2;
		v3= dface->v3;
		v4= dface->v4;
		
		cytest= 0;
		out= (v2[cox] - s)*(v2[coy] - v1[coy]) - (v2[coy] - t)*(v2[cox] - v1[cox]);
		if(out > -TOLER) {
			out= (v3[cox] - s)*(v3[coy] - v2[coy]) - (v3[coy] - t)*(v3[cox] - v2[cox]);
			if(out > -TOLER) {
				
				ok= 0;
				
				if(v4==0) {
					out= (v1[cox] - s)*(v1[coy] - v3[coy]) - (v1[coy] - t)*(v1[cox] - v3[cox]);
					if(out > -TOLER) ok= 1;
					else cytest= 3;
				}
				else {
					out= (v4[cox] - s)*(v4[coy] - v3[coy]) - (v4[coy] - t)*(v4[cox] - v3[cox]);

					if(out > -TOLER) {
						out= (v1[cox] - s)*(v1[coy] - v4[coy]) - (v1[coy] - t)*(v1[cox] - v4[cox]);

						if(out > -TOLER) ok= 1;
						else cytest= 5;
					}
					else cytest= 4;
				}
				if(ok) {

					sn->labda= labdacor;
					sn->inpspeed= inpspeed;
					sn->face= dface;
					sn->flag= 0;
					return 1;
				}
			}
			else cytest= 2;
		}
		else cytest= 1;
	
		/* edges (= cylinders) testen */
		ok= 0;
		ed= dface->edcode;
		
		switch(cytest) {
		case 1:
			if( (ed & DF_V1V2) && cylinder_edge_intersect(v1, v2, sn, oldloc, speed) )
				ok= 1;
			break;
		case 2:
			if( (ed & DF_V2V3) && cylinder_edge_intersect(v2, v3, sn, oldloc, speed) )
				ok= 1;
			break;
		case 3:
			if( (ed & DF_V3V1) && cylinder_edge_intersect(v3, v1, sn, oldloc, speed) )
				ok= 1;
			break;
		case 4:
			if( (ed & DF_V3V4) && cylinder_edge_intersect(v3, v4, sn, oldloc, speed) )
				ok= 1;
			break;
		case 5:
			if( (ed & DF_V4V1) && cylinder_edge_intersect(v4, v1, sn, oldloc, speed) )
				ok= 1;
			break;
		}

		if(ok) sn->face= dface;
		
		if(ok==0) {
			if( (ed & DF_V1) && vertex_sphere_intersect(v1, sn, oldloc, speed)) {
				sn->face= dface;
				
				return 1;
			}
			if( (ed & DF_V2) && vertex_sphere_intersect(v2, sn, oldloc, speed)) {
				sn->face= dface;
				
				return 1;
			}
			if( (ed & DF_V3) && vertex_sphere_intersect(v3, sn, oldloc, speed)) {
				sn->face= dface;
				
				return 1;
			}
			if(v4) if( (ed & DF_V4) && vertex_sphere_intersect(v4, sn, oldloc, speed)) {
				sn->face= dface;
				
				return 1;
			}
			
		}
		
		/* PATCH: onderzoeken hoe en wat en waarom: extreem negatieve labda's */
		/* waarschijnlijk iets van doen met meerdere snijps? */
		
		if(sn->labda<sn->minlabda) sn->labda= sn->minlabda;

		return ok;
	}
	
	return 0;
}

int actor_bounds_overlap(Object *ob1, Object *ob2)
{
	float axsize1, axsize2;
	float centre1[3], size[3];
	float centre2[3];

	if(ob1->gameflag & OB_DYNAMIC) {
		axsize1= ob1->life->axsize;
		VECCOPY(centre1, ob1->life->loc);
	}
	else {
		get_local_bounds(ob1, centre1, size);
		axsize1= MAX3(size[0]+fabs(centre1[0]), size[1]+fabs(centre1[1]), size[2]+fabs(centre1[2]));
		axsize1*= 1.41/ob1->sizefac;
		VECCOPY(centre1, ob1->obmat[3]);
	}

	if(ob2->gameflag & OB_DYNAMIC) {
		axsize2= ob2->life->axsize;
		VECCOPY(centre2, ob2->life->loc);
	}
	else {
		get_local_bounds(ob2, centre2, size);
		axsize2= MAX3(size[0]+fabs(centre2[0]), size[1]+fabs(centre2[1]), size[2]+fabs(centre2[2]));
		axsize2*= 1.41/ob2->sizefac;
		VECCOPY(centre2, ob2->obmat[3]);
	}

	if( centre1[0] + axsize1 < centre2[0] - axsize2) return 0;
	if( centre1[0] - axsize1 > centre2[0] + axsize2) return 0;
	
	if( centre1[1] + axsize1 < centre2[1] - axsize2) return 0;
	if( centre1[1] - axsize1 > centre2[1] + axsize2) return 0;
	
	if( centre1[2] + axsize1 < centre2[2] - axsize2) return 0;
	if( centre1[2] - axsize1 > centre2[2] + axsize2) return 0;
	
	return 1;
}

int intersect_dynalife(Object *ob, Life *lf, Snijp *sn)
{
	extern Material defmaterial;
	Object *obs;
	Life *lfs;
	float fac, mindist, vec[3], loc1[4];		/* loc1 is incl axsize */
	int a, b, found= 0;

	/* nodig voor genormaliseerde speed */
	init_snijp(0, sn, lf->oldloc1, lf->speed1);

	if(sn->slen < 0.0001) return 0;
	
	a= Gtotlife;
	while(a--) {
	
		obs= Glifebuf[a];
		if(obs->life && (obs->lay & ob->lay)) {
			lfs= obs->life;
			/* not intersect with itself or dupli's with the 'mother' */
			if(lf!=lfs && lfs->collision!=ob && lf->from!=obs && lfs->from!=ob && (lfs->flag & OB_DYNAMIC)) {
				
				/* manhattan pre-test */
				/* 3.0, for speed vector! */
				mindist= 3.0*(sn->radius+lfs->axsize);
				
				if( fabs(lf->loc[0]-lf->loc[0]) > mindist ) continue;
				if( fabs(lf->loc[1]-lf->loc[1]) > mindist ) continue;
				if( fabs(lf->loc[2]-lf->loc[2]) > mindist ) continue;
				
				/* gebruik lokale sector coords */
				ApplyMatrix(lf->sector->imat, lfs->loc, loc1);
				loc1[3]= lf->sector->sizefac*lfs->axsize;
			
				/* truuk: bollen met stralen r1 en r2 snijden is equiv. 
				   met lijn snijden met bol van straal r1+r2 ! */
				
				if(sphere_sphere_intersect(loc1, sn, lf->oldloc1, lf->speed1)) {
					found= 1;
					
					sn->ma= give_current_material(ob, 1);
					if(sn->ma==NULL) sn->ma= &defmaterial;
					sn->obmat= 0;
					sn->face= 0;
					
					lf->collision= obs;
					lfs->collision= ob;
					
					if(sn->ma->reflect!=0.0) {
						/* each dyna half the speed */
						VecMulf(sn->no, 0.5);
						
						fac= fsqrt(lf->speed[0]*lf->speed[0] + lf->speed[1]*lf->speed[1] + lf->speed[2]*lf->speed[2] );
						fac *= sn->ma->reflect;
						
						/* rotate to global coords */
						vec[0]= -sn->no[0]*fac;
						vec[1]= -sn->no[1]*fac;
						vec[2]= -sn->no[2]*fac;
						
						Mat4Mul3Vecfl(lfs->sector->obmat, vec);
						
						obs->life->force[0]+= vec[0];
						obs->life->force[1]+= vec[1];
						obs->life->force[2]+= vec[2];
					}
					
				}
			}
		}
	}
	
	/* hier geen patch: in ruil voor labdacor delen door 2 (sphere_sphere_intersect) */
		
	return found;
}


int intersect_prop(Object *se, Life *lf, Snijp *sn)
{
	/* botst Life *lf tegen een van de props uit *se ? */
	Object *ob;
	Life *lfs;
	DFace *dface;
	float oldloc2[3], loc2[3], speed2[3];
	int a, b, found= 0;

	for(b=0; b<se->lbuf.tot; b++) {
		ob= se->lbuf.ob[b];
		
		if(ob->lay & G.scene->lay) {	/* ivm layer event */
			
			lfs= ob->life;

			if(lfs && lf!=lfs && ob->type==OB_MESH && (lfs->flag & OB_PROP)) {
				if( actor_bounds_overlap(lf->ob, ob) ) {
				
					Mesh *me= ob->data;
					
					/* transformeren naar life coordinaten */
					ApplyMatrix(ob->imat, lf->loc, loc2);
					ApplyMatrix(ob->imat, lf->oldloc, oldloc2);
	
					sn->radius= ob->sizefac*lf->axsize;
					sn->radiusq= sn->radius*sn->radius;
	
					speed2[0]= loc2[0]-oldloc2[0];
					speed2[1]= loc2[1]-oldloc2[1];
					speed2[2]= loc2[2]-oldloc2[2];
		
					init_snijp(me, sn, oldloc2, speed2);
					if(sn->ocx) {
						dface= me->dface;
						a= me->totface;
	
						while(a--) {
							if( intersect_func(dface, sn, oldloc2, speed2)) {
								sn->obmat= ob->obmat[0];
								lf->collision= ob;	
								lfs->collision= lf->ob;	
								found= 1;
							}
							dface++;
						}
					}
				}
			}
		}
	}
	return found;
}

/* return 1: matrices differ */
int matrix_differ(float mat1[][4], float mat2[][4])
{
	/* pretty stupid, but still effective to avoid the 'gluing' on an edge (should be fixed!) */
	int a, b;
	
	for(a=0; a<4; a++) {
		for(b=0; b<4; b++) {
			if(mat1[a][b] != mat2[a][b]) return 1;
		}
	}
	return 0;
}

void force_from_prop(Life *lf, Snijp *sn)	/* maar 1 tegelijk!!! */
{
	Object *ob;
	Life *lfs;
	Object *se;
	DFace *dface;
	float fac, oldloc2[3], loc2[3], speed2[3], *no;
	int a, b, found;

	/* struct sn wordt doorgegeven vanwege de reeds ingevulde radius */

	/* mogelijke denkfout: het eerste de beste vlak wordt gepakt! is soms
	 * niet het vlak met de meeste/juiste force...
	 * Voorlopig afhandelen door Fh ook bij props te doen?
	 */

	se= lf->sector;

	for(b=0; b<se->lbuf.tot; b++) {
		ob= se->lbuf.ob[b];
		
		if(G.scene->lay & ob->lay) {
			lfs= ob->life;
	
			if(lfs && lf!=lfs && (lfs->dflag & LF_DYNACHANGED)) {
				if( (lfs->flag & OB_PROP) && ob->type==OB_MESH) {
					if( actor_bounds_overlap(ob, lf->ob) ) {
					if(matrix_differ(lfs->oldimat, ob->imat)) {
						Mesh *me= ob->data;
						
						sn->minlabda= 0.0;
						sn->labda= 1.0;
						found= 0;
						
						/* werken aan de hand van de 'virtuele' vorige positie van life */
						ApplyMatrix(ob->imat, lf->loc, loc2);
						ApplyMatrix(lfs->oldimat, lf->oldloc, oldloc2);

						sn->radius= ob->sizefac*lf->axsize;
						sn->radiusq= sn->radius*sn->radius;
	
						speed2[0]= loc2[0]-oldloc2[0];
						speed2[1]= loc2[1]-oldloc2[1];
						speed2[2]= loc2[2]-oldloc2[2];

						init_snijp(me, sn, oldloc2, speed2);
						if(sn->ocx) {
						
							dface= me->dface;
							a= me->totface;
							
							while(a--) {
								if( intersect_func(dface, sn, oldloc2, speed2)) {
									found= 1;
								}
								dface++;
							}
		
							if(found) {
			
								/* de bots loc */
								oldloc2[0]+= sn->labda*speed2[0];
								oldloc2[1]+= sn->labda*speed2[1];
								oldloc2[2]+= sn->labda*speed2[2];
								
								/* nieuwe speed, hier refl.demping: -1.0: parrallel aan vlak, -2.0: zuivere botsing */
								
								fac= (-1.0- (sn->face->ma->reflect))*sn->inpspeed;
								
								/* oplossing: hier stond +=, komt neer op min speed2 !!! */
								/* min speed2= exact de speed van het botsvlak!!! */
	
								if(sn->flag==0) no= sn->face->no; else no= sn->no;
								
								speed2[0]= fac*no[0];
								speed2[1]= fac*no[1];
								speed2[2]= fac*no[2];
					
								/* de virtuele oldloc */
								oldloc2[0]-= sn->labda*speed2[0];
								oldloc2[1]-= sn->labda*speed2[1];
								oldloc2[2]-= sn->labda*speed2[2];
				
								/* de (nieuwe) eindlocatie */
								loc2[0]= oldloc2[0]+speed2[0];
								loc2[1]= oldloc2[1]+speed2[1];
								loc2[2]= oldloc2[2]+speed2[2];
						
								ApplyMatrix(ob->obmat, loc2, lf->loc);
								
								Mat4Mul3Vecfl(ob->obmat, speed2);
			
								VECCOPY(lf->speed, speed2);
								/* oldloc niet meer nodig */
			
								return;
							}
						}
					}}
				}
			}
		}
	}
}


void collision_detect(Object *ob, Life *lf)
{
	/* een soort reetrees routine: zoek dichtstbijzijnde snijpunt */
	/* werken met de lokale life co's */
	Mesh *me;
	Object *se;
	DFace *dface, *from=0;
	float *oldloc1, *speed1, *no, fac, len;
	Snijp sn;
	int a, b, found=1, transback=0, colcount=0;

	lf->collision= 0;

	se= lf->sector;
	me= se->data;
	if(me==0) return;

	sn.minlabda= 0.0;
	sn.lay= lf->lay;
	
	intersect_func= intersect_dface_cyl;

	/* beetje speedup */
	oldloc1= lf->oldloc1;
	speed1= lf->speed1;

	while(found) {

		sn.labda= 1.0;
		sn.obmat= 0;
		sn.radius= lf->localaxsize;
		sn.radiusq= sn.radius*sn.radius;

		found= 0;
		
		init_snijp(me, &sn, oldloc1, speed1);
		if(sn.ocx) {

			dface= me->dface;
			a= me->totface;

			while(a--) {
				if(dface!=from) {
					if( intersect_func(dface, &sn, oldloc1, speed1)) found= 1;
				}
				dface++;
			}

			if(found) lf->collision= se;
		}
		
		/* de prop lifes */
		/* tijdelijk lopen we ze allemaal af */
		/* toch maar vast de globale opnieuw berekenen... */
		if(transback) {
			life_from_inv_sector_co(lf);
			transback= 0;
		}

		if(se->lbuf.tot) found |= intersect_prop(se, lf, &sn);
		
		found |= intersect_dynalife(ob, lf, &sn);
		
		/* als loc buiten sector ligt, doen ook vlakken van portals mee */
		b= se->port.tot;
		while(b--) {
			Object *obp= se->port.ob[b];
			Mesh *me= obp->data;
			float oldloc2[3], loc2[3], speed2[3];

			/* toch maar vast de globale opnieuw berekenen want... */
			if(transback) {
				life_from_inv_sector_co(lf);
				transback= 0;
			}
			
			/* ... we moeten transformeren naar nieuwe locale coordinaten */

			ApplyMatrix(obp->imat, lf->loc, loc2);
			ApplyMatrix(obp->imat, lf->oldloc, oldloc2);

			speed2[0]= loc2[0]-oldloc2[0];
			speed2[1]= loc2[1]-oldloc2[1];
			speed2[2]= loc2[2]-oldloc2[2];

			init_snijp(me, &sn, oldloc2, speed2);
			if(sn.ocx) {

				dface= me->dface;
				a= me->totface;
			
				while(a--) {
					if(dface!=from) {
						if( intersect_func(dface, &sn, oldloc2, speed2)) {
							sn.obmat= obp->obmat[0];
							lf->collision= obp;
							found= 1;
						}
					}
					dface++;
				}

				if(obp->lbuf.tot) found |= intersect_prop(obp, lf, &sn);
			}
		}
		
		if(found) {
			colcount++;
		
			transback= 1;
			from= sn.face;
			
			/* de bots loc */
			lf->colloc[0]= oldloc1[0]+ sn.labda*speed1[0];
			lf->colloc[1]= oldloc1[1]+ sn.labda*speed1[1];
			lf->colloc[2]= oldloc1[2]+ sn.labda*speed1[2];
		
			if(sn.flag==0) {
				no= sn.face->no; 
				sn.ma= sn.face->ma;
			}
			else {
				no= sn.no;
				if(sn.flag==FH_SECTOR) sn.ma= sn.face->ma;
			}

			/* nieuwe speed, hier refl.demping: -1.0: parrallel aan vlak, -2.0: zuivere botsing */
			fac= (-1.0- (sn.ma->reflect))*sn.inpspeed;
			
			/* pas op: als sn.obmat (in andere object) de no[] roteren */
			if(sn.obmat) {
				float nor[3];
				
				VECCOPY(nor, no);
				
				normal_rot_inv(sn.obmat, se->imat, nor);
				speed1[0]+= fac*nor[0];
				speed1[1]+= fac*nor[1];
				speed1[2]+= fac*nor[2];

			}
			else {
				speed1[0]+= fac*no[0];
				speed1[1]+= fac*no[1];
				speed1[2]+= fac*no[2];
			}
			
			/* de virtuele oldloc */
			oldloc1[0]= lf->colloc[0] - sn.labda*speed1[0];
			oldloc1[1]= lf->colloc[1] - sn.labda*speed1[1];
			oldloc1[2]= lf->colloc[2] - sn.labda*speed1[2];
			
			if(colcount>3) if( sn.labda <= sn.minlabda) found= 0;
			
			if(sn.minlabda>=1.0) found= 0;
			else sn.minlabda= sn.labda;
	
		}
	}
	
	/* als er gebotst is: terug transformeren naar globale coordinaten */
	if(transback) life_from_inv_sector_co(lf);

	/* als laatste: de kracht die static life uitoefenen kan */
	force_from_prop(lf, &sn);

}

/* ******************* hele fh spul: *********************** */


int intersect_fh(DFace *dface, float *oldloc, Snijp *sn, float *speed, float fhdist)
{
	float s, t, tlab, inploc, inpspeed;
	short cox=0, coy=1;
	
	/* als dface->v3==0 : dface->ocx==0 */
	if( (dface->ocx & sn->ocx)==0 ) return 0;
	if( (dface->ocy & sn->ocy)==0 ) return 0;
	if( (dface->ocz & sn->ocz)==0 ) return 0;

	inpspeed= -dface->no[0]*speed[0] - dface->no[1]*speed[1] - dface->no[2]*speed[2];

	/* single sided! */	
	if(inpspeed < -TOLER) {
	
		inploc= dface->no[0]*oldloc[0] + dface->no[1]*oldloc[1] + dface->no[2]*oldloc[2];

		tlab= (dface->dist - inploc)/inpspeed;
		if(tlab < 0.0 || tlab >= sn->labda) return 0;
		
		if(dface->proj==1) coy= 2;
		else if(dface->proj==2) {
			cox= 1; coy= 2;
		}
		
		s= oldloc[cox] + tlab*speed[cox];
		t= oldloc[coy] + tlab*speed[coy];

		if( (dface->v2[cox] - s)*(dface->v2[coy] - dface->v1[coy]) - 
			(dface->v2[coy] - t)*(dface->v2[cox] - dface->v1[cox]) < -TOLER ) 
			return 0;

		if( (dface->v3[cox] - s)*(dface->v3[coy] - dface->v2[coy]) -
			(dface->v3[coy] - t)*(dface->v3[cox] - dface->v2[cox])< -TOLER ) 
			return 0;
		
		if(dface->v4==0) {
			if( (dface->v1[cox] - s)*(dface->v1[coy] - dface->v3[coy]) -
				(dface->v1[coy] - t)*(dface->v1[cox] - dface->v3[cox])< -TOLER ) 
				return 0;
		}
		else {
			if( (dface->v4[cox] - s)*(dface->v4[coy] - dface->v3[coy]) -
				(dface->v4[coy] - t)*(dface->v4[cox] - dface->v3[cox])< -TOLER ) 
				return 0;

			if( (dface->v1[cox] - s)*(dface->v1[coy] - dface->v4[coy]) -
				(dface->v1[coy] - t)*(dface->v1[cox] - dface->v4[cox])< -TOLER )
				return 0;
		}
		
		sn->labda= tlab;
		sn->face= dface;
		if(tlab<= fhdist) {
			sn->flag= FH_SECTOR;
			sn->fhdist= fhdist;
			return 1;
		}

		return 0;
	}
	
	return 0;
}

void euler_rot_euler(float *eul1, float *eul2, int local)
{
	/* convert to matrix, mult, convert back */
	float mat[3][3], mat1[3][3], mat2[3][3];
	
	if(eul2[0]!=0.0 || eul2[1]!=0.0 || eul2[2]!=0.0) {
		EulToMat3(eul1, mat1);
		EulToMat3(eul2, mat2);
		if(local) Mat3MulMat3(mat, mat1, mat2);
		else Mat3MulMat3(mat, mat2, mat1);
		Mat3ToEul(mat, eul1);
	}
}


void update_Fheight(Life *lf)
{
	/* alleen anti-zwaartekracht: pas op met rotaties. Normaal vlak= F_afstoot */
	DFace *dface;
	Material *ma;
	Mesh *me;
	Life *lfs;
	Object *propob, *se, *foundprop=NULL;
	Snijp sn;
	float *mat, speed[3], fhdist, alpha, fac, loc2[3], oldloc2[3], loc1[3], no[3];
	int a, b;
	
	lf->contact= 0;
	lf->floor= 0;
	lf->floorloc[0]= lf->floorloc[1]= lf->floorloc[2]= 0.0;
	
	sn.labda= 80.0;
	sn.face= 0;
	sn.ma= 0;
	sn.flag= 0;
	sn.radius= UNSUREF;
	mat= lf->sector->obmat[0];
	
	/* rotate! intersect is in local coords */
	speed[0]= -80.0*mat[8];
	speed[1]= -80.0*mat[9];
	speed[2]= -80.0*mat[10];

	me= lf->sector->data;

	init_snijp(me, &sn, lf->loc1, speed);
	
	VECCOPY(speed, mat+8);
	Normalise(speed);
	
	dface= me->dface;
	a= me->totface;

	while(a--) {
		if((dface->ma->fhdist)!=0.0) {
			
			fhdist= lf->localaxsize + dface->ma->fhdist*lf->sector->sizefac;
			/* alleen loodrechte projektie!!! */
			if( intersect_fh(dface, lf->loc1, &sn, speed, fhdist) ) {
				
				/* no break: best fh! */
			}
		}
		
		dface++;
	}

	/* or maybe the props? */
	if(lf->sector->lbuf.tot) {

		se= lf->sector;
		for(b=0; b<se->lbuf.tot; b++) {
			propob= se->lbuf.ob[b];
			
			lfs= propob->life;
			
			if(lfs && (lfs->flag & OB_PROP) && lf!=lfs && propob->type==OB_MESH) {
				Mesh *me= propob->data;
				/* is er wel 'n Fh vlak in mesh ? */
				
				/* transformeren naar life coordinaten */
				ApplyMatrix(propob->imat, lf->loc, loc1);
				
				dface= me->dface;
				a= me->totface;
				speed[0]= -80.0*propob->obmat[2][0];
				speed[1]= -80.0*propob->obmat[2][1];
				speed[2]= -80.0*propob->obmat[2][2];
				
				init_snijp(me, &sn, loc1, speed);
					
				speed[0]= propob->obmat[2][0];
				speed[1]= propob->obmat[2][1];
				speed[2]= propob->obmat[2][2];
				Normalise(speed);
				
				while(a--) {
					if((dface->ma->fhdist)!=0.0) {
						
						fhdist= (lf->axsize + dface->ma->fhdist)*propob->sizefac;
						
						/* loodrechte projektie */
						if( intersect_fh(dface, loc1, &sn, speed, fhdist) ) {
							
							/* normaal roteren volgens life */
							mat= propob->obmat[0];
							foundprop= propob;
							sn.flag = FH_PROP;
						}
					}
					
					dface++;
				}
				if(sn.flag == FH_PROP) break;
			}
		}
	}
	
	if(sn.face) {
		/* floor locatie voor schaduw: altijd relatief en naar beneden! */

		fac= -1.0*sn.labda;		
		lf->floorloc[0]= 0.05*lf->localaxsize+fac*mat[8];
		lf->floorloc[1]= 0.05*lf->localaxsize+fac*mat[9];
		lf->floorloc[2]= 0.05*lf->localaxsize+fac*mat[10];

		lf->floor= sn.face;
		
		if(sn.flag) {
			if(sn.face->ma->dynamode & MA_FH_NOR) {
				/* normaal roteren naar world coords: 'wipeout' wandjes, duwen in richting normaal vlak */
				no[0]= sn.face->no[0]*mat[0] + sn.face->no[1]*mat[4] + sn.face->no[2]*mat[8];
				no[1]= sn.face->no[0]*mat[1] + sn.face->no[1]*mat[5] + sn.face->no[2]*mat[9];
				no[2]= sn.face->no[0]*mat[2] + sn.face->no[1]*mat[6] + sn.face->no[2]*mat[10];
			}
			else {
				/* alleen loodrecht, global */
				no[0]= mat[8];
				no[1]= mat[9];
				no[2]= mat[10];
			}
		
			Normalise(no);
	
			ma= sn.face->ma;
			lf->contact= ma;
		
			/* sn.labda==0.0: exact op afstand dist */
			sn.labda= 1.0 - sn.labda/(sn.fhdist);
			
			if(foundprop && sn.flag == FH_PROP) sn.labda*= ma->fh*foundprop->sizefac;
			else sn.labda*= ma->fh;
			
			lf->force[0]+= sn.labda*no[0];
			lf->force[1]+= sn.labda*no[1];
			lf->force[2]+= sn.labda*no[2];
	
			/* extra fh friction, alleen in richting normaal */
			alpha= lf->speed[0]*no[0] + lf->speed[1]*no[1] + lf->speed[2]*no[2] ;
			alpha*= (ma->xyfrict); /* wrong name */

			lf->speed[0] -= alpha*no[0];
			lf->speed[1] -= alpha*no[1];
			lf->speed[2] -= alpha*no[2];
			
			if( (sn.flag==FH_PROP) && (lfs->dflag & LF_DYNACHANGED) && (lfs->flag & OB_PROP)) {
								
				/* werken aan de hand van de 'virtuele' vorige positie van life */
				ApplyMatrix(propob->imat, lf->loc, loc2);
				ApplyMatrix(lfs->oldimat, lf->loc, oldloc2);
	
				no[0]= oldloc2[0]-loc2[0];
				no[1]= oldloc2[1]-loc2[1];
				no[2]= oldloc2[2]-loc2[2];
				
				/* terug transformeren */
				Mat4Mul3Vecfl(propob->obmat, no);
				
				/* dit is de 'stilstaande' speed */
				fac= ((ma->friction));
				fac*= lf->frictfac;
				alpha= 1.0-fac;
	
				lf->speed[0]= alpha*lf->speed[0] + fac*no[0];
				lf->speed[1]= alpha*lf->speed[1] + fac*no[1];
				lf->speed[2]= alpha*lf->speed[2] + fac*no[2];
						
			}
			else {
				alpha= 1.0 - lf->frictfac*((ma->friction));		/* xy frict */
				
				lf->speed[0] *= alpha;
				lf->speed[1] *= alpha;
			}
			
			if(lf->flag & OB_ROT_FH) {
				float *fp,  cross[3], quat[4], fno[3];
				int axis= 0;
				int upflag= 2;
				
				/* axis */
				VECCOPY(no, lf->ob->obmat[2]);
				/* face normal */
				VECCOPY(fno, sn.face->no);
				Mat4Mul3Vecfl(lf->sector->obmat, fno);
				
				fac= no[0]*fno[0]+ no[1]*fno[1]+ no[2]*fno[2];

				/* just be sure we can make a cross vector */
				if(fac<0.99990) {

					Crossf(cross, no, fno);
					Normalise(cross);
					fac= 0.2*safacos(fac);		/* 0.5 is 100%, 0.2 means damping */
					quat[0]= fcos(fac);
					quat[1]= cross[0]*fsin(fac);
					quat[2]= cross[1]*fsin(fac);
					quat[3]= cross[2]*fsin(fac);
					
					QuatToEul(quat, cross);

					euler_rot_euler(lf->rot, cross, 0);	/* global */
				}
			}
		}
	}	
}

void aerodynamics_life(Object *ob, Life *lf)
{
	float *q, speed[3], len, inp;
	
	if(lf->aero==0.0) return;
	
	/* the direction vector rotates the speed... */
	/* only X/Y axis now */
	
	if(lf->flag1 & LF_AERO_AXIS_Y) q= ob->obmat[1];
	else q= ob->obmat[0];
	
	VECCOPY(speed, lf->speed);
	
	len= Normalise(speed);
	if(len > TOLER) {
		inp= q[0]*speed[0] + q[1]*speed[1] + q[2]*speed[2] ;
		
		inp*= lf->aero;
		
		lf->speed[0]= inp*len*q[0] + (1.0-fabs(inp))*lf->speed[0];
		lf->speed[1]= inp*len*q[1] + (1.0-fabs(inp))*lf->speed[1];
		/* keep the z component? */
		/* lf->speed[2]= inp*len*q[2] + (1.0-fabs(inp))*lf->speed[2]; */
	}
}

void clear_life(Life *lf)
{
	lf->flag1 &= ~(LF_CALC_MATRIX);

	lf->force[0]=lf->force[1]=lf->force[2]= 0.0;
	lf->omega[0]=lf->omega[1]=lf->omega[2]= 0.0;
	lf->dloc[0]=lf->dloc[1]=lf->dloc[2]= 0.0;
	lf->drot[0]=lf->drot[1]=lf->drot[2]= 0.0;

	lf->frictfac= 1.0;
	/* lf->collision= 0; */
}


void update_motion(Object *ob)
{
	Life *lf;
	float frict, grav;
	short doit;
	
	if(ob->life==NULL) {
		if(ob->type==OB_CAMERA) {
			where_is_object_simul(ob);
			return;
		}
	}
	
	lf= ob->life;

	/* cameras update also when outside sector */	
	if(lf->sector==NULL && ob->type!=OB_CAMERA) return;

	if(lf->flag & OB_DYNAMIC) {

		/* aerodynamics_life(ob, lf); */
		
		/* zwaartekracht */
		if(G.scene->world) grav= -DTIME*0.1*G.scene->world->gravity*ob->mass;
		else grav= -DTIME*ob->mass;
		
		/* Fh: contact/schaduw. VOOR de sensor-input afhandelen: kan je de wrijving overwinnen */

		if(lf->sector && lf->flag & OB_DO_FH) update_Fheight(lf);

		/* toetsen */
		sca_handling(ob, lf);

		lf->speed[0]+= (lf->force[0]/ob->mass);
		lf->speed[1]+= (lf->force[1]/ob->mass);
		lf->speed[2]+= grav + (lf->force[2]/ob->mass);

		lf->rotspeed[0]+= lf->omega[0];
		lf->rotspeed[1]+= lf->omega[1];
		lf->rotspeed[2]+= lf->omega[2];
		
		/* wrijving */
		if(ob->damping!=0.0) {

			frict= 1.0-ob->damping;	/* lf->frictfac */
			
			lf->speed[0]*= frict;
			lf->speed[1]*= frict;
			lf->speed[2]*= frict;
		}
		if(ob->rdamping!=0.0) {
			frict= 1.0-ob->rdamping;
			
			lf->rotspeed[0]*= frict;
			lf->rotspeed[1]*= frict;
			lf->rotspeed[2]*= frict;
		}
		
		VECCOPY(lf->oldloc, lf->loc);
		
		lf->loc[0]+= lf->speed[0]+lf->dloc[0];
		lf->loc[1]+= lf->speed[1]+lf->dloc[1];
		lf->loc[2]+= lf->speed[2]+lf->dloc[2];

		/* lf->drot is set to zero for each time step */		
		VecAddf(lf->drot, lf->drot, lf->rotspeed);
		euler_rot_euler(lf->rot, lf->drot, lf->dflag & LF_ROT_LOCAL);
		
		life_to_local_sector_co(lf);

		if(lf->sector) collision_detect(ob, lf);
		if(lf->collision) collision_sensor_input(ob, lf);

		/* floorloc is berekend t.o.v. oldloc */
		lf->floorloc[2]-= lf->speed[2];

		VECCOPY(ob->loc, lf->loc);
		VECCOPY(ob->rot, lf->rot);
		where_is_object_simul(ob);
		Mat4InvertSimp(ob->imat, ob->obmat);
	}
	else {
		
		sca_handling(ob, lf);
		
		/* this here, not for dyna's! */
		/* the old engine had a special collision sensor input */	
		lf->collision= NULL;

		if(lf->flag & OB_CHILD) {
		
			/* omega stuff */
			lf->rotspeed[0]+= lf->omega[0];
			lf->rotspeed[1]+= lf->omega[1];
			lf->rotspeed[2]+= lf->omega[2];
			
			if(ob->rdamping!=0.0) {
				frict= 1.0-ob->rdamping;
				lf->rotspeed[0]*= frict;
				lf->rotspeed[1]*= frict;
				lf->rotspeed[2]*= frict;
			}
		}
		
		lf->rot[0]+= lf->rotspeed[0]+lf->drot[0];
		lf->rot[1]+= lf->rotspeed[1]+lf->drot[1];
		lf->rot[2]+= lf->rotspeed[2]+lf->drot[2];
			
		VECCOPY(ob->rot, lf->rot);
			
		ob->loc[0]+= lf->dloc[0];
		ob->loc[1]+= lf->dloc[1];
		ob->loc[2]+= lf->dloc[2];
	}

	/* check for dyna changed and parent transformation */
	
	if(lf->flag & OB_CHILD) lf->flag1 |= LF_CALC_MATRIX;
	else if(ob->parent) lf->flag1 |= LF_CALC_MATRIX;
	
	if(lf->flag & OB_PROP)
		if(lf->flag1 & LF_CALC_MATRIX) lf->dflag |= LF_DYNACHANGED;
	
	if(ob->flag & OB_FROMGROUP) {
		/* the where_is_object_time was called already */

		Mat4CpyMat4(lf->oldimat, ob->imat);
		Mat4InvertSimp(ob->imat, ob->obmat);
		VECCOPY(lf->loc, ob->obmat[3]);
	}
	else if(lf->flag1 & LF_CALC_MATRIX) {
		where_is_object_simul(ob);
		Mat4CpyMat4(lf->oldimat, ob->imat);
		Mat4InvertSimp(ob->imat, ob->obmat);
		VECCOPY(lf->loc, ob->obmat[3]);
	}
	else Mat4CpyMat4(lf->oldimat, ob->imat);
	
	/* this cycles, not really nice, but it allows controllers
	 * to write in actuators of other objects.
	 */
	clear_life(lf);
}


/* ************* REALTIME ************** */


void view_to_piramat(float mat[][4], float lens, float far)
{
	/* maakt viewmat piramidevormig voor eenvoudige clip */
	
	lens/= 12.0;	/* hier stond 16.0 */
	mat[0][0]*= (256.0/320.0)*lens;
	mat[1][0]*= (256.0/320.0)*lens;
	mat[2][0]*= (256.0/320.0)*lens;
	mat[3][0]*= (256.0/320.0)*lens;
	
	mat[0][1]*= lens;
	mat[1][1]*= lens;
	mat[2][1]*= lens;
	mat[3][1]*= lens;
	
	mat[0][2]*= -1;
	mat[1][2]*= -1;
	mat[2][2]*= -1;
	mat[3][2]*= -1;
	
	mat[3][3]= far;		/* cliptest_sector leest dit uit */
}

/* **************** INIT ***************************** */

/* deze versie voor vlakken */
void calculate_ocvec(short *ocvec, float *v1, float *dvec, float *size)
{
	int a;
	
	if(size[0]!=0.0) ocvec[0]= ffloor(16.0*(v1[0]-dvec[0])/size[0]);
	else ocvec[0]= 0;
	CLAMP(ocvec[0], 0, 15);
	
	if(size[1]!=0.0) ocvec[1]= ffloor(16.0*(v1[1]-dvec[1])/size[1]);
	else ocvec[1]= 0;
	CLAMP(ocvec[1], 0, 15);
	
	if(size[2]!=0.0) ocvec[2]= ffloor(16.0*(v1[2]-dvec[2])/size[2]);
	else ocvec[2]= 0;
	CLAMP(ocvec[2], 0, 15);
}

void init_mesh_octree(Mesh *me)
{
	DFace *dface;
	TFace *tface;
	int a, b;
	short min[3], max[3], ocvec[3];
	
	/* ocinfo maken */
	if(me->dface==0) return;
	me->oc= mallocN(sizeof(OcInfo), "oc");
	
	if(me->bb==0) boundbox_mesh(me, me->oc->dvec, me->oc->size);
	
	VecSubf(me->oc->size, me->bb->vec[6], me->bb->vec[0]);
	VECCOPY(me->oc->dvec, me->bb->vec[0]);
	
	if(me->oc->size[0] < 0.01) me->oc->size[0]= 0.0;
	if(me->oc->size[1] < 0.01) me->oc->size[1]= 0.0;
	if(me->oc->size[2] < 0.01) me->oc->size[2]= 0.0;

	/* dface->ocx/y/z berekenen */
	dface= me->dface;
	tface= me->tface;
	a= me->totface;
	while(a--) {
		
		if(tface && (tface->mode & TF_DYNAMIC)==0) {
			dface->ocx= 0;
		}
		else if(dface->v3==0) {
			dface->ocx= 0;
		}
		else {
			min[0]=min[1]=min[2]= 16;
			max[0]=max[1]=max[2]= 0;
			
			calculate_ocvec(ocvec, dface->v1, me->oc->dvec, me->oc->size);
			DO_MINMAX(ocvec, min, max);
			calculate_ocvec(ocvec, dface->v2, me->oc->dvec, me->oc->size);
			DO_MINMAX(ocvec, min, max);
			calculate_ocvec(ocvec, dface->v3, me->oc->dvec, me->oc->size);
			DO_MINMAX(ocvec, min, max);
			if(dface->v4) {
				calculate_ocvec(ocvec, dface->v4, me->oc->dvec, me->oc->size);
				DO_MINMAX(ocvec, min, max);
			}
			dface->ocx= BROW(min[0], max[0]);
			dface->ocy= BROW(min[1], max[1]);
			dface->ocz= BROW(min[2], max[2]);

		}
		dface++;
		if(tface) tface++;
	}
}
	

void switch_dir_dface(DFace *dface)
{
	short edcode;
	
	/* niet de normaal flippen: is alleen optim voor intersect */

	/* OP DEZE MANIER GAAT IE OP DE PSX MIS!!!!	
	 *	if(dface->v4) {
	 *		SWAP(float *, dface->v2, dface->v3);
	 *		SWAP(float *, dface->v1, dface->v4);
	 *	}
	 *	else {
	 *		SWAP(float *, dface->v1, dface->v3);
	 *	}
	 */

	/* deze is GOED!! (ook voor vierhoeken ) */
	
	SWAP(float *, dface->v1, dface->v3);

	edcode= 0;
	if(dface->edcode & DF_V1) edcode |= DF_V3;
	if(dface->edcode & DF_V2) edcode |= DF_V2;
	if(dface->edcode & DF_V3) edcode |= DF_V1;
	if(dface->edcode & DF_V4) edcode |= DF_V4;
	
	if(dface->v4) {
		if(dface->edcode & DF_V1V2) edcode |= DF_V2V3;
		if(dface->edcode & DF_V2V3) edcode |= DF_V1V2;
		if(dface->edcode & DF_V3V4) edcode |= DF_V4V1;
		if(dface->edcode & DF_V4V1) edcode |= DF_V3V4;
	}
	else {
		if(dface->edcode & DF_V1V2) edcode |= DF_V2V3;
		if(dface->edcode & DF_V2V3) edcode |= DF_V1V2;
	}
	dface->edcode= edcode;
}

void init_dynamesh(Object *ob, Mesh *me)
{
	extern Material defmaterial;
	MFace *mface;
	MVert *v1, *v2, *v3, *v4;
	DFace *dface;
	Material *ma;
	float xn, yn, zn;
	int a, act=1;
	
	if(me->totface==0) return;

	/* sphereflags vorbereiden */
	v1= me->mvert;
	for(a=0; a<me->totvert; a++, v1++) {
		v1->flag &= ~ME_SPHERETEMP;
		if(v1->flag & ME_SPHERETEST) v1->flag |= ME_SPHERETEMP;
	}

	me->dface= callocN( me->totface*sizeof(DFace), "DFace");
	mface= me->mface;
	dface= me->dface;
	
	ma= give_current_material(ob, 1);
	
	for(a=0; a<me->totface; a++, mface++, dface++) {
	
		v1= (me->mvert+mface->v1);
		v2= (me->mvert+mface->v2);
		if(mface->v3) v3= (me->mvert+mface->v3); else v3= 0;
		if(mface->v4) v4= (me->mvert+mface->v4); else v4= 0;
	
		dface->v1= v1->co;
		dface->v2= v2->co;
		if(v3) dface->v3= v3->co; else dface->v3= 0;
		if(v4) dface->v4= v4->co; else dface->v4= 0;
		
		/* ook dfaces met v3==0. wordt soms uitgelezen */
		if(mface->mat_nr != act-1) {
			act= mface->mat_nr+1;
			ma= give_current_material(ob, act);
		}
		if(ma) dface->ma= ma;
		else dface->ma= &defmaterial;
		
		if(dface->v3) {
			if(dface->v4) CalcNormFloat4(dface->v1, dface->v2, dface->v3, dface->v4, dface->no);
			else CalcNormFloat(dface->v1, dface->v2, dface->v3, dface->no);
			
			xn= fabs(dface->no[0]);
			yn= fabs(dface->no[1]);
			zn= fabs(dface->no[2]);
			
			/* edge en vertexcode voor cyl en sphere isect */
			dface->edcode= mface->edcode & (~15);
			
			if(FALSE) {
			if(v1->flag & ME_SPHERETEMP) {dface->edcode |= DF_V1; v1->flag -= ME_SPHERETEMP;}
			if(v2->flag & ME_SPHERETEMP) {dface->edcode |= DF_V2; v2->flag -= ME_SPHERETEMP;}
			if(v3->flag & ME_SPHERETEMP) {dface->edcode |= DF_V3; v3->flag -= ME_SPHERETEMP;}
			if(v4) if(v4->flag & ME_SPHERETEMP) {dface->edcode |= DF_V4; v4->flag -= ME_SPHERETEMP;}
			}
			else {	/* elk vlak moet sphere-code hebben: zie ook editmedh.c commentaar */
			if(v1->flag & ME_SPHERETEMP) {dface->edcode |= DF_V1;}
			if(v2->flag & ME_SPHERETEMP) {dface->edcode |= DF_V2;}
			if(v3->flag & ME_SPHERETEMP) {dface->edcode |= DF_V3;}
			if(v4) if(v4->flag & ME_SPHERETEMP) {dface->edcode |= DF_V4;}
			}
			
			/* optimalisering bij intersect_dface() */
			if(zn>=xn && zn>=yn) {
				dface->proj= 0;
				if( (dface->v2[0] - dface->v3[0])*(dface->v2[1] - dface->v1[1]) <
					(dface->v2[1] - dface->v3[1])*(dface->v2[0] - dface->v1[0]) )
					switch_dir_dface(dface);
					
			}
			else if(yn>=xn && yn>=zn) {
				dface->proj= 1;
				if( (dface->v2[0] - dface->v3[0])*(dface->v2[2] - dface->v1[2]) <
					(dface->v2[2] - dface->v3[2])*(dface->v2[0] - dface->v1[0]) )
					switch_dir_dface(dface);
			}
			else {
				dface->proj= 2;
				if( (dface->v2[1] - dface->v3[1])*(dface->v2[2] - dface->v1[2]) <
					(dface->v2[2] - dface->v3[2])*(dface->v2[1] - dface->v1[1]) )
					switch_dir_dface(dface);
			}
			
			dface->dist= (dface->no[0]*dface->v1[0] + dface->no[1]*dface->v1[1] + dface->no[2]*dface->v1[2] ); 
		}
	}

	init_mesh_octree(me);
}

void end_dynamesh(Mesh *me)
{
	
	if(me==0) return;
	if(me->dface) freeN(me->dface);
	me->dface= 0;
	if(me->oc) freeN(me->oc);
	me->oc= 0;
}


/* **************** INIT ***************************** */

/* objects with overlapping boundboxes, add to portal array */

void add_object_portals(Object *ob)
{
	Object *port;
	Base *base;
	float dist, bb1[3], bb2[3], cent1[3], cent2[3];
	int test;
	
	/* rotate bbsize to global coords */
	get_local_bounds(ob, cent1, bb1);
	Mat4Mul3Vecfl(ob->obmat, bb1);
	bb1[0]= fabs(bb1[0]);
	bb1[1]= fabs(bb1[1]);
	bb1[2]= fabs(bb1[2]);

	Mat4MulVecfl(ob->obmat, cent1);
	
	base= FIRSTBASE;
	while(base) {
		if(base->lay & G.scene->lay) {
			port= base->object;
			if(port!=ob && ( port->gameflag & OB_SECTOR )) {

				/* rotate bbsize to global coords */
				get_local_bounds(port, cent2, bb2);
				Mat4Mul3Vecfl(port->obmat, bb2);
				bb2[0]= fabs(bb2[0]);
				bb2[1]= fabs(bb2[1]);
				bb2[2]= fabs(bb2[2]);
			
				Mat4MulVecfl(port->obmat, cent2);

				/* is it close? */
				test= 0;
				dist= fabs(cent1[0] - cent2[0]) ;
				if( dist <  1.05*(bb1[0]+bb2[0]) ) test++;

				dist= fabs(cent1[1] - cent2[1]) ;
				if( dist < 1.05*(bb1[1]+bb2[1]) ) test++;
				
				dist= fabs(cent1[2] - cent2[2]) ;
				if( dist < 1.05*(bb1[2]+bb2[2]) ) test++;
		
				if(test==3) {
					add_to_lbuf(&ob->port, port);
				}
			}
		}
		base= base->next;
	}
}

void init_sectors()
{
	extern Material defmaterial;
	Base *base;
	Mesh *me;
	Life *lf;
	float centre[3], *fp;
	int a;
	
	Gcursector= 0;
	Gtotsect= 0;
	
	/* vlaggen resetten */
	me= G.main->mesh.first;
	while(me) {
		me->flag &= ~ME_ISDONE;
		me= me->id.next;
	}

	/*  dit zijn globale INT tellers (gaat 10000 uren mee) */
	Gdfra= Gdfrao= 1;
	
	base= FIRSTBASE;
	while(base) {
		
		base->object->dfras= 0;

		where_is_object_simul(base->object);
		Mat4Invert(base->object->imat, base->object->obmat);

		fp= base->object->imat[0];
		base->object->sizefac= fsqrt(fp[0]*fp[0] + fp[1]*fp[1] + fp[2]*fp[2]);

		if(base->object->type==OB_MESH) {
			BoundBox *bb= ( (Mesh *)base->object->data )->bb;
			if(bb==0) {
				tex_space_mesh(base->object->data);
			}
		
			/* geen layertest meer */
			if(base->object->gameflag & OB_SECTOR) {
				Object *ob= base->object;
				
				ob->lbuf.tot= ob->lbuf.max= 0;
				ob->lbuf.ob= 0;
				ob->port.tot= ob->port.max= 0;
				ob->port.ob= 0;

				get_local_bounds(ob, centre, ob->bbsize);
	
				me= ob->data;

				if(me->flag & ME_ISDONE);
				else {
					init_dynamesh(ob, me);
					me->flag |= ME_ISDONE;
				}
			}
		}
		base= base->next;
	}

	base= FIRSTBASE;
	while(base) {
		if(base->lay & G.scene->lay) {
			if(base->object->gameflag & OB_SECTOR) {
				add_object_portals(base->object);
			}
		}
		base= base->next;
	}
}

int test_visibility(float *lookat, float *from, Life *lf, Object *se)
{
	Snijp sn;
	Mesh *me;
	DFace *dface;
	float fac, loc1[3], oldloc1[3], speed1[3];
	short a, found;
	
	if(se==0) return 0;
	
	/* transformeren naar lokale sector coords en intersecten */

	ApplyMatrix(se->imat, from, loc1);
	ApplyMatrix(se->imat, lookat, oldloc1);
	
	/* hier oppassen met shorts! */
	
	speed1[0]= loc1[0]-oldloc1[0];
	speed1[1]= loc1[1]-oldloc1[1];
	speed1[2]= loc1[2]-oldloc1[2];

	me= se->data;

	sn.minlabda= sn.radius= 0.0;	/* radius op nul voor init_snijp */
	sn.labda= 1.0;
	sn.obmat= 0;
	sn.lay= lf->lay;
	found= 0;

	init_snijp(me, &sn, oldloc1, speed1);

	if(sn.ocx) {
	
		dface= me->dface;
		a= me->totface;
		
		while(a--) {
			if( intersect_dface(dface, &sn, oldloc1, speed1)) found= 1;
			dface++;
		}
		
		if(found) {
			/* de bots loc */
			oldloc1[0]+= sn.labda*speed1[0];
			oldloc1[1]+= sn.labda*speed1[1];
			oldloc1[2]+= sn.labda*speed1[2];

			/* reflectie berekenen */
			fac= (-2.0)*sn.inpspeed;
			speed1[0]+= fac*sn.face->no[0];
			speed1[1]+= fac*sn.face->no[1];
			speed1[2]+= fac*sn.face->no[2];
			
			fac= (1.0 - sn.labda);

			/* endloc */
			oldloc1[0]+= fac*speed1[0];
			oldloc1[1]+= fac*speed1[1];
			oldloc1[2]+= fac*speed1[2];
			
			/* blenden */
			loc1[0]= (20*loc1[0] + oldloc1[0])/21.0;
			loc1[1]= (20*loc1[1] + oldloc1[1])/21.0;
			loc1[2]= (20*loc1[2] + oldloc1[2])/21.0;
			
			ApplyMatrix(se->obmat, loc1, from);

			return 1;
		}

	}
	
	return 0;
}


void end_sectors()
{
	Base *base;
	
	base= FIRSTBASE;
	while(base) {
		
		if(base->object->type==OB_MESH) {
			end_dynamesh(base->object->data);
		}
		
		free_lbuf(&(base->object->lbuf));
		free_lbuf(&(base->object->port));
		
		base= base->next;
	}

	Gcursector= 0;
 
}


void add_dyna_life(Object *ob)
{
	int a, *ip=0;

	if(Gtotlife<LF_MAXBUF) {
		Glifebuf[Gtotlife]= ob;
		Gtotlife++;
	}
}

void add_dupli_life(Object *ob, Object *from, int time)
{
	Object *newob, *par;
	bSensor *sens;
	bController *cont;
	bActuator *act;
	bProperty *prop;
	Life *lfn, *lf;
	float mat[3][3];
	short a;
	
	if(from->life==NULL || ob->life==NULL) return;

	if(Gtotlife<LF_MAXBUF) {
		newob= dupallocN(ob);

		newob->lay= from->lay;
		
		lf= from->life;

		if(from->parent) {
			
			/* zeer primitieve rotatie ! */
			
			/* VECCOPY(newob->rot, from->rot); */
		/* 	par= from->parent; */
		/* 	while(par) { */
		/* 		newob->rot[0]+= par->rot[0]; */
		/* 		newob->rot[1]+= par->rot[1]; */
		/* 		newob->rot[2]+= par->rot[2]; */
		/* 		par= par->parent; */
		/* 	} */
		
			Mat3CpyMat4(mat, from->obmat);
			Mat3ToEul(mat, newob->rot);
			
			VECCOPY(newob->loc, from->obmat[3]);
			
		}
		else if((lf->flag & OB_DYNAMIC) && lf->collision) {
			VECCOPY(newob->loc, lf->colloc);
			VECCOPY(newob->rot, from->rot);
		}
		else {
			VECCOPY(newob->loc, from->loc);
			VECCOPY(newob->rot, from->rot);
		}
		
		where_is_object_simul(newob);
		add_dyna_life(newob);

		newob->life=lfn= dupallocN(ob->life);
		newob->life->ob= newob;
		
		VECCOPY(lfn->loc, newob->loc);
		VECCOPY(lfn->rot, newob->rot);

		VECCOPY(lfn->speed, lf->speed);
		VECCOPY(lfn->oldloc, lf->oldloc);

		/* copy SCA */
		
		clear_sca_new_poins_ob(newob);
		copy_sensors(&newob->sensors, &ob->sensors);
		copy_controllers(&newob->controllers, &ob->controllers);
		copy_actuators(&newob->actuators, &ob->actuators);
		set_sca_new_poins_ob(newob);
		
		/* link controllers to sensors */
		sens= newob->sensors.first;
		while(sens) {
			sens->ob= newob;
			for(a=0; a<sens->totlinks; a++) {
				add_sens_to_cont(sens, sens->links[a]);
			}
			sens= sens->next;
		}

		/* controller reset */
		cont= newob->controllers.first;
		while(cont) {
			cont->val= 0;
			cont->valo= 0;
			cont= cont->next;
		}

		/* actuator reset, init */
		act= newob->actuators.first;
		while(act) {
			act->ob= newob;
			act->go= 0;
			if(act->type==ACT_IPO) {
				bIpoActuator *ia;
				ia= act->data;
				ia->sta= 2*ia->sta;
				ia->end= 2*ia->end;
				ia->cur= ia->sta;
			}
			act= act->next;
		}
		
		/* property copy */
		copy_properties(&newob->prop, &ob->prop);
		
		prop= newob->prop.first;
		while(prop) {
			if(prop->type==PROP_TIME) prop->data= Gdfra - 2*prop->old;
			prop= prop->next;
		}
		
		do_obipo(newob, 2*lf->sfra, 0, 0);

		lfn->dflag |= LF_TEMPLIFE;
		lfn->timer= time;
		lfn->collision= 0;
		while(from->parent) from= from->parent;
		lfn->from= from;
		if(from->gameflag & OB_LIFE) lfn->sector= ((Life *)from->life)->sector;
	}
}

void del_dupli_life(Object *ob)
{
	bSensor *sens;
	Life *lf;
	int a, b;
	
	/* opzoeken in array */
	a= Gtotlife;
	while(a--) {
		if(Glifebuf[a]==ob) {
			if(ob->gameflag & OB_LIFE) {
				lf= ob->life;
				if(lf->dflag & LF_TEMPLIFE) {
					
					/* remove sensors, they can be linked to not-duplicated stuff */
					sens= ob->sensors.first;
					while(sens) {
						for(b=0; b<sens->totlinks; b++) {
							rem_sens_from_cont(sens, sens->links[b]);
						}
						sens= sens->next;
					}

					free_sensors(&ob->sensors);
					free_controllers(&ob->controllers);
					free_actuators(&ob->actuators);
					
					free_properties(&ob->prop);
					
					freeN(lf);
					freeN(ob);
					Gtotlife--;
					Glifebuf[a]= Glifebuf[Gtotlife];
				}
			}
			return;
		}
	}
}

void del_dupli_lifes()
{
	int a;
	
	a= Gtotlife;
	while(a--) {
		del_dupli_life(Glifebuf[a]);
	}
}

char clipcube[24]= {0, 1, 1,
					1, 0, 1, 
					0, 0, 0, 
					1, 1, 0, 
					1, 1, 1, 
					0, 0, 1, 
					1, 0, 0,
					0, 1, 0}; 

int cliptest_sector(float *vec, float *size, float *mat)
{
	/* deze pakt de vier tetraederpunten */
	/* interessante notitie: max= fabs(hoco[2]) zou je denken,
	 * fabs is echter veel te weinig kritisch. Op deze wijze (met sign)
	 * meer uitval achter de camera,  maar wel OK!
	 */
	float hoco[3], min, max, far=mat[15];
	short fl, fand, a, tot;
	char *ctab;


	tot= 5;
	ctab= clipcube;
	fand= 63;
	
	while(tot--) {
		
		if(tot==5) {
			hoco[0]= vec[0];
			hoco[1]= vec[1];
			hoco[2]= vec[2];
		}
		else {
			if(ctab[0]) hoco[0]= vec[0] - size[0];
			else hoco[0]= vec[0] + size[0];
	
			if(ctab[1]) hoco[1]= vec[1] - size[1];
			else hoco[1]= vec[1] + size[1];
	
			if(ctab[2]) hoco[2]= vec[2] - size[2];
			else hoco[2]= vec[2] + size[2];
		}
		
		Mat4MulVecfl(mat, hoco);
		max= (hoco[2]);
		min= -max;
		fl= 0;
		if(hoco[0] < min) fl+= 1; else if(hoco[0] > max) fl+= 2;
		if(hoco[1] < min) fl+= 4; else if(hoco[1] > max) fl+= 8;
		if(hoco[2] < 0.0) fl+= 16; else if(hoco[2] > far) fl+= 32;

		fand &= fl;
		
		if(fand==0) return 1;

		ctab+= 3;
	}
	return 0;
}

void build_sectorlist(Object *cam)
{
	Object *ob, *obp;
	Camera *ca;
	Base *base;
	float far, lens, piramat[4][4], dist, bbs;
	short a, b, beforesect, startsect;

	float viewfac;
	
	if(G.scene->camera==0) {
		lens= 35.0;
		far= 16.0;
	}
	else {
		ca= G.scene->camera->data;
		lens= ca->lens;
		far= ca->clipend;
	}
	
	startsect= 0;
	Gtotsect= 0;
	/* Gmaxsect= G.scene->maxdrawsector; */
	Gmaxsect= 32;

	/* uitzondering afhandelen */ 
	if(Gcursector==0 || cam==0) {
		base= FIRSTBASE;
		while(base) {
			if(base->lay & G.scene->lay) {
				if(base->object->gameflag & OB_SECTOR) {			
					Gsectorbuf[Gtotsect]= base->object;
					Gtotsect++;
					if(Gtotsect>=Gmaxsect) break;
				}
			}
			base= base->next;
		}
		return;
	}

	Gsectorbuf[0]= Gcursector;
	Gcursector->dfras= Gdfras;
	Gtotsect= 1;
	
	Mat4Ortho(cam->obmat);
	Mat4Invert(cam->imat, cam->obmat);	/* viewmat */
	Mat4CpyMat4(piramat, cam->imat);
	view_to_piramat(piramat, lens, far);
	
	/* add the adjoining sectors */
	a= Gcursector->port.tot;
	while(a--) {
		ob= Gcursector->port.ob[a];
		Gsectorbuf[Gtotsect]= ob;
		ob->dfras= Gdfras;
		
		Gtotsect++;
	}	
	
	while(Gtotsect<Gmaxsect) {
		beforesect= Gtotsect;
		
		for(b=Gtotsect-1; b>=startsect; b--) {
			ob= Gsectorbuf[b];

			a= ob->port.tot;
			while(a--) {
				obp= ob->port.ob[a];
				if(obp->dfras!=Gdfras) {
					obp->dfras= Gdfras;

					if(ob->lay & G.scene->lay) {
					
						/* in beeld */
						if(cliptest_sector(obp->obmat[3], obp->bbsize, piramat[0])) {
							Gsectorbuf[Gtotsect]= obp;	
							Gtotsect++;
							if(Gtotsect>=Gmaxsect) break;
						}
					}
				}
			}
			if(Gtotsect>=Gmaxsect) break;
		}
		
		/* geen meer bijgekomen */
		if(Gtotsect==beforesect) return;
		startsect= beforesect;
	}
		
}


/* ************* MAIN ************** */

void update_sector_lifes(Object *se)
{
	Object *ob;
	Life *lf;
	short a, b;

	if(se==0) return;
	
	for(b=0; b<se->lbuf.tot; b++) {
		ob= se->lbuf.ob[b];
		if(ob->lay & G.scene->lay) {
		
			/* lifes zitten in meerdere sectoren of sector 2x afgehandeld */
		
			if(ob->dfras!=Gdfras) {
				ob->dfras= Gdfras;
				
				lf= ob->life;

				if(lf->flag & (OB_DYNAMIC|OB_ACTOR));
				else {
					update_motion(ob);
				}
			}
		}
	}	
}

void lifebuf_sector_lifes(Object *se)
{
	Object *ob;
	Life *lf;
	short a, b;

	if(se==0) return;
	
	for(b=0; b<se->lbuf.tot; b++) {
		ob= se->lbuf.ob[b];
		if(ob->lay & G.scene->lay) {
		
			lf= ob->life;

			if(lf->flag & (OB_DYNAMIC|OB_ACTOR)) {
				if(lf->flag & OB_CHILD);
				else add_dyna_life(ob);
			}
		}
	}	
}

void update_lifes()
{
	Object *ob;
	Life *lf;
	int a, b;
	
	/* cleanup lifebuf  */
	for(a=0; a<Gtotlife; a++) {
		ob= Glifebuf[a];
		if(ob->gameflag & OB_LIFE) {
			/* if(ob->type!=OB_CAMERA) { */
				lf= ob->life;
				if( (lf->flag & OB_MAINACTOR)==0) {
					if( (lf->dflag & LF_TEMPLIFE)==0 ) {
						Gtotlife--;
						Glifebuf[a]= Glifebuf[Gtotlife];
						a--;
					}
				}
			/* } */
		}
	}

	/* build lifebuf */
	a= Gtotsect;
	while(a-- > 0) {
		lifebuf_sector_lifes(Gsectorbuf[a]);
	}
	
	G.scene->camera->dfras= Gdfras;
	
	/* events and motion */
	
	a= Gtotsect;
	while(a-- > 0) {
		update_sector_lifes(Gsectorbuf[a]);
	}

	
	/* alle main+link en temp lifes: in volgorde ivm parents! */
	for(a=0; a<Gtotlife; a++) {
		ob= Glifebuf[a];
		
		/* still test, because layers can change */
		if(ob->lay & G.scene->lay) {

			if(ob->life) {
				life_in_sector_test(ob);

				update_motion(ob);
			
				lf= ob->life;
				for(b=0; b<lf->links.tot; b++) update_motion(lf->links.ob[b]);
			}
		}
	}

	if(main_actor) Gcursector= ((Life *)main_actor->life)->sector;
	
	/* apart doen ivm delete */
	a= Gtotlife;
	while(a--) {
		ob= Glifebuf[a];
		if(ob->life) {
			lf= ob->life;
			if(lf->dflag & LF_TEMPLIFE) {
				if(lf->timer<=0 || lf->sector==0) {
					/* we let them exist for one gamecycle, for sensors */
					if(ob->lay == (1<<21)) del_dupli_life(ob);
					else ob->lay= (1<<21);
				}
			}		
		}
	}

}

void update_realtime_textures()
{
	/* wordt aangeroepen met constante */
	Image *ima;
	int a;
	
	ima= G.main->image.first;
	while(ima) {
		if(ima->tpageflag & IMA_TWINANIM) {
			if(ima->twend >= ima->xrep*ima->yrep) ima->twend= ima->xrep*ima->yrep-1;
		
			/* check: zit de bindcode niet het array? Vrijgeven. (nog doen) */
			
			ima->lastframe++;
			if(ima->lastframe > ima->twend) ima->lastframe= ima->twsta;
			
		}
		ima= ima->id.next;
	}
}

void drawlife(Object *ob, int dt, uint col)
{
	Life *lf;
	Object *obedit;
	float vec[3];
	int flag;
	
	lf= ob->life;
	if(lf->sector==NULL) return;

	if(dt<OB_SHADED) {

		cpack(col);

		/* schaduw */
		if(lf->flag & OB_DYNAMIC) {
			if(lf->floor) {
				float tmat[4][4];
			
				getmatrix(tmat);
				
				cpack(0);
				glEnable(GL_POLYGON_STIPPLE);
				
				loadmatrix(G.vd->viewmat);
				glTranslatef(ob->obmat[3][0],  ob->obmat[3][1],  ob->obmat[3][2]);
				
				/* floorloc is t.o.v. oldloc!!! */
				vec[0]= lf->floorloc[0];
				vec[1]= lf->floorloc[1];
				vec[2]= lf->floorloc[2];

				vec[2]+= 0.2*lf->axsize;

				glBegin(GL_POLYGON);
				vec[0]-= 0.4*lf->axsize;
				vec[1]-= 0.4*lf->axsize;
				glVertex3fv(vec);
				vec[0]+= 0.8*lf->axsize;
				glVertex3fv(vec);
				vec[1]+= 0.8*lf->axsize;
				glVertex3fv(vec);
				vec[0]-= 0.8*lf->axsize;
				glVertex3fv(vec);
				glEnd();
				
				glDisable(GL_POLYGON_STIPPLE);
				
				cpack(col);
				
				loadmatrix(tmat);
			}
		}
	}

	if(lf->collision) {
		cpack(0xFFFFFF);
	}
	if(lf->dflag & LF_NO_DAMAGE) cpack(0x00FF00);

	if(ob->type==OB_MESH) {
		if(ob==G.obedit) drawmeshwire(ob);
		else if(dt==OB_BOUNDBOX) draw_bounding_volume(ob);
		else if(dt==OB_WIRE) drawmeshwire(ob);
		else if(dt==OB_SOLID) drawDispList(ob, dt);
		else {
		
			draw_tface_mesh(ob, ob->data, dt);
		}
	}
	else if(G.vd->drawtype<=OB_SHADED) drawaxes( lf->axsize);
	
	if(dt<OB_SHADED) {
		if( lf->flag & OB_DYNAMIC) {
			float tmat[4][4], imat[4][4];

			vec[0]= vec[1]= vec[2]= 0.0;
			getmatrix(tmat);
			Mat4Invert(imat, tmat);
			
			setlinestyle(2);
			drawcircball(vec, lf->axsize, imat);
			setlinestyle(0);
		}
	}
}

void drawsector(Object *ob, int dt, uint col)		/* col: restore van ghost */
{	
	Mesh *me;
	
	me= ob->data;

	if(ob==G.obedit) drawmeshwire(ob);
	else if(dt==OB_BOUNDBOX) draw_bounding_volume(ob);
	else if(dt==OB_WIRE) drawmeshwire(ob);
	else if(dt==OB_SOLID) drawDispList(ob, dt);
	else {
		draw_tface_mesh(ob, me, dt);
	}

}


void drawview3d_simul(int make_disp)
{
	Object *ob, *obs;
	Life *lf, *lfl;
	Material *ma;
	uint col;
	int b, a, flag;

	/* hier doen: anders wordt in header getekend */
	areawinset(curarea->win);

	setwinmatrixview3d(0);	/* 0= geen pick rect */

	/* don't call setviewmatrix3d() because of where_is_object (slow cam goes wrong) */
	if(G.vd->persp>=2 && G.vd->camera) {
		obmat_to_viewmat(G.vd->camera);
	}
	else {
		QuatToMat4(G.vd->viewquat, G.vd->viewmat);
		if(G.vd->persp==1) G.vd->viewmat[3][2]-= G.vd->dist;
		i_translate(G.vd->ofs[0], G.vd->ofs[1], G.vd->ofs[2], G.vd->viewmat);
	}
	
	Mat4MulMat4(G.vd->persmat, G.vd->viewmat, curarea->winmat);
	Mat4Invert(G.vd->persinv, G.vd->persmat);
	Mat4Invert(G.vd->viewinv, G.vd->viewmat);

	if(G.vd->drawtype >= OB_SOLID) {
		G.zbuf= TRUE;
		glEnable(GL_DEPTH_TEST);
		
		if(G.scene->world) glClearColor(G.scene->world->horr, G.scene->world->horg, G.scene->world->horb, 0.0); 
		else glClearColor(0.4375, 0.4375, 0.4375, 0.0); 
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else {
		glClearColor(0.4375, 0.4375, 0.4375, 0.0); 
		glClear(GL_COLOR_BUFFER_BIT);
	}

	/* do this within the drawloop, fog ON doesnt work fine with drawing
       the interface (linux) */
	
	if(G.vd->drawtype > OB_WIRE) {
	    if(G.scene->world && (G.scene->world->mode & WO_MIST)) {
			float params[5];

			glFogi(GL_FOG_MODE, GL_LINEAR);
			glFogf(GL_FOG_DENSITY, 0.1);
			glFogf(GL_FOG_START, G.scene->world->miststa);
			glFogf(GL_FOG_END, G.scene->world->miststa+G.scene->world->mistdist);
			params[0]= G.scene->world->horr;
			params[1]= G.scene->world->horg;
			params[2]= G.scene->world->horb;
			params[3]= 0.0;
			glFogfv(GL_FOG_COLOR, params); 
			glEnable(GL_FOG);
		}
	}
	loadmatrix(G.vd->viewmat);
	
	/* extern: camera tekenen */
	if(G.vd->persp!=2) {
		ob= G.scene->camera;
		if(ob && (ob->lay & G.vd->lay)) {
			cpack(0x0);
			multmatrix(ob->obmat);
			drawcamera(ob);
			loadmatrix(G.vd->viewmat);
		}
	}
	
	a= Gtotsect;
	while(a-- > 0) {
		ob= Gsectorbuf[a];

		multmatrix(ob->obmat);
		cpack(0x0);
		drawsector(ob, MIN2(ob->dt, G.vd->drawtype), 0);
		loadmatrix(G.vd->viewmat);
		
		if(ob->lbuf.tot) {
			for(b=0; b<ob->lbuf.tot; b++) {
				obs= ob->lbuf.ob[b];

				if(obs->lay & G.vd->lay) {
					lf= obs->life;

					if(lf==NULL || (lf->flag & OB_DYNAMIC) || (lf->dflag & LF_DONTDRAW));
					else {
						multmatrix(obs->obmat);
						
						col= 0;
						if( ma=lf->contact) col= rgb_to_cpack(ma->r, ma->g, ma->b);
						if( lf->flag1 & LF_DRAWNEAR) col= (0xF0A020);
						
						drawlife(obs, MIN2(obs->dt, G.vd->drawtype), col);
						
						if(G.vd->drawtype!=OB_TEXTURE) {
						
							if(obs->dtx & OB_DRAWNAME) drawname(obs);
							if(obs->dtx & OB_AXIS) drawaxes( lf->axsize);
						}
						loadmatrix(G.vd->viewmat);
					}
				}
			}
		}
	}

	a= Gtotlife;
	while(a--) {
		ob= Glifebuf[a];
		if( (ob->gameflag & OB_LIFE) && (ob->lay & G.vd->lay)) {
			
			lf= ob->life;

			if(lf->dflag & LF_DONTDRAW);
			else {
				
				multmatrix(ob->obmat);
				if(G.vd->drawtype!=OB_TEXTURE) {
					col= 0;
					if( ma=lf->contact) col= rgb_to_cpack(ma->r, ma->g, ma->b);
					if( lf->flag1 & LF_DRAWNEAR) col= (0xF0A020);
					
					drawlife(ob, MIN2(ob->dt, G.vd->drawtype), col);
					
					if(ob->dtx & OB_DRAWNAME) drawname(ob);
					if(ob->dtx & OB_AXIS) drawaxes( lf->axsize);	
				}
				else drawlife(ob, MIN2(ob->dt, G.vd->drawtype), 0);
				
				loadmatrix(G.vd->viewmat);
			}

			for(b=0; b<lf->links.tot; b++) {
				
				ob= lf->links.ob[b];

				if(ob->gameflag & OB_LIFE) {
					if(ob->lay & G.vd->lay) {
					
						lfl= ob->life;
	
						if(lfl->dflag & LF_DONTDRAW);
						else {
							multmatrix(ob->obmat);
							if(G.vd->drawtype!=OB_TEXTURE) {
								col= 0;
								if( ma=lfl->contact) col= rgb_to_cpack(ma->r, ma->g, ma->b);
								if( lfl->flag & LF_DRAWNEAR) col= (0xF0A020);
								drawlife(ob, MIN2(ob->dt, G.vd->drawtype), col);
								if(ob->dtx & OB_DRAWNAME) drawname(ob);
								if(ob->dtx & OB_AXIS) drawaxes( lfl->axsize);	
							}
							else drawlife(ob, MIN2(ob->dt, G.vd->drawtype), 0);
							
							loadmatrix(G.vd->viewmat);
						}
					}
				}
			}
		}
	}

	draw_tface_meshes_tra();

	if(G.zbuf) {
		G.zbuf= FALSE;
		glDisable(GL_DEPTH_TEST);
	}
	glDisable(GL_FOG);
	
		/* life.c */
	draw_gamedebug_info();
	
	curarea->win_swap= WIN_BACK_OK;
}


void sector_go()
{
	extern double tottime;	/* drawview.c */
	extern int dbswaptime;	/* global in life.c, voor print */
	extern double speed_to_swaptime(int); 	/* drawview.c */
	extern int update_time();
	struct tms voidbuf;
	double swaptime;
	int time, ltime, dtime;
	int a, b;
	ushort event=0;
	short val;
	
	/* per life: sector test
	 *			 sensors
	 *			 damage
	 *			 update dynamics <- -> collision detectie
	 *			 
	 */
	
	if(G.scene->camera==0) {
		error("no camera");
		G.simulf= G_QUIT;
		return;
	}
	if(G.scene->camera->type!=OB_CAMERA) {
		error("no correct camera");
		G.simulf= G_QUIT;
		return;
	}

		/* doet ook mesh_isdone flag */
	init_sectors();
	init_lifes();
	init_devs();

	update_time();
	tottime= 0.0;
	swaptime= speed_to_swaptime(G.animspeed);
	#ifdef __WIN32
	ltime = times(&voidbuf)/10;
	#else
		#ifdef __BeOS
		ltime= (glut_system_time())/10000;
		#else
		ltime = times(&voidbuf);
		#endif
	#endif
	dtime= 4;	/* vantevoren invullen (1/25 sec) */	

	
	sectcount=spherecount=cylcount=facecount=matcount= 0;	
	
	
	while(TRUE) {
		/* dynamics lus: stapjes van 0.02 seconden */
		
		Gdfrao= Gdfra;
		G.fields=0;
		
		set_dtime(dtime);	/* global in object.c, ook voor cameranet */
		dbswaptime= dtime;	

		if(dtime>25) dtime= 25;
		
		while(dtime>0) {
			Gdfra++;
			Gdfras= Gdfra;

			event= pad_read();
			if(event==SPACEKEY || event==ESCKEY) {
				G.simulf |= G_QUIT;
				break;
			}
		
			update_lifes();
			
			if(Gdfra & 1) update_realtime_textures();
			
			dtime -= 2;
			G.fields++;
			sectcount++;
		
		}
 
		build_sectorlist(G.scene->camera);
	
		drawview3d_simul(0);

		screen_swapbuffers();
		
		/* tijdberekening: in hondersten van seconde */		
		while(dtime<2) {
			#ifdef __WIN32
			time = times(&voidbuf)/10;
			#else
				#ifdef __BeOS
				time= (glut_system_time())/10000;
				#else
				time = times(&voidbuf);
				#endif
			#endif

			dtime+= (time - ltime);
			ltime= time;

			if(dtime<2) usleep(1);
		}

		/* viewrotate */
		if(get_mbut() & M_MOUSE) {
			if(U.flag & VIEWMOVE) {
				if(G.qual & LR_SHIFTKEY) viewmove(0);
				else if(G.qual & LR_CTRLKEY) viewmove(2);
				else viewmove(1);
			}
			else {
				if(G.qual & LR_SHIFTKEY) viewmove(1);
				else if(G.qual & LR_CTRLKEY) viewmove(2);
				else viewmove(0);
			}
		}


		if(G.simulf & (G_RESTART|G_QUIT|G_SETSCENE|G_LOADFILE)) break;
	}

	end_sectors();
	end_lifes(event!=SPACEKEY);
	
	set_dtime(2);	/* restore */
	G.qual= 0;
	
	if(G.simulf & G_LOADFILE) {
		char str[64];
		/* two levels of load events. first we must make sure we leave the simul, then we can add to the queue */
		sprintf(str, "%s.blend", simul_load_str);
		add_readfile_event(str);
	}
	
	/* PRINT4(d, d, d, d, sectcount, facecount, cylcount, spherecount); */
	/* PRINT(d, matcount); */

}

void sector_simulate()
{
	Scene *lastscene;
	Object *lastcamera;
	
	if(G.vd==0) return;	

	lastscene= G.scene;
	lastcamera= G.vd->camera;
	
	G.f |= G_SIMULATION;
	G.simulf= 0;
		
	waitcursor(1);

	while(TRUE) {
		
		sector_go();
		
		if(G.simulf & G_LOADFILE) break;
		if(G.simulf & G_SETSCENE) {
			extern Scene *actuator_scene;	/* life.c */
			if(actuator_scene) set_scene(actuator_scene);
		}
		if(G.simulf & G_RESTART);
		if(G.simulf & G_QUIT) {
			reset_slowparents();	/* editobject.c */
			break;
		}
		G.simulf= 0;
	}

	/* this in fact should be solved for SPACE... */
	if(G.scene!=lastscene) set_scene(lastscene);
	G.vd->camera= lastcamera;
	
	G.f &= ~G_SIMULATION;
	allqueue(REDRAWVIEW3D, 0);
	allqueue(REDRAWBUTSALL, 0);
	
	waitcursor(0);
}


