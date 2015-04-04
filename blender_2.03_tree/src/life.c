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



/*  life.c      MIXED MODEL
 * 
 *  maart 96
 *  
 * 
 * Version: $Id: life.c,v 1.29 2000/09/25 22:02:54 ton Exp $
 */

#include "blender.h"
#include "screen.h"
#include "graphics.h"
#include "ipo.h"
#include "group.h"
#include "sector.h"
#include "sound.h"

extern Object *Glifebuf[LF_MAXLIFE];
extern Object *Gsectorbuf[256], *Gcursector;
extern int Gtotsect, Gtotlife, Gmaxsect;
extern int Gdfra;

Scene *actuator_scene=NULL;
int keypressed= 0;
short simulvals[256];

/* ******************************** */

void life_to_local_sector_co(Life *lf)
{
	
	if(lf->sector) {
		/* transformeren naar locale coordinaten */
		
		ApplyMatrix(lf->sector->imat, lf->loc, lf->loc1);
		ApplyMatrix(lf->sector->imat, lf->oldloc, lf->oldloc1);
		
		lf->localaxsize= lf->axsize*lf->sector->sizefac;

		lf->speed1[0]= lf->loc1[0]-lf->oldloc1[0];
		lf->speed1[1]= lf->loc1[1]-lf->oldloc1[1];
		lf->speed1[2]= lf->loc1[2]-lf->oldloc1[2];
	}
}

void life_from_inv_sector_co(Life *lf)
{
	if(lf->sector) {
		/* transformeren vanuit locale coordinaten naar globale */
		
		/* eerst de (nieuwe) eindlocatie */
		lf->loc1[0]= lf->oldloc1[0]+lf->speed1[0];
		lf->loc1[1]= lf->oldloc1[1]+lf->speed1[1];
		lf->loc1[2]= lf->oldloc1[2]+lf->speed1[2];

		ApplyMatrix(lf->sector->obmat, lf->loc1, lf->loc);
		ApplyMatrix(lf->sector->obmat, lf->oldloc1, lf->oldloc);
		if(lf->collision) ApplyMatrix(lf->sector->obmat, lf->colloc, lf->colloc);
		
		lf->speed[0]= lf->loc[0]-lf->oldloc[0];
		lf->speed[1]= lf->loc[1]-lf->oldloc[1];
		lf->speed[2]= lf->loc[2]-lf->oldloc[2];
	}
}

void aerodynamics(Object *ob)
{
	Life *lf;
	float mat[3][3], no[3], eul[3], ang;
	
	lf= ob->life;
	
	ang= lf->speed[0]*ob->obmat[2][0] + lf->speed[1]*ob->obmat[2][1] + lf->speed[2]*ob->obmat[2][2];
	if(ang<0.001) return;	
	
	Crossf(no, lf->speed, (float *)ob->mat[2]);
	

	/* met euler proberen! */
	
	
	Mat3ToEul(mat, eul);
	
	/* beetje interpoleren */
	
}

/* ***************** HET HELE PIPO SPUL ********************* */
/* *****************  (realtime ipo's)  ********************* */


pIpo *make_ob_pipo(Object *ob)
{
	pIpo *pipo;
	ListBase ipokey;
	IpoKey *ik;
	IpoCurve *icu;
	rctf bb;
	float cfra, *fpoin;
	int a, b, nr_elems, type, elemsize, sta, nr_keys=0;
	short *sp;
	
	/* 50 Hz!!! */
	/* elemsize is in float-eenheden, scheelt casten */
	
	if(ob->ipo->curve.first==0) return 0;
	
	/* */
	boundbox_ipo(ob->ipo, &bb);
	
	/* type en elemsize */
	type= 0;
	icu= ob->ipo->curve.first;
	while(icu) {
		if(icu->adrcode>=OB_LOC_X && icu->adrcode<=OB_DLOC_Z) type |= IP_LOC;
		else if(icu->adrcode>=OB_ROT_X && icu->adrcode<=OB_DROT_Z) type |= IP_ROT;
		else if(icu->adrcode>=OB_SIZE_X && icu->adrcode<=OB_SIZE_Z) type |= IP_SIZE;
		else if(icu->adrcode>=OB_COL_R && icu->adrcode<=OB_COL_B) type |= IP_OBCOL;
		icu= icu->next;
	}
	elemsize= 0;

	if(type & IP_OBCOL) elemsize+=3;
	if(type & IP_LOC) elemsize+=3;
	if(type & IP_ROT) elemsize+=3;
	if(type & IP_SIZE) elemsize+=3;
	
	if(elemsize==0) return 0;

	/* de keys tellen */
	ipokey.first= ipokey.last= 0;
	make_ipokey_spec(&ipokey, ob->ipo);

	ik= ipokey.first;
	while(ik) {
		nr_keys++;
		ik= ik->next;
	}
	if(nr_keys<2) nr_keys= 0;

	/* maken */	
	nr_elems= 2*ffloor( bb.xmax-bb.xmin + 0.5) + 1;
	sta= 2*ffloor(bb.xmin+0.5);
	if(nr_elems<1) return 0;
	
	pipo= callocN( sizeof(pIpo) + nr_elems*elemsize*sizeof(float) + nr_keys*sizeof(short), "pipo");
	pipo->type= type;
	pipo->nr_elems= nr_elems;
	pipo->elemsize= elemsize;
	pipo->nr_keys= nr_keys;
	pipo->sta= sta;
	fpoin= (float *)(pipo+1);
	
	a= nr_elems;

	cfra= bb.xmin;
	while(a--) {
		
		calc_ipo(ob->ipo, cfra);
		execute_ipo((ID *)ob, ob->ipo);
		
		if(type & IP_OBCOL) {
			for(b=0; b<3; b++) fpoin[b]= ob->col[b];
			fpoin+= 3;
		}
		if(type & IP_LOC) {
			for(b=0; b<3; b++) fpoin[b]= ob->loc[b]+ob->dloc[b];
			fpoin+= 3;
		}
		if(type & IP_ROT) {
			for(b=0; b<3; b++) fpoin[b]= ob->rot[b]+ob->drot[b];
			fpoin+= 3;
		}
		if(type & IP_SIZE) {
			for(b=0; b<3; b++) fpoin[b]= ob->size[b]+ob->dsize[b];
			fpoin+= 3;
		}
		
		cfra+= 0.5;
		CLAMP(cfra, bb.xmin, bb.xmax);
	}

	/* keys doen */
	if(nr_keys) {
		sp= (short *)fpoin;
		ik= ipokey.first;
		while(ik) {
			*sp= ffloor(2.0*ik->val + 0.5);
			sp++;
			ik= ik->next;
		}
	}
	free_ipokey(&ipokey);

	/* restore */
	do_ob_ipo(ob);
	
	return pipo;
}

void convert_ipo(Object *ob)
{
	/* van gewone ipo naar realtime-systeem */
	pIpo *pipo;
	ListBase *ipobase=0;
	
	if(ob->gameflag & OB_LIFE) {
		Life *lf= ob->life;
		ipobase= &lf->ipo;
	}
	
	if(ipobase==0) return;
	
	if(ob->ipo) {
		pipo= make_ob_pipo(ob);
		if(pipo) addtail(ipobase, pipo);
	}
	
}

void do_obipo(Object *ob, short cur, short delta, float *speed)	/* delta is ook flag! */
{
	/* alleen de realtime versies */
	Life *lf=NULL;
	pIpo *pipo=0;
	float *fpoin, *first;
	int icur;
	char *poin;

	lf= ob->life;
	if(lf==NULL) return;

	pipo= lf->ipo.first;

	while(pipo) {
		if(pipo->type<=IP_FROMOB) {
			
			icur= cur - pipo->sta;
			lf->flag1 |= LF_CALC_MATRIX;
			
			if(delta==0) {		/* standaard ipo */
			
				CLAMP(icur, 0, pipo->nr_elems-1);
				fpoin= ((float *)(pipo+1))+icur*pipo->elemsize;
				
				if(pipo->type & IP_OBCOL) {
					VECCOPY(ob->col, fpoin);
					fpoin+= 3;
				}
				if(pipo->type & IP_LOC) {
					VECCOPY(ob->loc, fpoin);
					fpoin+= 3;
				}
				if(pipo->type & IP_ROT) {
					VECCOPY(lf->rot, fpoin);
					fpoin+= 3;
				}
				if(pipo->type & IP_SIZE) {
					VECCOPY(ob->size, fpoin);
				}
			}
			else {				/* delta ipo */

				/*  deze test is niet overbodig: interval sensors != interval ipo */
				if(delta==1 && (icur<=0 || icur>= pipo->nr_elems));
				else if(delta== -1 && (icur<0 || icur>= pipo->nr_elems-1));
				else {
					
					fpoin= ((float *)(pipo+1))+icur*pipo->elemsize;
					first= fpoin - delta*pipo->elemsize;
				
					if(pipo->type & IP_OBCOL) {
						ob->col[0]+= fpoin[0] - first[0];
						ob->col[1]+= fpoin[1] - first[1];
						ob->col[2]+= fpoin[2] - first[2];
						fpoin+= 3; first+= 3;
					}
					if(pipo->type & IP_LOC) {
						if(speed) {
							speed[0]+= fpoin[0] - first[0];
							speed[1]+= fpoin[1] - first[1];
							speed[2]+= fpoin[2] - first[2];
						}
						else {
							ob->loc[0]+= fpoin[0] - first[0];
							ob->loc[1]+= fpoin[1] - first[1];
							ob->loc[2]+= fpoin[2] - first[2];
						}
						fpoin+= 3; first+= 3;
					}
					if(pipo->type & IP_ROT) {
						lf->rot[0]+= fpoin[0] - first[0];
						lf->rot[1]+= fpoin[1] - first[1];
						lf->rot[2]+= fpoin[2] - first[2];
						fpoin+= 3; first+= 3;
					}
					if(pipo->type & IP_SIZE) {
						ob->size[0]+= fpoin[0] - first[0];
						ob->size[1]+= fpoin[1] - first[1];
						ob->size[2]+= fpoin[2] - first[2];
					}				
				}
				
			}
			
		}
		pipo= pipo->next;
	}
	
}

void do_force_obipo(Object *ob, short cur, float *force, float *omega)
{
	/* alleen de realtime versies */
	pIpo *pipo=0;
	float *fpoin, *first, vec[3];
	int icur;
	
	if(ob->life) {
		Life *lf= ob->life;
		pipo= lf->ipo.first;
	}
	
	while(pipo) {
		if(pipo->type<=IP_FROMOB) {
			
			icur= cur - pipo->sta;
			
			if(icur<=0 || icur>= pipo->nr_elems);
			else {
				
				fpoin= ((float *)(pipo+1))+icur*pipo->elemsize;
				first= fpoin - pipo->elemsize;
			
				if(pipo->type & IP_OBCOL) {
					VECCOPY(ob->col, fpoin);
					fpoin+= 3; first+= 3;
				}
				if(pipo->type & IP_LOC) {
					vec[0]= fpoin[0] - first[0];
					vec[1]= fpoin[1] - first[1];
					vec[2]= fpoin[2] - first[2];
					Mat4Mul3Vecfl(ob->obmat, vec);
					VecAddf(force, force, vec);
					
					fpoin+= 3; first+= 3;
				}
				if(pipo->type & IP_ROT) {
					omega[0]+= fpoin[0] - first[0];
					omega[1]+= fpoin[1] - first[1];
					omega[2]+= fpoin[2] - first[2];
				}
			}
		}
		pipo= pipo->next;
	}
}

/* return 1: einde bereikt */
int set_k2k_interval(short mode, bIpoActuator *ia, Life *lf)
{
	pIpo *pipo;
	short a, *sp;

	pipo= lf->ipo.first;
	while(pipo) {
		if(pipo->type<=IP_FROMOB) {
			if(pipo->nr_keys) {
				sp=  (short *)(((float *)(pipo+1))+ pipo->nr_elems*pipo->elemsize);

				if(mode==0) {		/* forw first */
					ia->sta= sp[0];
					ia->end= sp[1];
					lf->cfra= ia->sta;
				}
				else if(mode==2) {		/* backw last */
					sp += (pipo->nr_keys-1);
					ia->sta= sp[0];
					ia->end= sp[-1];
					lf->cfra= ia->sta;
				}
				else if(mode==1) {	/* next */
				
					a= pipo->nr_keys-1;
					while(a--) {
						if(sp[0]==lf->cfra) {
							ia->sta= sp[0];
							ia->end= sp[1];
							return 0;
						}
						sp++;
					}
					/* we zijn hier: eind bereikt */
					if(lf->cfra==sp[0]) return 1;
					/* of niets te vinden */
					return 2;
					
				}
				else if(mode== -1) {	/* prev */
				
					a= pipo->nr_keys-1;
					
					/* kan niet verder terug */
					if(lf->cfra==sp[0]) return 1;
					
					sp++;
					while(a--) {
						if(sp[0]==lf->cfra) {
							ia->sta= sp[0];
							ia->end= sp[-1];
							return 0;
						}
						sp++;
					}
					/* we zijn hier: niets te vinden */
					return 2;
				}
			}
		}
		pipo= pipo->next;
	}
	
	return 0;
}




/* *********************** END IPO ******************************** */

/* *********************** DEBUG ********************* */

int dbswaptime;		/* set in sector.c */


typedef struct ActionDebug {
	
	struct ActionDebug *next, *prev;
	bActuator *ac;
	int timer;
	
} ActionDebug;

ListBase ADbase= {0, 0};

void add_action_debug(bActuator *ac, int timer)
{
	ActionDebug *ad;
	
	ad= mallocN(sizeof(ActionDebug), "adbug");
	addtail(&ADbase, ad);
	ad->ac= ac;
	ad->timer= timer;
}

void print_gamedebug_line(Object *ob, bProperty *prop, int yco)
{	
	char str[256];

	glRasterPos2s(10, curarea->winy-yco);

	fmprstr(ob->id.name+2);
	
	switch(prop->type) {
	case PROP_BOOL:
	case PROP_INT:
		sprintf(str , "   %s %d", prop->name, prop->data);
		break;
	case PROP_FLOAT:
		sprintf(str , "   %s %f", prop->name, *((float *)&prop->data));
		break;
	case PROP_STRING:
		sprintf(str , "   %s %s", prop->name, prop->poin);
		break;
	case PROP_TIME:
		sprintf(str , "   %s %d", prop->name, (Gdfra - prop->data)/2);
		break;
	}

	fmprstr(str);
	
}

void draw_gamedebug_info()
{
	ActionDebug *ad, *adn;
	bProperty *prop;
	Object *ob;
	Life *lf;
	bActuator *ac;
	int a, b, yco;
	char str[256];
	
	if(curarea->headertype==0) return;

	persp(0);
	cpack(0xFFFFFF);
	fmsetfont(G.fonts);

	glRasterPos2s(curarea->winx/2 -40, curarea->winy-14);
	sprintf(str, "swap: %d", dbswaptime);
	fmprstr(str);

	if(G.vd->drawtype==OB_TEXTURE) return;

	a= 0;
	ad= ADbase.first;
	while(ad) {
		adn= ad->next;
		ad->timer--;
		
		if(ad->timer==0) {
			remlink(&ADbase, ad);
			freeN(ad);
		}
		else {
			a++;
			glRasterPos2s(curarea->winx-150, curarea->winy-14*a);

			/* switch(ad->ac->type) { */
			/* case ACT_LOADFILE: */
			/* 	fmprstr("LOAD: "); */
			/* 	fmprstr(ad->ac->name); */
			/* 	break; */
			/* case ACT_PLAYMOVIE: */
			/* 	fmprstr("PLAY MOVIE: "); */
			/* 	fmprstr(ad->ac->name); */
			/* 	break; */
			/* default: */
			/* 	sprintf(str, "Action %d\n", ad->ac->action); */
			/* 	fmprstr(str); */
			/* } */
		}
		
		ad= adn;
	}
	
	yco= 14;

	if(Gcursector) {
		for(a=0; a<Gcursector->lbuf.tot; a++) {
			ob= Gcursector->lbuf.ob[a];

			prop= ob->prop.first;
			while(prop) {
		
				if(prop->flag & PROP_DEBUG) {
					print_gamedebug_line(ob, prop, yco);
					yco+= 14;
				}
				prop= prop->next;
			}
		}
	}

	for(a=0; a<Gtotlife; a++) {
		ob= Glifebuf[a];
		
		prop= ob->prop.first;
		while(prop) {

			if(prop->flag & PROP_DEBUG) {
				print_gamedebug_line(ob, prop, yco);
				yco+= 14;
			}
			prop= prop->next;
		}
		
		lf= ob->life;
		if(lf==NULL) continue;
		
		for(b=0; b<lf->links.tot; b++) {
			
			ob= lf->links.ob[b];

			prop= ob->prop.first;
			while(prop) {
	
				if(prop->flag & PROP_DEBUG) {
					print_gamedebug_line(ob, prop, yco);
					yco+= 14;
				}
				prop= prop->next;
			}
		}
	}

	persp(1);	
}

/* ******************************* END DEBUG ************************** */

/* NOTITIE: wat te doen met layers? */

void add_sens_to_cont(bSensor *sens, bController *cont)
{
	bSensor **sar;
	
	sar= mallocN(sizeof(void*) * (cont->totslinks + 1), "cont sens");
	
	if(cont->totslinks) {
		memcpy(sar, cont->slinks, sizeof(void *)*cont->totslinks);
		freeN(cont->slinks);
	}
	sar[cont->totslinks]= sens;
	cont->slinks= sar;
	cont->totslinks++;
}

void rem_sens_from_cont(bSensor *sens, bController *cont)
{
	int a;
	
	for(a=0; a<cont->totslinks; a++) {
		if(cont->slinks[a]==sens) {
			cont->slinks[a]= cont->slinks[ cont->totslinks-1];
			cont->totslinks--;
			break;
		}
	}
}

int has_robbie(Object *ob)
{
	bActuator *act;
	
	act= ob->actuators.first;
	while(act) {
		if(act->type==ACT_CAMERA) return 1;
		act= act->next;
	}
	return 0;
}

void init_lifes()		/* bij start simulation */
{
	extern Object *main_actor;
	Life *lf;
	bProperty *prop;
	bSensor *sens;
	bController *cont;
	bActuator *act;
	Base *base;
	Object *ob, *par, *actor=NULL;	/* for when a user didn't set the main_actor */
	float temp, fvec[3], *fp;
	int a, b, c;
	
	Gtotlife= 0;
	main_actor= 0;
	actuator_scene= NULL;

	/* fmodf(0.2, 2PI) is 0.2
	 * fmodf(-0.2, 2PI) is -0.2
	 * de integerversie is '%'
	 */

	/* for dupli life */
	clear_sca_new_poins();

	/* well, do we think for the user? */
	if(G.vd->camera) G.vd->camera->gameflag |= OB_ACTOR;

	base= FIRSTBASE;
	while(base) {
		ob= base->object;
		
		if ELEM4(ob->type, OB_LAMP, OB_MESH, OB_EMPTY, OB_CAMERA) {
			if(ob->gameflag & OB_LIFE) {
				lf=ob->life= callocN(sizeof(Life), "life");
				
				if(ob->gameflag & OB_DYNAMIC) ob->size[0]= ob->size[1]= ob->size[2]= 1.0;
				
				/* detect child status, only for links to dyna's or actors */
				ob->gameflag &= ~OB_CHILD;
				par= ob->parent;
				while(par && par->parent) {
					par= par->parent;
				}
				
				if(par) {
					if( (par->gameflag & OB_DYNAMIC) || has_robbie(par) ) {
						ob->gameflag &= ~OB_LIFE;
						ob->gameflag |= OB_CHILD|OB_ACTOR;
					}
				}
				
				if(ob->gameflag & OB_MAINACTOR) ob->gameflag |= OB_ACTOR;
				
				lf->ob= ob;
				lf->axsize= ob->inertia;
				lf->flag= ob->gameflag;

				lf->flag1= 0;
				lf->timer= -1;
				lf->frictfac= 1.0;
				lf->oldmesh= ob->data;
			}
		}
		base= base->next;
	}

	base= FIRSTBASE;
	while(base) {
		ob= base->object;
		
		/* do it here for camera */
		if(ob->life) {
			VECCOPY(ob->life->startloc, ob->loc);
			VECCOPY(ob->life->startrot, ob->rot);
		}
		
		if(ob->life) {			
			
			lf= ob->life;
			if(lf->flag & OB_MAINACTOR) {
				
				/* add a warning here when too many main_actors */
				main_actor= ob;
			}
			
			/* !!! */ 
			VECCOPY(lf->loc, ob->obmat[3]);	
			VECCOPY(lf->rot, lf->startrot);
			
			VECCOPY(lf->oldloc, lf->loc);
			
			convert_ipo(ob);
				
			do_obipo(ob, 0, 0, 0);
			
			where_is_object_simul(ob);
			Mat4Invert(ob->imat, ob->obmat);
			Mat4CpyMat4(lf->oldimat, ob->imat);
			
			fp= base->object->imat[0];
			base->object->sizefac= fsqrt(fp[0]*fp[0] + fp[1]*fp[1] + fp[2]*fp[2]);

			if(ob->type==OB_MESH) {
				Mesh *me= ob->data;
				if(me->flag & ME_ISDONE);
				else {
					init_dynamesh(ob, me);
					me->flag |= ME_ISDONE;
				}
			}
			
			lf->sector= find_sector(lf->loc, lf->loc1);
			life_to_local_sector_co(lf);
			
			if(has_robbie(ob)) lf->flag |= OB_MAINACTOR;

			/* alleen zichtbare, ivm ADDLIFE */
			if(base->lay & G.scene->lay ) {
				
				if(lf->flag & OB_MAINACTOR) add_dyna_life(ob);
				else if(lf->flag & OB_DYNAMIC) {
					
					if(lf->sector) add_to_lbuf(&(lf->sector->lbuf), ob);
					
					actor= ob;	/* for when a user forgets to set main_actor */
				}
				else if(lf->flag & (OB_CHILD|OB_ACTOR)) {
					Object *par;
					Life *lfp;

					if( (lf->flag & (OB_CHILD|OB_ACTOR))==OB_ACTOR) {
						if(lf->sector) add_to_lbuf(&(lf->sector->lbuf), ob);
					}
					else {
						par= ob->parent;
						while(par && par->parent) par= par->parent;
						
						if(par) {
							lfp= par->life;
							add_to_lbuf(&(lfp->links), ob);
						}
						else {
							if(lf->sector) add_to_lbuf(&(lf->sector->lbuf), ob);
						}
					}
				}
				else if(lf->sector && (lf->flag & OB_PROP)) {
				
					add_to_lbuf(&(lf->sector->lbuf), ob);

					/* meerdere sectoren? */
					/* po= lf->sector->portals; */
					/* a= lf->sector->totport; */
					
					/* while(a--) { */
					/* 	if(po->sector) { */
					/* 		if( sector_intersect(po->sector, ob)) { */
					/* 			add_to_lbuf(&(po->sector->lbuf), ob); */
					/* 		} */
					/* 	} */
					/* 	po++; */
					/* } */
				}
				else if(ob->type==OB_CAMERA) add_dyna_life(ob);
				
			}
		}

		prop= ob->prop.first;
		while(prop) {
			prop->old= prop->data;
			if(prop->poin && prop->poin != &prop->data) prop->oldpoin= dupallocN(prop->poin);
			if(prop->type==PROP_TIME) {
				prop->data= Gdfra - 2*prop->data;
			}
			prop= prop->next;
		}
		
		/* link controllers to sensors, init sensors */
		sens= ob->sensors.first;
		while(sens) {
			sens->ob= ob;
			for(a=0; a<sens->totlinks; a++) {
				add_sens_to_cont(sens, sens->links[a]);
				if(sens->type==SENS_NEAR) {
					bNearSensor *ns=sens->data;
					ns->lastval= 0;
					if(ns->resetdist < ns->dist) ns->resetdist= ns->dist;
				}
				else if(sens->type==SENS_PROPERTY) {
					bPropertySensor *ps=sens->data;
					if(ps->type==SENS_PROP_CHANGED) {
						prop= get_property(ob, ps->name);
						if(prop) {
							set_property_valstr(prop, ps->value);
						}
						else ps->value[0]= 0;
					}
				}
			}
			sens= sens->next;
		}
		
		/* some of these should be redone at add_dupli_life */		
		cont= ob->controllers.first;
		while(cont) {
			cont->val= 0;
			cont->valo= 0;
			cont= cont->next;
		}
		
		act= ob->actuators.first;
		while(act) {
			act->go= 0;
			if(act->type==ACT_OBJECT) {
				bObjectActuator *oa;
				oa= act->data;
				VecMulf(oa->forceloc, DTIME);
				VecMulf(oa->forcerot, DTIME);
				VecMulf(oa->dloc, DTIME);
				VecMulf(oa->drot, M_PI/180.0);
			}
			else if(act->type==ACT_IPO) {
				bIpoActuator *ia;
				ia= act->data;
				ia->sta= 2*ia->sta;
				ia->end= 2*ia->end;
				ia->cur= ia->sta;
				ia->flag &= ~ACT_IPOEND;
				
				if(ia->type==ACT_IPO_KEY2KEY) {
					/* always set at first key */
					set_k2k_interval(0, ia, lf);
					
					prop= get_property(ob, ia->name);
					if(prop) set_property(prop, "1");

					ia->cur= ia->sta;
				}
				
			}
			else if(act->type==ACT_GROUP) {
				bGroupActuator *ga;
				ga= act->data;
				ga->sta= 2*ga->sta;
				ga->end= 2*ga->end;
				ga->cur= ga->sta;
				ga->flag &= ~ACT_IPOEND;
				
				ga->group= find_group(ob);
			}
			
			act->ob= ob;
			act= act->next;
		}

		base= base->next;
	}
	
	if(main_actor==NULL) main_actor= actor;

}

void end_lifes(int restore)
{
	Life *lf;
	Base *base;
	Object *ob;
	bProperty *prop;
	bSensor *sens;
	bController *cont;
	bActuator *act;
	int a, b;
	
	del_dupli_lifes();

	/* the scene... */
	base= FIRSTBASE;
	while(base) {
		/* layer can be set in game... */
		base->object->lay= base->lay;
		
		ob= base->object;
		
		prop= ob->prop.first;
		while(prop) {
			prop->data= prop->old;
			if(prop->poin && prop->poin != &prop->data) {
				freeN(prop->poin);
				prop->poin= prop->oldpoin;
			}
			prop= prop->next;
		}

		sens= ob->sensors.first;
		while(sens) {
			sens->ob= NULL;
			sens= sens->next;
		}
		cont= ob->controllers.first;
		while(cont) {
			if(cont->slinks) freeN(cont->slinks);
			cont->totslinks= 0;
			cont->slinks= NULL;
			cont= cont->next;
		} 
		act= ob->actuators.first;
		while(act) {
			if(act->type==ACT_OBJECT) {
				bObjectActuator *oa;
				oa= act->data;
				VecMulf(oa->forceloc, 1.0/DTIME);
				VecMulf(oa->forcerot, 1.0/DTIME);
				VecMulf(oa->dloc, 1.0/DTIME);
				VecMulf(oa->drot, 180.0/M_PI);
			}
			else if(act->type==ACT_IPO) {
				bIpoActuator *ia;
				ia= act->data;
				ia->sta= ia->sta/2;
				ia->end= ia->end/2;
			}
			else if(act->type==ACT_GROUP) {
				bGroupActuator *ga;
				ga= act->data;
				ga->sta= ga->sta/2;
				ga->end= ga->end/2;
			}
			act->ob= NULL;
			act= act->next;
		}
	
		if(ob->life) {
		
			lf= ob->life;
			
			if(restore || (lf->flag & OB_PROP)) {
				VECCOPY(ob->loc, lf->startloc);
				VECCOPY(ob->rot, lf->startrot);
				where_is_object(ob);
			}
			else {
				VecSubf(ob->loc, ob->loc, ob->dloc);
				
			}
			
			if(ob->type==OB_MESH) {
				end_dynamesh(ob->data);
				if(lf->oldmesh!=ob->data) end_dynamesh(lf->oldmesh);
				ob->data= lf->oldmesh;
			}
			
			freelistN(&lf->ipo);
			free_lbuf(&lf->links);

			freeN(ob->life);
			ob->life= NULL;
		}
		
		base= base->next;
	}
}

void init_devs()
{
	bzero(simulvals, 256*2);
}


/* iets soortgelijks op de psx maken: combinatie van getbutton en qread */
/* het geheugen (hold, vorige stand) zit in sensors zelf */
short pad_read()
{
	short a, val;
	ushort event= 0;
	
	
	/* queue lezen */
	while(qtest()) {
		
		event= special_qread(&val);
		
		if(event==SPACEKEY || event==ESCKEY) break;	

		if(event>20) {
			simulvals[ event & 255 ] = val;
		}
	}
	return event;
}

void Mat3ToEulFast(mat, eul)
float mat[][3], *eul;
{
	float cy;
	
	cy = fsqrt(mat[0][0]*mat[0][0] + mat[0][1]*mat[0][1]);

	if (cy > 16.0*FLT_EPSILON) {
		eul[0] = fatan2(mat[1][2], mat[2][2]);
		eul[1] = fatan2(-mat[0][2], cy);
		eul[2] = fatan2(mat[0][1], mat[0][0]);
	} else {
		eul[0] = fatan2(-mat[2][1], mat[1][1]);
		eul[1] = fatan2(-mat[0][2], cy);
		eul[2] = 0.0;
	}
}

void compatible_eulFast(float *eul, float *oldrot)
{
	float dx, dy, dz;
	
	/* verschillen van ong 360 graden corrigeren */

	dx= eul[0] - oldrot[0];
	dy= eul[1] - oldrot[1];
	dz= eul[2] - oldrot[2];

	if( fabs(dx) > 5.1) {
		if(dx > 0.0) eul[0] -= 2.0*M_PI; else eul[0]+= 2.0*M_PI;
	}
	if( fabs(dy) > 5.1) {
		if(dy > 0.0) eul[1] -= 2.0*M_PI; else eul[1]+= 2.0*M_PI;
	}
	if( fabs(dz) > 5.1 ) {
		if(dz > 0.0) eul[2] -= 2.0*M_PI; else eul[2]+= 2.0*M_PI;
	}
}


void track_life_to_life(Life *lf, Life *to, float fac, short mode)
{
	float *quat, vec[3], mat[3][3];
	
	if(lf==NULL || to==NULL) return;
	
	vec[0]= lf->loc[0] - to->loc[0];
	vec[1]= lf->loc[1] - to->loc[1];
	vec[2]= lf->loc[2] - to->loc[2];
	
	if(mode==0) {	/* alleen z-rot */
		vec[2]= fatan2(vec[1], vec[0]);		/* -x */
		
		if(lf->ob->trackflag==0) vec[2]+= M_PI;				/* x */
		else if(lf->ob->trackflag==1) vec[2]+= 0.5*M_PI;	/* y */
		else if(lf->ob->trackflag==4) vec[2]-= 0.5*M_PI;	/* -y */
		
		vec[0]= vec[1]= 0;
	}
	else {
		quat= vectoquat(vec, lf->ob->trackflag, lf->ob->upflag);
		QuatToEul(quat, vec);
	}
	
	if(fac==0.0) {
		VECCOPY(lf->rot, vec);
	}
	else {
		/* lf rot aanpassen !!!*/
		compatible_eulFast(lf->rot, vec);
		
		lf->rot[0]= (fac*lf->rot[0] + vec[0])/(1.0+fac);
		lf->rot[1]= (fac*lf->rot[1] + vec[1])/(1.0+fac);
		lf->rot[2]= (fac*lf->rot[2] + vec[2])/(1.0+fac);
	}
	
	lf->flag1 |= LF_CALC_MATRIX;
}

void camera_behaviour(Object *cam, Life *lfcam, bCameraActuator *ac)
{
	Object *actor;
	Life *lfactor;
	float *fp1, *fp2, mindistsq, maxdistsq;
	float inp, fac, distsq, mat[3][3], lookat[3], from[3], rc[3];
	short a, ok;
	
	actor= ac->ob;
	lfactor= actor->life;
	mindistsq= ac->min*ac->min;
	maxdistsq= ac->max*ac->max;

	/* init */
	lookat[0]= lfactor->loc[0];
	lookat[1]= lfactor->loc[1];
	lookat[2]= lfactor->loc[2];
	
	/* floorloc is relative to lf->actor->loc */
	inp= MAX2(lfactor->floorloc[2], -5.0 * (lfactor->axsize));

	lookat[2] +=  inp;
	
	from[0]= cam->loc[0];
	from[1]= cam->loc[1];
	from[2]= cam->loc[2];

	/* CONSTRAINT 1: staat camera goed geroteerd in sector (90 graden grid)? */

	/* CONSTRAINT 2: kan cam actor zien? */
	/*				 niet vanuit schaduw!!! */

	ok= test_visibility(lfactor->loc, from, lfcam, lfactor->sector);

	/* if(ok==0 && lfactor->sector) { */
	/* 	a= lfactor->sector->totport; */
	/* 	po= lfactor->sector->portals; */
	/* 	while(a--) { */
	/* 		if( test_visibility(lfactor->loc, from, po->sector) ) break; */
	/* 		po++; */
	/* 	} */
	/* } */
	
	/* CONSTRAINT 3: vaste hoogte boven schaduw */
	from[2]= (15.0*from[2] + lookat[2] + ac->height)/16.0;
	
	/* CONSTRAINT: achterliggende camera */
	if(TRUE) {		/* here should come a check for a key being pressed */

		if(ac->axis=='x') {
			fp1= actor->obmat[0];
			fp2= cam->obmat[0];
		}
		else {
			fp1= actor->obmat[1];
			fp2= cam->obmat[1];
		}

		inp= fp1[0]*fp2[0] + fp1[1]*fp2[1] + fp1[2]*fp2[2];
		fac= (-1.0 + inp)/32.0;

		from[0]+= fac*fp1[0];
		from[1]+= fac*fp1[1];
		from[2]+= fac*fp1[2];
		
		/* alleen alstie ervoor ligt: cross testen en loodrechte bijtellen */
		if(inp<0.0) {
			if(fp1[0]*fp2[1] - fp1[1]*fp2[0] > 0.0) {
				from[0]-= fac*fp1[1];
				from[1]+= fac*fp1[0];
			}
			else {
				from[0]+= fac*fp1[1];
				from[1]-= fac*fp1[0];
			}
		}
	}

	/* CONSTRAINT 4: minimum / maximum afstand */

	rc[0]= (lookat[0]-from[0]);
	rc[1]= (lookat[1]-from[1]);
	rc[2]= (lookat[2]-from[2]);
	distsq= rc[0]*rc[0] + rc[1]*rc[1] + rc[2]*rc[2];

	if(distsq > maxdistsq) {
		distsq = 0.15*(distsq-maxdistsq)/distsq;
		
		from[0] += distsq*rc[0];
		from[1] += distsq*rc[1];
		from[2] += distsq*rc[2];
	}
	else if(distsq < mindistsq) {
		distsq = 0.15*(mindistsq-distsq)/mindistsq;
		
		from[0] -= distsq*rc[0];
		from[1] -= distsq*rc[1];
		from[2] -= distsq*rc[2];
	}

	/* CONSTRAINT 4a: nog eens vaste hoogte boven schaduw */
	from[2]= (20.0*from[2] + lookat[2] + ac->height)/21.0;

	/* CONSTRAINT 5: track naar schaduw */
	rc[0]= (lookat[0]-from[0]);
	rc[1]= (lookat[1]-from[1]);
	rc[2]= (lookat[2]-from[2]);
	VecUpMat3(rc, mat, 3);	/* y up Track -z */
	

	/* CONSTRAINT: klein beetje met aktie meekijken: projecteer x-vec op scherm? */

	fp1= actor->obmat[0];
	fp2= G.vd->viewinv[0];
		
	inp= 0.2*(fp2[0]*fp1[0] + fp2[1]*fp1[1] + fp2[2]*fp1[2]);

	ac->fac= (15.0*ac->fac + inp)/16.0;

	Mat3ToEulFast(mat, cam->rot);
	cam->rot[2]-= ac->fac;

	/* patch, for update_motion */
	VECCOPY(lfcam->rot, cam->rot);	
	
	VECCOPY(cam->loc, from);
	lfcam->flag1 |= LF_CALC_MATRIX;
}


int dyna_near_life(Object *prob, float mindist, char *propname)
{
	Life *lf;
	Object *ob;
	float *vec, *test, dist;
	int a;
	
	/* alle dyna lifes */
	test= prob->obmat[3];
	
	a= Gtotlife;
	while(a--) {
		ob= Glifebuf[a];
		if( prob != ob && (ob->gameflag & OB_LIFE)) {
			lf= ob->life;
			if( (lf->flag & OB_DYNAMIC)) {
				vec= ob->obmat[3];

				/**/
				dist= fabs(vec[0]-test[0]);

				if(dist<mindist) {
					dist= fabs(vec[1]-test[1]);
					if(dist<mindist) {
						dist= fabs(vec[2]-test[2]);
						if(dist<mindist) {
							if(propname[0]) {
								if(get_property(ob, propname)) return 1;
							}
							else return 1;
						}
					}
				}
			}
		}
	}

	return 0;
}

int is_life_visible(Object *prob, char *propname, float angle, short mode)
{
	Object *ob;
	bProperty *prop;
	float vec[3], view[3], inp; 
	short a, test;

	if(mode==0) {
		VECCOPY(view, prob->obmat[0]);
	}
	else if(mode==1) {
		VECCOPY(view, prob->obmat[1]);
	}
	else {
		VECCOPY(view, prob->obmat[2]);
	}
	
	Normalise(view);
	
	a= Gtotlife;
	while(a--) {
		ob= Glifebuf[a];
		if( prob != ob && (ob->gameflag & OB_LIFE)) {
			prop= get_property(ob, propname);
			if(prop) {
				VecSubf(vec, prob->obmat[3], ob->obmat[3]);
				Normalise(vec);
				inp= view[0]*vec[0]+ view[1]*vec[1]+ view[2]*vec[2];
		
				inp= 180.0 - 180.0*safacos(inp)/M_PI;

				if( inp < angle) return 1;
			}
		}
	}

	return 0;	
}

int test_sensor(bSensor *sens)
{
	extern int Gdfras;
	bKeyboardSensor *ks;
	bNearSensor *ns;
	bPropertySensor *ps;
	bTouchSensor *ts;
	bCollisionSensor *cs;
	bRadarSensor *rs;
	bProperty *prop;
	Object *ob;
	Life *lf;
	int h1, h2, event, val, val2;
	
	ob= sens->ob;
	
	switch(sens->type) {
	case SENS_ALWAYS:
		if(sens->pulse) {
			val= Gdfra % (sens->freq+1);
			if(val==0) return 1;
			else return 0;
		}
		else return 1;
		break;
	case SENS_KEYBOARD:
		ks= sens->data;
		if(ks->type==SENS_ALL_KEYS) {
			for(h1=0; h1<256; h1++) if(simulvals[h1]) return 1;
		}
		else if( simulvals[ ks->key & 255]) {
			h1= h2= 0;
			
			if( ks->qual ) h1= simulvals[ ks->qual & 255];
			if( ks->qual2 ) h2= simulvals[ ks->qual2 & 255];
			
			if( ks->qual && ks->qual2) {
				if(h1 && h2) return 1;
			}
			else if( ks->qual) {
				if(h1) return 1;
			}
			else if( ks->qual2) {
				if(h2) return 1;
			}
			else {
				return 1;
			}
		}
		break;
	case SENS_NEAR:

		/* if a sensor is not handled anymore (end object) */
		if(ob->dfras+1 < Gdfras) return 0;
	
		ns= sens->data;
		if(ns->lastval) event= dyna_near_life(ob, ns->resetdist, ns->name);
		else event= dyna_near_life(ob, ns->dist, ns->name);

		if(event) {
			ns->lastval= 1;
			ob->life->flag1 |= LF_DRAWNEAR;
		}
		else {
			ns->lastval= 0;
			ob->life->flag1 &= ~LF_DRAWNEAR;
		}
		return event;
		
	case SENS_RADAR:
		
		/* sensors are called from a controller, can be in different layers */
		if((ob->lay & G.scene->lay)==0) return 0;

		rs= sens->data;
		if(  is_life_visible(ob, rs->name, rs->angle, rs->axis) ) {
			ob->life->flag1 |= LF_DRAWNEAR;
			return 1;
		}
		else {
			ob->life->flag1 &= ~LF_DRAWNEAR;
		}
		break;
		
	case SENS_PROPERTY:
		
		ps= sens->data;
		prop= get_property(ob, ps->name);
		if(prop) {
			val= compare_property(prop, ps->value);
			
			if(ps->type==SENS_PROP_EQUAL) {
				if(val==0) return 1;
			}
			else if(ps->type==SENS_PROP_NEQUAL) {
				if(val!=0) return 1;
			}
			else if(ps->type==SENS_PROP_INTERVAL) {
				val2= compare_property(prop, ps->maxvalue);
				if(val >= 0 && val2 < 0) return 1;
			}
			else if(ps->type==SENS_PROP_CHANGED) {
				if(val==0) return 0;
				else {
					set_property_valstr(prop, ps->value);
					return 1;
				}
			}
		}
		break;
	case SENS_TOUCH:
		ts= sens->data;
		if(ob->life) {
			if(ob->life->contact) {
				if(ts->ma==NULL) return 1;
				else if(ts->ma==ob->life->contact) return 1;
			}
		}
		break;
		
	case SENS_COLLISION:
		cs= sens->data;
		if(ob->life) {
			
			lf= ob->life;
			
			if(lf->type==OB_CHILD) {
				lf->collision= NULL;
				lf->sector= find_sector(lf->loc, lf->loc1);
				if( test_visibility(lf->oldloc, lf->loc, lf, lf->sector) )
					lf->collision= ob;
			}

			if(lf->collision==NULL || cs->damptimer>0) {
				if(cs->damptimer>0) cs->damptimer--;
			}
			else if(lf->collision && cs->name[0]) {
				prop= get_property(lf->collision, cs->name);
				if(prop) {
					cs->damptimer= 2*cs->damp;
					return 1;
				}
			}
			else if(lf->collision) {
				cs->damptimer= 2*cs->damp;
				return 1;
			}
		}
		break;
	}
	
	return 0;
}

void activate_actuator(bActuator *act, short val, short valo)
{
	bObjectActuator *oa;
	bCameraActuator *ca;
	bIpoActuator *ia;
	bPropertyActuator *pa;
	bSoundActuator *sa;
	bProperty *prop;
	bEditObjectActuator *eoa;
	bConstraintActuator *coa;
	bSceneActuator *sca;
	bGroupActuator *ga;
	Object *ob;
	Life *lf;
	float vec[3], *loc;
	int cur, mode;
	
	ob= act->ob;
	lf= ob->life;
	
	if(lf==NULL) return;
	
	switch(act->type) {
	case ACT_OBJECT:
		if(val) {
			oa= act->data;
			
			if(oa->flag & ACT_FORCE_LOCAL) {
				VECCOPY(vec, oa->forceloc);
				Mat4Mul3Vecfl(ob->obmat, vec);
				VecAddf(lf->force, lf->force, vec);	
			}
			else VecAddf(lf->force, lf->force, oa->forceloc);
			
			if(oa->forcerot[0]!=0.0 || oa->forcerot[1]!=0.0 || oa->forcerot[2]!=0.0) {
				VecAddf(lf->omega, lf->omega, oa->forcerot);
				if(oa->flag & ACT_TORQUE_LOCAL) lf->dflag |= LF_ROT_LOCAL;
				else lf->dflag &= ~LF_ROT_LOCAL;
			}
			
			if(oa->flag & ACT_DLOC_LOCAL) {
				VECCOPY(vec, oa->dloc);
				Mat4Mul3Vecfl(ob->obmat, vec);
				VecAddf(lf->dloc, lf->dloc, vec);	
			}
			else VecAddf(lf->dloc, lf->dloc, oa->dloc);
	
			if(oa->drot[0]!=0.0 || oa->drot[1]!=0.0 || oa->drot[2]!=0.0) {
				VecAddf(lf->drot, lf->drot, oa->drot);
				if(oa->flag & ACT_DROT_LOCAL) lf->dflag |= LF_ROT_LOCAL;
				else lf->dflag &= ~LF_ROT_LOCAL;
			}

			lf->flag1 |= LF_CALC_MATRIX;
		}
		break;
		
	case ACT_CAMERA:
		ca= act->data;
		if(ca->ob) {
			camera_behaviour(ob, lf, ca);
		}
		break;
	case ACT_SCENE:
		sca= act->data;
		if(sca->type==ACT_SCENE_SET) {
			actuator_scene= sca->scene;
			if(actuator_scene) G.simulf |= G_SETSCENE;
		}
		else if(sca->type==ACT_SCENE_CAMERA) {
			if(sca->camera) {
				G.vd->camera= G.scene->camera= sca->camera;
			}
		}
		else if(sca->type==ACT_SCENE_RESTART) {
			G.simulf |= G_RESTART;
		}
		break;
	case ACT_SOUND:
		sa= act->data;
		if (valo==0) {	/* KEY_IN */
			if (sa->sound) {
				init_sound(sa->sound);
				play_sound(ob, sa->sound->alindex);
			}
		}	
		break;
	case ACT_PROPERTY:
		pa= act->data;
		prop= get_property(ob, pa->name);
		if(prop) {
			if( valo==0) {	/* KEY_IN */
				if(pa->type==ACT_PROP_ASSIGN) set_property(prop, pa->value);
				else if(pa->type==ACT_PROP_ADD) add_property(prop, pa->value);
			}

			if(val && pa->type==ACT_PROP_COPY) {
				bProperty *propother= get_property(pa->ob, pa->value);
				if(propother) {
					cp_property(prop, propother);
				}
			}
		}
		
		break;
	case ACT_EDIT_OBJECT:
		eoa= act->data;
		if(eoa->type==ACT_EDOB_ADD_OBJECT) {
			if( valo==0) {	/* KEY_IN */
				if(eoa->ob) add_dupli_life(eoa->ob, ob, 2*eoa->time);
			}
		}
		else if(eoa->type==ACT_EDOB_END_OBJECT) {
			if(lf->dflag & LF_TEMPLIFE)
				lf->timer= 0;
			else {
				/* hide */
				ob->lay= 1<<21; 
				if(lf->sector) del_from_lbuf( &(lf->sector->lbuf), ob);
			}
		}
		else if(eoa->type==ACT_EDOB_REPLACE_MESH) {
			if( valo==0) {	/* KEY_IN */
				if(ob->type==OB_MESH && eoa->me) {
					if( (eoa->me->flag & ME_ISDONE)==0) {
						init_dynamesh(ob, eoa->me);
						eoa->me->flag |= ME_ISDONE;
					}
					ob->data= eoa->me;
				}
			}
		}
		else if(eoa->type==ACT_EDOB_TRACK_TO) {
			if(eoa->ob) {
				if(eoa->time) act->go= eoa->time;
				else track_life_to_life(lf, eoa->ob->life, 0.0, eoa->flag);
			}
		}
		break;

	case ACT_CONSTRAINT:

		coa= act->data;
		
		if(coa->damp) {
			coa->slow= 1.0-(1.0/coa->damp);
			coa->slow*= coa->slow*coa->slow;
			act->go= coa->damp;
		}
		else {
			if(lf->flag & OB_DYNAMIC) loc= lf->loc;
			else loc= ob->loc;
			
			switch(coa->flag) {
			case ACT_CONST_LOCX:
				CLAMP(loc[0], coa->minloc[0], coa->maxloc[0]);
				break;
			case ACT_CONST_LOCY:
				CLAMP(loc[1], coa->minloc[1], coa->maxloc[1]);
				break;
			case ACT_CONST_LOCZ:
				CLAMP(loc[2], coa->minloc[2], coa->maxloc[2]);
				break;
			case ACT_CONST_ROTX:
				CLAMP(lf->rot[0], M_PI*coa->minrot[0]/180.0, M_PI*coa->maxrot[0]/180.0);
				break;
			case ACT_CONST_ROTY:
				CLAMP(lf->rot[1], M_PI*coa->minrot[1]/180.0, M_PI*coa->maxrot[1]/180.0);
				break;
			case ACT_CONST_ROTZ:
				CLAMP(lf->rot[2], M_PI*coa->minrot[2]/180.0, M_PI*coa->maxrot[2]/180.0);
				break;
			}
			lf->flag1 |= LF_CALC_MATRIX;
		}
		break;
		
	case ACT_IPO:
		ia= act->data;
		
		switch(ia->type) {
		
		case ACT_IPO_FROM_PROP:
			if(ia->name[0]) {
				prop= get_property(ob, ia->name);
				if(prop) {
					if ELEM(prop->type, PROP_INT, PROP_TIME) cur= 2*prop->data;
					else if(prop->type==PROP_FLOAT) cur= 2* (*((float *)prop->poin));
					
					CLAMPTEST(cur, ia->sta, ia->end);
					if(cur != ia->cur) {
						ia->cur= cur;
	
						do_obipo(ob, ia->cur, 0, lf->dloc);
						lf->flag1 |= LF_CALC_MATRIX;
						
						break;
					}
				}
			}
			break;

		case ACT_IPO_PLAY:
			
			if( valo==0) {	/* KEY_IN */
			
				if(act->go==0) {
					ia->cur= ia->sta;
					if(ia->end > ia->sta) act->go= 1;
					else act->go= -1;
				}
				/* else eventlock= 1; */
			}
			break;
			
		case ACT_IPO_PINGPONG:
			
			if( valo==0) {	/* KEY_IN */
				if(act->go) {
					act->go= -act->go;
				}
				else if(ia->cur==ia->sta) { /* sta== ook initwaarde en rustwaarde */
					if(ia->end > ia->sta) act->go= 1;
					else act->go= -1;
				}
				else if(ia->cur==ia->end) { /* end==rustwaarde */
					if(ia->sta > ia->end) act->go= 1;
					else act->go= -1;
				}
			}
			break;
			
		case ACT_IPO_FLIPPER:
			if( valo==0) {	/* KEY_IN */
				/* niet cur zetten: is flipper! */
				if(ia->end > ia->sta) act->go= 1;
				else act->go= -1;
			}
			else if(val==0) {
				if(ia->end > ia->sta) act->go= -1;
				else act->go= 1;
			}
			break;
			
		case ACT_IPO_LOOP_STOP:
		case ACT_IPO_LOOP_END:
			
			if( valo==0) {	/* KEY_IN */
				
				if(ia->cur==ia->end) {	/* end== ook initwaarde en rustwaarde */
					ia->cur= ia->sta;
				}
				if(ia->end > ia->sta) act->go= 1;
				else act->go= -1;
			}
			else if(val==0) {
				if(ia->type==ACT_IPO_LOOP_STOP) act->go= 0;
				else ia->flag |= ACT_IPOEND;
			}
			break;
			
		case ACT_IPO_KEY2KEY:
			
			if( valo==0  || (val && (ia->flag & ACT_K2K_HOLD)) ) {	/* KEY_IN */
			
				if(act->go==0) {
					prop= get_property(ob, ia->name);
					
					if(ia->flag & ACT_K2K_PREV) {
						mode= set_k2k_interval(-1, ia, lf);
						if(mode!=1 && prop) add_property(prop, "-1");
					}
					else {
						mode= set_k2k_interval(1, ia, lf);
						if(mode!=1 && prop) add_property(prop, "1");
					}
					
					if(mode==1) {	/* extrema bereikt */
						if(ia->flag & ACT_K2K_CYCLIC) {
							if(ia->flag & ACT_K2K_PREV) {
								set_k2k_interval(2, ia, lf);
								if(prop && lf->ipo.first) {
									char str[32];
									sprintf(str, "%d", ((pIpo *)lf->ipo.first)->nr_keys);
									set_property(prop, str);
								}
							}
							else {
								set_k2k_interval(0, ia, lf);
								if(prop) set_property(prop, "1");
							}
							mode= 0;
						}
						else if(ia->flag & ACT_K2K_PINGPONG) {
							if(ia->flag & ACT_K2K_PREV) ia->flag &= ~ACT_K2K_PREV;
							else ia->flag |= ACT_K2K_PREV;
								
							SWAP(short, ia->end, ia->sta);
							mode= 0;
						}
					}

					if(mode==0) {
						ia->cur= ia->sta;
						if(ia->end > ia->sta) act->go= 1;
						else act->go= -1;
					}
					
					/* if(mode==2) { */
					/* 	eventlock= 1;	 */
					/* } */
					
				}
				/* else eventlock= 1; */
			}
			break;
					

		}
		
		break;
	case ACT_GROUP:
		ga= act->data;
		
		switch(ga->type) {
		
		case ACT_GROUP_SET:
			set_group_key_name(ga->group, ga->name);
			set_group_key_frame(ga->group, ((float)ga->sta)/2.0);
			break;
		
		case ACT_GROUP_FROM_PROP:
			if(ga->name[0]) {
				prop= get_property(ob, ga->name);
				if(prop) {
					if ELEM(prop->type, PROP_INT, PROP_TIME) cur= 2*prop->data;
					else if(prop->type==PROP_FLOAT) cur= 2* (*((float *)prop->poin));
					
					CLAMPTEST(cur, ga->sta, ga->end);
					if(cur != ga->cur) {
						ga->cur= cur;
	
						set_group_key_frame(ga->group, ((float)ga->cur)/2.0);
						lf->flag1 |= LF_CALC_MATRIX;
						
						break;
					}
				}
			}
			break;
			
		case ACT_GROUP_PLAY:
			
			if( valo==0) {	/* KEY_IN */
			
				if(act->go==0) {
					ga->cur= ga->sta;
					if(ga->end > ga->sta) act->go= 1;
					else act->go= -1;
				}
			}
			break;
		case ACT_GROUP_PINGPONG:
			
			if( valo==0) {	/* KEY_IN */
				if(act->go) {
					act->go= -act->go;
				}
				else if(ga->cur==ga->sta) { /* sta== ook initwaarde en rustwaarde */
					if(ga->end > ga->sta) act->go= 1;
					else act->go= -1;
				}
				else if(ga->cur==ga->end) { /* end==rustwaarde */
					if(ga->sta > ga->end) act->go= 1;
					else act->go= -1;
				}
			}
			break;
			
		case ACT_GROUP_FLIPPER:
			if( valo==0) {	/* KEY_IN */
				/* niet cur zetten: is flipper! */
				if(ga->end > ga->sta) act->go= 1;
				else act->go= -1;
			}
			else if(val==0) {
				if(ga->end > ga->sta) act->go= -1;
				else act->go= 1;
			}
			break;
			
		case ACT_GROUP_LOOP_STOP:
		case ACT_GROUP_LOOP_END:
			
			if( valo==0) {	/* KEY_IN */
				
				if(ga->cur==ga->end) {	/* end== ook initwaarde en rustwaarde */
					ga->cur= ga->sta;
				}
				if(ga->end > ga->sta) act->go= 1;
				else act->go= -1;
			}
			else if(val==0) {
				if(ga->type==ACT_GROUP_LOOP_STOP) act->go= 0;
				else ga->flag |= ACT_IPOEND;
			}
			break;
			
		}			
				
		break;
		
	}
}

void handle_actuator(bActuator *act)
{
	bObjectActuator *oa;
	bCameraActuator *ca;
	bIpoActuator *ia;
	bConstraintActuator *coa;
	bEditObjectActuator *eoa;
	bGroupActuator *ga;
	Object *ob;
	Life *lf;
	float *fp, *dfp, minval, maxval, vec[3];

	if(act->go==0) return;

	ob= act->ob;
	lf= ob->life;
	
	if(lf==NULL) return;
	
	switch(act->type) {
	
	case ACT_IPO:
		ia= act->data;
		
		lf->flag1 |= LF_CALC_MATRIX;
		
		switch(ia->type) {

		case ACT_IPO_PLAY:
		case ACT_IPO_PINGPONG:
		case ACT_IPO_FLIPPER:
		
			ia->cur+= act->go;
			CLAMPTEST(ia->cur, ia->sta, ia->end);

			if(lf->flag & OB_DYNAMIC) {
				if(ia->flag & ACT_IPOFORCE) {
					do_force_obipo(ob, ia->cur, lf->force, lf->omega);
				}
				else {
					do_obipo(ob, ia->cur, act->go, lf->dloc);
				}
			}
			else do_obipo(ob, ia->cur, 0, 0);

			if(ia->cur==ia->end) act->go= 0;
			break;
			
		case ACT_IPO_LOOP_STOP:
		case ACT_IPO_LOOP_END:
		
			ia->cur+= act->go;
			CLAMPTEST(ia->cur, ia->sta, ia->end);
			
			if(lf->flag & OB_DYNAMIC) {
				if(ia->flag & ACT_IPOFORCE) {
					do_force_obipo(ob, ia->cur, lf->force, lf->omega);
				}
				else {
					do_obipo(ob, ia->cur, act->go, lf->dloc);
				}
			}
			else do_obipo(ob, ia->cur, 0, 0);
			
			if(ia->cur==ia->end) {	/* end== ook initwaarde en rustwaarde */
				if(ia->type==ACT_IPO_LOOP_END) {
					if(ia->flag & ACT_IPOEND) {
						act->go= 0;
						ia->flag &= ~ACT_IPOEND;
					}
					else ia->cur= ia->sta;
				}
				else ia->cur= ia->sta;
			}
			break;
				
		case ACT_IPO_KEY2KEY:
		
			ia->cur+= act->go;
			CLAMPTEST(ia->cur, ia->sta, ia->end);
			lf->cfra= ia->cur;
			
			if(lf->type==OB_DYNAMIC) {
				do_obipo(ob, ia->cur, act->go, lf->dloc);
			}
			else do_obipo(ob, ia->cur, 0, 0);

			if(ia->cur==ia->end) act->go= 0;				
			break;
		}
		break;
		
	case ACT_CONSTRAINT:
		coa= act->data;
		
		switch(coa->flag) {
		case ACT_CONST_LOCX:
			if(lf->flag & OB_DYNAMIC) fp= lf->loc;
			else fp= ob->loc;
			dfp= lf->speed;
			minval= coa->minloc[0];
			maxval= coa->maxloc[0];
			break;
		case ACT_CONST_LOCY:
			if(lf->flag & OB_DYNAMIC) fp= lf->loc+1;
			else fp= ob->loc+1;
			dfp= lf->speed+1;
			minval= coa->minloc[1];
			maxval= coa->maxloc[1];
			break;
		case ACT_CONST_LOCZ:
			if(lf->flag & OB_DYNAMIC) fp= lf->loc+2;
			else fp= ob->loc+2;
			dfp= lf->speed+2;
			minval= coa->minloc[2];
			maxval= coa->maxloc[2];
			break;
		case ACT_CONST_ROTX:
			fp= lf->rot;
			dfp= lf->rotspeed;
			minval= M_PI*coa->minrot[0]/180.0;
			maxval= M_PI*coa->maxrot[0]/180.0;
			break;
		case ACT_CONST_ROTY:
			fp= lf->rot+1;
			dfp= lf->rotspeed+1;
			minval= M_PI*coa->minrot[1]/180.0;
			maxval= M_PI*coa->maxrot[1]/180.0;
			break;
		case ACT_CONST_ROTZ:
			fp= lf->rot+2;
			dfp= lf->rotspeed+2;
			minval= M_PI*coa->minrot[2]/180.0;
			maxval= M_PI*coa->maxrot[2]/180.0;
			break;
		}
		
		if( *fp < minval) {
			*fp= coa->slow*(*fp) + (1.0-coa->slow)*minval;
			*dfp *= coa->slow;
			act->go--;
			lf->flag1 |= LF_CALC_MATRIX;
		}
		else if( *fp > maxval) {
			*fp= coa->slow*(*fp) + (1.0-coa->slow)*maxval;
			*dfp *= coa->slow;
			act->go--;
			lf->flag1 |= LF_CALC_MATRIX;
		}
		else act->go= 0;

		break;
	
	case ACT_EDIT_OBJECT:
		eoa= act->data;
		if(eoa->type==ACT_EDOB_TRACK_TO) {
			track_life_to_life(lf, eoa->ob->life, (float)eoa->time, eoa->flag);
			act->go--;
		}
		break;
		
	case ACT_GROUP:
		ga= act->data;
		
		lf->flag1 |= LF_CALC_MATRIX;
		
		switch(ga->type) {

		case ACT_GROUP_PLAY:
		case ACT_GROUP_PINGPONG:
		case ACT_GROUP_FLIPPER:
		
			ga->cur+= act->go;
			CLAMPTEST(ga->cur, ga->sta, ga->end);

			set_group_key_frame(ga->group, ((float)ga->cur)/2.0);

			if(ga->cur==ga->end) act->go= 0;
			break;

		case ACT_GROUP_LOOP_STOP:
		case ACT_GROUP_LOOP_END:
		
			ga->cur+= act->go;
			CLAMPTEST(ga->cur, ga->sta, ga->end);
			
			set_group_key_frame(ga->group, ((float)ga->cur)/2.0);
			
			if(ga->cur==ga->end) {	/* end== ook initwaarde en rustwaarde */
				if(ga->type==ACT_GROUP_LOOP_END) {
					if(ga->flag & ACT_IPOEND) {
						act->go= 0;
						ga->flag &= ~ACT_IPOEND;
					}
					else ga->cur= ga->sta;
				}
				else ga->cur= ga->sta;
			}
			break;

		}
	}
}

int sca_handling(Object *ob, Life *lf)
{
	bSensor *sens;
	bController *cont;
	bActuator *act;
	int a, val, and, or;
	
	if(lf->timer>=0) lf->timer--;
	
	/* evaluate all sensors of the controllers, set actuators */
	cont= ob->controllers.first;
	while(cont) {
	
		and= 1;
		if(cont->totslinks==0) and= 0;
		or= 0;
		cont->val= 0;
		
		for(a=0; a<cont->totslinks; a++) {
			val= test_sensor(cont->slinks[a]);
			or |= val;
			and &= val;
			
			if(or && cont->type==CONT_LOGIC_OR) {
				cont->val= 1;
				break;
			}
		}
		
		if(and && cont->type==CONT_LOGIC_AND) cont->val= 1;
		
		/* activate actuators */
		if(cont->val || cont->valo) {
			for(a=0; a<cont->totlinks; a++) {
				activate_actuator(cont->links[a], cont->val, cont->valo);
			}
		}
		
		cont->valo= cont->val;
		
		cont= cont->next;
	}	
	
	/* do actuators */
	act= ob->actuators.first;
	while(act) {
		handle_actuator(act);
		act= act->next;
	}
	
	return 0;
}



void collision_sensor_input(Object *ob, Life *lf)
{

}


