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

//#define NAN_GAME


/* drawview.c  april 94		GRAPHICS
 * 
 * 
 * 
 * Version: $Id: drawview.c,v 1.27 2000/09/26 09:00:26 gino Exp $
 */

#include "blender.h"
#include "graphics.h"
#include "interface.h"
#include "game.h"
#include "group.h"

/* Modules used */
#include "render.h"

/* own include */
#include "drawview_ext.h"

/* for physics in animation playback */
#ifdef NAN_GAME
#include "sumo.h"
#endif
void setalpha_bgpic(BGpic *bgpic)
{
	int x, y, alph;
	char *rect;
	
	alph= (int)(255.0*(1.0-bgpic->blend));
	
	rect= (char *)bgpic->rect;
	for(y=0; y< bgpic->yim; y++) {
		for(x= bgpic->xim; x>0; x--, rect+=4) {
			rect[3]= alph;
		}
	}
}


float light_pos1[] = { -0.3, 0.3, 0.90, 0.0 }; 
float light_pos2[] = { 0.3, -0.3, -0.90, 0.0 }; 

void default_gl_light()
{
	float mat_specular[] = { 0.5, 0.5, 0.5, 1.0 };
	float light_col1[] = { 0.8, 0.8, 0.8, 0.0 }; 

	int a;
		
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos1); 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_col1); 
	glLightfv(GL_LIGHT0, GL_SPECULAR, mat_specular); 

	glEnable(GL_LIGHT0);
	for(a=1; a<8; a++) glDisable(GL_LIGHT0+a);
	glDisable(GL_LIGHTING);

	glDisable(GL_COLOR_MATERIAL);
}

void init_gl_stuff()	
{
	float mat_specular[] = { 0.5, 0.5, 0.5, 1.0 };
	float mat_shininess[] = { 35.0 };
	float one= 1.0;
	int a, x, y;
	GLubyte pat[32*32];
	const GLubyte *patc= pat;
		
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

	default_gl_light();
	
#if defined(__FreeBSD) || defined(linux)
	glDisable(GL_DITHER);	/* op sgi/sun hardware && 12 bits */
#endif
	
	/* no local viewer, looks ugly in ortho mode */
	/* glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, &one); */
	
	glDepthFunc(GL_LEQUAL);
	/* scaling matrices */
	glEnable(GL_NORMALIZE);

	glShadeModel(GL_FLAT);

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glDisable(GL_LOGIC_OP);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);

	glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
	glPixelTransferi(GL_RED_SCALE, 1);
	glPixelTransferi(GL_RED_BIAS, 0);
	glPixelTransferi(GL_GREEN_SCALE, 1);
	glPixelTransferi(GL_GREEN_BIAS, 0);
	glPixelTransferi(GL_BLUE_SCALE, 1);
	glPixelTransferi(GL_BLUE_BIAS, 0);
	glPixelTransferi(GL_ALPHA_SCALE, 1);
	glPixelTransferi(GL_ALPHA_BIAS, 0);

	a= 0;
	for(x=0; x<32; x++) {
		for(y=0; y<4; y++) {
			if( (x) & 1) pat[a++]= 0x88;
			else pat[a++]= 0x22;
		}
	}
	
	glPolygonStipple(patc);


	init_realtime_GL();	
}

void two_sided(int val)
{

	/* twosided aan: geft errors bij x flip! */
	glLightModeliv(GL_LIGHT_MODEL_TWO_SIDE, (GLint *)&val);
}

void circf(float x, float y, float rad)
{
	GLUquadricObj *qobj = gluNewQuadric(); 
	
	gluQuadricDrawStyle(qobj, GLU_FILL); 
	
	glPushMatrix(); 
	
	glTranslatef(x,  y, 0.); 
	
	gluDisk( qobj, 0.0,  rad, 32, 1); 
	
	glPopMatrix(); 
	
	gluDeleteQuadric(qobj);
}

void circ(float x, float y, float rad)
{
	GLUquadricObj *qobj = gluNewQuadric(); 
	
	gluQuadricDrawStyle(qobj, GLU_SILHOUETTE); 
	
	glPushMatrix(); 
	
	glTranslatef(x,  y, 0.); 
	
	gluDisk( qobj, 0.0,  rad, 32, 1); 
	
	glPopMatrix(); 
	
	gluDeleteQuadric(qobj);
}

/* ********** IN ONTWIKKELING ********** */

#define MAXMATBUF 16
float matbuf[MAXMATBUF][2][3];

void init_gl_materials(Object *ob)
{
	extern Material defmaterial;
	Material *ma;
	int a;
	
	if(ob->totcol==0) {
		matbuf[0][0][0]= defmaterial.r;
		matbuf[0][0][1]= defmaterial.g;
		matbuf[0][0][2]= defmaterial.b;
		
		matbuf[0][1][0]= defmaterial.specr;
		matbuf[0][1][1]= defmaterial.specg;
		matbuf[0][1][2]= defmaterial.specb;
		
		/* ook matnr 1, displists! */
		VECCOPY(matbuf[1][0], matbuf[0][0]);
		VECCOPY(matbuf[1][1], matbuf[0][1]);
	}
	
	for(a=1; a<=ob->totcol; a++) {
		ma= give_current_material(ob, a);
		if(ma==NULL) ma= &defmaterial;
		if(a<MAXMATBUF) {
			matbuf[a][0][0]= (ma->ref+ma->emit)*ma->r;
			matbuf[a][0][1]= (ma->ref+ma->emit)*ma->g;
			matbuf[a][0][2]= (ma->ref+ma->emit)*ma->b;
			
			matbuf[a][1][0]= ma->spec*ma->specr;
			matbuf[a][1][1]= ma->spec*ma->specg;
			matbuf[a][1][2]= ma->spec*ma->specb;
		}
	}
}

void set_gl_material(int nr)
{

	if(nr<MAXMATBUF) {
		if(matbuf[nr][0]) {
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matbuf[nr][0]);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matbuf[nr][1]);
		}
	}
}


void draw_bgpic()
{
	BGpic *bgpic;
	Image *ima;
	float vec[3], fac, asp, zoomx, zoomy;
	int x1, y1, x2, y2, cx, cy;
	short mval[2];
	
	bgpic= G.vd->bgpic;
	if(bgpic==0) return;
	
	if(bgpic->tex) {
		init_render_texture(bgpic->tex);
		free_unused_animimages();
		ima= bgpic->tex->ima;
		end_render_texture(bgpic->tex);
	}
	else {
		ima= bgpic->ima;
	}
	
	if(ima==0) return;
	if(ima->ok==0) return;
	
	/* plaatje testen */
	if(ima->ibuf==0) {
	
		if(bgpic->rect) freeN(bgpic->rect);
		bgpic->rect= 0;
		
		if(bgpic->tex) {
			ima_ibuf_is_nul(bgpic->tex);
		}
		else {
			waitcursor(1);
			load_image(ima, IB_rect);
			waitcursor(0);
		}
		if(ima->ibuf==0) {
			ima->ok= 0;
			return;
		}
	}

	if(bgpic->rect==0) {
		
		bgpic->rect= dupallocN(ima->ibuf->rect);
		bgpic->xim= ima->ibuf->x;
		bgpic->yim= ima->ibuf->y;
		setalpha_bgpic(bgpic);
	}

	if(G.vd->persp==2) {
		x1= G.vd->pr_xmin;
		y1= G.vd->pr_ymin;
		x2= G.vd->pr_xmax;
		y2= G.vd->pr_ymax;
	}
	else {
		/* windowco berekenen */
		initgrabz(0.0, 0.0, 0.0);
		window_to_3d(vec, 1, 0);
		fac= MAX3( fabs(vec[0]), fabs(vec[1]), fabs(vec[1]) );
		fac= 1.0/fac;
	
		asp= ( (float)ima->ibuf->y)/(float)ima->ibuf->x;
	
		vec[0]= vec[1]= vec[2]= 0.0;
		project_short_noclip(vec, mval);
		cx= mval[0];
		cy= mval[1];
	
		x1=  cx+ fac*(bgpic->xof-bgpic->size);
		y1=  cy+ asp*fac*(bgpic->yof-bgpic->size);
		x2=  cx+ fac*(bgpic->xof+bgpic->size);
		y2=  cy+ asp*fac*(bgpic->yof+bgpic->size);
	}
	
	/* volledige clip? */
	
	if(x2 < 0 ) return;
	if(y2 < 0 ) return;
	if(x1 > curarea->winx ) return;
	if(y1 > curarea->winy ) return;
	
	zoomx= x2-x1;
	zoomx /= (float)ima->ibuf->x;
	zoomy= y2-y1;
	zoomy /= (float)ima->ibuf->y;

	glEnable(GL_BLEND);
	if(G.zbuf) glDisable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA); 
	 
	rectwrite_part(curarea->winrct.xmin, curarea->winrct.ymin, curarea->winrct.xmax, curarea->winrct.ymax, 
                   x1+curarea->winrct.xmin, y1+curarea->winrct.ymin, ima->ibuf->x, ima->ibuf->y, zoomx, zoomy, bgpic->rect);

	glBlendFunc(GL_ONE,  GL_ZERO); 
	glDisable(GL_BLEND);
	if(G.zbuf) glEnable(GL_DEPTH_TEST);
	 
}

void timestr(int time, char *str)
{
	/* formaat 00:00:00.00 (hr:min:sec) string moet 12 lang */

	float temp;
	short hr,min,sec,hun;

	temp= ((float)time)/(100.0);
	min= ffloor(temp/60.0);
	hr= min/60;
	min-= 60*hr;
	temp-= (float)60*min;
	sec= ffloor(temp);
	temp-= (float)sec;
	hun= ffloor(100*temp);

	if(hr) sprintf(str,"%.2d:%.2d:%.2d.%.2d",hr,min,sec,hun);
	else sprintf(str,"%.2d:%.2d.%.2d",min,sec,hun);
	str[11]=0;
}


void drawgrid()
{
	/* extern short bgpicmode; */
	float wx, wy, x, y, fz, fw, fx, fy, dx;
	float vec4[4];

	vec4[0]=vec4[1]=vec4[2]=0.0; 
	vec4[3]= 1.0;
	Mat4MulVec4fl(G.vd->persmat, vec4);
	fx= vec4[0]; 
	fy= vec4[1]; 
	fz= vec4[2]; 
	fw= vec4[3];

	wx= (curarea->winx/2.0);	/* ivm afrondfoutjes, grid op verkeerde plek */
	wy= (curarea->winy/2.0);

	x= (wx)*fx/fw;
	y= (wy)*fy/fw;

	vec4[0]=vec4[1]=G.vd->grid; 
	vec4[2]= 0.0;
	vec4[3]= 1.0;
	Mat4MulVec4fl(G.vd->persmat, vec4);
	fx= vec4[0]; 
	fy= vec4[1]; 
	fz= vec4[2]; 
	fw= vec4[3];

	dx= fabs(x-(wx)*fx/fw);
	if(dx==0) dx= fabs(y-(wy)*fy/fw);

	if(dx<6.0) {
		dx*= 10.0;
		setlinestyle(3);
		if(dx<6.0) {
			dx*= 10.0;
			if(dx<6.0) {
				setlinestyle(0);
				return;
			}
		}
	}
	
	persp(0);

	cpack(0x484848);
	
	x+= (wx); 
	y+= (wy);
	fx= x/dx;
	fx= x-dx*ffloor(fx);
	
	while(fx< curarea->winx) {
		fdrawline(fx,  0.0,  fx,  (float)curarea->winy); 
		fx+= dx; 
	}

	fy= y/dx;
	fy= y-dx*ffloor(fy);
	

	while(fy< curarea->winy) {
		fdrawline(0.0,  fy,  (float)curarea->winx,  fy); 
		fy+= dx;
	}

	/* kruis in midden */
	if(G.vd->view==3) cpack(0xA0D0A0); /* y-as */
	else cpack(0xA0A0D0);	/* x-as */

	fdrawline(0.0,  y,  (float)curarea->winx,  y); 
	
	if(G.vd->view==7) cpack(0xA0D0A0);	/* y-as */
	else cpack(0xE0A0A0);	/* z-as */

	fdrawline(x,  0.0,  x,  (float)curarea->winy); 

	persp(1);
	setlinestyle(0);
}


void drawfloor()
{
	View3D *vd;
	float vert[3], grid;
	int a, gridlines;
	
	vd= curarea->spacedata.first;

	vert[2]= 0.0;

	if(vd->gridlines<3) return;

	gridlines= vd->gridlines/2;
	grid= gridlines*vd->grid;
	
	cpack(0x484848);

	for(a= -gridlines;a<=gridlines;a++) {

		if(a==0) {
			if(vd->persp==0) cpack(0xA0D0A0);
			else cpack(0x402000);
		}
		else if(a==1) {
			cpack(0x484848);
		}
		
	
		glBegin(GL_LINE_STRIP);
        vert[0]= a*vd->grid;
        vert[1]= grid;
        glVertex3fv(vert);
        vert[1]= -grid;
        glVertex3fv(vert);
		glEnd();
	}
	
	cpack(0x484848);
	
	for(a= -gridlines;a<=gridlines;a++) {
		if(a==0) {
			if(vd->persp==0) cpack(0xA0A0D0);
			else cpack(0);
		}
		else if(a==1) {
			cpack(0x484848);
		}
	
		glBegin(GL_LINE_STRIP);
        vert[1]= a*vd->grid;
        vert[0]= grid;
        glVertex3fv(vert );
        vert[0]= -grid;
        glVertex3fv(vert);
		glEnd();
	}

}

void drawcursor()
{

	if(G.f & G_PLAYANIM) return;
	
	project_short( give_cursor(), &G.vd->mx);

	G.vd->mxo= G.vd->mx;
	G.vd->myo= G.vd->my;

	if( G.vd->mx!=3200) {
		
		setlinestyle(0); 
		cpack(0xFF);
		circ((float)G.vd->mx, (float)G.vd->my, 10.0);
		setlinestyle(4); 
		cpack(0xFFFFFF);
		circ((float)G.vd->mx, (float)G.vd->my, 10.0);
		setlinestyle(0);
		cpack(0x0);
		
		sdrawline(G.vd->mx-20, G.vd->my, G.vd->mx-5, G.vd->my);
		sdrawline(G.vd->mx+5, G.vd->my, G.vd->mx+20, G.vd->my);
		sdrawline(G.vd->mx, G.vd->my-20, G.vd->mx, G.vd->my-5);
		sdrawline(G.vd->mx, G.vd->my+5, G.vd->mx, G.vd->my+20);
	}
}

void calc_viewborder()
{
	float a, fac, wx, wy, x1, x2, y1, y2;
	float imax, imay, width;
	
	wx= curarea->winx;
	wy= curarea->winy;
	imax= G.scene->r.xsch;
	imay= G.scene->r.ysch;
	
	fac= ((float)imay*G.scene->r.yasp)/((float)imax*G.scene->r.xasp);
	imay= fac*imax;
	
    /* liggend plaatje */
	if(imax>imay) {
		if(wx<wy) width= wy;
		else width= wx;
		
		fac= width/(2.0*imax);
		
		x1= 0.5*wx-0.25*width;
		x2= 0.5*wx+0.25*width;
		y1= 0.5*wy - 0.5*fac*imay;
		y2= 0.5*wy + 0.5*fac*imay;
	}
	else {
		if(wx<wy) width= wy;
		else width= wx;

		fac= width/(2.0*imay);
		
		y1= 0.5*wy-0.25*width;
		y2= 0.5*wy+0.25*width;
		x1= 0.5*wx - 0.5*fac*imax;
		x2= 0.5*wx + 0.5*fac*imax;
	}


	/* zoom van viewborder */
	fac= (1.41421+( (float)G.vd->camzoom )/50.0);
	fac*= fac;
	
	width= (x2-x1)/4.0;
	a= (x2+x1)/2.0;
	x1= a-width*fac;
	x2= a+width*fac;

	width= (y2-y1)/4.0;
	a= (y2+y1)/2.0;
	y1= a-width*fac;
	y2= a+width*fac;
	
	/* deze getallen voor renderdisplay */
	G.vd->pr_xmin= fceil(x1);
	G.vd->pr_ymin= fceil(y1);
	G.vd->pr_xmax= fceil(x2);
	G.vd->pr_ymax= fceil(y2);
}

void drawviewborder()
{
	float fac, a;
	float x1, x2, y1, y2;
	float x3, y3, x4, y4;

	x1= G.vd->pr_xmin;
	y1= G.vd->pr_ymin;
	x2= G.vd->pr_xmax;
	y2= G.vd->pr_ymax;

	/* rand */
	setlinestyle(3);
	cpack(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
	glRectf(x1+1,  y1-1,  x2+1,  y2-1); 
	
	cpack(0xFFFFFF);
	glRectf(x1,  y1,  x2,  y2); 

	/* border */
	if(G.scene->r.mode & R_BORDER) {
		
		cpack(0);
		x3= x1+ G.scene->r.border.xmin*(x2-x1);
		y3= y1+ G.scene->r.border.ymin*(y2-y1);
		x4= x1+ G.scene->r.border.xmax*(x2-x1);
		y4= y1+ G.scene->r.border.ymax*(y2-y1);
		
		glRectf(x3+1,  y3-1,  x4+1,  y4-1); 
		
		cpack(0x4040FF);
		glRectf(x3,  y3,  x4,  y4); 
	}

	/* safetykader */

	fac= 0.1;
	
	a= fac*(x2-x1);
	x1+= a; 
	x2-= a;

	a= fac*(y2-y1);
	y1+= a;
	y2-= a;

	cpack(0);
	glRectf(x1+1,  y1-1,  x2+1,  y2-1);
	cpack(0xFFFFFF);
	glRectf(x1,  y1,  x2,  y2);

	setlinestyle(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void backdrawview3d(int test)
{
	struct Base *base;
	int tel=1;
	uint kleur;

#if defined(WIN32)
	if(G.f & (G_VERTEXPAINT|G_FACESELECT));
	else {
		G.vd->flag &= ~V3D_NEEDBACKBUFDRAW;
		return;
	}
#endif

	if(G.vd->flag & V3D_NEEDBACKBUFDRAW); else return;
	if(G.obedit) {
		G.vd->flag &= ~V3D_NEEDBACKBUFDRAW;
		return;
	}
	
	if(test) {
		if(qtest()) {
			addafterqueue(curarea->win, BACKBUFDRAW, 1);
			return;
		}
	}

#if defined(__sgi) || defined(__SUN) || defined(__BeOS) || defined(WIN32)
	glDisable(GL_DITHER);
#endif

	if(G.vd->drawtype > OB_WIRE) G.zbuf= TRUE;
	curarea->win_swap &= ~WIN_BACK_OK;
	
	glClearColor(0.0, 0.0, 0.0, 0.0); 
	if(G.zbuf) {
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	}
	else {
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
	}
	
	G.f |= G_BACKBUFSEL;
	
	if(G.f & (G_VERTEXPAINT|G_FACESELECT)) {
		base= BASACT;
		if(base && (base->lay & G.vd->lay)) {
			draw_object(base);
		}
	}
	else {

		base= FIRSTBASE;
		while(base) {
			
			/* elke base ivm meerdere windows */
			base->selcol= 0x070707 | ( ((tel & 0xF00)<<12) + ((tel & 0xF0)<<8) + ((tel & 0xF)<<4) );
			tel++;
	
			if(base->lay & G.vd->lay) {
				
				if(test) {
					if(qtest()) {
						addafterqueue(curarea->win, BACKBUFDRAW, 1);
						break;
					}
				}
				
				cpack(base->selcol);
				draw_object(base);
			}
			base= base->next;
		}
	}
	
	if(base==0) G.vd->flag &= ~V3D_NEEDBACKBUFDRAW;

	G.f &= ~G_BACKBUFSEL;
	G.zbuf= FALSE; 
	glDisable(GL_DEPTH_TEST);

#if defined(__sgi) || defined(__SUN) || defined(__BeOS) || defined (WIN32)
	glEnable(GL_DITHER);
#endif
}

		
void drawname(Object *ob)
{
	char str[8];
	
	cpack(0x404040);
	glRasterPos3f(0.0,  0.0,  0.0);
	str[0]= ' '; str[1]= 0;
	fmsetfont(G.font);
	fmprstr(str);
	fmprstr(ob->id.name+2);
}

void draw_view_icon()
{
	extern struct uiIconImage UIicon[];
	int iconx=0, icony=3;
	uint *rect;
	
	if(G.vd->view==7) iconx= 5;
	else if(G.vd->view==1) iconx= 4;
	else if(G.vd->view==3) iconx= 3;

	if(iconx) {
		glPixelStorei(GL_UNPACK_ROW_LENGTH,  UIicon[0].xim);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA); 
	
		rect= UIicon[0].rect+ (3 + icony*UIicon[0].yofs)*UIicon[0].xim+ (iconx)*UIicon[0].xofs + 3 ;
	
		glRasterPos3f(5.0, 5.0, 0.0);	
		glDrawPixels(UIicon[0].xofs-6, UIicon[0].yofs-6, GL_RGBA, GL_UNSIGNED_BYTE,  rect);
	
		glBlendFunc(GL_ONE,  GL_ZERO); 
		glDisable(GL_BLEND);
		glPixelStorei(GL_UNPACK_ROW_LENGTH,  0);
	}
}

void drawview3d()
{
	Base *base;
	Object *ob;
	
	setwinmatrixview3d(0);	/* 0= geen pick rect */

	setviewmatrixview3d();

	Mat4MulMat4(G.vd->persmat, G.vd->viewmat, curarea->winmat);
	Mat4Invert(G.vd->persinv, G.vd->persmat);
	Mat4Invert(G.vd->viewinv, G.vd->viewmat);

	if(G.vd->drawtype > OB_WIRE) {
		G.zbuf= TRUE;
		glEnable(GL_DEPTH_TEST);
		if(G.f & G_SIMULATION) {
			glClearColor(0.0, 0.0, 0.0, 0.0); 
		}
		else {
			glClearColor(0.45, 0.45, 0.45, 0.0); 
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glLoadIdentity();
	}
	else {
		glClearColor(0.45, 0.45, 0.45, 0.0); 
		glClear(GL_COLOR_BUFFER_BIT);
	}
	
	loadmatrix(G.vd->viewmat);

	if(G.vd->view==0 || G.vd->persp!=0) {
		drawfloor();
		if(G.vd->persp==2) {
			if(G.scene->world) {
				if(G.scene->world->mode & WO_STARS) RE_make_stars(1);
			}
			calc_viewborder();
			if(G.vd->flag & V3D_DISPBGPIC) draw_bgpic();
		}
	}
	else {
		drawgrid();

		if(G.vd->flag & V3D_DISPBGPIC) {
			draw_bgpic();
		}
	}
	
	/* eerst set tekenen */
	if(G.scene->set) {
	
		/* patchje: kleur blijft constant */ 
		G.f |= G_PICKSEL;
		
		base= G.scene->set->base.first;
		while(base) {
			if(G.vd->lay & base->lay) {
				where_is_object(base->object);

				cpack(0x404040);
				draw_object(base);

				if(base->object->transflag & OB_DUPLI) {
					extern ListBase duplilist;
					Base tbase;
					
					tbase= *base;
					
					tbase.flag= OB_FROMDUPLI;
					make_duplilist(G.scene->set, base->object);
					ob= duplilist.first;
					while(ob) {
						tbase.object= ob;
						draw_object(&tbase);
						ob= ob->id.next;
					}
					free_duplilist();
					
				}
			}
			base= base->next;
		}
		
		G.f &= ~G_PICKSEL;
	}
	
/* SILLY CODE!!!! */
/* See next silly code... why is the same code
 * ~ duplicated twice, and then this silly if(FALSE)
 * in part... wacky! and bad!
 */
	/* eerst niet selected en dupli's */
	base= G.scene->base.first;
	while(base) {
		
		if(G.vd->lay & base->lay) {
		
			where_is_object(base->object);

			if(FALSE && base->object->transflag & OB_DUPLI) {
				extern ListBase duplilist;
				Base tbase;

				/* altijd eerst original tekenen vanwege make_displist */
				draw_object(base);

				/* patchje: kleur blijft constant */ 
				G.f |= G_PICKSEL;
				cpack(0x404040);
				
				tbase.flag= OB_FROMDUPLI;
				make_duplilist(G.scene, base->object);

				ob= duplilist.first;
				while(ob) {
					tbase.object= ob;
					draw_object(&tbase);
					ob= ob->id.next;
				}
				free_duplilist();
				
				G.f &= ~G_PICKSEL;				
			}
			else if((base->flag & SELECT)==0) {
				draw_object(base);
			}
			
		}
		
		base= base->next;
	}
	/*  selected */
	base= G.scene->base.first;
	while(base) {
		
		if TESTBASE(base) {
			draw_object(base);
		}
		
		base= base->next;
	}

/* SILLY CODE!!!! */
	/* dupli's, als laatste om zeker te zijn de displisten zijn ok */
	base= G.scene->base.first;
	while(base) {
		
		if(G.vd->lay & base->lay) {
			if(base->object->transflag & OB_DUPLI) {
				extern ListBase duplilist;
				Base tbase;

				/* patchje: kleur blijft constant */ 
				G.f |= G_PICKSEL;
				cpack(0x404040);
				
				tbase.flag= OB_FROMDUPLI;
				make_duplilist(G.scene, base->object);

				ob= duplilist.first;
				while(ob) {
					tbase.object= ob;
					draw_object(&tbase);
					ob= ob->id.next;
				}
				free_duplilist();
				
				G.f &= ~G_PICKSEL;				
			}
		}
		base= base->next;
	}


	if(G.scene->radio) rad_drawall();	/* radview.c */
	
	persp(0);
	
	if(G.vd->persp>1) drawviewborder();
	drawcursor();
	draw_view_icon();
	
	persp(1);

	G.vd->flag &= ~V3D_DISPIMAGE;
	curarea->win_swap= WIN_BACK_OK;
	
	if(G.zbuf) {
		G.zbuf= FALSE;
		glDisable(GL_DEPTH_TEST);
	}
	
#if defined(__WIN32)

	if(G.f & (G_VERTEXPAINT|G_FACESELECT)) {
		G.vd->flag |= V3D_NEEDBACKBUFDRAW;
		addafterqueue(curarea->win, BACKBUFDRAW, 1);
	}

#else
	
	G.vd->flag |= V3D_NEEDBACKBUFDRAW;
	addafterqueue(curarea->win, BACKBUFDRAW, 1);
	
#endif
}

void drawview3d_render()
{
	extern short v3d_windowmode;
	Base *base;
	Object *ob;
	
	winset(R.win);
	
	v3d_windowmode= 1;
	setwinmatrixview3d(0);
	v3d_windowmode= 0;
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(R.winmat);
	glMatrixMode(GL_MODELVIEW);
	
	setviewmatrixview3d();
	glLoadMatrixf(G.vd->viewmat);

	Mat4MulMat4(G.vd->persmat, G.vd->viewmat, R.winmat);
	Mat4Invert(G.vd->persinv, G.vd->persmat);
	Mat4Invert(G.vd->viewinv, G.vd->viewmat);

	if(G.vd->drawtype > OB_WIRE) {
		G.zbuf= TRUE;
		glEnable(GL_DEPTH_TEST);
	}

	if(G.scene->world) glClearColor(G.scene->world->horr, G.scene->world->horg, G.scene->world->horb, 0.0); 
	else glClearColor(0.45, 0.45, 0.45, 0.0); 
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glLoadIdentity();

	loadmatrix(G.vd->viewmat);
	
	/* abuse! to make sure it doesnt draw the helpstuff */
	G.f |= G_SIMULATION;

	do_all_ipos();
	do_all_scripts(SCRIPT_FRAMECHANGED);
	do_all_keys();
	do_all_ikas();
	test_all_displists();

	/* niet erg nette calc_ipo en where_is forceer */
	ob= G.main->object.first;
	while(ob) {
		ob->ctime= -123.456;
		ob= ob->id.next;
	}

	/* eerst set tekenen */
	if(G.scene->set) {
	
		/* patchje: kleur blijft constant */ 
		G.f |= G_PICKSEL;
		
		base= G.scene->set->base.first;
		while(base) {
			if(G.vd->lay & base->lay) {
				if ELEM3(base->object->type, OB_LAMP, OB_CAMERA, OB_LATTICE);
				else {
					where_is_object(base->object);
	
					cpack(0x404040);
					draw_object(base);
	
					if(base->object->transflag & OB_DUPLI) {
						extern ListBase duplilist;
						Base tbase;
						
						tbase.flag= OB_FROMDUPLI;
						make_duplilist(G.scene->set, base->object);
						ob= duplilist.first;
						while(ob) {
							tbase.object= ob;
							draw_object(&tbase);
							ob= ob->id.next;
						}
						free_duplilist();
					}
				}
			}
			base= base->next;
		}
		
		G.f &= ~G_PICKSEL;
	}
	
	
	/* eerst niet selected en dupli's */
	base= G.scene->base.first;
	while(base) {
		
		if(G.vd->lay & base->lay) {
			if ELEM3(base->object->type, OB_LAMP, OB_CAMERA, OB_LATTICE);
			else {
				where_is_object(base->object);
	
				if(base->object->transflag & OB_DUPLI) {
					extern ListBase duplilist;
					Base tbase;
					
					/* altijd eerst original tekenen vanwege make_displist */
					draw_object(base);
					
					/* patchje: kleur blijft constant */ 
					G.f |= G_PICKSEL;
					cpack(0x404040);
					
					tbase.flag= OB_FROMDUPLI;
					make_duplilist(G.scene, base->object);
					ob= duplilist.first;
					while(ob) {
						tbase.object= ob;
						draw_object(&tbase);
						ob= ob->id.next;
					}
					free_duplilist();
					
					G.f &= ~G_PICKSEL;				
				}
				else if((base->flag & SELECT)==0) {
					draw_object(base);
				}
			}
		}
		
		base= base->next;
	}
	/*  selected */
	base= G.scene->base.first;
	while(base) {
		
		if TESTBASE(base) {
			if ELEM3(base->object->type, OB_LAMP, OB_CAMERA, OB_LATTICE);
			else draw_object(base);
		}
		
		base= base->next;
	}

	if(G.scene->radio) rad_drawall();	/* radview.c */
	
	if(G.zbuf) {
		G.zbuf= FALSE;
		glDisable(GL_DEPTH_TEST);
	}
	
	G.f &= ~G_SIMULATION;
	
	glFinish();
	
	glReadPixels(0, 0, R.rectx, R.recty, GL_RGBA, GL_UNSIGNED_BYTE, R.rectot);
	ortho2(-0.5, (float)R.winx-0.5, -0.5, (float)R.winy-0.5);
	glLoadIdentity();
}


double tottime = 0.0;

int update_time()
{
	struct tms voidbuf;
	static int ltime;
	int time;

#ifdef __BeOS
	time= glut_system_time()/10000;
#else
	time = times(&voidbuf);
#endif
	
	tottime += (time - ltime) / TIME_INTERVAL;
	ltime = time;
	return (tottime < 0.0);
}

double speed_to_swaptime(int speed)
{
	switch(speed) {
	case 1:
		return 1.0/60.0;
	case 2:
		return 1.0/50.0;
	case 3:
		return 1.0/30.0;
	case 4:
		return 1.0/25.0;
	case 5:
		return 1.0/20.0;
	case 6:
		return 1.0/15.0;
	case 7:
		return 1.0/12.5;
	case 8:
		return 1.0/10.0;
	case 9:
		return 1.0/6.0;
	}
	return 1.0/4.0;
}

double key_to_swaptime(int key)
{
	switch(key) {
	case PAD1:
		G.animspeed= 1;
		tottime= 0;
		return speed_to_swaptime(1);
	case PAD2:
		G.animspeed= 2;
		tottime= 0;
		return speed_to_swaptime(2);
	case PAD3:
		G.animspeed= 3;
		tottime= 0;
		return speed_to_swaptime(3);
	case PAD4:
		G.animspeed= 4;
		tottime= 0;
		return speed_to_swaptime(4);
	case PAD5:
		G.animspeed= 5;
		tottime= 0;
		return speed_to_swaptime(5);
	case PAD6:
		G.animspeed= 6;
		tottime= 0;
		return speed_to_swaptime(6);
	case PAD7:
		G.animspeed= 7;
		tottime= 0;
		return speed_to_swaptime(7);
	case PAD8:
		G.animspeed= 8;
		tottime= 0;
		return speed_to_swaptime(8);
	case PAD9:
		G.animspeed= 9;
		tottime= 0;
		return speed_to_swaptime(9);
	}
	
	return speed_to_swaptime(G.animspeed);
}

#ifdef NAN_GAME

void sumo_callback(void *obp)
{
	Object *ob= obp;
	SM_Vector3 vec;
	float matf[3][3];
	int i, j;

    SM_GetMatrixf(ob->sumohandle, ob->obmat[0]);

	VECCOPY(ob->loc, ob->obmat[3]);
	
    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 3; ++j) {
            matf[i][j] = ob->obmat[i][j];
        }
    }
    Mat3ToEul(matf, ob->rot);
}

void init_anim_sumo()
{
	extern Material defmaterial;
	Base *base;
    Mesh *me;
	Object *ob;
    Material *mat;
	MFace *mface;
	MVert *mvert;
    float centre[3], size[3];
	int a;
    SM_ShapeHandle shape;
	SM_SceneHandle scene;
    SM_Material material;
    SM_MassProps massprops;
    SM_Vector3 vec;
    SM_Vector3 scaling;
	
	scene= SM_CreateScene();
	G.scene->sumohandle = scene;
	
	vec[0]=  0.0;
	vec[1]=  0.0;
	vec[2]= -9.8;
	SM_SetForceField(scene, vec);
	
    /* ton: cylinders & cones are still Y-axis up, will be Z-axis later */
    /* ton: write location/rotation save and restore */
	
	base= FIRSTBASE;
	while (base) {
		if (G.vd->lay & base->lay) {
            ob= base->object;
			 
            /* define shape, for now only meshes take part in physics */
            get_local_bounds(ob, centre, size);
            
            if (ob->type==OB_MESH) {
                me= ob->data;
                
                if (ob->gameflag & OB_DYNAMIC) {
                    if (me->sumohandle)
                        shape= me->sumohandle;
                    else {
                        /* make new handle */
                        switch(ob->boundtype) {
                        case OB_BOUND_BOX:
                            shape= SM_Box(2.0*size[0], 2.0*size[1], 2.0*size[2]);
                            break;
                        case OB_BOUND_SPHERE:
                            shape= SM_Sphere(size[0]);
                            break;
                        case OB_BOUND_CYLINDER:
                            shape= SM_Cylinder(size[0], 2.0*size[2]);
                            break;
                        case OB_BOUND_CONE:
                            shape= SM_Cone(size[0], 2.0*size[2]);
                            break;
                        }
                        
						me->sumohandle= shape;
					}
                    /* sumo material properties */
                	mat= give_current_material(ob, 0);
                	if(mat==NULL)
                        mat= &defmaterial;
                    
                	material.restitution= mat->reflect;
                	material.static_friction= mat->friction;
                	material.dynamic_friction= mat->friction;
                    
                	/* sumo mass properties */
                	massprops.mass= ob->mass;
                	massprops.center[0]= 0.0;
                	massprops.center[1]= 0.0;
                	massprops.center[2]= 0.0;

                	massprops.inertia[0]= 0.5*ob->mass;
                	massprops.inertia[1]= 0.5*ob->mass;
                	massprops.inertia[2]= 0.5*ob->mass;

                	massprops.orientation[0]= 0.0;
                	massprops.orientation[1]= 0.0;
                	massprops.orientation[2]= 0.0;
                	massprops.orientation[3]= 1.0;

                	ob->sumohandle = SM_CreateObject(ob, shape, &material, 
                                                     &massprops, sumo_callback);
					SM_AddObject(scene, ob->sumohandle);
					
                    scaling[0] = ob->size[0];
                    scaling[1] = ob->size[1];
                    scaling[2] = ob->size[2];
					SM_SetMatrixf(ob->sumohandle, ob->obmat[0]);
					SM_SetScaling(ob->sumohandle, scaling);

				}
 				else {
 					if(me->sumohandle) shape= me->sumohandle;
					else {
						/* make new handle */
            			shape= SM_NewComplexShape();
						
						mface= me->mface;
						mvert= me->mvert;
						for(a=0; a<me->totface; a++,mface++) {
							if(mface->v3) {
								SM_Begin();
								SM_Vertex( (mvert+mface->v1)->co[0], (mvert+mface->v1)->co[1], (mvert+mface->v1)->co[2]);
								SM_Vertex( (mvert+mface->v2)->co[0], (mvert+mface->v2)->co[1], (mvert+mface->v2)->co[2]);
								SM_Vertex( (mvert+mface->v3)->co[0], (mvert+mface->v3)->co[1], (mvert+mface->v3)->co[2]);
								if(mface->v4)
									SM_Vertex( (mvert+mface->v4)->co[0], (mvert+mface->v4)->co[1], (mvert+mface->v4)->co[2]);
								SM_End();
							}
						}
						
						SM_EndComplexShape();
						
						me->sumohandle= shape;
					}
                    /* sumo material properties */
                	mat= give_current_material(ob, 0);
                	if(mat==NULL)
                        mat= &defmaterial;
                	material.restitution= mat->reflect;
                	material.static_friction= mat->friction;
                	material.dynamic_friction= mat->friction;

                	/* sumo mass properties */
                	massprops.mass= ob->mass;
                	massprops.center[0]= 0.0;
                	massprops.center[1]= 0.0;
                	massprops.center[2]= 0.0;

                	massprops.inertia[0]= 0.5*ob->mass;
                	massprops.inertia[1]= 0.5*ob->mass;
                	massprops.inertia[2]= 0.5*ob->mass;

                	massprops.orientation[0]= 0.0;
                	massprops.orientation[1]= 0.0;
                	massprops.orientation[2]= 0.0;
                	massprops.orientation[3]= 1.0;

                	ob->sumohandle= SM_CreateObject(ob, shape, &material, NULL, NULL);
					SM_AddObject(scene, ob->sumohandle);

                    scaling[0] = ob->size[0];
                    scaling[1] = ob->size[1];
                    scaling[2] = ob->size[2];
					SM_SetMatrixf(ob->sumohandle, ob->obmat[0]);
					SM_SetScaling(ob->sumohandle, scaling);
				}
            }
        }    	
    	base= base->next;
    }
}

/* update animated objects */
void update_anim_sumo()
{
    SM_Vector3 scaling;

	Base *base;
	Object *ob;
	Mesh *me;
	
	base= FIRSTBASE;
	while(base) {
		if(G.vd->lay & base->lay) {
			ob= base->object;
			
			if(ob->sumohandle) {
				if((ob->gameflag & OB_DYNAMIC)==0) {
					/* evt: optimise, check for anim */
                    scaling[0] = ob->size[0];
                    scaling[1] = ob->size[1];
                    scaling[2] = ob->size[2];
					SM_SetMatrixf(ob->sumohandle, ob->obmat[0]);
					SM_SetScaling(ob->sumohandle, scaling);
				}
			}				
		}
		base= base->next;
	}

}

void end_anim_sumo()
{
	Base *base;
	Object *ob;
	Mesh *me;
	
	base= FIRSTBASE;
	while(base) {
		if(G.vd->lay & base->lay) {
			ob= base->object;
			
            if(ob->type==OB_MESH) {
				if(ob->sumohandle) {
					SM_RemoveObject(G.scene->sumohandle, ob->sumohandle);
					SM_DeleteObject(ob->sumohandle);
					ob->sumohandle= NULL;
				}
				me= ob->data;
				if(me->sumohandle) {
					SM_DeleteShape(me->sumohandle);
					me->sumohandle= NULL;
				}
			}
		}
		base= base->next;
	}
	if(G.scene->sumohandle) {
		SM_DeleteScene(G.scene->sumohandle);
		G.scene->sumohandle= NULL;
	}
}

#endif

void inner_play_anim_loop(int init, int mode)
{
	ScrArea *sa;
	static ScrArea *oldsa;
	static double swaptime;
	static int curmode;
	
	/* init */
	if(init) {
		oldsa= curarea;
		swaptime= speed_to_swaptime(G.animspeed);
		tottime= 0.0;
		curmode= mode;
#ifdef NAN_GAME
        init_anim_sumo();
#endif        
		return;
	}

	RE_set_timecursor(CFRA);
	do_all_ipos();
	do_all_scripts(SCRIPT_FRAMECHANGED);
	do_all_keys();
	do_all_ikas();

	test_all_displists();
#ifdef NAN_GAME	
	update_anim_sumo();
	
	SM_Proceed(G.scene->sumohandle, swaptime, 40, NULL);
#endif
	sa= G.curscreen->areabase.first;
	while(sa) {
		if(sa==oldsa) {
			if(sa->win && sa->windraw) {
				/* hier winget() gebruiken: anders wordt in header getekend */
				if(sa->win != winget()) areawinset(sa->win);
				sa->windraw();
			}
		}
		else if(curmode) {
			if ELEM(sa->spacetype, SPACE_VIEW3D, SPACE_SEQ) {
				if(sa->win && sa->windraw) {
					/* hier winget() gebruiken: anders wordt in header getekend */
					if(sa->win != winget()) areawinset(sa->win);
					sa->windraw();
				}
			}
		}
		
		sa= sa->next;	
	}
	
	/* minimaal swaptime laten voorbijgaan */
	tottime -= swaptime;
	while (update_time()) usleep(1);

	if(CFRA==EFRA) {
		if (tottime > 0.0) tottime = 0.0;
		CFRA= SFRA;
	}
	else CFRA++;
	
}

int play_anim(int mode)
{
	ScrArea *sa, *oldsa;
	int cfraont;
	ushort event=0;
	short val;
	
	/* patch voor zeer oude scenes */
	if(SFRA==0) SFRA= 1;
	if(EFRA==0) EFRA= 250;
	
	if(SFRA>EFRA) return 0;
	
	update_time();

	/* waitcursor(1); */
	G.f |= G_PLAYANIM;		/* in sequence.c en view.c wordt dit afgevangen */

	cfraont= CFRA;
	oldsa= curarea;
	
	inner_play_anim_loop(1, mode);	/* 1==init */

	while(TRUE) {

		inner_play_anim_loop(0, 0);
	
		screen_swapbuffers();
		
		while(qtest()) {
		
			event= extern_qread(&val);
			if(event==ESCKEY) break;
			else if(event==MIDDLEMOUSE) {
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
			else if(val) {
				if(event==PAGEUPKEY) {
					Group *group= G.main->group.first;
					while(group) {
						next_group_key(group);
						group= group->id.next;
					}
				}
				else if(event==PAGEDOWNKEY) {
					Group *group= G.main->group.first;
					while(group) {
						prev_group_key(group);
						group= group->id.next;
					}
				}
			}
		}
		if(event==ESCKEY || event==SPACEKEY) break;
				
		if(mode==2 && CFRA==EFRA) break;	
	}

	if(event==SPACEKEY);
	else CFRA= cfraont;
	
	do_all_ipos();
	do_all_keys();
	do_all_ikas();

	if(oldsa!=curarea) areawinset(oldsa->win);
	
	/* restore all areas */
	sa= G.curscreen->areabase.first;
	while(sa) {
		if( (mode && sa->spacetype==SPACE_VIEW3D) || sa==curarea) addqueue(sa->win, REDRAW, 1);
		
		sa= sa->next;	
	}
	
	/* speed button */
	allqueue(REDRAWBUTSANIM, 0);
	/* groups could have changed ipo */
	allspace(REMAKEIPO, 0);
	allqueue(REDRAWIPO, 0);
	
	/* vooropig */
	do_global_buttons(B_NEWFRAME);
#ifdef NAN_GAME	
	end_anim_sumo();
#endif
	waitcursor(0);
	G.f &= ~G_PLAYANIM;
	
	if (event==ESCKEY || event==SPACEKEY) return 1;
	else return 0;
}

