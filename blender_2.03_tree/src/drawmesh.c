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



/*  drawmesh.c     GRAPHICS
 * 
 *  may 2000
 *  
 * 
 * Version: $Id: drawmesh.c,v 1.15 2000/09/13 16:19:01 frank Exp $
 */


#include "blender.h"
#include "graphics.h"
#include "game.h"
#include "sector.h"


#if defined(GL_EXT_texture_object) && !defined(__SUN)

	/* exception for mesa... not according th opengl specs */
	#ifndef __linux__
		#define glBindTexture(A,B)     glBindTextureEXT(A,B)
	#endif

	#define glGenTextures(A,B)     glGenTexturesEXT(A,B)
	#define glDeleteTextures(A,B)  glDeleteTexturesEXT(A,B)
	#define glPolygonOffset(A,B)  glPolygonOffsetEXT(A,B)

#else

#define GL_FUNC_ADD_EXT					GL_FUNC_ADD
/* #define GL_FUNC_REVERSE_SUBTRACT_EXT	GL_FUNC_REVERSE_SUBTRACT */
/* #define GL_POLYGON_OFFSET_EXT			GL_POLYGON_OFFSET */

#endif

float texmat1[4][4], texmat4[4][4];

Image *curpage=0;
int curtile=0, curmode=0;
short texwindx, texwindy, texwinsx, texwinsy;

int source, dest;
float texmat1[4][4], texmat4[4][4];


void copy_part_from_ibuf(ImBuf *ibuf, uint *rect, short startx, short starty, short endx, short endy)
{
	uint *rt, *rp;
	short y, heigth, len;

	/* de juiste offset in rectot */

	rt= ibuf->rect+ (starty*ibuf->x+ startx);

	len= (endx-startx);
	heigth= (endy-starty);

	rp=rect;
	
	for(y=0; y<heigth; y++) {
		memcpy(rp, rt, len*4);
		rt+= ibuf->x;
		rp+= len;
	}

}

void free_realtime_image(Image *ima)
{
	if(ima->bindcode) {
		glDeleteTextures(1, &ima->bindcode);
		ima->bindcode= 0;
	}
	if(ima->repbind) {
		glDeleteTextures(ima->totbind, ima->repbind);
	
		freeN(ima->repbind);
		ima->repbind= 0;
	}
}

void make_repbind(Image *ima)
{
	int a;
	
	if(ima==0 || ima->ibuf==0) return;

	if(ima->repbind) {
		glDeleteTextures(ima->totbind, ima->repbind);
		freeN(ima->repbind);
		ima->repbind= 0;
	}
	ima->totbind= ima->xrep*ima->yrep;
	if(ima->totbind>1) {
		ima->repbind= callocN(sizeof(int)*ima->totbind, "repbind");
	}
	
}

int set_tpage(TFace *tface)
{	
	static int alphamode= -1;
	static TFace *lasttface= 0;
	Image *ima;
	float mat[4][4];
	uint *rect, *bind;
	int tpx, tpy, tilemode, mode;
	
	/* afschakelen */
	if(tface==0) {
		if(lasttface==0) return 0;
		
		lasttface= 0;
		curtile= 0;
		curpage= 0;
		if(curmode!=0) {
			glMatrixMode(GL_TEXTURE);
			glLoadMatrixf(texmat1);
			glMatrixMode(GL_MODELVIEW);
		}
		curmode= 0;
		alphamode= -1;
		
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		return 0;
	}
	lasttface= tface;

	if( alphamode != tface->transp) {
		alphamode= tface->transp;

		if(alphamode) {
			glEnable(GL_BLEND);
			
			if(alphamode==TF_ADD) {
				glBlendFunc(GL_ONE, GL_ONE);
			/* 	glBlendEquationEXT(GL_FUNC_ADD_EXT); */
			}
			else if(alphamode==TF_ALPHA) {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			/* 	glBlendEquationEXT(GL_FUNC_ADD_EXT); */
			}
			/* else { */
			/* 	glBlendFunc(GL_ONE, GL_ONE); */
			/* 	glBlendEquationEXT(GL_FUNC_REVERSE_SUBTRACT_EXT); */
			/* } */
		}
		else glDisable(GL_BLEND);
	}

	ima= tface->tpage;

	tilemode= tface->mode & TF_TILES;

	if(ima==curpage && curtile==tface->tile && tilemode==curmode ) return ima!=0;

	if(tilemode!=curmode) {
		
		glMatrixMode(GL_TEXTURE);
		
		if(tilemode) {
			glLoadMatrixf(texmat4);
		}
		else glLoadMatrixf(texmat1);

		glMatrixMode(GL_MODELVIEW);
		
	}

	if(ima==0 || ima->ok==0) {
		glDisable(GL_TEXTURE_2D);
		
		curtile= tface->tile;
		curpage= 0;
		curmode= tilemode;

		return 0;
	}

	if(ima->ibuf==0) {
		load_image(ima, IB_rect);
		
		if(ima->ibuf==0) {
			ima->ok= 0;

			curtile= tface->tile;
			curpage= 0;
			curmode= tilemode;
			
			glDisable(GL_TEXTURE_2D);
			return 0;
		}
		
	}

	if(ima->tpageflag & IMA_TWINANIM) curtile= ima->lastframe;
	else curtile= tface->tile;

	if(tilemode) {
		if(ima->repbind==0) make_repbind(ima);
		
		if(curtile>=ima->totbind) curtile= 0;
		
		/* this happens when you change repeat buttons */
		if(ima->repbind) bind= ima->repbind+curtile;
		else bind= &ima->bindcode;
		
		if(*bind==0) {
			
			texwindx= ima->ibuf->x/ima->xrep;
			texwindy= ima->ibuf->y/ima->yrep;
			
			if(curtile>=ima->xrep*ima->yrep) curtile= ima->xrep*ima->yrep-1;
	
			texwinsy= curtile / ima->xrep;
			texwinsx= curtile - texwinsy*ima->xrep;
	
			texwinsx*= texwindx;
			texwinsy*= texwindy;
	
			tpx= texwindx;
			tpy= texwindy;
			rect= ima->ibuf->rect + texwinsy*ima->ibuf->x + texwinsx;
		}
	}
	else {
		bind= &ima->bindcode;
		
		if(*bind==0) {
			tpx= ima->ibuf->x;
			tpy= ima->ibuf->y;
			rect= ima->ibuf->rect;
		}
	}

	if(*bind==0) {
		glGenTextures(1, bind);
		
		if(G.f & G_DEBUG) {
			PRINT(s, ima->id.name+2);
			PRINT2(d, d, *bind, tpx);
			PRINT2(d, d, curtile, tilemode);
		}
		glBindTexture( GL_TEXTURE_2D, *bind);

		if(tilemode) glPixelStorei(GL_UNPACK_ROW_LENGTH,  ima->ibuf->x);
		glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGBA,  tpx,  tpy, 0, GL_RGBA, GL_UNSIGNED_BYTE, rect);
		if(tilemode) glPixelStorei(GL_UNPACK_ROW_LENGTH,  0);
		
		/* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); */
		/* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); */

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		
	}
	else glBindTexture( GL_TEXTURE_2D, *bind);
	
	glEnable(GL_TEXTURE_2D);

	curpage= ima;
	curmode= tilemode;

	return 1;
}

void spack(uint ucol)
{
	char *cp= (char *)&ucol;
	
	glColor3ub(cp[3], cp[2], cp[1]);
}

void draw_hide_tfaces(Object *ob, Mesh *me)
{
	TFace *tface;
	MFace *mface;
	float *v1, *v2, *v3, *v4;
	int a;
	
	if(me==0 || me->tface==0) return;

	mface= me->mface;
	tface= me->tface;

	cpack(0x0);
	setlinestyle(1);
	for(a=me->totface; a>0; a--, mface++, tface++) {
		if(mface->v3==0) continue;
		
		if( (tface->flag & TF_HIDE)) {

			v1= (me->mvert+mface->v1)->co;
			v2= (me->mvert+mface->v2)->co;
			v3= (me->mvert+mface->v3)->co;
			if(mface->v4) v4= (me->mvert+mface->v4)->co; else v4= 0;
		
			glBegin(GL_LINE_LOOP);
				glVertex3fv( v1 );
				glVertex3fv( v2 );
				glVertex3fv( v3 );
				if(mface->v4) glVertex3fv( v4 );
			glEnd();			
		}
	}
	setlinestyle(0);
}


void draw_tfaces3D(Object *ob, Mesh *me)
{
	MFace *mface;
	TFace *tface;
	float *v1, *v2, *v3, *v4;
	int a;
	
	if(me==0 || me->tface==0) return;

	glDisable(GL_DEPTH_TEST);

	mface= me->mface;
	tface= me->tface;
	
	/* SELECT faces */
	for(a=me->totface; a>0; a--, mface++, tface++) {
		if(mface->v3==0) continue;
		if(tface->flag & TF_HIDE) continue;
		
		if( tface->flag & (ACTIVE|SELECT) ) {
		
			v1= (me->mvert+mface->v1)->co;
			v2= (me->mvert+mface->v2)->co;
			v3= (me->mvert+mface->v3)->co;
			if(mface->v4) v4= (me->mvert+mface->v4)->co; else v4= 0;
			
			if(tface->flag & ACTIVE) {
				/* kleuren: R=x G=y */
				cpack(0xFF);
				glBegin(GL_LINE_STRIP); glVertex3fv(v1); if(v4) glVertex3fv(v4); else glVertex3fv(v3); glEnd();
				cpack(0xFF00);
				glBegin(GL_LINE_STRIP); glVertex3fv(v1); glVertex3fv(v2); glEnd();
				cpack(0x0);
				glBegin(GL_LINE_STRIP); glVertex3fv(v2); glVertex3fv(v3); if(v4) glVertex3fv(v4); glEnd();
			}
			else {
				cpack(0x0);
				glBegin(GL_LINE_LOOP);
					glVertex3fv( v1 );
					glVertex3fv( v2 );
					glVertex3fv( v3 );
					if(v4) glVertex3fv( v4 );
				glEnd();
			}
			
			if(tface->flag & SELECT) {
				cpack(0xFFFFFF);
				setlinestyle(1);
				glBegin(GL_LINE_LOOP);
					glVertex3fv( v1 );
					glVertex3fv( v2 );
					glVertex3fv( v3 );
					if(v4) glVertex3fv( v4 );
				glEnd();
				setlinestyle(0);
			}
		}
	}

	glEnable(GL_DEPTH_TEST);
}

int set_gl_light(Object *ob)
{
	extern float matone[4][4];
	Base *base;
	Lamp *la;
	int count;
	float zero[4]= {0.0, 0.0, 0.0, 0.0}, vec[4];
	
	vec[3]= 1.0;
	
	for(count=0; count<8; count++) glDisable(GL_LIGHT0+count);
	
	count= 0;
	
	base= FIRSTBASE;
	while(base) {
		if(base->object->type==OB_LAMP ) {
			if(base->lay & G.vd->lay) {
				if(base->lay & ob->lay) {
					la= base->object->data;
					
					glPushMatrix();
					glLoadMatrixf(G.vd->viewmat);
					
					where_is_object_simul(base->object);
					VECCOPY(vec, base->object->obmat[3]);
					
					if(la->type==LA_SUN) {
						vec[0]= base->object->obmat[2][0];
						vec[1]= base->object->obmat[2][1];
						vec[2]= base->object->obmat[2][2];
						vec[3]= 0.0;
						glLightfv(GL_LIGHT0+count, GL_POSITION, vec); 
					}
					else {
						vec[3]= 1.0;
						glLightfv(GL_LIGHT0+count, GL_POSITION, vec); 
						glLightf(GL_LIGHT0+count, GL_CONSTANT_ATTENUATION, 1.0);
						glLightf(GL_LIGHT0+count, GL_LINEAR_ATTENUATION, la->att1/la->dist);
						/* without this next line it looks backward compatible. attennuation still is acceptable */
						/* glLightf(GL_LIGHT0+count, GL_QUADRATIC_ATTENUATION, la->att2/(la->dist*la->dist)); */
						
						if(la->type==LA_SPOT) {
							vec[0]= -base->object->obmat[2][0];
							vec[1]= -base->object->obmat[2][1];
							vec[2]= -base->object->obmat[2][2];
							glLightfv(GL_LIGHT0+count, GL_SPOT_DIRECTION, vec);
							glLightf(GL_LIGHT0+count, GL_SPOT_CUTOFF, la->spotsize/2.0);
							glLightf(GL_LIGHT0+count, GL_SPOT_EXPONENT, 128.0*la->spotblend);
						}
						else glLightf(GL_LIGHT0+count, GL_SPOT_CUTOFF, 180.0);
					}
					
					vec[0]= la->energy*la->r;
					vec[1]= la->energy*la->g;
					vec[2]= la->energy*la->b;
					vec[3]= 1.0;
					glLightfv(GL_LIGHT0+count, GL_DIFFUSE, vec); 
					glLightfv(GL_LIGHT0+count, GL_SPECULAR, zero); 
					glEnable(GL_LIGHT0+count);
					
					glPopMatrix();					
					
					count++;
					if(count>7) break;
				}
			}
		}
		base= base->next;
	}

	return count;
}

typedef struct tra_ob {
	struct tra_ob *next, *prev;
	Object *ob;
	Mesh *me;
	int dt;
} tra_ob;

#define MAX_TRA_OB	64

static int tot_tra_ob=0;

static tra_ob tra_ob_ar[MAX_TRA_OB];

void add_tra_object(Object *ob, Mesh *me, int dt)
{
	if(tot_tra_ob>=MAX_TRA_OB) return;
	tra_ob_ar[tot_tra_ob].ob= ob;
	tra_ob_ar[tot_tra_ob].me= me;
	tra_ob_ar[tot_tra_ob].dt= dt;
	tot_tra_ob++;
}


void draw_tface_mesh(Object *ob, Mesh *me, int dt)	/* maximum dt: precies volgens ingestelde waardes */
{
	TFace *tface;
	MFace *mface;
	DFace *dface;
	MVert *mvert;
	Image *ima;
	float *v1, *v2, *v3, *v4, col[4][3], coli[4], colf[4], nor[3];
	uint obcol;
	int a, mode;
	short lastmode=0, islight, istex, istra=0, isdface=0, invis=0;
	char *cp;
	
	if(me==0) return;

	glShadeModel(GL_SMOOTH);

	islight= set_gl_light(ob);
	
	obcol= rgb_to_mcol(ob->col[0], ob->col[1], ob->col[2]);

	glGetFloatv(GL_CURRENT_COLOR, coli);

	/* als meshes uit lib gelezen zijn en alleen mcol hebben: */
	if(me->tface==0) make_tfaces(me);

	/* eerst alle texture polys */
	
	glCullFace(GL_BACK); glEnable(GL_CULL_FACE);
	if(G.vd->drawtype==OB_TEXTURE) istex= 1;
	else istex= 0;

	/* signal to NOT draw transparant separate */
	if((G.f & G_SIMULATION)==0) istra= 2;

	/* colf[0]=colf[1]=colf[2]=colf[3]= 0.8; */

	if(me->dface) isdface= 1;
	
	if(G.f & G_SIMULATION) invis= 1;

	if(dt > OB_SOLID) {
		
		mface= me->mface;
		tface= me->tface;
		dface= me->dface;
		
		for(a=me->totface; a>0; a--, mface++, tface++, dface++) {
			if(mface->v3==0) continue;
			if(tface->flag & TF_HIDE) continue;
			if(tface->mode & TF_INVISIBLE) continue;
			
			if(istex && tface->transp) {
				if(istra==0) {
					add_tra_object(ob, me, dt);
					istra= 1;
				}
				if(istra==1) continue;
			}
			mode= tface->mode;

			if(mode & TF_OBCOL) {
				tface->col[0]= obcol;
				tface->col[1]= obcol;
				tface->col[2]= obcol;
				tface->col[3]= obcol;
			}
			
			v1= (me->mvert+mface->v1)->co;
			v2= (me->mvert+mface->v2)->co;
			v3= (me->mvert+mface->v3)->co;
			if(mface->v4) v4= (me->mvert+mface->v4)->co; else v4= 0;
			
			if(mode != lastmode) {
				if(mode & TF_TWOSIDE) glDisable(GL_CULL_FACE);
				else glEnable(GL_CULL_FACE);
				
				if(islight && (mode & TF_LIGHT)) {
					
					glEnable(GL_LIGHTING); 
					glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
					glEnable(GL_COLOR_MATERIAL);
				}
				else {
					glDisable(GL_LIGHTING); 
					glDisable(GL_COLOR_MATERIAL);
				}
				
				if(istex && (mode & TF_TEX));
				else set_tpage(0);
			}

			if(istex && (mode & TF_TEX) ) {
				
				/* in set_tpage worden dingen gedaan die niet binnen een bgnpolygon mogen liggen */
				if( set_tpage(tface) ) {
					if(islight && (mode & TF_LIGHT)) {
						
						if(isdface==0) CalcNormFloat(v1, v2, v3, nor);
						
						glBegin(GL_POLYGON);
						
						glTexCoord2fv(tface->uv[0]);
						spack(tface->col[0]);
						if(mface->flag & ME_SMOOTH) glNormal3sv((me->mvert+mface->v1)->no);
						else if(isdface) glNormal3fv(dface->no);
						else glNormal3fv(nor);
						glVertex3fv(v1);
						
						glTexCoord2fv(tface->uv[1]);
						spack(tface->col[1]);
						if(mface->flag & ME_SMOOTH) glNormal3sv((me->mvert+mface->v2)->no);
						glVertex3fv(v2);
						
						glTexCoord2fv(tface->uv[2]);
						spack(tface->col[2]);
						if(mface->flag & ME_SMOOTH) glNormal3sv((me->mvert+mface->v3)->no);
						glVertex3fv(v3);
			
						if(v4) {
							glTexCoord2fv(tface->uv[3]);
							spack(tface->col[3]);
							if(mface->flag & ME_SMOOTH) glNormal3sv((me->mvert+mface->v4)->no);
							glVertex3fv(v4);
						}
						glEnd();
					}
					else {
						
						glBegin(GL_POLYGON);
						glTexCoord2fv(tface->uv[0]);

						spack(tface->col[0]);
						glVertex3fv(v1);
						
						glTexCoord2fv(tface->uv[1]);
						spack(tface->col[1]);
						glVertex3fv(v2);
			
						glTexCoord2fv(tface->uv[2]);
						spack(tface->col[2]);
						glVertex3fv(v3);
			
						if(v4) {
							glTexCoord2fv(tface->uv[3]);
							spack(tface->col[3]);
							glVertex3fv(v4);
						}
						glEnd();
					}
				}
				else {
					/* waarschuwings polygoon */

					glBegin(GL_POLYGON);
					cpack(0xFF00FF);
					glVertex3fv(v1);
					glVertex3fv(v2);
					glVertex3fv(v3);
					if(v4) glVertex3fv(v4);
					glEnd();
				}
			}
			else {
				
				
				glBegin(GL_POLYGON);
				
				if(islight && (mode & TF_LIGHT)) {

					if(isdface==0) CalcNormFloat(v1, v2, v3, nor);
					
					cp= (char *)&(tface->col[0]);

					glColor3ub(cp[3], cp[2], cp[1]);
					if(mface->flag & ME_SMOOTH) glNormal3sv((me->mvert+mface->v1)->no);
					else if(isdface) glNormal3fv(dface->no);
					else glNormal3fv(nor);
					glVertex3fv(v1);
					
					glColor3ub(cp[7], cp[6], cp[5]);
					if(mface->flag & ME_SMOOTH) glNormal3sv((me->mvert+mface->v2)->no);
					glVertex3fv(v2);
					
					glColor3ub(cp[11], cp[10], cp[9]);
					if(mface->flag & ME_SMOOTH) glNormal3sv((me->mvert+mface->v3)->no);
					glVertex3fv(v3);
		
					if(v4) {
						glColor3ub(cp[15], cp[14], cp[13]);
						if(mface->flag & ME_SMOOTH) glNormal3sv((me->mvert+mface->v4)->no);
						glVertex3fv(v4);
					}
				}
				else {

					cp= (char *)&(tface->col[0]);
					glColor3ub(cp[3], cp[2], cp[1]);
					glVertex3fv(v1);
					
					glColor3ub(cp[7], cp[6], cp[5]);
					glVertex3fv(v2);
					
					glColor3ub(cp[11], cp[10], cp[9]);
					glVertex3fv(v3);
					if(v4) {
						glColor3ub(cp[15], cp[14], cp[13]);
						glVertex3fv(v4);
					}
					
				}
				glEnd();
			}
			lastmode= mode;
		}
		
		/* textures uitzetten */
		set_tpage(0);

	}
	
	glShadeModel(GL_FLAT);
	glDisable(GL_CULL_FACE);
	
	draw_hide_tfaces(ob, me);

	if(ob==OBACT && (G.f & G_FACESELECT)) {
		draw_tfaces3D(ob, me);
	}
	glColor3f(coli[0],  coli[1],  coli[2]);
	
	default_gl_light();
}


void draw_tface_meshes_tra()
{
	Object *ob;
	Mesh *me;
	Life *lf;
	TFace *tface;
	MFace *mface;
	DFace *dface;
	MVert *mvert;
	Image *ima;
	float *v1, *v2, *v3, *v4, col[4][3], coli[4], nor[3];
	uint obcol;
	int a, t, mode, dt;
	short lastmode=0, islight, istra=0, isdface=0, invis=0;
	char *cp;
	
	if(G.vd->drawtype!=OB_TEXTURE) return;

	obcol= 0xFFFFFF;
	
	glDepthMask(0);
	glShadeModel(GL_SMOOTH);
	glGetFloatv(GL_CURRENT_COLOR, coli);

	glCullFace(GL_BACK); glEnable(GL_CULL_FACE);

	for(t=0; t<tot_tra_ob; t++) {
		
		ob= tra_ob_ar[t].ob;
		me= tra_ob_ar[t].me;
		dt= tra_ob_ar[t].dt;

		multmatrix(ob->obmat);
		
		if(me==0) continue;
		if(ob->gameflag & OB_LIFE) lf= ob->life;
		else lf= 0;
		
		islight= set_gl_light(ob);
		lastmode= 0;
		obcol= rgb_to_mcol(ob->col[0], ob->col[1], ob->col[2]);
			
		if(dt>OB_SOLID) {
			
			mface= me->mface;
			tface= me->tface;
			dface= me->dface;

			for(a=me->totface; a>0; a--, mface++, tface++, dface++) {
				if(mface->v3==0) continue;
				if(tface->flag & TF_HIDE) continue;
				if(tface->mode & TF_INVISIBLE) continue;
				
				if(tface->transp==0) continue;

				mode= tface->mode;

				if(mode & TF_OBCOL) {
					tface->col[0]= obcol;
					tface->col[1]= obcol;
					tface->col[2]= obcol;
					tface->col[3]= obcol;
				}
				
				v1= (me->mvert+mface->v1)->co;
				v2= (me->mvert+mface->v2)->co;
				v3= (me->mvert+mface->v3)->co;
				if(mface->v4) v4= (me->mvert+mface->v4)->co; else v4= 0;
				
				if(mode != lastmode) {
					if(mode & TF_TWOSIDE) glDisable(GL_CULL_FACE);
					else glEnable(GL_CULL_FACE);
					
					if(islight && (mode & TF_LIGHT)) {
						
						glEnable(GL_LIGHTING); 
						glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
						glEnable(GL_COLOR_MATERIAL);
					}
					else {
						glDisable(GL_LIGHTING); 
						glDisable(GL_COLOR_MATERIAL);
					}
					
					if((mode & TF_TEX));
					else set_tpage(0);
				}
	
				if(mode & TF_TEX) {
					/* in set_tpage worden dingen gedaan die niet binnen een bgnpolygon mogen liggen */
					if( set_tpage(tface) ) {
						
						if(mode & TF_SHADOW) {
							if(lf) {
								float tmat[4][4];			
								float vec[3];

								getmatrix(tmat);
								loadmatrix(G.vd->viewmat);
								glTranslatef(ob->obmat[3][0],  ob->obmat[3][1],  ob->obmat[3][2]);
								glTranslatef(lf->floorloc[0],  lf->floorloc[1],  lf->floorloc[2]);

								glBegin(GL_POLYGON);
								
								glTexCoord2fv(tface->uv[0]);
								spack(tface->col[0]);
								glVertex3fv(v1);
								
								glTexCoord2fv(tface->uv[1]);
								spack(tface->col[1]);
								glVertex3fv(v2);
					
								glTexCoord2fv(tface->uv[2]);
								spack(tface->col[2]);
								glVertex3fv(v3);
					
								if(v4) {
									glTexCoord2fv(tface->uv[3]);
									spack(tface->col[3]);
									glVertex3fv(v4);
								}
								glEnd();

								loadmatrix(tmat);
				
							}
						}
						else if(mode & TF_BILLBOARD) {	/* actually halo! */
							float mat[4][4];
							float len;
							
							glPushMatrix();
							getmatrix(mat);
							len= fsqrt(mat[0][0]*mat[0][0] + mat[0][1]*mat[0][1] + mat[0][2]*mat[0][2] );

							mat[0][0]= mat[1][1]= mat[2][2]= len;
							mat[0][1]= mat[0][2]= 0.0;
							mat[1][0]= mat[1][2]= 0.0;
							mat[2][0]= mat[2][1]= 0.0;
							loadmatrix(mat);
							
							glBegin(GL_POLYGON);
							
							glTexCoord2fv(tface->uv[0]);
	
							spack(tface->col[0]);
							glVertex3fv(v1);
							
							glTexCoord2fv(tface->uv[1]);
							spack(tface->col[1]);
							glVertex3fv(v2);
				
							glTexCoord2fv(tface->uv[2]);
							spack(tface->col[2]);
							glVertex3fv(v3);
				
							if(v4) {
								glTexCoord2fv(tface->uv[3]);
								spack(tface->col[3]);
								glVertex3fv(v4);
							}
							glEnd();
							
							glPopMatrix();
						}
						else if(islight && (mode & TF_LIGHT)) {
							
							if(isdface==0) CalcNormFloat(v1, v2, v3, nor);
							
							glBegin(GL_POLYGON);
							
							glTexCoord2fv(tface->uv[0]);
							spack(tface->col[0]);
							if(mface->flag & ME_SMOOTH) glNormal3sv((me->mvert+mface->v1)->no);
							else if(isdface) glNormal3fv(dface->no);
							else glNormal3fv(nor);
							glVertex3fv(v1);
							
							glTexCoord2fv(tface->uv[1]);
							spack(tface->col[1]);
							if(mface->flag & ME_SMOOTH) glNormal3sv((me->mvert+mface->v2)->no);
							glVertex3fv(v2);
							
							glTexCoord2fv(tface->uv[2]);
							spack(tface->col[2]);
							if(mface->flag & ME_SMOOTH) glNormal3sv((me->mvert+mface->v3)->no);
							glVertex3fv(v3);
				
							if(v4) {
								glTexCoord2fv(tface->uv[3]);
								spack(tface->col[3]);
								if(mface->flag & ME_SMOOTH) glNormal3sv((me->mvert+mface->v4)->no);
								glVertex3fv(v4);
							}
							glEnd();
						}
						else {
							glBegin(GL_POLYGON);
							glTexCoord2fv(tface->uv[0]);
	
							spack(tface->col[0]);
							glVertex3fv(v1);
							
							glTexCoord2fv(tface->uv[1]);
							spack(tface->col[1]);
							glVertex3fv(v2);
				
							glTexCoord2fv(tface->uv[2]);
							spack(tface->col[2]);
							glVertex3fv(v3);
				
							if(v4) {
								glTexCoord2fv(tface->uv[3]);
								spack(tface->col[3]);
								glVertex3fv(v4);
							}
							glEnd();
						}
					}
				}

				lastmode= mode;
			}
			
		}
		loadmatrix(G.vd->viewmat);
	}
	
	/* textures uitzetten */
	set_tpage(0);

	tot_tra_ob= 0;

	glDepthMask(1);
	glShadeModel(GL_FLAT);
	glDisable(GL_CULL_FACE);
	
	default_gl_light();
	
	glColor3f(coli[0],  coli[1],  coli[2]);
}


void init_realtime_GL()
{		
	Mat4One(texmat1);
	Mat4One(texmat4);
	Mat4MulFloat3((float *)texmat4, 4.0);
	
	glMatrixMode(GL_TEXTURE);
	glLoadMatrixf(texmat1);
	glMatrixMode(GL_MODELVIEW);
	
	#ifdef __sgi
	glBlendEquationEXT(GL_FUNC_ADD_EXT);
	#endif
	
	/* glPolygonOffset(-0.001, -0.001); */
}


