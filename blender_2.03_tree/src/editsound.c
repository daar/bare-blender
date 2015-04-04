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



/*  editsca.c   july 2000
 *  
 *  sound editing code
 * 
 * 
 *  ton roosendaal
 * Version: $Id: editsound.c,v 1.5 2000/08/26 15:14:35 ton Exp $
 */



#include "blender.h"
#include "graphics.h"
#include "interface.h"
#include "sound.h"


void winqreadsoundspace(ushort event, short val)
{
	SpaceSound *ssound;
	Object *ob;
	float dx, dy;
	int doredraw= 0, cfra, first;
	short mval[2];
	
	if(curarea->win==0) return;

	ssound= curarea->spacedata.first;

	if(val) {
		
		if( uiDoBlocks(&curarea->uiblocks, event)!=UI_NOTHING ) event= 0;

		switch(event) {
		case LEFTMOUSE:
			if( view2dmove()==0 ) {
				
				do {
					getmouseco_areawin(mval);
					areamouseco_to_ipoco(mval, &dx, &dy);
					
					cfra= (int)dx;
					if(cfra< 1) cfra= 1;
					
					if( cfra!=CFRA || first ) {
						first= 0;
				
						CFRA= cfra;
						do_global_buttons(B_NEWFRAME);
						force_draw_plus(SPACE_VIEW3D);
					}
				
				} while(get_mbut()&L_MOUSE);
				
			}
			break;
		case MIDDLEMOUSE:
			view2dmove();	/* in drawipo.c */
			break;
		case RIGHTMOUSE:
			/* mouse_select_seq(); */
			break;
		case PADPLUSKEY:
			dx= 0.1154*(G.v2d->cur.xmax-G.v2d->cur.xmin);
			G.v2d->cur.xmin+= dx;
			G.v2d->cur.xmax-= dx;
			test_view2d();

			doredraw= 1;
			break;
		case PADMINUS:
			dx= 0.15*(G.v2d->cur.xmax-G.v2d->cur.xmin);
			G.v2d->cur.xmin-= dx;
			G.v2d->cur.xmax+= dx;
			test_view2d();

			doredraw= 1;
			break;
		case HOMEKEY:
			do_sound_buttons(B_SOUNDHOME);
			break;
		}
	}

	if(doredraw) addqueue(curarea->win, REDRAW, 1);
	
}

