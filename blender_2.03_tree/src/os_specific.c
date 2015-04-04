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




/* os_specific.c  july 2000 */

/* for now included:
 * 
 * - blender_timer()
 * - sdrawXORline()
 * - sdrawXORline4()
 * 
 * 
 * 
 * 
 * Version: $Id: os_specific.c,v 1.5 2000/09/13 11:52:30 ton Exp $
 */

#include "blender.h"
#include "screen.h"
#include "graphics.h"


int blender_timer()
{
	struct tms voidbuf;

	#ifdef __WIN32
	return times(&voidbuf)/10;
	#else
		#ifdef __BeOS
		return (glut_system_time())/10000;
		#else
		return times(&voidbuf);
		#endif
	#endif
}


#if defined(__BeOS) || defined(__WIN32)
extern int curswin;
void myContextSetup()
{
}
void sdrawXORline(int x0, int y0, int x1, int y1)
{
	if(x0==x1 && y0==y1) return;

	/* sdrawXORline expects current blender win coordinates */
	if(curswin>3) {
		x0+= curarea->winrct.xmin;
		x1+= curarea->winrct.xmin;
		y0+= curarea->winrct.ymin;
		y1+= curarea->winrct.ymin;
	}

	glutInvertLine(x0, y0, x1, y1);	
}


void sdrawXORline4(int nr, int x0, int y0, int x1, int y1)
{
	static short old[4][4];
	static char flags[4]= {0, 0, 0, 0};

	/* automatische onthoud, max 4 lijnen */

	/* flush */
	if(nr== -1) {
		if(flags[0]) sdrawXORline(old[0][0], old[0][1], old[0][2], old[0][3]);
		flags[0]= 0;
		if(flags[1]) sdrawXORline(old[1][0], old[1][1], old[1][2], old[1][3]);
		flags[1]= 0;
		if(flags[2]) sdrawXORline(old[2][0], old[2][1], old[2][2], old[2][3]);
		flags[2]= 0;
		if(flags[3]) sdrawXORline(old[3][0], old[3][1], old[3][2], old[3][3]);
		flags[3]= 0;
	}
	else {

		if(nr>=0 && nr<4) {
			if(flags[nr]) sdrawXORline(old[nr][0], old[nr][1], old[nr][2], old[nr][3]);
			old[nr][0]= x0;
			old[nr][1]= y0;
			old[nr][2]= x1;
			old[nr][3]= y1;
			flags[nr]= 1;
		}
		sdrawXORline(x0, y0, x1, y1);
	}

}

#else

void sdrawXORline(int x0, int y0, int x1, int y1)
{
	
	if(x0==x1 && y0==y1) return;

	#if defined(__sgi) || defined(__SUN)
	glDisable(GL_DITHER);
	#endif
	
    glBlendEquationEXT(GL_LOGIC_OP);
	glEnable(GL_LOGIC_OP); glEnable(GL_BLEND);
	glLogicOp(GL_INVERT);
	
	sdrawline(x0, y0, x1, y1);
	
	glBlendEquationEXT(GL_FUNC_ADD_EXT);
	glDisable(GL_LOGIC_OP); glDisable(GL_BLEND);
	
	#if defined(__sgi) || defined(__SUN)
	glEnable(GL_DITHER);
	#endif
	
}

void sdrawXORline4(int nr, int x0, int y0, int x1, int y1)
{
	static short old[4][4];
	static char flags[4]= {0, 0, 0, 0};

	/* automatische onthoud, max 4 lijnen */

	#if defined(__sgi) || defined(__SUN)
	glDisable(GL_DITHER);
	#endif

	glBlendEquationEXT(GL_LOGIC_OP);
	glEnable(GL_LOGIC_OP); glEnable(GL_BLEND);
	glLogicOp(GL_INVERT);

	/* flush */
	if(nr== -1) {
		if(flags[0]) sdrawline(old[0][0], old[0][1], old[0][2], old[0][3]);
		flags[0]= 0;
		if(flags[1]) sdrawline(old[1][0], old[1][1], old[1][2], old[1][3]);
		flags[1]= 0;
		if(flags[2]) sdrawline(old[2][0], old[2][1], old[2][2], old[2][3]);
		flags[2]= 0;
		if(flags[3]) sdrawline(old[3][0], old[3][1], old[3][2], old[3][3]);
		flags[3]= 0;
	}
	else {

		if(nr>=0 && nr<4) {
			if(flags[nr]) sdrawline(old[nr][0], old[nr][1], old[nr][2], old[nr][3]);
			old[nr][0]= x0;
			old[nr][1]= y0;
			old[nr][2]= x1;
			old[nr][3]= y1;
			flags[nr]= 1;
		}
		sdrawline(x0, y0, x1, y1);
	}

	glDisable(GL_LOGIC_OP); glDisable(GL_BLEND);
	glBlendEquationEXT(GL_FUNC_ADD_EXT);
	#if defined(__sgi) || defined(__SUN)
	glEnable(GL_DITHER);
	#endif

}

#endif

