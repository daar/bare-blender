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



/* drawsound.c  juli 2000		GRAPHICS
 * 
 * 
 * 
 * Version: $Id: drawsound.c,v 1.8 2000/07/25 02:55:11 ton Exp $
 */

#include "blender.h"
#include "graphics.h"
#include "sound.h"
/* not used:  #include "render.h" */



void draw_wave(int startsamp, int endsamp, short sampdx, short offset, short *sp, float sampfac, float y)
{
	float min, max, v1[2], v2[3];
	int i, j;
	short value, deltasp;
	
	sp+= offset*startsamp;

	deltasp= offset*sampdx;
	
	glBegin(GL_LINES);
	for(i=startsamp; i<endsamp; i+=sampdx, sp+=deltasp) {
	
		/* filter */
		min= max= 0.0;
		for(j=0; j<sampdx; j++) {
			value= sp[offset*j];
			if(value < min) min= value;
			else if(value > max) max= value;
		}
		v1[1]= y + 0.002*min;
		v2[1]= y + 0.002*max;
		
		v1[0]=v2[0]= sampfac*i;

		glVertex2fv(v1);
		glVertex2fv(v2);
	}
	glEnd();
}

void draw_sample(bSound *sound)
{
	float totseconds, sampxlen, sampfac;
	int samples, startsamp, endsamp;
	short *sp, sampdx;
	
	if(sound==NULL || sound->data==NULL) return;
	
	/* one sample is where in v2d space? (v2d space in frames!) */
	sampfac= 25.0/(sound->rate);

	/* how many samples? */
	samples= sound->len/(sound->channels*(sound->bits/8));
	/* total len in v2d space */
	sampxlen= sampfac*samples;

	/* one pixel is how many samples? */
	sampdx= (samples*((G.v2d->cur.xmax-G.v2d->cur.xmin)/sampxlen))/curarea->winx;

	if(sampdx==0) sampdx= 1;
	
	/* start and and */
	startsamp = G.v2d->cur.xmin/sampfac;
	CLAMP(startsamp, 0, samples-1);
	endsamp= G.v2d->cur.xmax/sampfac;
	CLAMP(endsamp, 0, samples-1);
	endsamp-= sampdx;
	
	/* set 'tot' for sliders */
	G.v2d->tot.xmax= sampfac*samples;

	/* channels? */
	if(sound->channels==2) {
		
		cpack(0x905050);
		sp= (short *)(sound->data);
		draw_wave(startsamp, endsamp, sampdx, 2, sp, sampfac, 85.0);

		cpack(0x506890);
		sp++;
		draw_wave(startsamp, endsamp, sampdx, 2, sp, sampfac, 190.0);
	}
	else {
		cpack(0x905050);
		sp= (short *)(sound->data);

		draw_wave(startsamp, endsamp, sampdx, 1, sp, sampfac, 128.0);		
	}
}

void draw_cfra_sound()
{
	float vec[2];
	
	vec[0]= CFRA;
	vec[0]*= G.scene->r.framelen;

	vec[1]= G.v2d->cur.ymin;
	glColor3ub(0x20, 0x80, 0x20);
	glLineWidth(3.0);

	glBegin(GL_LINE_STRIP);
		glVertex2fv(vec);
		vec[1]= G.v2d->cur.ymax;
		glVertex2fv(vec);
	glEnd();
	
	glLineWidth(1.0);
}


void drawsoundspace()
{
	short ofsx, ofsy;
	char str[120];
	
	glClearColor(.6275, .6275, .6275, 0.0); 
	glClear(GL_COLOR_BUFFER_BIT);

	init_sound(G.ssound->sound);

	calc_scrollrcts();

	if(curarea->winx>SCROLLB+10 && curarea->winy>SCROLLH+10) {
		if(G.v2d->scroll) {	
			ofsx= curarea->winrct.xmin;	/* ivm mywin */
			ofsy= curarea->winrct.ymin;
			glViewport(ofsx+G.v2d->mask.xmin,  ofsy+G.v2d->mask.ymin, ( ofsx+G.v2d->mask.xmax-1)-(ofsx+G.v2d->mask.xmin)+1, ( ofsy+G.v2d->mask.ymax-1)-( ofsy+G.v2d->mask.ymin)+1); 
			glScissor(ofsx+G.v2d->mask.xmin,  ofsy+G.v2d->mask.ymin, ( ofsx+G.v2d->mask.xmax-1)-(ofsx+G.v2d->mask.xmin)+1, ( ofsy+G.v2d->mask.ymax-1)-( ofsy+G.v2d->mask.ymin)+1);
		}
	}

	ortho2(G.v2d->cur.xmin, G.v2d->cur.xmax, G.v2d->cur.ymin, G.v2d->cur.ymax);
	
	/* boundbox_seq(); */
	calc_ipogrid();	
	draw_ipogrid();
	
	draw_sample(G.ssound->sound);
	
	draw_cfra_sound();

	/* restore viewport */
	winset(curarea->win);

	if(curarea->winx>SCROLLB+10 && curarea->winy>SCROLLH+10) {
		
		/* ortho op pixelnivo curarea */
		ortho2(-0.5, curarea->winx+0.5, -0.5, curarea->winy+0.5);
		
		if(G.v2d->scroll) {
			drawscroll(0);
		}
	}
	
	curarea->win_swap= WIN_BACK_OK;
}

