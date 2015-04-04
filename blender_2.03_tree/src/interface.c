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

/*  interface.c   june 2000

 * 
 *  ton roosendaal
 * Version: $Id: interface.c,v 1.31 2000/09/25 22:02:54 ton Exp $
 */

#include "blender.h"
#include "graphics.h"
#include "screen.h"
#include "interface.h"


/* naming conventions:
 * 
 * uiBlahBlah()		external function
 * ui_blah_blah()	internal function
 */

/* ************ GLOBALS ************* */

#define BUTM_ACTIVE 0xc07070

float UIwinmat[4][4];
int UIfrontbuf= 0, UIlock= 0, UIafterval;
char *UIlockstr=NULL;
void (*UIafterfunc)();

uiCol  UIcol[UI_ARRAY];
uiIconImage UIicon[UI_ARRAY];
uiFont UIfont[UI_ARRAY];
static uiBut *UIbuttip;

/* ****************************** */


void uiInit()
{
	int a;
	
	bzero(UIcol, sizeof(UIcol));
	bzero(UIicon, sizeof(UIicon));
	bzero(UIfont, sizeof(UIfont));
}

void uiEnd()
{
	int a;
	
	for(a=0; a<UI_ARRAY; a++) {
		if(UIicon[a].rect) freeN(UIicon[a].rect);
		UIicon[a].rect= NULL;
	}
}

void disable_capslock(int val)
{
	
}


void uiDefButIcon(short nr, uint *rect, short xim, short yim, short xofs, short yofs)
{
	uint transp, *dr;
	int a;
	
	UIicon[nr].rect= rect;
	UIicon[nr].xim= xim;
	UIicon[nr].yim= yim;
	UIicon[nr].xofs= xofs;
	UIicon[nr].yofs= yofs;
	
	/* transparant color */
	transp= rect[3*xim + 3];

	dr= rect;
	a= xim*yim;
	while(a--) {
		if( *dr == transp) ((char *)dr)[3]= 0;
		dr++;
	}
}


/* ************* DRAW ************** */


static void ui_graphics_to_window(float *x, float *y)	/* voor rectwrite b.v. */
{
	float gx, gy;
	int sx, sy;
	int getsizex, getsizey;
		
	getsize(&getsizex, &getsizey);
	
	gx= *x;
	gy= *y;
	*x= getsizex*(0.5+ 0.5*(gx*UIwinmat[0][0]+ gy*UIwinmat[1][0]+ UIwinmat[3][0]));
	*y= getsizey*(0.5+ 0.5*(gx*UIwinmat[0][1]+ gy*UIwinmat[1][1]+ UIwinmat[3][1]));
	
	mygetsuborigin(&sx, &sy);
	*x+= sx;
	*y+= sy;
}



static void ui_window_to_graphics(float *x, float *y)	/* voor muiscursor b.v. */
{
	float a, b, c, d, e, f, px, py;
	int getsizex, getsizey;
		
	getsize(&getsizex, &getsizey);

	a= .5*getsizex*UIwinmat[0][0];
	b= .5*getsizex*UIwinmat[1][0];
	c= .5*getsizex*(1.0+UIwinmat[3][0]);

	d= .5*getsizey*UIwinmat[0][1];
	e= .5*getsizey*UIwinmat[1][1];
	f= .5*getsizey*(1.0+UIwinmat[3][1]);
	
	px= *x;
	py= *y;
	
	*y=  (a*(py-f) + d*(c-px))/(a*e-d*b);
	*x= (px- b*(*y)- c)/a;
	
}

static short UIoldcursor= 0, UIoldwin;

void ui_bgnpupdraw(int startx, int starty, int endx, int endy, int cursor)
{
	#if defined(__sgi) || defined(__SUN)
	/* this is a dirty patch: XgetImage gets sometimes the backbuffer */
	my_get_frontbuffer_image(0, 0, 1, 1);
	my_put_frontbuffer_image();
	#endif

	UIoldwin= winget();

	winset(G.curscreen->mainwin);
	
	/* pietsje groter, 1 pixel aan de rand */
	
	glReadBuffer(GL_FRONT);
	glDrawBuffer(GL_FRONT);
	
	/* for geforce and other cards */
	glFinish();

	my_get_frontbuffer_image(startx-1, starty-1, endx-startx+2, endy-starty+6);

	if(cursor) {
		mygetcursor(&UIoldcursor);
		glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
	}
	else UIoldcursor= 0;
		
}

void ui_endpupdraw()
{
	int x;
	
	/* for geforce and other cards */
	glFinish();
	
	my_put_frontbuffer_image();
	
	if(UIoldwin) {
		winset(UIoldwin);
		if(UIoldcursor) glutSetCursor(UIoldcursor);
	}

	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);
}


static uint temprect[40*40*4];


void uiDrawIcon(uiBut *but)
{
	uint *rii, paper, firstcol;
	float xs, ys;
	int high, a, b, sizea, sizeb, rfac=256, gfac=256, bfac=256, fac;
	char *rd, *ri, *col;	/* rectdraw, recticon */
	
	if(but->icon==0) return;
	
	rd= (char *)temprect;
	rii= but->icon->rect;
	if(rii==0) {
		printf("Non existing iconrect\n");
		return;
	}

	/* eerste pixels zijn zwart: grid, en daarbij rand: 3 pixels totaal offset*/
	rii+= (3 + but->icony*but->icon->yofs)*but->icon->xim+ (but->iconx+but->iconxadd)*but->icon->xofs + 3 ;
	/* en natuurlijk de andere rand eraf: */
	sizea= but->icon->xofs-5;
	sizeb= but->icon->yofs-5;
	
	firstcol= *rii;
	
	if(*rii) {

		/* papercol */
		if(but->flag & UI_SELECT) paper= but->col->grey;
		else {
			if(but->flag & UI_ACTIVE) {
				if( but->type == BUTM) paper= BUTM_ACTIVE;
				else paper= but->col->hilite;
			}
			else paper= but->col->medium;
		}
	
		col= (char *)&paper;				/* ABGR */
		ri= (char *)rii;					/* eerste kleur icon==paperkleur */
		
		if(ri[0] && ri[1] && ri[2]) {
			rfac= (col[RCOMP]<<8)/ri[0];
			gfac= (col[GCOMP]<<8)/ri[1];
			bfac= (col[BCOMP]<<8)/ri[2];
		}
	}
	
	for(b=sizeb; b>0; b--) {
		ri= (char *)rii;
		for(a=sizea; a>0; a--, ri+=4, rd+=4) {
			if( firstcol == *((uint *)ri) ) rd[3]= 0;
			else {
				fac= (rfac*ri[0])>>8;
				if(fac>255) rd[0]= 255; else rd[0]= fac;
				fac= (gfac*ri[1])>>8;
				if(fac>255) rd[1]= 255; else rd[1]= fac;
				fac= (bfac*ri[2])>>8;
				if(fac>255) rd[2]= 255; else rd[2]= fac;
				rd[3]= 255;
			}
		}
		rii+= but->icon->xim;
	}
	
	rii= temprect;
	
	/* since we zoom. don't clip icons! */
	
	xs= (but->x1+but->x2- sizea)/2.0;
	ys= (but->y1+but->y2- sizeb)/2.0;

	glRasterPos2f(xs, ys);

	if(but->aspect>1.1) glPixelZoom(1.0/but->aspect, 1.0/but->aspect);	
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA); 
	glDrawPixels(sizea, sizeb, GL_RGBA, GL_UNSIGNED_BYTE,  rii);
	glBlendFunc(GL_ONE,  GL_ZERO); 
	glDisable(GL_BLEND);

	glPixelZoom(1.0, 1.0);	
}

void ui_draw_outlineX(float x1, float y1, float x2, float y2, float asp1)
{
	float vec[2];
	
	glBegin(GL_LINE_LOOP);
	vec[0]= x1+asp1; vec[1]= y1-asp1;
	glVertex2fv(vec);
	vec[0]= x2-asp1; 
	glVertex2fv(vec);
	vec[0]= x2+asp1; vec[1]= y1+asp1;
	glVertex2fv(vec);
	vec[1]= y2-asp1;
	glVertex2fv(vec);
	vec[0]= x2-asp1; vec[1]= y2+asp1;
	glVertex2fv(vec);
	vec[0]= x1+asp1;
	glVertex2fv(vec);
	vec[0]= x1-asp1; vec[1]= y2-asp1;
	glVertex2fv(vec);
	vec[1]= y1+asp1;
	glVertex2fv(vec);
	glEnd();		
	
}

void uiEmbossX(uiCol *bc, float asp, float x1, float y1, float x2, float y2, int sel)
{
	float asp1, asp2;
	float vec[2];
	
	asp1= asp;
	asp2= asp1+asp;

	x1+= asp1;
	x2-= asp1;
	y1+= asp1;
	y2-= asp1;

	/* below */
	if(sel) cpack(bc->medium);
	else cpack(bc->dark);
	fdrawline(x1, y1, x2, y1);

	/* right */
	fdrawline(x2, y1, x2, y2);
	
	/* top */
	if(sel) cpack(bc->dark);
	else cpack(bc->white);
	fdrawline(x1, y2, x2, y2);

	/* left */
	fdrawline(x1, y1, x1, y2);
	
	/* shadow */
	if(UIfrontbuf==0) {
		cpack(bc->grey);
		glBegin(GL_LINE_STRIP);
		vec[0]= x1+asp2; vec[1]= y1-asp2;
		glVertex2fv(vec);
		vec[0]= x2-asp1; 
		glVertex2fv(vec);
		vec[0]= x2+asp2; vec[1]= y1+asp1;
		glVertex2fv(vec);
		vec[1]= y2-asp2;
		glVertex2fv(vec);
		glEnd();	
	}
	/* outline */
	cpack(0x0);
	ui_draw_outlineX(x1, y1, x2, y2, asp1);
}

void uiEmboss(float x1, float y1, float x2, float y2, int sel)
{
	float vec[2];
		
	/* below */
	if(sel) cpack(0xFFFFFF);
	else cpack(0x0);
	fdrawline(x1, y1, x2, y1);

	/* right */
	fdrawline(x2, y1, x2, y2);
	
	/* top */
	if(sel) cpack(0x0);
	else cpack(0xFFFFFF);
	fdrawline(x1, y2, x2, y2);

	/* left */
	fdrawline(x1, y1, x1, y2);
	
}

void uiEmbossW(uiCol *bc, float asp, float x1, float y1, float x2, float y2, int sel)
{
	float asp1, asp2;
	float vec[2];
	
	if(bc==NULL) bc= &UIcol[0];
	
	asp1= asp;
	asp2= asp1+asp;

	/* below */
	if(sel) cpack(bc->white);
	else cpack(bc->dark);
	fdrawline(x1, y1, x2, y1);

	/* right */
	fdrawline(x2, y1, x2, y2);
	
	/* top */
	if(sel) cpack(bc->dark);
	else cpack(bc->white);
	fdrawline(x1, y2, x2, y2);

	/* left */
	fdrawline(x1, y1, x1, y2);
	
	/* cpack(0x0); */
	/* fdrawbox(x1-asp1, y1-asp1, x2+asp1, y2+asp1); */
}

void uiEmbossF(uiCol *bc, float asp, float x1, float y1, float x2, float y2, int sel)
{
	float asp1, asp2;
	float vec[2];
	
	asp1= asp;
	asp2= asp1+asp;

	x1+= asp1;
	x2-= asp1;
	y1+= asp1;
	y2-= asp1;

	/* below */
	if(sel) cpack(bc->white);
	else cpack(bc->dark);
	fdrawline(x1, y1, x2, y1);

	/* right */
	fdrawline(x2, y1, x2, y2);
	
	/* top */
	if(sel) cpack(bc->dark);
	else cpack(bc->white);
	fdrawline(x1, y2, x2, y2);

	/* left */
	fdrawline(x1, y1, x1, y2);
	
	cpack(0x0);
	fdrawbox(x1-asp1, y1-asp1, x2+asp1, y2+asp1);
}

/* minimal */
void uiEmbossM(uiCol *bc, float asp, float x1, float y1, float x2, float y2, int sel)
{
	float asp1, asp2;
	float vec[2];
	
	if(bc==NULL) bc= &UIcol[0];
	
	asp1= asp;
	asp2= asp1+asp;
	x1+= 1.0;
	y1+= 1.0;
	x2-= 1.0+asp;
	y2-= 1.0+asp;
	
	if(sel) {
		cpack(bc->light);
		
		/* below */
		fdrawline(x1, y1, x2, y1);

		/* right */
		fdrawline(x2, y1, x2, y2);
	}
	else {
		cpack(bc->white);

		/* top */
		fdrawline(x1, y2, x2, y2);
	
		/* left */
		fdrawline(x1, y1, x1, y2);
	}
}


void uiEmbossN(uiCol *bc, float asp, float x1, float y1, float x2, float y2, int sel)
{
	/* nothing! */	
}

void uiEmbossSlider(uiBut *but, float fac)
{
	float h;

	h= (but->y2-but->y1);

	cpack(but->col->dark);
	glRectf(but->x1, but->y1, but->x2, but->y2);
	cpack(0x0);
	ui_draw_outlineX(but->x1+1, but->y1+1, but->x2-1, but->y2-1, but->aspect);

	/* het blokje */
	if(but->flag & UI_SELECT) cpack(but->col->light);
	else cpack(but->col->grey);
	glRects(but->x1+fac, but->y1+1, but->x1+fac+h, but->y2-1);

	cpack(but->col->white);	
	fdrawline(but->x1+fac, but->y2-1, but->x1+fac+h, but->y2-1);
	fdrawline(but->x1+fac, but->y1+1, but->x1+fac, but->y2-1);

	cpack(0x0);
	fdrawline(but->x1+fac, but->y1+1, but->x1+fac+h, but->y1+1);
	fdrawline(but->x1+fac+h, but->y1+1, but->x1+fac+h, but->y2-1);
}

void uiDrawBut_BUT(uiBut *but)
{
	float x;
	short pos, sel, t, h;
	char ch;
	
	sel= but->flag & UI_SELECT;
	
	/* paper */
	if(sel) {
		if(but->flag & UI_ACTIVE) cpack(but->col->dark);
		else cpack(but->col->grey);
	}
	else {
		if(but->flag & UI_ACTIVE) cpack(but->col->hilite);
		else cpack(but->col->medium);
	}
	
	if( but->flag & UI_NO_BACK);
	else glRectf(but->x1+1, but->y1+1, but->x2-1, but->y2-1);

	but->embossfunc(but->col, but->aspect, but->x1, but->y1, but->x2, but->y2, sel);
	
	if( but->flag & UI_HAS_ICON ) {
		uiDrawIcon(but);
	}
	else if(but->drawstr[0]!=0) {
		if(sel) cpack(but->col->pen_sel);
		else cpack(but->col->pen);

		if(but->flag & UI_TEXT_LEFT) x= but->x1+4.0;
		else x= (but->x1+but->x2-but->strwidth+1)/2.0;
		
		glRasterPos2f( x, (but->y1+but->y2- 9.0)/2.0);
		
		fmprstr(but->drawstr+but->ofs);
	}
}

void uiDrawBut_TEX(uiBut *but)
{
	float x;
	short pos, sel, t, h;
	char ch;
	
	sel= but->flag & UI_SELECT;
	
	/* paper */
	if(but->embossfunc == uiEmbossF) {
		cpack(but->col->grey);
		sel= 1;		/* for special draw */
	}
	else {
		if(sel) {
			if(but->flag & UI_ACTIVE) cpack(but->col->dark);
			else cpack(but->col->grey);
		}
		else {
			if(but->flag & UI_ACTIVE) cpack(but->col->hilite);
			else cpack(but->col->medium);
		}
	}
	glRectf(but->x1+1, but->y1+1, but->x2-1, but->y2-1);

	but->embossfunc(but->col, but->aspect, but->x1, but->y1, but->x2, but->y2, sel);
	
	sel= but->flag & UI_SELECT;

	/* draw cursor */
	if(but->pos != -1) {
		
		pos= but->pos+strlen(but->str);
		if(pos >= but->ofs) {
			ch= but->drawstr[pos];
			but->drawstr[pos]= 0;
			t= but->aspect*fmgetstrwidth(but->font, but->drawstr+but->ofs) + 3;
			h=( but->y1 + but->y2 -8)/2;

			but->drawstr[pos]= ch;
			cpack(0xFF);
	
			glRects(but->x1+t, but->y1+2, but->x1+t+3, but->y2-2);
		}	
	}
	if(but->drawstr[0]!=0) {
		if(sel) cpack(but->col->pen_sel);
		else cpack(but->col->pen);

		if(but->flag & UI_TEXT_LEFT) x= but->x1+4.0;
		else x= (but->x1+but->x2-but->strwidth+1)/2.0;
		
		glRasterPos2f( x, (but->y1+but->y2- 9.0)/2.0);
		
		fmprstr(but->drawstr+but->ofs);
	}
}

void uiDrawBut_BLOCK(uiBut *but)
{
	float x;
	short pos, sel, t, h;
	char ch;
	
	sel= but->flag & UI_SELECT;
	
	cpack(but->paper);
	fdrawbox(but->x1, but->y1, but->x2, but->y2);
	
	if(but->flag & UI_ACTIVE) {
		uiEmbossW(but->col, but->aspect, but->x1, but->y1, but->x2, but->y2, sel);
	}
	
	if( but->flag & UI_HAS_ICON ) {
		uiDrawIcon(but);
	}
	else {
	
		if(but->drawstr[0]!=0) {
			/* no pen_sel for readability */
			cpack(but->col->pen);
	
			if(but->flag & UI_TEXT_LEFT) x= but->x1+4.0;
			else x= (but->x1+but->x2-but->strwidth+1)/2.0;
			
			glRasterPos2f( x, (but->y1+but->y2- 9.0)/2.0);
			
			fmprstr(but->drawstr+but->ofs);
		}
	}
}

void uiDrawBut_BUTM(uiBut *but)
{
	float x;
	short sel, len;
	char ch, *cpoin;
	
	sel= but->flag & UI_SELECT;
	
	cpack(but->col->medium);
	glRectf(but->x1, but->y1, but->x2, but->y2);
	
	if(but->flag & UI_ACTIVE) {
		cpack(BUTM_ACTIVE);
		glRectf(but->x1, but->y1, but->x2, but->y2);
	}
	
	if( but->flag & UI_HAS_ICON ) {
		uiDrawIcon(but);
	}
	else {
	
		if(but->drawstr[0]!=0) {
			
			cpoin= strchr(but->drawstr, '|');
			if(cpoin) *cpoin= 0;
			
			if(but->flag & UI_ACTIVE) cpack(but->col->pen_sel);
			else cpack(but->col->pen);
	
			x= but->x1+4.0;
			
			glRasterPos2f( x, (but->y1+but->y2- 9.0)/2.0);
			fmprstr(but->drawstr);
			
			if(cpoin) {
				len= fmstrwidth(cpoin+1);
				glRasterPos2f( but->x2 - len*but->aspect-3, (but->y1+but->y2- 9.0)/2.0);
				fmprstr(cpoin+1);
				*cpoin= '|';
			}
		}
	}
}

void uiDrawBut_LABEL(uiBut *but)
{
	float x;
	int sel;
	
	sel= but->min!=0.0;

	if(sel) cpack(but->col->pen_sel);
	else cpack(but->col->pen);

	if( but->flag & UI_HAS_ICON ) {
		uiDrawIcon(but);
	}
	else if(but->drawstr[0]!=0) {
		
		if(but->flag & UI_TEXT_LEFT) x= but->x1+4.0;
		else x= (but->x1+but->x2-but->strwidth+1)/2.0;
		
		glRasterPos2f( x, (but->y1+but->y2- 9.0)/2.0);
		
		fmprstr(but->drawstr+but->ofs);
	}
}

void uiDrawBut_SEPR(uiBut *but)
{
	float y= (but->y1+but->y2)/2.0;
	
	cpack(0x0);
	fdrawline(but->x1, y+but->aspect, but->x2, y+but->aspect);
	cpack(0xFFFFFF);
	fdrawline(but->x1, y, but->x2, y);
}

void uiDrawBut_LINK(uiBut *but)
{
	uiDrawIcon(but);
}


void uiDrawBut(uiBut *but)
{
	double value;
	float fac, x1, y1, x2, y2, *fp;
	uint tempcol;
	int a;
	char colr, colg, colb;
	
	if(but==0) return;

	if(UIfrontbuf) {
		glDrawBuffer(GL_FRONT);
		if(but->win==curarea->headwin) curarea->head_swap= WIN_FRONT_OK;
		else curarea->win_swap= WIN_FRONT_OK;
	}
	
	fmsetfont(but->font);

	switch (but->type) {

	case BUT: 
	case ROW: 
	case TOG: 
	case TOGR: 
	case TOGN:
	case ICONTOG: 
	case NUM:
	case IDPOIN:
		uiDrawBut_BUT(but);
		break;
	
	case TEX:
		uiDrawBut_TEX(but);
		break;

	case BLOCK:
		uiDrawBut_BLOCK(but);
		break;

	case BUTM:
		uiDrawBut_BUTM(but);
		break;
		
	case ICONROW:
		uiDrawBut_BUT(but);
		
		/* teken pijltjes, icon is standaard RGB */
		a= (but->y1+but->y2)/2;
		
		cpack(0);
		sdrawline(but->x1-1, a-2, but->x1-1, a+2);
		sdrawline(but->x1-2, a-1, but->x1-2, a+1);
		sdrawline(but->x1-3, a, but->x1-3, a);
		cpack(0xFFFFFF);
		sdrawline(but->x1-3, a-1, but->x1-1, a-3);
		
		cpack(0);
		sdrawline(but->x2+1, a-2, but->x2+1, a+2);
		sdrawline(but->x2+2, a-1, but->x2+2, a+1);
		sdrawline(but->x2+3, a, but->x2+3, a);
		cpack(0xFFFFFF);
		sdrawline(but->x2+3, a-1, but->x2+1, a-3);

		break;
		
	case MENU:
	
		uiDrawBut_BUT(but);

		/* als er ruimte is: teken symbooltje */
		if(but->strwidth+10 < but->x2-but->x1) {
			int h;
			
			h= but->y2- but->y1;
			x1= but->x2-0.66*h; x2= x1+.33*h;
			y1= but->y1+.42*h; y2= y1+.16*h;
		
			cpack(0x0);
			glRecti(x1,  y1,  x2,  y2);
			cpack(0xffffff);
			glRecti(x1-1,  y1+1,  x2-1,  y2+1);
		}
		break;
		
	case NUMSLI:
	case HSVSLI:
	
		uiDrawBut_BUT(but);
		
		/* de slider */

		x1= but->x1; x2= but->x2;
		y1= but->y1; y2= but->y2;
		
		but->x1= (but->x1+but->x2)/2;
		but->x2-= 9;
		but->y1= -2+(but->y1+but->y2)/2;
		but->y2= but->y1+6;
		
		value= uiGetButVal(but);
		fac= (value-but->min)*(but->x2-but->x1-but->y2+but->y1)/(but->max - but->min);
		uiEmbossSlider(but, fac);
		
		but->x1= x1; but->x2= x2;
		but->y1= y1; but->y2= y2;
		
		break;
		
	case TOG3:
	
		tempcol= but->col->pen_sel;
		if(but->flag & SELECT) {
			int ok= 0;
			
			if( but->pointype==CHA ) {
				if( BTST( *(but->poin+2), but->bitnr )) ok= 1;
			}
			else if( but->pointype ==SHO ) {
				short *sp= (short *)but->poin;
				if( BTST( sp[1], but->bitnr )) ok= 1;
			}
			
			if(ok) but->col->pen_sel= 0xFFFF;
		}
		uiDrawBut_BUT(but);

		but->col->pen_sel= tempcol;

		break;
	case LABEL:
		
		uiDrawBut_LABEL(but);

		break;
	case SLI:

		break;
	case SCROLL:
		break;
		
	case SEPR:
		uiDrawBut_SEPR(but);
		break;
		
	case COL:
	
		uiDrawBut_BUT(but);
		
		if( but->pointype==FLO ) {
			fp= (float *)but->poin;
			colr= ffloor(255.0*fp[0]+0.5);
			colg= ffloor(255.0*fp[1]+0.5);
			colb= ffloor(255.0*fp[2]+0.5);
		}
		else {
			char *cp= (char *)but->poin;
			colr= cp[0];
			colg= cp[1];
			colb= cp[2];
		}
		glColor3ub(colr,  colg,  colb);
		glRects(but->x1+2,  but->y1+2,  but->x2-2,  but->y2-2);
		break;
	case LINK:
		uiDrawBut_LINK(but);
		
		break;
	case INLINK:
		uiDrawBut_LINK(but);
		
		break;
	}
	
	/* windows has troubles with this sometimes (numsli), before switching
	 * to backbuffer drawing,  better to finish drawing
	 */
	glFinish();
	
	if(UIfrontbuf) glDrawBuffer(GL_BACK);
}

void uiDrawMenuBox(uint paper, float minx, float miny, float maxx, float maxy)
{

	cpack(paper);
	glRectf(minx, miny, maxx, maxy);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glColor4ub(0, 0, 0, 100);
	fdrawline(minx+4, miny-1, maxx+1, miny-1);
	fdrawline(maxx+1, miny-1, maxx+1, maxy-4);

	glColor4ub(0, 0, 0, 75);
	fdrawline(minx+4, miny-2, maxx+2, miny-2);
	fdrawline(maxx+2, miny-2, maxx+2, maxy-4);

	glColor4ub(0, 0, 0, 50);
	fdrawline(minx+4, miny-3, maxx+3, miny-3);
	fdrawline(maxx+3, miny-3, maxx+3, maxy-4);

	glDisable(GL_BLEND);
	
	/* below */
	cpack(0x0);
	fdrawline(minx, miny, maxx, miny);

	/* right */
	fdrawline(maxx, miny, maxx, maxy);
	
	/* top */
	cpack(0xffffff);
	fdrawline(minx, maxy, maxx, maxy);

	/* left */
	fdrawline(minx, miny, minx, maxy);
}

void ui_draw_linkline(uiCol *col, uiLinkLine *line)
{
	float vec1[2], vec2[2];

	if(line->from==NULL || line->to==NULL) return;
	
	vec1[0]= (line->from->x1+line->from->x2)/2.0;
	vec1[1]= (line->from->y1+line->from->y2)/2.0;
	vec2[0]= (line->to->x1+line->to->x2)/2.0;
	vec2[1]= (line->to->y1+line->to->y2)/2.0;
	
	
	if(line->flag & UI_SELECT) cpack(col->light);
	else cpack(0x0);
	fdrawline(vec1[0], vec1[1], vec2[0], vec2[1]);
}

void ui_draw_links(uiBlock *block)
{
	uiBut *but;
	uiLinkLine *line;
	
	if(UIfrontbuf) glDrawBuffer(GL_FRONT);

	but= block->buttons.first;
	while(but) {
		if(but->type==LINK && but->link) {
			line= but->link->lines.first;
			while(line) {
				ui_draw_linkline(but->col, line);
				line= line->next;
			}
		}
		but= but->next;
	}	

	if(UIfrontbuf) glDrawBuffer(GL_BACK);
}



/* ******************* block calc ************************* */

void ui_boundsblock(uiBlock *block, int addval)
{
	uiBut *bt;
	int min[2], max[2];
	
	block->minx= block->miny= 10000;
	block->maxx= block->maxy= -10000;
	
	bt= block->buttons.first;
	while(bt) {
		if(bt->x1 < block->minx) block->minx= bt->x1;
		if(bt->y1 < block->miny) block->miny= bt->y1;

		if(bt->x2 > block->maxx) block->maxx= bt->x2;
		if(bt->y2 > block->maxy) block->maxy= bt->y2;
		
		bt= bt->next;
	}
	
	block->minx -= addval;
	block->miny -= addval;
	block->maxx += addval;
	block->maxy += addval;
}

void ui_positionblock(uiBlock *block, uiBut *but)
{
	/* position block relative to but */
	uiBut *bt;
	int min[2], max[2];
	int xsize, ysize, xof=0, yof=0;
	
	block->minx= block->miny= 10000;
	block->maxx= block->maxy= -10000;
	
	bt= block->buttons.first;
	while(bt) {
		if(bt->x1 < block->minx) block->minx= bt->x1;
		if(bt->y1 < block->miny) block->miny= bt->y1;

		if(bt->x2 > block->maxx) block->maxx= bt->x2;
		if(bt->y2 > block->maxy) block->maxy= bt->y2;
		
		bt= bt->next;
	}
	
	block->minx-= 2.0; block->miny-= 2.0;
	block->maxx+= 2.0; block->maxy+= 2.0;
	
	xsize= block->maxx - block->minx;
	ysize= block->maxy - block->miny;
	
	if(but) {
		rctf butrct, sizex, sizey;
		short left=0, right=0, top=0, down=0;
		short dir1, dir2;
		
		butrct.xmin= but->x1; butrct.xmax= but->x2;
		butrct.ymin= but->y1; butrct.ymax= but->y2;
		ui_graphics_to_window(&butrct.xmin, &butrct.ymin);
		ui_graphics_to_window(&butrct.xmax, &butrct.ymax);
/* PRINT4(f, f, f, f, butrct.xmin, butrct.ymin, butrct.xmax, butrct.ymax); */

		if( butrct.xmin-xsize > 0.0) left= 1;
		if( butrct.xmax+xsize < G.curscreen->sizex) right= 1;
		if( butrct.ymin-ysize > 0.0) down= 1;
		if( butrct.ymax+ysize < G.curscreen->sizey) top= 1;
/* PRINT4(d, d, d, d, left, right, top, down); */
		
		dir1= block->direction;
		if(dir1==UI_LEFT || dir1==UI_RIGHT) dir2= UI_DOWN;
		if(dir1==UI_TOP || dir1==UI_DOWN) dir2= UI_LEFT;
		
		if(dir1==UI_LEFT && left==0) dir1= UI_RIGHT;
		if(dir1==UI_RIGHT && right==0) dir1= UI_LEFT;
			/* this is aligning, not append! */
		if(dir2==UI_LEFT && right==0) dir2= UI_RIGHT;
		if(dir2==UI_RIGHT && left==0) dir2= UI_LEFT;
		
		if(dir1==UI_TOP && top==0) dir1= UI_DOWN;
		if(dir1==UI_DOWN && down==0) dir1= UI_TOP;
		if(dir2==UI_TOP && top==0) dir2= UI_DOWN;
		if(dir2==UI_DOWN && down==0) dir2= UI_TOP;

		if(dir1==UI_LEFT) {
			xof= but->x1 - block->maxx;
			if(dir2==UI_TOP) yof= but->y1 - block->miny;
			else yof= but->y2 - block->maxy;
		}
		else if(dir1==UI_RIGHT) {
			xof= but->x2 - block->minx;
			if(dir2==UI_TOP) yof= but->y1 - block->miny;
			else yof= but->y2 - block->maxy;
		}
		else if(dir1==UI_TOP) {
			yof= but->y2 - block->miny+1;
			if(dir2==UI_RIGHT) xof= but->x2 - block->maxx;
			else xof= but->x1 - block->minx;
		}
		else if(dir1==UI_DOWN) {
			yof= but->y1 - block->maxy-1;
			if(dir2==UI_RIGHT) xof= but->x2 - block->maxx;
			else xof= but->x1 - block->minx;
		}
		
	}
	
	/* apply */
	bt= block->buttons.first;
	while(bt) {
		bt->x1 += xof;
		bt->x2 += xof;
		bt->y1 += yof;
		bt->y2 += yof;
		
		ui_graphics_to_window(&bt->x1, &bt->y1);
		ui_graphics_to_window(&bt->x2, &bt->y2);
		
		bt->aspect= 1.0;
		
		bt= bt->next;
	}
	
	block->minx += xof;
	block->miny += yof;
	block->maxx += xof;
	block->maxy += yof;
	
	ui_graphics_to_window(&block->minx, &block->miny);
	ui_graphics_to_window(&block->maxx, &block->maxy);
}


void ui_autofill(uiBlock *block)
{
	uiBut *but;
	float *maxw, *maxh, fac, startx, starty, height;
	float totmaxh;
	int rows=0, cols=0, i, lasti;
	
	
	/* first count rows */
	but= block->buttons.last;
	rows= but->x1+1;

	/* calculate max width / height for each row */
	maxw= callocN(sizeof(float)*rows, "maxw");
	maxh= callocN(sizeof(float)*rows, "maxh");
	but= block->buttons.first;
	while(but) {
		i= but->x1;
		if( maxh[i] < but->y2) maxh[i]= but->y2;
		maxw[i] += but->x2;
		but= but->next;
	}
	
	totmaxh= 0.0;
	for(i=0; i<rows; i++) totmaxh+= maxh[i];
	
	/* apply widths/heights */
	starty= block->maxy;
	but= block->buttons.first;
	lasti= -1;
	while(but) {
		
		i= but->x1;

		if(i!=lasti) {
			startx= block->minx;
			height= (maxh[i]*(block->maxy-block->miny))/totmaxh;
			starty-= height;
			lasti= i;
		}
		
		but->y1= starty+but->aspect;
		but->y2= but->y1+height-but->aspect;
		
		but->x2= (but->x2*(block->maxx-block->minx))/maxw[i];
		but->x1= startx+but->aspect;
		
		startx+= but->x2;
		but->x2+= but->x1-but->aspect;
		
		uiCheckBut(but);
		
		but= but->next;
	}
	
	freeN(maxw); freeN(maxh);	
	block->autofill= 0;
}

void uiDrawBlock(uiBlock *block)
{
	uiBut *but;

	if(block->autofill) ui_autofill(block);
	if(block->minx==0.0 && block->maxx==0.0) ui_boundsblock(block, 0);

	if(block->flag & UI_BLOCK_LOOP) {
		uiDrawMenuBox(UIcol[block->col].hilite, block->minx, block->miny, block->maxx, block->maxy);
	}
	
	if(block->flag & UI_BLOCK_TEST_ACTIVE) {
		uiEvent event;
		
		event.val= 1;
		event.event= MOUSEY;
		uiGetMouse(event.mval);
		uiDoBlock(block, &event);
		block->flag &= ~UI_BLOCK_TEST_ACTIVE;
	}
	
	but= block->buttons.first;
	while(but) {
		uiDrawBut(but);
		but= but->next;
	}

	ui_draw_links(block);
}

/* ************* MENUBUTS *********** */


/* return items */
int decompose_menu_string(char *str, char **title, char **cpoin, int *retval)
{
	int items= 0;
	
	cpoin[0]= str;
	retval[0]= 1;
	
	while( *str) {
		switch( *str ) {
			case '%':
				if(str[1]=='x') {
					retval[items]= atoi(str+2);
					*str= 0;
				}
				else if(str[1]=='t') {
					*title= cpoin[0];
					*str= 0;
					items--;
				}
				break;
			case '|':
				if(str[1]) {
					items++;					
					cpoin[items]= str+1;
					retval[items]= items+1;
				}
				*str= 0;
				break;
		}
		str++;
	}
	
	return items+1;
}

void uiGetNameMenu(char *name, char *menu, int value)
{
	int a, items, retvals[100];
	char *str, *title=0, *cpoin[100];
	
	str= mallocN(strlen(menu)+1, "setname menu");
	strcpy(str, menu);
	
	items= decompose_menu_string(str, &title, cpoin, retvals);
	
	for(a=0; a<items; a++) {
		if(retvals[a] == value) strcpy(name, cpoin[a]);
	}
	freeN(str);
}

void uiSetNameMenu(uiBut *but, int value)
{
	uiGetNameMenu(but->drawstr, but->str, value);
}


int uiDoBut_MENU(uiBut *but)
{
	uiBlock *block;
	ListBase listb={NULL, NULL};
	double fvalue;
	int width, height, a, retvals[100], items, xmax, ymax, startx, starty, endx, endy;
	int columns=1, rows=0, x1, y1, boxh, event;
	short val, mval[2], mousemove[2];
	char *instr, *str, *title=0, *cpoin[100];

	but->flag |= UI_SELECT;
	uiDrawBut(but);

	block= uiNewBlock(&listb, "menu", UI_EMBOSSW, UI_HELV, 0x808080, G.curscreen->mainwin);
	block->flag= UI_BLOCK_LOOP|UI_BLOCK_REDRAW|UI_BLOCK_NUMSELECT;

	block->aspect= 1.0;
	uiSetCurFont(block, block->font);

	instr= but->str;
	
	/* kopie string maken */
	str= mallocN(strlen(instr)+1, "pupmenu");
	strcpy(str, instr);
	
	/* eerst string uitelkaar pulken, tellen hoeveel elementen, return values */
	items= decompose_menu_string(str, &title, cpoin, retvals);

	/* collumns and row calculation */
	columns= (items+20)/20;
	if (columns<1) columns= 1;
	
	rows= (int) items/columns;
	if (rows<1) rows= 1;
	
	while (rows*columns<items) rows++;
		
	/* size and location */
	if(title) width= 2*strlen(title)+fmgetstrwidth(block->curfont, title);
	else width= 0;
	for(a=0; a<items; a++) {
		xmax= fmgetstrwidth(block->curfont, cpoin[a]);
		if(xmax>width) width= xmax;
	}

	width+= 10;
	if (width<50) width=50;

	boxh= TBOXH;
	
	height= rows*boxh;
	if (title) height+= boxh;
	
	xmax = G.curscreen->sizex;
	ymax = G.curscreen->sizey;

	getmouseco_sc(mval);
	
	/* find active item */
	fvalue= uiGetButVal(but);
	for(a=0; a<items; a++) {
		if( retvals[a]== (int)fvalue ) break;
	}
	/* no active item? */
	if(a==items) {
		if(title) a= -1;
		else a= 0;
	}

	if(a>0) startx = mval[0]-width/2 - ((int)(a)/rows)*width;
	else startx= mval[0]-width/2;
	starty = mval[1]-height + boxh/2 + ((a)%rows)*boxh;

	if (title) starty+= boxh;
	
	mousemove[0]= mousemove[1]= 0;
	
	if(startx<10) {
		mousemove[0]= 10-startx;
		startx= 10;
	}
	if(starty<10) {
		mousemove[1]= 10-starty;
		starty= 10;
	}
	
	endx= startx+width*columns;
	endy= starty+height;
	
	if(endx>xmax) {
		mousemove[0]= xmax-endx-10;
		endx= xmax-10;
		startx= endx-width*columns;
	}
	if(endy>ymax) {
		mousemove[1]= ymax-endy-10;
		endy= ymax-10;
		starty= endy-height;
	}

	warp_pointer(mval[0]+mousemove[0], mval[1]+mousemove[1]);

	mousemove[0]= mval[0];
	mousemove[1]= mval[1];

	/* here we go! */

	if(title) {
		uiBut *bt;
		uiSetCurFont(block, block->font+1);
		bt= uiDefBut(block, LABEL, 0, title, startx, starty+rows*boxh, width, boxh, NULL, 0.0, 0.0, 0, 0, "");
		uiSetCurFont(block, block->font);
		bt->flag= UI_TEXT_LEFT;
	}

	for(a=0; a<items; a++) {

		x1= startx + width*((int)a/rows);
		y1= starty - boxh*(a%rows) + (rows-1)*boxh; 

		uiDefBut(block, BUTM|but->pointype, but->retval, cpoin[a], x1, y1, width-(rows>1), boxh-1, but->poin, (float)retvals[a], 0.0, 0, 0, "");
	}
	
	ui_boundsblock(block, 3);

	event= uiDoBlocks(&listb, 0);
	
	/* ready, restore stuff */
	UIfrontbuf= 1;	
	
	freeN(str);
	if((event & UI_RETURN_OUT)==0) warp_pointer(mousemove[0], mousemove[1]);
	
	but->flag &= ~UI_SELECT;
	uiCheckBut(but);
	uiDrawBut(but);
	
	return event;	
}

/* ************* EVENTS ************* */

void uiGetMouse(short *adr)
{
	int x, y;
	float xwin, ywin;
	int automove;
	
	getmouseco_sc(adr);
	if(winget() == G.curscreen->mainwin) return;
	
	mygetsuborigin(&x, &y);

	adr[0]-= x;
	adr[1]-= y;

	xwin= adr[0];
	ywin= adr[1];

	ui_window_to_graphics(&xwin, &ywin);

	adr[0]= (short)(xwin+0.5);
	adr[1]= (short)(ywin+0.5);
}

int uiDoBut_BUT(uiBut *but)
{
	short a, push;
	short mval[2];
	
	but->flag |= UI_SELECT;
	uiDrawBut(but);
	push= 1;
	
	if(but->func==NULL && but->butfunc==NULL) {
		while (get_mbut() & L_MOUSE) {
			uiGetMouse(mval);
			a= 0;
			if(mval[0]>but->x1)
				if(mval[0]<but->x2)
					if(mval[1]>=but->y1-1)
						if(mval[1]<=but->y2+1) a=1;

			if(a!=push) {
				push= a;
				
				if(push) but->flag |= UI_SELECT;
				else but->flag &= ~UI_SELECT;
				
				uiDrawBut(but);
			
			}
			usleep(1);
		}
	}
	if(push) {
		if(but->func) {
			
			if(but->poin) {
				but->func( (int)uiGetButVal(but) );
			}
			else if(but->a1) but->func((int)but->a2);
			else but->func();
			
			while (get_mbut() & L_MOUSE) usleep(1);
		}
		if(but->butfunc) but->butfunc(but);
	}
	
	but->flag &= ~UI_SELECT;
	uiDrawBut(but);

	if(push==0) return 0;
	else return but->retval;
}

int uiDoBut_TOG(uiBlock *block, uiBut *but)
{
	uiBut *bt;
	double value;
	int w, lvalue, push;
	
	value= uiGetButVal(but);
	lvalue= (int)value;
	
	if(but->bit) {
		w= BTST(lvalue, but->bitnr);
		if(w) lvalue = BCLR(lvalue, but->bitnr);
		else lvalue = BSET(lvalue, but->bitnr);
		
		if(but->type==TOGR) {
			if( (get_qual() & LR_SHIFTKEY)==0 ) {
				lvalue= 1<<(but->bitnr);
	
				uiSetButVal(but, (double)lvalue);

				bt= block->buttons.first;
				while(bt) {
					if( bt!=but && bt->poin==but->poin ) {
						uiIsButSel(bt);
						uiDrawBut(bt);
					}
					bt= bt->next;
				}
			}
			else {
				if(lvalue==0) lvalue= 1<<(but->bitnr);
			}
		}
		uiSetButVal(but, (double)lvalue);

		uiDrawBut(but);
	}
	else {
		
		if(value==0.0) push= 1; 
		else push= 0;
		
		if(but->type==TOGN) push= !push;
		uiSetButVal(but, (double)push);
		uiDrawBut(but);
	}
	
	/* no while loop...this button is used for viewmove */
	
	if(but->func) {
		but->func(lvalue);
	}
	
	return but->retval;
}

int uiDoBut_ROW(uiBlock *block, uiBut *but)
{
	uiBut *bt;
	
	uiSetButVal(but, but->max);
	uiDrawBut(but);

	bt= block->buttons.first;
	while(bt) {
		if( bt!=but && bt->type==ROW ) {
			if(bt->min==but->min) {
				uiIsButSel(bt);
				uiDrawBut(bt);
			}
		}
		bt= bt->next;
	}
	return but->retval;
}

int uiDoBut_TEX(uiBut *but)
{
	ushort dev;
	short val, temp, x, mval[2], ch, len=0, qual, dodraw;
	char *str, backstr[UI_MAX_DRAW_STR];
	
	str= (char *)but->poin;
	
	but->flag |= UI_SELECT;

	uiGetMouse(mval);

	/* calculate cursor pos with current mousecoords */
	strncpy(backstr, but->drawstr, UI_MAX_DRAW_STR);
	but->pos= strlen(backstr)-but->ofs;
	while((but->aspect*fmgetstrwidth(but->font, backstr+but->ofs) + but->x1) > mval[0]) {
		if (but->pos <= 0) break;
		but->pos--;
		backstr[but->pos+but->ofs] = 0;
	}
	
	but->pos -= strlen(but->str);
	but->pos += but->ofs;
	if(but->pos<0) but->pos= 0;

	/* backup */
	strncpy(backstr, but->poin, UI_MAX_DRAW_STR);

	uiDrawBut(but);

	while (get_mbut() & L_MOUSE) usleep(1);
	len= strlen(str);
	but->min= 0.0;
	
	while(TRUE) {
	
		dodraw= 0;
		dev = extern_qread(&val);

		if(dev==INPUTCHANGE) break;
		else if(get_mbut() & L_MOUSE) break;
		else if(get_mbut() & R_MOUSE) break;
		else if(dev==ESCKEY) break;
		else if(dev==MOUSEX) val= 0;
		else if(dev==MOUSEY) val= 0;
		else if(dev==0) usleep(1);

		if(dev==KEYBD && val) {
			ch= val;

			switch(ch) {
			case 0:
				break;
			case '\b':	case 'b'+100: /* backspace */
				if(len!=0) {
					if(get_qual() & LR_SHIFTKEY) {
						str[0]= 0;
						but->pos= 0;
						len= 0;
						dodraw= 1;
					}
					else {
						temp= but->pos;
						if(temp>0) {
							for(x=temp; x<=strlen(str); x++)
								str[x-1]= str[x];
							but->pos--;
							str[--len]='\0';
							dodraw= 1;
						}
					}
				} 
				break;
			case '\n':
			case '\r':
				/* niet doen: rawkey afhandelen */
				break;
			default:
				if( ch>31 && ch<127) {
					if(len < but->max) {
						temp= but->pos;
						for(x= but->max; x>temp; x--)
							str[x]= str[x-1];
						str[temp]= ch;
						but->pos++; 
						len++;
						str[len]= '\0';
						dodraw= 1;
					}
				}
			}
		}
		else if(val) {
		
			if(dev==RIGHTARROWKEY) {
				if(G.qual & LR_SHIFTKEY) but->pos= strlen(str);
				else but->pos++;
				if(but->pos>strlen(str)) but->pos= strlen(str);
				dodraw= 1;
			}
			else if(dev==LEFTARROWKEY) {
				if(G.qual & LR_SHIFTKEY) but->pos= 0;
				else if(but->pos>0) but->pos--;
				dodraw= 1;
			}
			else if(dev==PADENTER || dev==RETKEY) {
				break;
			}
		}
		if(dodraw) {
			uiCheckBut(but);
			uiDrawBut(but);
		}
	}
	
	if(dev==ESCKEY) strcpy(but->poin, backstr);
	but->pos= -1;
	but->flag &= ~UI_SELECT;

	if(but->func) but->func(but->poin);

	uiCheckBut(but);
	uiDrawBut(but);
	
	if(dev!=ESCKEY) return but->retval;
	else return 0;
}


static int uiActAsTextBut(uiBut *but)
{
	double value;
	float min, max;
	int temp, retval, textleft;
	char str[UI_MAX_DRAW_STR], *point;
	
	
	value= uiGetButVal(but);
	if( but->pointype==FLO ) {
		sprintf(str, "%.3f", value);
	}
	else {
		sprintf(str, "%d", (int)value);
	}
	point= but->poin;
	but->poin= str;
	min= but->min;
	max= but->max;
	but->min= 0.0;
	but->max= 15.0;
	temp= but->type;
	but->type= TEX;
	textleft= but->flag & UI_TEXT_LEFT;
	but->flag |= UI_TEXT_LEFT;
	uiCheckBut(but);
	
	retval= uiDoBut_TEX(but);
	
	but->type= temp;
	but->poin= point;
	but->min= min;
	but->max= max;
	if(textleft==0) but->flag &= ~UI_TEXT_LEFT;

	if( but->pointype==FLO ) value= atof(str);
	else value= atoi(str);

	if(value<min) value= min;
	if(value>max) value= max;

	uiSetButVal(but, value);
	uiCheckBut(but);
	uiDrawBut(but);
	
	return retval;
}

int uiDoBut_NUM(uiBut *but)
{
	double value;
	float deler, fstart, f, tempf;
	int lvalue, temp, firsttime=1;
	short qual, sx, mval[2], pos=0;
	

	but->flag |= UI_SELECT;
	uiDrawBut(but);
	
	uiGetMouse(mval);
	value= uiGetButVal(but);
	
	sx= mval[0];
	fstart= (value - but->min)/(but->max-but->min);
	f= fstart;
	
	temp= (int)value;
	tempf= value;
	
	if(get_qual() & LR_SHIFTKEY) {	/* make it textbut */
		if( uiActAsTextBut(but) ) return but->retval;
		else return 0;
	}
	
	/* firsttime: this button can be approached with enter as well */
	while (get_mbut() & L_MOUSE) {
		qual= get_qual();
		
		deler= 500;
		if( but->pointype!=FLO ) {

			if( (but->max-but->min)<100 ) deler= 200.0;
			if( (but->max-but->min)<25 ) deler= 50.0;

		}
		if(qual & LR_SHIFTKEY) deler*= 10.0;
		if(qual & LR_ALTKEY) deler*= 20.0;

		uiGetMouse(mval);
		
		if(mval[0] != sx) {
		
			f+= ((float)(mval[0]-sx))/deler;
			if(f>1.0) f= 1.0;
			if(f<0.0) f= 0.0;
			sx= mval[0];
			tempf= ( but->min + f*(but->max-but->min));
			
			if( but->pointype!=FLO ) {
				
				temp= ffloor(tempf+.5);
				
				if(tempf==but->min || tempf==but->max);
				else if(qual & LR_CTRLKEY) temp= 10*(temp/10);
				
				if( temp>=but->min && temp<=but->max) {
				
					value= uiGetButVal(but);
					lvalue= (int)value;
					
					if(temp != lvalue ) {
						pos= 1;
						uiSetButVal(but, (double)temp);
						uiCheckBut(but);
						uiDrawBut(but);
						
						if(but->func) but->func();
						/* winset(winakt); */
					}
				}

			}
			else {
				temp= 0;
				if(qual & LR_CTRLKEY) {
					if(tempf==but->min || tempf==but->max);
					else if(but->max-but->min < 2.10) tempf= 0.1*ffloor(10*tempf);
					else if(but->max-but->min < 21.0) tempf= ffloor(tempf);
					else tempf= 10.0*ffloor(tempf/10.0);
				}

				if( tempf>=but->min && tempf<=but->max) {
					value= uiGetButVal(but);
					
					if(tempf != value ) {
						pos= 1;
						uiSetButVal(but, tempf);
						uiCheckBut(but);
						uiDrawBut(but);
					}
				}

			}
		}
		usleep(1);
	}
	
	if(pos==0) {  /* plus 1 or minus 1 */
		if( but->pointype!=FLO ) {

			if(sx<(but->x1+but->x2)/2) temp--;
			else temp++;
			
			if( temp>=but->min && temp<=but->max)
				uiSetButVal(but, (double)temp);

		}
		else {
		
			if(sx<(but->x1+but->x2)/2) tempf-= 0.01*but->a1;
				else tempf+= 0.01*but->a1;
				
			if( tempf>=but->min && tempf<=but->max)
				uiSetButVal(but, tempf);

		}
	}
	
	but->flag &= ~UI_SELECT;
	uiCheckBut(but);
	uiDrawBut(but);	
	
	return but->retval;
}

int uiDoBut_TOG3(uiBut *but)
{ 

	if( but->pointype==SHO ) {
		short *sp= (short *)but->poin;
		
		if( BTST(sp[1], but->bitnr)) {
			sp[1]= BCLR(sp[1], but->bitnr);
			sp[0]= BCLR(sp[0], but->bitnr);
		}
		else if( BTST(sp[0], but->bitnr)) {
			sp[1]= BSET(sp[1], but->bitnr);
		} else {
			sp[0]= BSET(sp[0], but->bitnr);
		}
	}
	else {
		if( BTST(*(but->poin+2), but->bitnr)) {
			*(but->poin+2)= BCLR(*(but->poin+2), but->bitnr);
			*(but->poin)= BCLR(*(but->poin), but->bitnr);
		}
		else if( BTST(*(but->poin), but->bitnr)) {
			*(but->poin+2)= BSET(*(but->poin+2), but->bitnr);
		} else {
			*(but->poin)= BSET(*(but->poin), but->bitnr);
		}
	}
	
	uiIsButSel(but);
	if(but->type==ICONTOG) uiCheckBut(but);
	uiDrawBut(but);
	
	return but->retval;
}

int uiDoBut_ICONROW(uiBut *but)
{
	ListBase listb= {NULL, NULL};
	uiBlock *block;
	int a, nr, x, y, event;
	char str[32];
	
	but->flag |= UI_SELECT;
	uiDrawBut(but);
	
	/* here we go! */
	block= uiNewBlock(&listb, "menu", UI_EMBOSSW, UI_HELV, 0x808080, G.curscreen->mainwin);
	block->flag= UI_BLOCK_LOOP|UI_BLOCK_REDRAW|UI_BLOCK_NUMSELECT;

	sscanf(but->str+4, "%d %d %d\n", &nr, &x, &y);

	for(a=(int)but->min; a<=(int)but->max; a++) {
		sprintf(str, "ICON %d %d %d", nr, (int)(x+a-but->min), y);
		uiDefBut(block, BUTM|but->pointype, but->retval, str, 0, 18*a, (but->x2-but->x1-4), 18, but->poin, (float)a, 0.0, 0, 0, "");
	}
	block->direction= UI_TOP;	
	ui_positionblock(block, but);

	event= uiDoBlocks(&listb, 0);
	
	/* ready, restore stuff */
	UIfrontbuf= 1;	

	but->flag &= ~UI_SELECT;
	uiCheckBut(but);
	uiDrawBut(but);	
	
	return but->retval;
}

int uiDoBut_IDPOIN(uiBut *but)
{
	ID **idpp, *id;
	void (*func)();
	char str[UI_MAX_DRAW_STR];
	
	idpp= (ID **)but->poin;
	id= *idpp;
	if(id) strcpy(str, id->name+2);
	else str[0]= 0;
	
	but->type= TEX;
	func= but->func;
	but->func= 0;
	but->poin= str;
	but->min= 0.0;
	but->max= 22.0;
	uiDoBut_TEX(but);
	but->poin= (char *)idpp;
	but->func= func;
	but->type= IDPOIN;
	
	but->func(str, idpp);

	uiDrawBut(but);
	
	return but->retval;
}

int uiDoBut_SLI(uiBut *but)
{
	float f, fstart, tempf, deler, value;
	int sx, h, temp, pos=0, lvalue, redraw;
	short mval[2], qual;

	value= uiGetButVal(but);
	uiGetMouse(mval);

	sx= mval[0];
	h= but->y2-but->y1;
	fstart= but->max-but->min;
	fstart= (value - but->min)/fstart;
	temp= 32767;

	if( but->type==NUMSLI) deler= ( (but->x2-but->x1)/2 - h);
	else if( but->type==HSVSLI) deler= ( (but->x2-but->x1)/2 - h);
	else deler= (but->x2-but->x1-h);
	
	while (get_mbut() & L_MOUSE) {
	
		qual= get_qual();
		uiGetMouse(mval);
		
		f= (float)(mval[0]-sx)/deler +fstart;
		
		if(qual & LR_CTRLKEY) {
			if(qual & LR_SHIFTKEY) f= ffloor(f*100.0)/100.0;
			else f= ffloor(f*10.0)/10.0;
		} 
		else if (qual & LR_SHIFTKEY) {
			f= (f-fstart)/10.0 + fstart;
		}
		
		CLAMP(f, 0.0, 1.0);
		tempf= but->min+f*(but->max-but->min);
		
		temp= ffloor(tempf+.5);
		
		value= uiGetButVal(but);
		lvalue= (int) value;
		
		if( but->pointype!=FLO )
			redraw= (temp != lvalue);
		else
			redraw= (tempf != value);

		if (redraw) {
			pos= 1;
			
			uiSetButVal(but, tempf);
			uiCheckBut(but);
			uiDrawBut(but);
			
			if(but->a1) {	/* colornummer */
				uiBut *bt= but->prev;
				while(bt) {
					if(bt->retval == but->a1) uiDrawBut(bt);
					bt= bt->prev;
				}
				bt= but->next;
				while(bt) {
					if(bt->retval == but->a1) uiDrawBut(bt);
					bt= bt->next;
				}
			}
			
			if(but->func) but->func(but);
		} 
		else usleep(1);
	}
	
	if(temp!=32767 && pos==0) {  /* plus 1 of min 1 */
		
		if( but->type==SLI) f= (float)(mval[0]-but->x1)/(but->x2-but->x1-h);
		else f= (float)(mval[0]- (but->x1+but->x2)/2)/( (but->x2-but->x1)/2 - h);
		
		f= but->min+f*(but->max-but->min);
		
		if( but->pointype!=FLO ) {

			if(f<temp) temp--;
			else temp++;
			if( temp>=but->min && temp<=but->max)
				uiSetButVal(but, (float)temp);
		
		} 
		else {

			if(f<tempf) tempf-=.01;
			else tempf+=.01;
			if( tempf>=but->min && tempf<=but->max)
				uiSetButVal(but, tempf);

		}
	}
	uiCheckBut(but);
	uiDrawBut(but);
	
	return but->retval;
}

int uiDoBut_NUMSLI(uiBut *but)
{
	float value;
	int retval= 1;
	short mval[2];
	
	/* eerste bepalen of het slider is of textbut */
	uiGetMouse(mval);
	
	if(mval[0]>= -6+(but->x1+but->x2)/2 ) {	/* slider */
		but->flag |= UI_SELECT;
		uiDrawBut(but);
		uiDoBut_SLI(but);
		but->flag &= ~UI_SELECT;
	}
	else {
		retval= uiActAsTextBut(but);
	}

	while(get_mbut() & L_MOUSE) usleep(1);
	
	uiDrawBut(but);	
	
	/* hsv patch */
	if(but->type==HSVSLI) {
	
		if(but->str[0]=='H') {
			uiDrawBut(but->next);
			uiDrawBut(but->next->next);
		} 
		else if(but->str[0]=='S') {
			uiDrawBut(but->next);
			uiDrawBut(but->prev);
		} 
		else if(but->str[0]=='V') {
			uiDrawBut(but->prev);
			uiDrawBut(but->prev->prev);
		}
	}
	
	return but->retval;
}

int uiDoBut_BLOCK(uiBut *but)
{
	uiBlock *block;
	
	but->flag |= UI_SELECT;
	uiDrawBut(but);	

	if(but->blockfunc) {
		block= but->blockfunc();

		ui_positionblock(block, but);
		block->flag |= UI_BLOCK_LOOP;
		block->win= G.curscreen->mainwin;
		
		/* postpone draw, this will cause a new window matrix, first finish all other buttons */
		block->flag |= UI_BLOCK_REDRAW;
	}
	
	but->flag &= ~UI_SELECT;

	return 0;
}

int uiDoBut_BUTM(uiBut *but)
{

	uiSetButVal(but, but->min);
	UIafterfunc= but->func;
	UIafterval= (int)but->a2;
	
	return but->retval;
}

uiBut *ui_get_valid_link_button(uiBlock *block, uiBut *but, short *mval)
{
	uiBut *bt;
	
	/* check boundbox */
	bt= block->buttons.first;
	while(bt) {
		if(bt!=but) {
			if( bt->x1 < mval[0] && bt->x2 >= mval[0] ) {
				if( bt->y1 < mval[1] && bt->y2 >= mval[1] ) {
					
					if(but->type==LINK && bt->type==INLINK) {
						if( but->link->tocode == (int)bt->min ) {
							return bt;
						}
					}
					else if(but->type==INLINK && bt->type==LINK) {
						if( bt->link->tocode == (int)but->min ) {
							return bt;
						}
					}
					break;
				}
			}
		}
		bt= bt->next;
	}
	return NULL;
}

int ui_is_a_link(uiBut *from, uiBut *to)
{
	uiLinkLine *line;
	uiLink *link;
	
	link= from->link;
	if(link) {
		line= link->lines.first;
		while(line) {
			if(line->from==from && line->to==to) return 1;
			line= line->next;
		}
	}
	return 0;
}

uiBut *ui_find_inlink(uiBlock *block, void *poin)
{
	uiBut *but;
	
	but= block->buttons.first;
	while(but) {
		if(but->type==INLINK) {
			if(but->poin == poin) return but;
		}
		but= but->next;
	}
	return NULL;
}

void ui_add_link_line(ListBase *listb, uiBut *but, uiBut *bt)
{
	uiLinkLine *line;
	
	line= callocN(sizeof(uiLinkLine), "linkline");
	addtail(listb, line);
	line->from= but;
	line->to= bt;
}


void uiComposeLinks(uiBlock *block)
{
	uiBut *but, *bt;
	uiLink *link;
	void ***ppoin;
	int a;
	
	but= block->buttons.first;
	while(but) {
		if(but->type==LINK) {
			link= but->link;
			
			/* for all pointers in the array */
			if(link) {
				if(link->ppoin) {
					ppoin= link->ppoin;
					for(a=0; a < *(link->totlink); a++) {
						bt= ui_find_inlink(block, (*ppoin)[a] );
						if(bt) {
							ui_add_link_line(&link->lines, but, bt);
						}
					}
				}
				else if(link->poin) {
					bt= ui_find_inlink(block, *(link->poin) );
					if(bt) {
						ui_add_link_line(&link->lines, but, bt);
					}
				}
			}
		}
		but= but->next;
	}
}

void ui_add_link(uiBut *from, uiBut *to)
{
	/* in 'from' we have to add a link to 'to' */
	uiLink *link;
	void **oldppoin;
	int a;
	
	if(ui_is_a_link(from, to)) {
		printf("already exists\n");
		return;
	}
	
	link= from->link;

	/* are there more pointers allowed? */
	if(link->ppoin) {
		oldppoin= *(link->ppoin);
		
		(*(link->totlink))++;
		*(link->ppoin)= callocN( *(link->totlink)*sizeof(void *), "new link");

		for(a=0; a< (*(link->totlink))-1; a++) {
			(*(link->ppoin))[a]= oldppoin[a];
		}
		(*(link->ppoin))[a]= to->poin;
		
		if(oldppoin) freeN(oldppoin);
	}
	else {
		*(link->poin)= to->poin;
	}
	
	/* temporal! these buttons can be everywhere... */
	addqueue(curarea->win, REDRAW, 1);
}

int uiDoBut_LINK(uiBlock *block, uiBut *but)
{
	/* 
	 * This button only visualizes, the dobutton mode
	 * can add a new link, but then the whole system
	 * should be redrawn/initialized. 
	 * 
	 */
	uiBut *bt=0, *bto=NULL;
	short sval[2], mval[2], mvalo[2], mvalb[2];

	/* because inverted lines for windows and beos are (still) not opengl */
	persp(0);

	getmouseco_areawin(mvalo);
	getmouseco_areawin(sval);
	mvalo[0]-= 100;
	
	while (get_mbut() & L_MOUSE) {
		getmouseco_areawin(mval);

		if(mval[0]!=mvalo[0] || mval[1]!=mvalo[1]) {
			
			mvalo[0]= mval[0];
			mvalo[1]= mval[1];
			
			/* clear completely, because of drawbuttons */
			glDrawBuffer(GL_FRONT);
			sdrawXORline4(-1, 0, 0, 0, 0);
			glDrawBuffer(GL_BACK);
			
			uiGetMouse(mvalb);	/* projected mouseco */
			bt= ui_get_valid_link_button(block, but, mvalb);
			if(bt) {
				persp(1);
				bt->flag |= UI_ACTIVE;
				uiDrawBut(bt);
				persp(0);
			}
			if(bto && bto!=bt) {
				persp(1);
				bto->flag &= ~UI_ACTIVE;
				uiDrawBut(bto);
				persp(0);
			}
			bto= bt;

			glDrawBuffer(GL_FRONT);
			sdrawXORline4(0, sval[0], sval[1], mval[0], mval[1]);
			glDrawBuffer(GL_BACK);
		}
		else usleep(2);		
	}
	
	persp(1);

	if(bt) {
		if(but->type==LINK) ui_add_link(but, bt);
		else ui_add_link(bt, but);
	}
	
	glDrawBuffer(GL_FRONT);
	sdrawXORline4(-1, 0, 0, 0, 0);
	glDrawBuffer(GL_BACK);

	return 0;
}


/* ************************************************ */

void uiSetButLock(int val, char *lockstr)
{
	UIlock |= val;
	if (val) UIlockstr= lockstr;
}

void uiClearButLock()
{
	UIlock= 0;
	UIlockstr= NULL;
}

/* ********************** NEXT/PREV for arrowkeys etc ************** */

uiBut *ui_but_prev(uiBut *but)
{
	while(but->prev) {
		but= but->prev;
		if(but->type!=LABEL && but->type!=SEPR) return but;
	}
	return NULL;
}

uiBut *ui_but_next(uiBut *but)
{
	while(but->next) {
		but= but->next;
		if(but->type!=LABEL && but->type!=SEPR) return but;
	}
	return NULL;
}

uiBut *ui_but_first(uiBlock *block)
{
	uiBut *but;
	
	but= block->buttons.first;
	while(but) {
		if(but->type!=LABEL && but->type!=SEPR) return but;
		but= but->next;
	}
	return NULL;
}

uiBut *ui_but_last(uiBlock *block)
{
	uiBut *but;
	
	but= block->buttons.last;
	while(but) {
		if(but->type!=LABEL && but->type!=SEPR) return but;
		but= but->prev;
	}
	return NULL;
}

/* *************************************************************** */


/* is called when LEFTMOUSE is pressed or released
 * return: butval or zero
 */
int uiDoButton(uiBlock *block, uiBut *but, uiEvent *uevent)
{
	int retval= 0;

	if(but->lock) {
		if (but->lockstr) {
			error(but->lockstr);
			return 0;
		}
	} 
	else {
		if( but->pointype ) {		/* er is pointer nodig */
			if(but->poin==0 ) {
				printf("DoButton pointer error: %s\n",but->str);
				return 0;
			}
		}
	}
	
	block->flag |= UI_BLOCK_BUSY;

	switch(but->type) {
	case BUT:
		if(uevent->val) retval= uiDoBut_BUT(but);		
		break;
		
	case TOG: 
	case TOGR: 
	case ICONTOG: 
	case TOGN:
		if(uevent->val) {
			retval= uiDoBut_TOG(block, but);
		}
		break;
		
	case ROW:
		if(uevent->val) retval= uiDoBut_ROW(block, but);
		break;

	case SCROLL:
		/* DrawBut(b, 1); */
		/* do_scrollbut(b); */
		/* DrawBut(b,0); */
		break;

	case NUM:
		if(uevent->val) retval= uiDoBut_NUM(but);
		break;
		
	case SLI:
	case NUMSLI:
	case HSVSLI:
		if(uevent->val) retval= uiDoBut_NUMSLI(but);
		break;
		
	case LABEL:	
		if(uevent->val) retval= but->retval;
		if(but->butfunc) but->butfunc(but);
		break;
		
	case TOG3:	
		if(uevent->val) retval= uiDoBut_TOG3(but);
		break;
		
	case TEX:
		if(uevent->val) retval= uiDoBut_TEX(but);
		break;
		
	case MENU:
		if(uevent->val) retval= uiDoBut_MENU(but);
		break;
		
	case ICONROW:
		if(uevent->val) retval= uiDoBut_ICONROW(but);		
		break;
		
	case IDPOIN:
		if(uevent->val) retval= uiDoBut_IDPOIN(but);	
		break;
	
	case BLOCK:
		if(uevent->val) retval= uiDoBut_BLOCK(but);
		break;

	case BUTM:
		retval= uiDoBut_BUTM(but);
		break;

	case LINK:
	case INLINK:
		retval= uiDoBut_LINK(block, but);
		break;
	}
	
	block->flag &= ~UI_BLOCK_BUSY;

	return retval;
}

void ui_delete_active_linkline(uiBlock *block)
{
	uiBut *but;
	uiLink *link;
	uiLinkLine *line, *nline;
	int a, b;
	
	but= block->buttons.first;
	while(but) {
		if(but->type==LINK && but->link) {
			line= but->link->lines.first;
			while(line) {
				
				nline= line->next;
				
				if(line->flag & UI_SELECT) {
					remlink(&but->link->lines, line);

					link= line->from->link;

					/* are there more pointers allowed? */
					if(link->ppoin) {
						
						if(*(link->totlink)==1) {
							*(link->totlink)= 0;
							freeN(*(link->ppoin));
							*(link->ppoin)= NULL;
						}
						else {
							b= 0;
							for(a=0; a< (*(link->totlink)); a++) {
								
								if( (*(link->ppoin))[a] != line->to->poin ) {
									(*(link->ppoin))[b]= (*(link->ppoin))[a];
									b++;
								}
							}	
							(*(link->totlink))--;
						}
					}
					else {
						*(link->poin)= NULL;
					}

					freeN(line);
				}
				line= nline;
			}
		}
		but= but->next;
	}
	
	/* temporal! these buttons can be everywhere... */
	allqueue(REDRAWBUTSGAME, 0);
}

void ui_do_active_linklines(uiBlock *block, short *mval)
{
	uiBut *but;
	uiLinkLine *line, *act=NULL;
	float mindist= 12.0, fac, v1[2], v2[2], v3[3];
	int foundone=0;
	
	if(mval) {
		v1[0]= mval[0];
		v1[1]= mval[1];
		
		/* find a line close to the mouse */
		but= block->buttons.first;
		while(but) {
			if(but->type==LINK && but->link) {
				line= but->link->lines.first;
				while(line) {
					v2[0]= line->from->x2;
					v2[1]= (line->from->y1+line->from->y2)/2.0;
					v3[0]= line->to->x1;
					v3[1]= (line->to->y1+line->to->y2)/2.0;
					
					fac= PdistVL2Dfl(v1, v2, v3);
					if(fac < mindist) {
						mindist= fac;
						act= line;
					}
					line= line->next;
				}
			}
			but= but->next;
		}
	}
	/* draw */
	glDrawBuffer(GL_FRONT);
	
	but= block->buttons.first;
	while(but) {
		if(but->type==LINK && but->link) {
			line= but->link->lines.first;
			while(line) {
				if(line==act) {
					if((line->flag & UI_SELECT)==0) {
						line->flag |= UI_SELECT;
						ui_draw_linkline(but->col, line);
					}
				}
				else if(line->flag & UI_SELECT) {
					line->flag &= ~UI_SELECT;
					ui_draw_linkline(but->col, line);
				}
				line= line->next;
			}
		}
		but= but->next;
	}

	glDrawBuffer(GL_BACK);
}

/* return: 
 * UI_NOTHING	pass event to other ui's
 * UI_CONT		don't pass event to other ui's
 * UI_RETURN	something happened, return, swallow event
 */
int uiDoBlock(uiBlock *block, uiEvent *uevent)
{
	uiBut *but, *bt;
	int butevent, event, retval=UI_NOTHING, count, act=0;
	int inside= 0, active=0;
	
	if(block->win != winget()) return UI_NOTHING;

	/* filter some unwanted events */
	if(uevent->event==LEFTSHIFTKEY || uevent->event==RIGHTSHIFTKEY) return UI_NOTHING;

	if(block->flag & UI_BLOCK_ENTER_OK) {
		if(uevent->event==RETKEY) return UI_RETURN_OK;
	}		

	Mat4CpyMat4(UIwinmat, block->winmat);
	uiGetMouse(uevent->mval);	/* transformed mouseco */

	/* check boundbox */
	if( block->minx <= uevent->mval[0] && block->maxx >= uevent->mval[0] ) {
		if( block->miny <= uevent->mval[1] && block->maxy >= uevent->mval[1] ) {
			inside= 1;
		}
	}

	switch(uevent->event) {
	case PAD8: case PAD2:
	case UPARROWKEY:
	case DOWNARROWKEY:
		if(inside || (block->flag & UI_BLOCK_LOOP)) {
			/* arrowkeys: only handle for block_loop blocks */
			event= 0;
			if(block->flag & UI_BLOCK_LOOP) {
				event= uevent->event;
				if(event==PAD8) event= UPARROWKEY;
				if(event==PAD2) event= DOWNARROWKEY;
			}
			else {
				if(uevent->event==PAD8) event= UPARROWKEY;
				if(uevent->event==PAD2) event= DOWNARROWKEY;
			}
			if(event && uevent->val) {
	
				but= block->buttons.first;
				while(but) {
					
					but->flag &= ~UI_MOUSE_OVER;
		
					if(but->flag & UI_ACTIVE) {
						but->flag &= ~UI_ACTIVE;
						uiDrawBut(but);
	
						bt= ui_but_prev(but);
						if(bt && event==UPARROWKEY) {
							bt->flag |= UI_ACTIVE;
							uiDrawBut(bt);
							break;
						}
						bt= ui_but_next(but);
						if(bt && event==DOWNARROWKEY) {
							bt->flag |= UI_ACTIVE;
							uiDrawBut(bt);
							break;
						}
					}
					but= but->next;
				}
	
				/* nothing done */
				if(but==NULL) {
				
					if(event==UPARROWKEY) but= ui_but_last(block);
					else but= ui_but_first(block);
					
					if(but) {
						but->flag |= UI_ACTIVE;
						uiDrawBut(but);
					}
				}
				retval= UI_CONT;
			}
		}
		break;
	
	case ONEKEY: act= 1;
	case TWOKEY: if(act==0) act= 2;
	case THREEKEY: if(act==0) act= 3;
	case FOURKEY: if(act==0) act= 4;
	case FIVEKEY: if(act==0) act= 5;
	case SIXKEY: if(act==0) act= 6;
	case SEVENKEY: if(act==0) act= 7;
	case EIGHTKEY: if(act==0) act= 8;
	case NINEKEY: if(act==0) act= 9;
	case ZEROKEY: if(act==0) act= 10;
	
		if( block->flag & UI_BLOCK_NUMSELECT ) {
			
			if(get_qual() & LR_ALTKEY) act+= 10;
			
			but= block->buttons.first;
			count= 0;
			while(but) {
				if( but->type!=LABEL && but->type!=SEPR) count++;
				if(count==act) {
					but->flag |= UI_ACTIVE;
					if(uevent->val==1) uiDrawBut(but);
					else {
						uevent->event= RETKEY;
						uevent->val= 1;			/* patch: to avoid UI_BLOCK_RET_1 type not working */
						addqueue(block->winq, RIGHTARROWKEY, 1);
					}
				}
				else if(but->flag & UI_ACTIVE) {
					but->flag &= ~UI_ACTIVE;
					uiDrawBut(but);
				}
				but= but->next;
			}
		}
		
		break;
		
	default:

		if (uevent->event!=KEYBD && uevent->event!=RETKEY) {	/* when previous command was arrow */
			but= block->buttons.first;
			while(but) {
			
				but->flag &= ~UI_MOUSE_OVER;
				
				/* check boundbox */
				if( but->x1 < uevent->mval[0] && but->x2 >= uevent->mval[0] ) {
					if( but->y1 < uevent->mval[1] && but->y2 >= uevent->mval[1] ) {
						but->flag |= UI_MOUSE_OVER;
						UIbuttip= but;
					}
				}
				/* hilite case 1 */
				if(but->flag & UI_MOUSE_OVER) {
					if( (but->flag & UI_ACTIVE)==0) {
						but->flag |= UI_ACTIVE;
						uiDrawBut(but);
					}
				}
				/* hilite case 2 */
				if(but->flag & UI_ACTIVE) {
					if( (but->flag & UI_MOUSE_OVER)==0) {
						but->flag &= ~UI_ACTIVE;
						uiDrawBut(but);
					}
					if(but->flag & UI_ACTIVE) active= 1;
				}
				
				but= but->next;
			}
			
			/* if there are no active buttons... otherwise clear lines */
			if(active) ui_do_active_linklines(block, 0);
			else ui_do_active_linklines(block, uevent->mval);
			
		}
	}

	/* middlemouse exception, not for regular blocks */
	if( (block->flag & UI_BLOCK_LOOP) && uevent->event==MIDDLEMOUSE) uevent->event= LEFTMOUSE;

	/* the final dobutton */
	but= block->buttons.first;
	while(but) {
		if(but->flag & UI_ACTIVE) {
			
			/* UI_BLOCK_RET_1: not return when val==0 */
			
			if(uevent->val || (block->flag & UI_BLOCK_RET_1)==0) {
				if ELEM3(uevent->event, LEFTMOUSE, PADENTER, RETKEY) {
					
					butevent= uiDoButton(block, but, uevent);
					if(butevent) addqueue(block->winq, UI_BUT_EVENT, butevent);

					/* i doubt about the next line! */
					/* if(but->func) winset(block->win); */
					if(but->func || butevent) retval= UI_RETURN_OK;
				}
			}
		}
		
		but= but->next;
	}

	/* the linkines... why not make buttons from it? Speed? Memory? */
	if(uevent->val && (uevent->event==LEFTMOUSE || uevent->event==DELKEY)) 
		ui_delete_active_linkline(block);

	if(block->flag & UI_BLOCK_LOOP) {

		if(inside==0 && uevent->val==1) {
			if ELEM3(uevent->event, LEFTMOUSE, MIDDLEMOUSE, RIGHTMOUSE)
				return UI_RETURN_OUT;
		}

		if(uevent->event==ESCKEY) return UI_RETURN_CANCEL;
		
		/* check outside */
		if(uevent->mval[0]<block->minx-60) return UI_RETURN_OUT;
		if(uevent->mval[1]<block->miny-60) return UI_RETURN_OUT;
		if(uevent->mval[0]>block->maxx+60) return UI_RETURN_OUT;
		if(uevent->mval[1]>block->maxy+60) return UI_RETURN_OUT;
		
	}

	return retval;
}

void uiDrawButTip(uiBut *but)
{
	float x1, x2, y1, y2;
	
	x1= (but->x1+but->x2)/2; x2= 10+x1+ but->aspect*fmgetstrwidth(but->font, but->tip);
	y1= but->y1-19; y2= but->y1-2;

	ui_graphics_to_window(&x1, &y1);
	ui_graphics_to_window(&x2, &y2);

	if(x2 > G.curscreen->sizex) {
		x1 -= x2-G.curscreen->sizex;
		x2= G.curscreen->sizex;
	}
	if(y1 < 0) {
		y1 += 36;
		y2 += 36;
	}

	ui_bgnpupdraw((int)(x1-1), (int)(y1-1), (int)(x2+4), (int)(y2+4), 0);

	cpack(0xC0D0D0);
	glRectf(x1, y1, x2, y2);
	
	/* below */
	cpack(0x0);
	fdrawline(x1, y1, x2, y1);
	/* right */
	fdrawline(x2, y1, x2, y2);
	/* top */
	cpack(0xFFFFFF);
	fdrawline(x1, y2, x2, y2);
	/* left */
	fdrawline(x1, y1, x1, y2);
	
	fmsetfont(but->font);
	cpack(0x0);
	glRasterPos2f( x1+3, y1+4);
	fmprstr(but->tip);
	
}

void uiDoButTip()
{
	int val, time;
	
	if(UIbuttip && UIbuttip->tip && UIbuttip->tip[0]) {
		
		/* for some stupid reason usleep(1) doesnt work. qtest returns a val then */
		for(time= 30; time>0; time--) {
			if(winqtest(curarea) || headqtest(curarea) || afterqtest() || qtest()) break;
			usleep(5);
		}
		
		if(time==0) {
			uiDrawButTip(UIbuttip);
			
			while(qtest()==0 && time<300) {
				time++;
				usleep(5);
			}
			ui_endpupdraw();
			
			UIbuttip= NULL;
		}
	}
}

/* returns UI_NOTHING, if nothing happened */
int uiDoBlocks(ListBase *lb, int event)
{
	/* return when:  firstblock != BLOCK_LOOP
	 * The mainloop is constructed in such a way
	 * that the last mouse event from a sub-block
	 * is passed on to the next block.
	 */

	uiBlock *block;
	uiEvent uevent;
	int retval= UI_NOTHING, cont= 1;
	
	if(lb->first==0) return UI_NOTHING;
	
	UIfrontbuf= 1;
	UIbuttip= NULL;

	uevent.qual= G.qual;
	uevent.event= event;
	uevent.val= 1;

	while(cont) {

		block= lb->first;
		while(block) {
			
			/* this here, to make sure it also draws when event==0 */
			if(block->flag & UI_BLOCK_REDRAW) {
				if( block->flag & UI_BLOCK_LOOP) {
					ui_bgnpupdraw((int)block->minx-1, (int)block->miny-4, (int)block->maxx+4, (int)block->maxy+1, 1);
				}
				uiDrawBlock(block);
				block->flag &= ~UI_BLOCK_REDRAW;
			}

			retval= uiDoBlock(block, &uevent);
			if(retval==UI_CONT || retval & UI_RETURN) break;

			block= block->next;
		}
	
		/* this is here, to allow closed loop-blocks (menu's) to return to the previous block */
		block= lb->first;
		if(block==NULL || (block->flag & UI_BLOCK_LOOP)==0) cont= 0;

		/* while loop blocks... */
		while( block= lb->first) {
			if(block->flag & UI_BLOCK_LOOP) {
				
				/* this here, for menu buts */
				if(block->flag & UI_BLOCK_REDRAW) {
					if( block->flag & UI_BLOCK_LOOP) {
						ui_bgnpupdraw((int)block->minx-1, (int)block->miny-4, (int)block->maxx+4, (int)block->maxy+1, 1);
					}
					uiDrawBlock(block);
					block->flag &= ~UI_BLOCK_REDRAW;
				}

				uevent.event= extern_qread(&uevent.val);
				
				if(uevent.event) {
					retval= uiDoBlock(block, &uevent);
					if(retval & UI_RETURN) {
						remlink(lb, block);
						uiFreeBlock(block);
						ui_endpupdraw();
					}
				}
			}
			else break;
		}

		if(retval==UI_CONT || retval & UI_RETURN_OK) cont= 0;
	}
	UIfrontbuf= 0;

	if(retval & UI_RETURN_OK) {
		if(UIafterfunc) UIafterfunc(UIafterval);
		UIafterfunc= NULL;
	}

	/* tooltip */	
	if(retval==UI_NOTHING && (uevent.event==MOUSEX || uevent.event==MOUSEY)) {
		if(U.flag & TOOLTIPS) uiDoButTip();
	}

	return retval;
}

/* ************** DATA *************** */


double uiGetButVal(uiBut *but)
{
	void *poin;
	double value;

	poin= but->poin;

	if(but->type== HSVSLI) {
		float h, s, v, *fp= (float *) poin;
		
		rgb_to_hsv(fp[0], fp[1], fp[2], &h, &s, &v);

		switch(but->str[0]) {
			case 'H': value= h; break;
			case 'S': value= s; break;
			case 'V': value= v; break;
		}
		
	} 
	else if( but->pointype == CHA ) {
		value= *(char *)poin;
	}
	else if( but->pointype == SHO ) {
		value= *(short *)poin;
	} 
	else if( but->pointype == INT ) {
		value= *(int *)poin;		
	} 
	else if( but->pointype == FLO ) {
		value= *(float *)poin;
	}

	return value;
}

void uiIsButSel(uiBut *but)
{
	double value;
	int lvalue;
	short push=0, true=1;

	value= uiGetButVal(but);

	if( but->type==TOGN ) true= 0;

	if( but->bit ) {
		lvalue= (int)value;
		if( BTST(lvalue, (but->bitnr)) ) push= true;
		else push= !true;
	}
	else {
		switch(but->type) {
		case BUT:
			push= 0;
			break;
		case TOG:
		case TOGR:
		case TOG3:
		case ICONTOG:
			if(value!=0.0) push= 1;
			break;
		case TOGN:
			if(value==0.0) push= 1;
			break;
		case ROW:
			if(value == but->max) push= 1;
			break;
		case COL:
			push= 1;
			break;
		default:
			push= 2;
			break;
		}
	}
	
	if(push==2);
	else if(push==1) but->flag |= UI_SELECT;
	else but->flag &= ~UI_SELECT;
}

void uiSetButVal(uiBut *but, double value)
{
	void *poin;

	if(but->pointype==0) return;
	poin= but->poin;

	/* value is een hsvwaarde: omzetten naar de rgb */
	if( but->type==HSVSLI ) {
		float h, s, v, *fp= (float *)but->poin;
		
		rgb_to_hsv(fp[0], fp[1], fp[2], &h, &s, &v);
		
		switch(but->str[0]) {
		case 'H': h= value; break;
		case 'S': s= value; break;
		case 'V': v= value; break;
		}
		
		hsv_to_rgb(h, s, v, fp, fp+1, fp+2);
		
	}
	else if( but->pointype==CHA )
		*((char *)poin)= (char)value;
	else if( but->pointype==SHO )
		*((short *)poin)= (short)value;
	else if( but->pointype==INT )
		*((int *)poin)= (int)value;
	else if( but->pointype==FLO )
		*((float *)poin)= value;
	
	/* update select flag */
	uiIsButSel(but);

}

void uiSetCurFont(uiBlock *block, int index)
{

	if(block->aspect<0.60) {
		block->curfont= UIfont[index].xl;		
	}
	else if(block->aspect<1.15) {
		block->curfont= UIfont[index].large;
	}
	else if(block->aspect<1.59) {
		block->curfont= UIfont[index].medium;		
	}
	else {
		block->curfont= UIfont[index].small;		
	}

	if(block->curfont==NULL) block->curfont= UIfont[index].large;	
	if(block->curfont==NULL) block->curfont= UIfont[index].medium;	
	if(block->curfont==NULL) printf("error block no font %s\n", block->name);
}

void uiDefFont(uint index, void *xl, void *large, void *medium, void *small)
{
	if(index>=UI_ARRAY) return;
	
	UIfont[index].xl= xl;
	UIfont[index].large= large;
	UIfont[index].medium= medium;
	UIfont[index].small= small;
}

void uiDefCol(uint index, uint medium, uint pen, uint pen_sel)
{
	struct uiCol *bc;
	char *cp1, *cp2;
	
	if(index>=UI_ARRAY) return;
	
	bc= &UIcol[index];

	bc->medium= medium;
	bc->pen_sel= pen_sel;
	bc->pen= pen;

	cp1= (char *)&medium;
	
	/* white */
	cp2= (char *)&(bc->white);
	if(cp1[BCOMP]+60>255) cp2[BCOMP]= 255; else cp2[BCOMP]= cp1[BCOMP]+60;
	if(cp1[GCOMP]+60>255) cp2[GCOMP]= 255; else cp2[GCOMP]= cp1[GCOMP]+60;
	if(cp1[RCOMP]+60>255) cp2[RCOMP]= 255; else cp2[RCOMP]= cp1[RCOMP]+60;

	/* light */
	cp2= (char *)&(bc->light);
	if(cp1[BCOMP]+35>255) cp2[BCOMP]= 255; else cp2[BCOMP]= cp1[BCOMP]+35;
	if(cp1[GCOMP]+35>255) cp2[GCOMP]= 255; else cp2[GCOMP]= cp1[GCOMP]+35;
	if(cp1[RCOMP]+35>255) cp2[RCOMP]= 255; else cp2[RCOMP]= cp1[RCOMP]+35;

	/* hilite */
	cp2= (char *)&(bc->hilite);
	if(cp1[BCOMP]+20>255) cp2[BCOMP]= 255; else cp2[BCOMP]= cp1[BCOMP]+20;
	if(cp1[GCOMP]+20>255) cp2[GCOMP]= 255; else cp2[GCOMP]= cp1[GCOMP]+20;
	if(cp1[RCOMP]+20>255) cp2[RCOMP]= 255; else cp2[RCOMP]= cp1[RCOMP]+20;

	/* grey */
	cp2= (char *)&(bc->grey);
	if(cp1[BCOMP]-35<0) cp2[BCOMP]= 0; else cp2[BCOMP]= cp1[BCOMP]-45;
	if(cp1[GCOMP]-35<0) cp2[GCOMP]= 0; else cp2[GCOMP]= cp1[GCOMP]-45;
	if(cp1[RCOMP]-35<0) cp2[RCOMP]= 0; else cp2[RCOMP]= cp1[RCOMP]-45;

	/* dark */
	cp2= (char *)&(bc->dark);
	if(cp1[BCOMP]-60<0) cp2[BCOMP]= 0; else cp2[BCOMP]= cp1[BCOMP]-60;
	if(cp1[GCOMP]-60<0) cp2[GCOMP]= 0; else cp2[GCOMP]= cp1[GCOMP]-60;
	if(cp1[RCOMP]-60<0) cp2[RCOMP]= 0; else cp2[RCOMP]= cp1[RCOMP]-60;
	
}

void uiFreeLink(uiLink *link)
{
	
	if(link) {	
		freelistN(&link->lines);
		freeN(link);
	}
}

void uiFreeBut(uiBut *but)
{
	if(but->str && but->str != but->strdata) freeN(but->str);
	uiFreeLink(but->link);

	freeN(but);
}

void uiFreeBlock(uiBlock *block)
{
	uiBut *but;

	if(block->flag & UI_BLOCK_BUSY) PRINT(x, block);	

	while(but= block->buttons.first) {
		remlink(&block->buttons, but);	
		uiFreeBut(but);
	}

	freeN(block);
	UIbuttip= NULL;
}

void uiFreeBlocks(ListBase *lb)
{
	uiBlock *block;
	
	while(block= lb->first) {
		remlink(lb, block);
		uiFreeBlock(block);
	}
}

void uiFreeBlocksWin(ListBase *lb, int win)
{
	uiBlock *block, *blockn;
	
	block= lb->first;
	while(block) {
		blockn= block->next;
		if(block->win==win) {
			remlink(lb, block);
			uiFreeBlock(block);
		}
		block= blockn;
	}
}

uiBlock *uiNewBlock(ListBase *lb, char *name, short dt, short font, uint paper, short win)
{
	uiBlock *block;
	int getsizex, getsizey;
	
	/* each listbase only has one block with this name */
	if(lb) {
		block= lb->first;
		while(block) {
			if(strcmp(name, block->name)==0) {
				remlink(lb, block);
				uiFreeBlock(block);
				break;
			}
			block= block->next;
		}
	}
	
	block= callocN(sizeof(uiBlock), "iuBlock");
	if(lb) addhead(lb, block);		/* at the beginning of the list! */

	strcpy(block->name, name);
	/* draw win */
	block->win= win;
	/* window where queue event should be added, pretty weak this way!
	   this is because the 'mainwin' pup menu's */
	block->winq= winget();
	block->dt= dt;
	block->font= font;
	block->paper= paper;
	
	/* aspect */
	mygetsingmatrix_win(block->winmat, win);

	getsize(&getsizex, &getsizey);
	block->aspect= 2.0/( (getsizex)*block->winmat[0][0]);

	uiSetCurFont(block, block->font);

	block->flag= UI_BLOCK_TEST_ACTIVE;

	return block;
}

uiBlock *uiGetBlock(char *name, ScrArea *sa)
{
	uiBlock *block= sa->uiblocks.first;
	
	while(block) {
		if( strcmp(name, block->name)==0 ) return block;
		block= block->next;
	}
	
	return NULL;
}

void uiCheckBut(uiBut *but)
{
	/* if something changed in the button */
	ID *id;
	double value;
	short pos;
	
	uiIsButSel(but);

	/* name: */
	switch( but->type ) {
	
	case MENU:
		
		if(but->x2 - but->x1 > 24) {
			value= uiGetButVal(but);
			uiSetNameMenu(but, (int)value);
		}
		break;
	
	case NUM:
	case NUMSLI:
	case HSVSLI:

		value= uiGetButVal(but);

		if( but->pointype==FLO ) {
			if(but->max<10.001) sprintf(but->drawstr, "%s%.3f", but->str, value);
			else sprintf(but->drawstr, "%s%.2f", but->str, value);
		}
		else {
			sprintf(but->drawstr, "%s%d", but->str, (int)value);
		}
		break;

	case IDPOIN:
		id= *( (ID **)but->poin );
		strcpy(but->drawstr, but->str);
		if(id) strcat(but->drawstr, id->name+2);
		break;
	
	case TEX:
		strcpy(but->drawstr, but->str);
		strcat(but->drawstr, but->poin);
		break;
		
	default:
		strcpy(but->drawstr, but->str);
		
	}
	
	if(but->drawstr[0]) but->strwidth= but->aspect*fmgetstrwidth(but->font, but->drawstr);
	else but->strwidth= 0;

	/* automatic width */
	if(but->x2==0.0) {
		but->x2= (but->x1+but->strwidth+6); 
	}

	/* calc but->ofs, to draw the string shorter if too long */
	but->ofs= 0;
	while(but->strwidth > (int)(but->x2-but->x1-7) ) {
		but->ofs++;

		if(but->drawstr[but->ofs]) 
			but->strwidth= but->aspect*fmgetstrwidth(but->font, but->drawstr+but->ofs);
		else but->strwidth= 0;

		/* textbut exception */
		if(but->pos != -1) {
			pos= but->pos+strlen(but->str);
			if(pos-1 < but->ofs) {
				pos= but->ofs-pos+1;
				but->ofs -= pos;
				if(but->ofs<0) {
					but->ofs= 0;
					pos--;
				}
				but->drawstr[ strlen(but->drawstr)-pos ]= 0;
			}
		}
		
		if(but->strwidth < 10) break;
	}

	/* test for min and max, icon sliders, etc */
	
	switch( but->type ) {
	case NUM:
	case SLI:
	case SCROLL:
	case NUMSLI:
	case HSVSLI:
		value= uiGetButVal(but);
		if(value < but->min) value= but->min;
		if(value > but->max) value= but->max;
		uiSetButVal(but, value);
		break;
		
	case ICONTOG: 
		if(but->flag & UI_SELECT) but->iconxadd= 1;
		else but->iconxadd= 0;
		break;
		
	case ICONROW:
		value= uiGetButVal(but);
		but->iconxadd= (int)value- (int)(but->min);
		break;
	}
}

uiBut *uiDefBut(uiBlock *block, int type, int retval, char *str, short x1, short y1, short x2, short y2, void *poin, float min, float max, float a1, float a2,  char *tip)
{
	uiBut *but;
	float value;
	int nr, x, y;
	short slen;
	
	if(type & BUTPOIN) {		/* er is pointer nodig */
		if(poin==0) {
			/* als pointer nul is wordt button gewist en niet gedefinieerd */

			cpack(UIcol[block->col].medium);
			glRects(x1,  y1,  x1+x2,  y1+y2);
			return NULL;
		}
	}

	but= callocN(sizeof(uiBut), "uiBut");

	but->type= type & BUTTYPE;
	but->pointype= type & BUTPOIN;
	but->bit= type & BIT;
	but->bitnr= type & 31;

	addtail(&block->buttons, but);

	but->retval= retval;
	if( strlen(str)>=UI_MAX_NAME_STR-1 ) {
		but->str= callocN( strlen(str)+2, "uiDefBut");
		strcpy(but->str, str);
	}
	else {
		but->str= but->strdata;
		strcpy(but->str, str);
	}
	but->x1= x1; 
	but->y1= y1;
	if(block->autofill) {
		but->x2= x2; 
		but->y2= y2;
	}
	else {
		but->x2= (x1+x2); 
		but->y2= (y1+y2);
	}
	but->poin= poin;
	but->min= min; 
	but->max= max;
	but->a1= a1; 
	but->a2= a2;
	but->tip= tip;
	
	but->font= block->curfont;
	but->col= &UIcol[block->col];

	but->lock= UIlock;
	but->lockstr= UIlockstr;

	but->aspect= block->aspect;
	but->paper= block->paper;
	but->win= block->win;
	but->func= block->func;
	
	if(block->dt==UI_EMBOSSX) but->embossfunc= uiEmbossX;
	else if(block->dt==UI_EMBOSSW) but->embossfunc= uiEmbossW;
	else if(block->dt==UI_EMBOSSF) but->embossfunc= uiEmbossF;
	else if(block->dt==UI_EMBOSSM) but->embossfunc= uiEmbossM;
	else but->embossfunc= uiEmbossN;
	
	but->pos= -1;	/* cursor invisible */

	/* icon */
	if( strncmp(but->str, "ICON", 4)==0 ) {
		sscanf(str+4, "%d %d %d\n", &nr, &x, &y);
		but->icon= &UIicon[nr];	
		but->iconx= x;
		but->icony= y;
		but->flag |= UI_HAS_ICON;
	}

	if(but->type==NUM) {	/* spatie toevoegen achter naam */
		slen= strlen(but->str);
		if(slen>0 && slen<UI_MAX_NAME_STR-2) {
			if(but->str[slen-1]!=' ') {
				but->str[slen]= ' ';
				but->str[slen+1]= 0;
			}
		}
	}
	
	if ELEM5(but->type, HSVSLI , NUMSLI, TEX, LABEL, IDPOIN) {
		but->flag |= UI_TEXT_LEFT;
	}
	
	uiCheckBut(but);
	
	return but;
}

void uiAutoBlock(uiBlock *block, float minx, float miny, float sizex, float sizey, int flag)
{
	block->minx= minx;
	block->maxx= minx+sizex;
	block->miny= miny;
	block->maxy= miny+sizey;
	
	block->autofill= flag;	/* also check for if it has to be done */

}

void uiSetButLink(uiBut *but, void **poin, void ***ppoin, short *tot, int from, int to)
{
	uiLink *link;
	
	link= but->link= callocN(sizeof(uiLink), "new uilink");
	
	link->poin= poin;
	link->ppoin= ppoin;
	link->totlink= tot;
	link->fromcode= from;
	link->tocode= to;
}

/* ******************** PUPmenu ****************** */

short pupmenu(char *instr)
{
	uiBlock *block;
	ListBase listb= {NULL, NULL};
	int retval[100], items, event;
	static int lastselected= 0;
	short width, height, mousexmove, mouseymove, xmax, ymax, mval[2], val= -1;
	short a, startx, starty, endx, endy, boxh=TBOXH, x1, y1;
	char *str, *title=0, *cpoin[100];
	static char laststring[UI_MAX_NAME_STR];
	
	/* block stuff first, need to know the font */
	block= uiNewBlock(&listb, "menu", UI_EMBOSSW, UI_HELV, 0x808080, G.curscreen->mainwin);
	block->flag= UI_BLOCK_LOOP|UI_BLOCK_REDRAW|UI_BLOCK_RET_1|UI_BLOCK_NUMSELECT;
	block->aspect= 1.0;
	uiSetCurFont(block, block->font);
	
	/* copy string */
	str= mallocN(strlen(instr)+1, "pupmenu");
	strcpy(str, instr);
	
	/* eerst string uitelkaar pulken, tellen hoeveel elementen, return values */
	items= decompose_menu_string(str, &title, cpoin, retval);

	/* size and location, title slightly bigger for bold */
	if(title) width= 2*strlen(title)+fmgetstrwidth(block->curfont, title);
	else width= 0;
	for(a=0; a<items; a++) {
		xmax= fmgetstrwidth(block->curfont, cpoin[a]);
		if(xmax>width) width= xmax;
	}
	width+= 10;
	
	height= boxh*items;
	
	xmax = G.curscreen->sizex;
	ymax = G.curscreen->sizey;

	getmouseco_sc(mval);
	
	if(strncmp(laststring, instr, UI_MAX_NAME_STR-1)!=0) lastselected= 0;
	strncpy(laststring, instr, UI_MAX_NAME_STR-1);
	
	startx= mval[0]-width/2;
	if(lastselected>=0 && lastselected<items) {
		starty= mval[1]-height+boxh/2+lastselected*boxh;
	}
	else starty= mval[1]-height/2;
	
	mouseymove= 0;
	
	if(startx<10) startx= 10;
	if(starty<10) {
		mouseymove= 10-starty;
		starty= 10;
	}
	
	endx= startx+width;
	endy= starty+height;
	if(endx>xmax) {
		endx= xmax-10;
		startx= endx-width;
	}
	if(endy>ymax-20) {
		mouseymove= ymax-endy-20;
		endy= ymax-20;
		starty= endy-height;
		
	}

	if(mouseymove) {
		warp_pointer(mval[0], mouseymove+mval[1]);
		mousexmove= mval[0];
		mouseymove= mval[1];
	}
	
	/* here we go! */	
	if(title) {
		uiBut *bt;
		uiSetCurFont(block, block->font+1);
		bt= uiDefBut(block, LABEL, 0, title, startx, starty+items*boxh, width, boxh, NULL, 0.0, 0.0, 0, 0, "");
		bt->flag= UI_TEXT_LEFT;
		uiSetCurFont(block, block->font);
	}

	y1= starty + boxh*(items-1);
	x1= startx;
	for(a=0; a<items; a++, y1-=boxh) {
		
		if( strcmp(cpoin[a], "%l")==0) {
			uiDefBut(block, SEPR, B_NOP, "", x1, y1, width, boxh, NULL, 0, 0.0, 0, 0, "");
		}
		else {
			uiDefBut(block, BUTM|SHO, B_NOP, cpoin[a], x1, y1, width, boxh-1, &val, (float)retval[a], 0.0, 0, 0, "");
		}
	}
	
	ui_boundsblock(block, 2);

	event= uiDoBlocks(&listb, 0);

	/* calculate last selected */
	lastselected= 0;
	for(a=0; a<items; a++) {
		if(val==retval[a]) lastselected= a;
	}

	/* ready, restore stuff */
	UIfrontbuf= 0;	
	
	freeN(str);

	if(mouseymove && (event & UI_RETURN_OUT)==0) warp_pointer(mousexmove, mouseymove);
	return val;
}

