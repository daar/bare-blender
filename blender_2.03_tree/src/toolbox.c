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



/*		toolbox.c	GRAPHICS
 * 
 *		feb 93
 *		dec 93 voor blender
 * Version: $Id: toolbox.c,v 1.14 2000/09/26 09:20:42 frank Exp $
 */

#include "blender.h"
#include "graphics.h"
#include "interface.h"
#include "edit.h"
#include "screen.h"
#include "render.h"


/*   ********  NOTES  ***********	*****************************
	
	- Toolbox items zelf invullen
	- de colormap kleuren staan met comments in de bgntoolbox()
	- de funktie extern_qread eventueel vervangen
	- let op de benaming van bijzondere toetsen (NumL etc) 
	- meelinken: Button.c,  ivm rawcodes
	
	*****************************	*****************************
*/	


float tbwinmat[4][4], tbprojmat[4][4];
int tbx1, tbx2, tby1, tby2, tbfontyofs, tbmain=0;
int tbmemx=TBOXX/2, tbmemy=(TBOXEL-0.5)*TBOXH, tboldwin, addmode= 0;
ushort tbpat[16]; 
short oldcursor=0, oldmap[4][3];

	/* variabelen per item */
char *tbstr, *tbstr1;		
void (*tbfunc)();
int tbval;

static GLubyte p50[] =
{
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
};

/* *********** PC PATCH ************* */

void ColorFunc(int i)
{
	if(i==TBOXBLACK) glColor3ub(0, 0, 0);
	else if(i==TBOXWHITE) glColor3ub(240, 240, 240);
	else if(i==TBOXGREY) glColor3ub(160, 160, 160);
	else glColor3ub(0, 0, 0);
}

/* *********** ANSI COMPATIBLE **************** */

void mygetcursor(short *index)
{
	*index= glutGet(GLUT_WINDOW_CURSOR);
}

/* ********************* TOOLBOX ITEMS ************************* */

void tbox_setinfo(int x, int y)
{
	/* afhankelijk van tbmain worden vars gezet */
	tbstr= 0;
	tbstr1= 0;
	tbfunc= 0;
	tbval= 0;

/* HOOFDMENU ITEMS LINKS */
	if(x==0) {

		switch(y) {
			case 0:		
				if(addmode==OB_MESH) tbstr= "  MESH";
				else if(addmode==OB_CURVE) tbstr= "  CURVE";
				else if(addmode==OB_SURF) tbstr= "  SURF";
				else tbstr= "ADD";
				break;
			case 1:		tbstr= "VIEW";		break;
			case 2:		tbstr= "EDIT";		break;
			case 3:		tbstr= "OBJECT";	break;
			case 4:		tbstr= "OBJECT";	break;
			case 5:		tbstr= "MESH";		break;
			case 6:		tbstr= "CURVE";		break;
			case 7:		tbstr= "KEY";		break;
			case 8:		tbstr= "RENDER";	break;
			case 9:		tbstr= "FILE";		break;
		}
	}
	
/* TOPICS */
	else {
		
		
/* VIEW TOPICS */
		if(tbmain==1) {
			switch(y) {
				case 0: tbstr= "";		tbstr1= "";	break;
				case 1: tbstr= "";		tbstr1= "";	break;
				case 2: tbstr= "";		tbstr1= "";	break;
				case 3: tbstr= "";		tbstr1= "";	break;
				case 4: tbstr= "Centre";		tbstr1= "c";	break;
				case 5: tbstr= "Home";			tbstr1= "C";	break;
				case 6: tbstr= "";		tbstr1= "";	break;
				case 7: tbstr= "";		tbstr1= "";	break;
				case 8: tbstr= "";		tbstr1= "";	break;
				case 9: tbstr= "Zbuffer";		tbstr1= "z";	break;
				case 10: tbstr= "";		tbstr1= "";	break;
				case 11: tbstr= "";		tbstr1= "";	break;
			}
		}

/* EDIT TOPICS */
		else if(tbmain==2) {
			switch(y) {
				case 0: tbstr= "Grabber";	tbstr1= "g";	break;
				case 1: tbstr= "Rotate";	tbstr1= "r";	break;
				case 2: tbstr= "Size";		tbstr1= "s";	break;
				case 3: tbstr= "";			tbstr1= "";		break;
				case 4: tbstr= "Shear";		tbstr1= "c|s";	break;
				case 5: tbstr= "Warp/bend";	tbstr1= "W";	break;
				case 6: tbstr= "Snapmenu";	tbstr1= "S";	break;
				case 7: tbstr= "";	tbstr1= "";	break;
				case 8: tbstr= "(De)sel All";	tbstr1= "a";	break;
				case 9: tbstr= "BorderSelect";	tbstr1= "b";	break;
				case 10: tbstr= "";				tbstr1= "";	break;
				case 11: tbstr= "Quit";	tbstr1= "q";	break;
			}
		}

/* ADD TOPICS */
		else if(tbmain==0) {

			if(addmode==0) {
				switch(y) {
					case 0: tbstr= "Mesh";		tbstr1= ">>";	tbval=OB_MESH; break;
					case 1: tbstr= "Curve";		tbstr1= ">>";	tbval=OB_CURVE; break;
					case 2: tbstr= "Surface";	tbstr1= ">>";	tbval=OB_SURF; break;
					case 3: tbstr= "Text";		tbstr1= "";		tbval=OB_FONT; tbfunc= add_primitiveFont; break;
					case 4: tbstr= "MetaBall";	tbstr1= "";		tbval=OB_MBALL; tbfunc= add_primitiveMball; break;
					case 5: tbstr= "Empty";		tbstr1= "A";	tbval=OB_EMPTY; break;
					case 6: tbstr= "";			tbstr1= "";		tbval=0; break;
					case 7: tbstr= "Camera";	tbstr1= "A";	tbval=OB_CAMERA; break;
					case 8: tbstr= "Lamp";		tbstr1= "A";	tbval=OB_LAMP; break;
					case 9: tbstr= "Ika";		tbstr1= "A";	tbval=OB_IKA; break;
					case 10: tbstr= "";			tbstr1= "A";	tbval=0; break;
					case 11: tbstr= "";			tbstr1= "A";	tbval=0; break;
					case 12: tbstr= "";			tbstr1= "";		tbval=0; break;
					case 13: tbstr= "Lattice";	tbstr1= "A";	tbval=OB_LATTICE; break;
				}
				if(tbstr1 && tbstr1[0]=='A') tbfunc= (void (*)() )add_object_draw;
			}
			else if(addmode==OB_MESH) {		
				switch(y) {
					case 0: tbstr= ">Plane";	tbstr1= "A";	tbval=0; break;
					case 1: tbstr= ">Cube";		tbstr1= "A";	tbval=1; break;
					case 2: tbstr= ">Circle";	tbstr1= "A";	tbval=4; break;
					case 3: tbstr= ">UVsphere";	tbstr1= "A";	tbval=11; break;
					case 4: tbstr= ">Icosphere";tbstr1= "A";	tbval=12; break;
					case 5: tbstr= ">Cylinder";	tbstr1= "A";	tbval=5; break;
					case 6: tbstr= ">Tube";		tbstr1= "A";	tbval=6; break;
					case 7: tbstr= ">Cone";		tbstr1= "A";	tbval=7; break;
					case 8: tbstr= ">";			tbstr1= "";				break;
					case 9: tbstr= ">Grid";		tbstr1= "A";	tbval=10; break;
					case 10: tbstr= ">";		tbstr1= "";		break;
					case 11: tbstr= ">Duplicate";tbstr1= "D";		break;
				}
				if(tbstr1 && tbstr1[0]=='A') tbfunc= add_primitiveMesh;
			}
			else if(addmode==OB_SURF) {
				switch(y) {
					case 0: tbstr= ">Curve";		tbstr1= "A";	tbval=0; break;
					case 1: tbstr= ">Circle";	tbstr1= "A";	tbval=1; break;
					case 2: tbstr= ">Surface";	tbstr1= "A";	tbval=2; break;
					case 3: tbstr= ">Tube";		tbstr1= "A";	tbval=3; break;
					case 4: tbstr= ">Sphere";	tbstr1= "A";	tbval=4; break;
					case 5: tbstr= ">Donut";		tbstr1= "A";	tbval=5; break;
					case 6: tbstr= "";			tbstr1= "";		break;
					case 7: tbstr= "";			tbstr1= "";		break;
					case 8: tbstr= "";			tbstr1= "";		break;
					case 9: tbstr= "";			tbstr1= "";		break;
					case 10: tbstr= "";			tbstr1= "";		break;
					case 11: tbstr= ">Duplicate";tbstr1= "D";	break;
				}
				if(tbstr1 && tbstr1[0]=='A') tbfunc= add_primitiveNurb;
			}
			else if(addmode==OB_CURVE) {
				switch(y) {
					case 0: tbstr= ">Bezier Curve";tbstr1= "A";	tbval=10; break;
					case 1: tbstr= ">Bezier Circle";tbstr1= "A";	tbval=11; break;
					case 2: tbstr= "";			tbstr1= "";		break;
					case 3: tbstr= "";			tbstr1= "";		break;
					case 4: tbstr= ">Nurbs Curve";tbstr1= "A";	tbval=40; break;
					case 5: tbstr= ">Nurbs Circle";tbstr1= "A";	tbval=41; break;
					case 6: tbstr= "";			tbstr1= "";		break;
					case 7: tbstr= ">Path";		tbstr1= "A";	tbval=46; break;
					case 8: tbstr= "";			tbstr1= "";		break;
					case 9: tbstr= "";			tbstr1= "";		break;
					case 10: tbstr= "";			tbstr1= "";		break;
					case 11: tbstr= ">Duplicate";tbstr1= "D";	break;
				}
				if(tbstr1 && tbstr1[0]=='A') tbfunc= add_primitiveCurve;
			}
			else if(addmode==OB_MBALL) {
				switch(y) {
					case 0: tbstr= "Ball";		tbstr1= "A";	tbval=1; break;
					case 1: tbstr= "";			tbstr1= "";		break;
					case 2: tbstr= "";			tbstr1= "";		break;
					case 3: tbstr= "";			tbstr1= "";		break;
					case 4: tbstr= "";			tbstr1= "";		break;
					case 5: tbstr= "";			tbstr1= "";		break;
					case 6: tbstr= "";			tbstr1= "";		break;
					case 7: tbstr= "";			tbstr1= "";		break;
					case 8: tbstr= "";			tbstr1= "";		break;
					case 9: tbstr= "";			tbstr1= "";		break;
					case 10: tbstr= "";			tbstr1= "";		break;
					case 11: tbstr= "Duplicate";tbstr1= "D";	break;
				}
				if(tbstr1 && tbstr1[0]=='A') tbfunc= add_primitiveMball;
			}
		}
		
/* OB TOPICS 1 */
		else if(tbmain==3) {
			switch(y) {
				case 0: tbstr= "Clear size";	tbstr1= "a|s";	break;
				case 1: tbstr= "Clear rotation";	tbstr1= "a|r";	break;
				case 2: tbstr= "Clear location";	tbstr1= "a|g";	break;
				case 3: tbstr= "Clear origin";	tbstr1= "a|o";	break;
				case 4: tbstr= "Make Parent";	tbstr1= "c|p";	break;
				case 5: tbstr= "Clear Parent";	tbstr1= "a|p";	break;
				case 6: tbstr= "Make Track";	tbstr1= "c|t";	break;
				case 7: tbstr= "Clear Track";	tbstr1= "a|t";	break;
				case 8: tbstr= "";	tbstr1= "";	break;
				case 9: tbstr= "";	tbstr1= "";	break;
				case 10: tbstr= "Image Displist";	tbstr1= "c|d";	break;
				case 11: tbstr= "Image Aspect";	tbstr1= "a|v";	break;
			}
		}
		
/* OB TOPICS 2 */
		else if(tbmain==4) {
			switch(y) {
				case 0: tbstr= "EditMode";		tbstr1= "Tab";	break;
				case 1: tbstr= "Move To Layer";	tbstr1= "m";	break;
				case 2: tbstr= "Delete";		tbstr1= "x";	break;
				case 3: tbstr= "Delete All";		tbstr1= "c|x";	break;
				case 4: tbstr= "Apply Size/Rot";	tbstr1= "c|a";	break;
				case 5: tbstr= "Apply Deform";		tbstr1= "c|A";	break;
				case 6: tbstr= "Join";		tbstr1= "c|j";	break;
				case 7: tbstr= "make local";	tbstr1= "l";	break;
				case 8: tbstr= "select Links";		tbstr1= "L";	break;
				case 9: tbstr= "make Links";	tbstr1= "c|l";	break;
				case 10: tbstr= "Copy menu";	tbstr1= "c|c";	break;
				case 11: tbstr= "Convert menu";	tbstr1= "a|c";	break;
			}
		}

/* mesh TOPICS */
		else if(tbmain==5) {
			switch(y) {
				case 0: tbstr= "Select Linked";	tbstr1= "l";	break;
				case 1: tbstr= "Deselect Linked";	tbstr1= "L";	break;
				case 2: tbstr= "Extrude";		tbstr1= "e";	break;
				case 3: tbstr= "Delete Menu";	tbstr1= "x";	break;
				case 4: tbstr= "Make edge/face";	tbstr1= "f";	break;
				case 5: tbstr= "Fill";			tbstr1= "F";	break;
				case 6: tbstr= "Split";			tbstr1= "y";	break;
				case 7: tbstr= "Undo/reload";	tbstr1= "u";	break;
				case 8: tbstr= "Calc Normals";	tbstr1= "c|n";	break;
				case 9: tbstr= "Separate";		tbstr1= "p";	break;
				case 10: tbstr= "Write Videosc";	tbstr1= "a|w";	break;
				case 11: tbstr= "";	tbstr1= "";	break;
			}
		}
	
/* CURVE TOPICS */
		else if(tbmain==6) {
			switch(y) {
				case 0: tbstr= "Select Linked";	tbstr1= "l";	break;
				case 1: tbstr= "Deselect Link";	tbstr1= "L";	break;
				case 2: tbstr= "Extrude";		tbstr1= "e";	break;
				case 3: tbstr= "Delete Menu";	tbstr1= "x";	break;
				case 4: tbstr= "Make Segment";	tbstr1= "f";	break;
				case 5: tbstr= "Cyclic";		tbstr1= "c";	break;
				case 6: tbstr= "";	tbstr1= "";	break;
				case 7: tbstr= "SelectRow";		tbstr1= "R";	break;
				case 8: tbstr= "Calc Handle";	tbstr1= "h";	break;
				case 9: tbstr= "Auto Handle";	tbstr1= "H";	break;
				case 10: tbstr= "Vect Handle";	tbstr1= "v";	break;
				case 11: tbstr= "Specials";	tbstr1= "w";	break;
			}
		}
	
/* KEY TOPICS */
		else if(tbmain==7) {
			switch(y) {
				case 0: tbstr= "Insert";	tbstr1= "i";	break;
				case 1: tbstr= "Show";		tbstr1= "k";	break;
				case 2: tbstr= "Next";		tbstr1= "PageUp";	break;
				case 3: tbstr= "Prev";		tbstr1= "PageDn";	break;
				case 4: tbstr= "Show+sel";	tbstr1= "K";	break;
				case 5: tbstr= "";	tbstr1= "";	break;
				case 6: tbstr= "";	tbstr1= "";	break;
				case 7: tbstr= "";	tbstr1= "";	break;
				case 8: tbstr= "";	tbstr1= "";	break;
				case 9: tbstr= "";	tbstr1= "";	break;
				case 10: tbstr= "";	tbstr1= "";	break;
				case 11: tbstr= "";	tbstr1= "";	break;
			}
		}

/* RENDER TOPICS */
		else if(tbmain==8) {
			switch(y) {
				case 0: tbstr= "RenderWindow";	tbstr1= "F11";	break;
				case 1: tbstr= "Render";		tbstr1= "F12";	break;
				case 2: tbstr= "Set Border";	tbstr1= "B";	break;
				case 3: tbstr= "Image Zoom";	tbstr1= "z";	break;
				case 4: tbstr= "";	tbstr1= "";	break;
				case 5: tbstr= "";	tbstr1= "";	break;
				case 6: tbstr= "";	tbstr1= "";	break;
				case 7: tbstr= "";	tbstr1= "";	break;
				case 8: tbstr= "";	tbstr1= "";	break;
				case 9: tbstr= "";	tbstr1= "";	break;
				case 10: tbstr= "";	tbstr1= "";	break;
				case 11: tbstr= "";	tbstr1= "";	break;
			}
		}
	
/* FILE TOPICS */
		else if(tbmain==9) {
			switch(y) {
				case 0: tbstr= "Load File As";	tbstr1= "F1";	break;
				case 1: tbstr= "Load Last";			tbstr1= "c|o";		break;
				case 2: tbstr= "Append File";	tbstr1= "shift+F1";	break;
				case 3: tbstr= "";	tbstr1= "";	break;
				case 4: tbstr= "Save File as";	tbstr1= "F2";	break;
				case 5: tbstr= "Save";			tbstr1= "c|w";	break;
				case 6: tbstr= "Save Image";	tbstr1= "F3";	break;
				case 7: tbstr= "Save VRML";	tbstr1= "c|F2";	break;
				case 8: tbstr= "Save DXF";	tbstr1= "shift+F2";	break;
				case 9: tbstr= "Save VideoScape";	tbstr1= "a|w";	break;
				case 10: tbstr= "";	tbstr1= "";	break;
				case 11: tbstr= "";	tbstr1= "";	break;
				case 12: tbstr= "";	tbstr1= "";	break;
				case 13: tbstr= "Quit Blender";	tbstr1= "q";	break;
			}
		}
	}
}

/* ******************** INIT ************************** */

void dummy()
{
	
}

void bgnpupdraw(int startx, int starty, int endx, int endy)
{
	#if defined(__sgi) || defined(__SUN)
	/* this is a dirty patch: XgetImage gets sometimes the backbuffer */
	my_get_frontbuffer_image(0, 0, 1, 1);
	my_put_frontbuffer_image();
	#endif

	tboldwin= winget();

	winset(G.curscreen->mainwin);
	
	/* pietsje groter, 1 pixel aan de rand */
	
	glReadBuffer(GL_FRONT);
	glDrawBuffer(GL_FRONT);

	glFinish();

	my_get_frontbuffer_image(startx-1, starty-4, endx-startx+5, endy-starty+6);

	mygetcursor(&oldcursor);
	glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
	fmsetfont(G.font);
	
	tbfontyofs= (TBOXH-11)/2;	/* toolbox, hier stond ooit getheigh */
}

void endpupdraw()
{
	int x;
	
	glFinish();
	my_put_frontbuffer_image();
	
	if(tboldwin) {
		winset(tboldwin);
		glutSetCursor(oldcursor);
	}

	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);
}

/* ********************************************** */

void asciitoraw(int ch, ushort *event, ushort *qual)
{
	if( isalpha(ch)==0 ) return;
	
	if( isupper(ch) ) {
		*qual= LEFTSHIFTKEY;
		ch= tolower(ch);
	}
	
	switch(ch) {
	case 'a': *event= AKEY; break;
	case 'b': *event= BKEY; break;
	case 'c': *event= CKEY; break;
	case 'd': *event= DKEY; break;
	case 'e': *event= EKEY; break;
	case 'f': *event= FKEY; break;
	case 'g': *event= GKEY; break;
	case 'h': *event= HKEY; break;
	case 'i': *event= IKEY; break;
	case 'j': *event= JKEY; break;
	case 'k': *event= KKEY; break;
	case 'l': *event= LKEY; break;
	case 'm': *event= MKEY; break;
	case 'n': *event= NKEY; break;
	case 'o': *event= OKEY; break;
	case 'p': *event= PKEY; break;
	case 'q': *event= QKEY; break;
	case 'r': *event= RKEY; break;
	case 's': *event= SKEY; break;
	case 't': *event= TKEY; break;
	case 'u': *event= UKEY; break;
	case 'v': *event= VKEY; break;
	case 'w': *event= WKEY; break;
	case 'x': *event= XKEY; break;
	case 'y': *event= YKEY; break;
	case 'z': *event= ZKEY; break;
	}
}

void tbox_execute()
{
	/* als tbfunc: functie aanroepen */
	/* als tbstr1 is een string: value tbval in queue stopen */
	ushort event=0;
	ushort qual1=0, qual2=0;

	if(tbfunc) {
		tbfunc(tbval);
	}
	else if(tbstr1) {
		if(strcmp(tbstr1, "Tab")==0) {
			event= TABKEY;
		}
		else if(strcmp(tbstr1, "PageUp")==0) {
			event= PAGEUPKEY;
		}
		else if(strcmp(tbstr1, "PageDn")==0) {
			event= PAGEDOWNKEY;
		}
		else if(strcmp(tbstr1, "shift+F1")==0) {
			qual1= LEFTSHIFTKEY;
			event= F1KEY;
		}
		else if(strcmp(tbstr1, "shift+F2")==0) {
			qual1= LEFTSHIFTKEY;
			event= F2KEY;
		}
		else if(strlen(tbstr1)<4 || (strlen(tbstr1)==4 && tbstr1[2]=='F')) {
				
			if(tbstr1[1]=='|') {
				if(tbstr1[0]=='c') qual1= LEFTCTRLKEY;
				else if(tbstr1[0]=='a') qual1= LEFTALTKEY;
				
				if (tbstr1[2]=='F') {
					switch(tbstr1[3]) {
					case '1': event= F1KEY; break;
					case '2': event= F2KEY; break;
					case '3': event= F3KEY; break;
					case '4': event= F4KEY; break;
					case '5': event= F5KEY; break;
					case '6': event= F6KEY; break;
					case '7': event= F7KEY; break;
					case '8': event= F8KEY; break;
					case '9': event= F9KEY; break;
					}
				}
				else asciitoraw(tbstr1[2], &event, &qual2);
			}
			else if(tbstr1[1]==0) {
				asciitoraw(tbstr1[0], &event, &qual2);
			}
			else if(tbstr1[0]=='F') {
				event= atoi(tbstr1+1);
				switch(event) {
					case 1: event= F1KEY; break;
					case 2: event= F2KEY; break;
					case 3: event= F3KEY; break;
					case 4: event= F4KEY; break;
					case 5: event= F5KEY; break;
					case 6: event= F6KEY; break;
					case 7: event= F7KEY; break;
					case 8: event= F8KEY; break;
					case 9: event= F9KEY; break;
					case 10: event= F10KEY; break;
					case 11: event= F11KEY; break;
					case 12: event= F12KEY; break;
				}
			}
		}
		
		if(event) {
			if(qual1) qenter(qual1, 1);
			if(qual2) qenter(qual2, 1);
			qenter(event, 1);
			qenter(event, 0);
			qenter(EXECUTE, 1);
			if(qual1) qenter(qual1, 0);
			if(qual2) qenter(qual2, 0);
		}
	}
	
}

void tbox_getmouse(mval)
short *mval;
{

	getmouseco_sc(mval);

}

void tbox_setmain(int val)
{
	tbmain= val;

	if(tbmain==0 && G.obedit) {
		addmode= G.obedit->type;
	}
}

void bgntoolbox()
{
	extern int displaysizex, displaysizey;
	short xmax, ymax, mval[2];
	
	xmax = G.curscreen->sizex;
	ymax = G.curscreen->sizey;

	tbox_getmouse(mval);
	
	if(mval[0]<95) mval[0]= 95;
	if(mval[0]>xmax-95) mval[0]= xmax-95;

	warp_pointer(mval[0], mval[1]);

	tbx1= mval[0]-tbmemx;
	tby1= mval[1]-tbmemy;
	if(tbx1<10) tbx1= 10;
	if(tby1<10) tby1= 10;
	
	tbx2= tbx1+TBOXX;
	tby2= tby1+TBOXY;
	if(tbx2>xmax) {
		tbx2= xmax-10;
		tbx1= tbx2-TBOXX;
	}
	if(tby2>ymax) {
		tby2= ymax-10;
		tby1= tby2-TBOXY;
	}

	bgnpupdraw(tbx1, tby1, tbx2, tby2);
}

void endtoolbox()
{
	short mval[2];
	
	tbox_getmouse(mval);
	if(mval[0]>tbx1 && mval[0]<tbx2)
		if(mval[1]>tby1 && mval[1]<tby2) {
			tbmemx= mval[0]-(tbx1);
			tbmemy= mval[1]-(tby1);
	}
	
	endpupdraw();
}


void tbox_embossbox(short x1, short y1, short x2, short y2, short type)	
/* type: 0=menu, 1=menusel, 2=topic, 3=topicsel */
{
	
	if(type==0) {
		glColor3ub(160, 160, 160);
		glRects(x1+1, y1+1, x2-1, y2-1);
	}
	if(type==1) {
		glColor3ub(50, 50, 100);
		glRects(x1+1, y1+1, x2-1, y2-1);
	}
	if(type==2) {
		glColor3ub(190, 190, 190);
		glRects(x1+1, y1+1, x2-1, y2-1);
	}
	if(type==3) {
		cpack(0xc07070);
		glRects(x1+1, y1+1, x2-1, y2-1);
	}
	
	if(type & 1) cpack(0xFFFFFF);
	else cpack(0x0);
}


void tbox_drawelem_body(x, y, type)
{
	int x1, y1, x2, y2, len1, len2;
	
	if(x==0) {
		x1= tbx1; x2= tbx1+TBOXXL;
	}
	else if(x==1) {
		x1= tbx1+TBOXXL;
		x2= x1+ TBOXXR-1;
	}
	
	y1= tby1+ (TBOXEL-y-1)*TBOXH;
	y2= y1+TBOXH-1;
	
	tbox_embossbox(x1, y1, x2, y2, type);
	
}

void tbox_drawelem_text(x, y, type)
{
	int x1, y1, x2, y2, len1, len2;
	
	if(x==0) {
		x1= tbx1; x2= tbx1+TBOXXL;
	}
	else if(x==1) {
		x1= tbx1+TBOXXL;
		x2= x1+ TBOXXR-1;
	}
	
	y1= tby1+ (TBOXEL-y-1)*TBOXH;
	y2= y1+TBOXH-1;
	
	if(type==0 || type==2) {
		ColorFunc(TBOXBLACK);
	}
	else {
		glColor3ub(240, 240, 240);
	}
	
	/* tekst */
	tbox_setinfo(x, y);
	if(tbstr && tbstr[0]) {
	
		len1= 5+fmgetstrwidth(G.font, tbstr);
		if(tbstr1) len2= 5+fmgetstrwidth(G.font, tbstr1); else len2= 0;
		
		while(len1>0 && (len1+len2+5>x2-x1) ) {
			tbstr[strlen(tbstr)-1]= 0;
			len1= fmgetstrwidth(G.font, tbstr);
		}
		
		if(type<2) fmsetfont(GLUT_BITMAP_HELVETICAB_12);
		else fmsetfont(G.font);
		
		glRasterPos2i(x1+5, y1+tbfontyofs);
		fmprstr(tbstr);
		
		if(tbstr1 && tbstr1[0]) {
			if(type & 1) {
				ColorFunc(TBOXBLACK);
	
				glRecti(x2-len2-2,  y1+2,  x2-3,  y2-2);
				ColorFunc(TBOXWHITE);
				glRasterPos2i(x2-len2,  y1+tbfontyofs);
				fmprstr(tbstr1);
			}
			else {
				ColorFunc(TBOXBLACK);
				glRasterPos2i(x2-len2,  y1+tbfontyofs);
				fmprstr(tbstr1);
			}
		}
	}
	

}


void tbox_drawelem(x, y, type)
int x, y, type;	
{
	/* type: 0=menu, 1=menusel, 2=topic, 3=topicsel */

	tbox_drawelem_body(x, y, type);
	tbox_drawelem_text(x, y, type);
	
}

void tbox_getactive(x, y)
int *x, *y;
{
	short mval[2];
	
	tbox_getmouse(mval);
	
	mval[0]-=tbx1;
	if(mval[0]<TBOXXL) *x= 0;
	else *x= 1;
	
	*y= mval[1]-tby1;
	*y/= TBOXH;
	*y= TBOXEL- *y-1;
	if(*y<0) *y= 0;
	if(*y>TBOXEL-1) *y= TBOXEL-1;
	
}

void drawtoolbox()
{
	int x, y, actx, acty, type;

	tbox_getactive(&actx, &acty);

	/* de background */
	for(x=0; x<2; x++) {
		
		for(y=0; y<TBOXEL; y++) {
			
			if(x==0) type= 0; 
			else type= 2;
			
			if(actx==x && acty==y) type++;
			if(type==0) {
				if(tbmain==y) type= 1;
			}
			
			tbox_drawelem_body(x, y, type);
			
		}
	}

	/* de text */
	for(x=0; x<2; x++) {
		
		for(y=0; y<TBOXEL; y++) {
			
			if(x==0) type= 0; 
			else type= 2;
			
			if(actx==x && acty==y) type++;
			if(type==0) {
				if(tbmain==y) type= 1;
			}
			
			tbox_drawelem_text(x, y, type);
			
		}
	}
}


void toolbox()
{
	int actx, acty, x, y;
	ushort event;
	short val, mval[2], xo= -1, yo=0;
	
	/* geen overdraw, dus... */
	if( R.win && R.win==G.curscreen->winakt) return;

	bgntoolbox();
	uiDrawMenuBox(0xB0B0B0, (float)tbx1, (float)tby1-1, (float)tbx2, (float)tby2);
	drawtoolbox();
	
	/* 
	 *	De aktieve window wordt in queue terug gestopt.
	 */

	while(1) {
		event= extern_qread(&val);
		if(event) {
			switch(event) {
				case LEFTMOUSE: case MIDDLEMOUSE: case RIGHTMOUSE: case RETKEY: case PADENTER:
					if(val==1) {
						tbox_getactive(&actx, &acty);
						tbox_setinfo(actx, acty);
						
						if(event==RIGHTMOUSE) {
							if(addmode) {
								addmode= 0;
								drawtoolbox();
							}
						}
						else if(tbstr1 && tbstr1[0]=='>') {
							addmode= tbval;
							drawtoolbox();
						}
						else {
							endtoolbox();
							tbox_execute();
							return;
						}
					}
					break;
				case ESCKEY:
					/* altkeys: om conflicten met overdraw en stow/push/pop te voorkomen */
				case LEFTALTKEY:
				case RIGHTALTKEY:
					if(val) endtoolbox();
					return;
			}
		}
		else usleep(2);
		
		tbox_getmouse(mval);
		if(mval[0]<tbx1-10 || mval[0]>tbx2+10 || mval[1]<tby1-10 || mval[1]>tby2+10) break;
		
		tbox_getactive(&actx, &acty);
		
		/* muisafhandeling en redraw */
		if(xo!=actx || yo!=acty) {
			if(actx==0) {
				if (acty==0) addmode=0;
				
				tbox_drawelem(0, tbmain, 0);
				tbox_drawelem(0, acty, 1);
				
				tbmain= acty;
				addmode= 0;
				for(y=0; y<TBOXEL; y++) tbox_drawelem(1, y, 2);
			}
			else if(xo> -1) {
				if(xo==0) tbox_drawelem(xo, yo, 1);
				else tbox_drawelem(xo, yo, 2);
				tbox_drawelem(actx, acty, 3);
			}
			xo= actx;
			yo= acty;
		}
	}

	endtoolbox();
}

/* ************************************  */

short confirm(char *title, char *item)
{
	short ret;
	char *str;
	
	if(item[0]==0) return 0;
	
	str= mallocN( strlen(title)+strlen(item)+4, "confirm");
	if(title[0]==0) sprintf(str, "%s", item);
	else sprintf(str, "%s%%t|%s", title, item);
	
	ret= pupmenu(str);
	freeN(str);

	return ret>=0;
}


void notice(char *str)
{
	confirm("", str);
}


int saveover(char *str)
{
	int file;
	
	if(G.f & G_DISABLE_OK) return 1;
	
	file= open(str, O_BINARY|O_RDONLY);
	close(file);
	if(file==-1) return 1;
	else if(confirm("SAVE OVER", str)) return 1;
	
	return 0;
}

short okee(char *str)
{
	if(G.f & G_DISABLE_OK) return 1;
	return confirm("OK?", str);
}

void error(char *str)
{
	char str1[100];
	
	if(G.background || G.curscreen==0) {
		printf("ERROR %s\n", str);
		return;
	}
	
	if(strlen(str)>90) str[90]= 0;
	sprintf(str1, "ERROR: %s", str);
	confirm("", str1);
	
}

void errorstr(char *str1, char *str2, char *str3)
{
	char str[256];
	
	if(str1 && strlen(str1)>79) str1[79]= 0;
	if(str2 && strlen(str2)>79) str2[79]= 0;
	if(str3 && strlen(str3)>79) str3[79]= 0;

	strcpy(str, "ERROR ");
	if(str1) strcat(str, str1);
	strcat(str, " ");
	if(str2) strcat(str, str2);
	strcat(str, " ");
	if(str3) strcat(str, str3);

	if(G.background) {
		printf("ERROR %s\n", str);
		return;
	}
	
	confirm("", str);
	
}

/* ****************** EXTRAATJE **************** */

short button(short *var, short min, short max, char *str)
{
	uiBlock *block;
	ListBase listb={0, 0};
	short x1,x2,y1,y2;
	short oldmap[4][3],val,mval[2], ret=0;

	if(min>max) min= max;

	getmouseco_sc(mval);
	
	if(mval[0]<150) mval[0]=150;
	if(mval[1]<30) mval[1]=30;
	if(mval[0]>G.curscreen->sizex) mval[0]= G.curscreen->sizex-10;
	if(mval[1]>G.curscreen->sizey) mval[1]= G.curscreen->sizey-10;

	block= uiNewBlock(&listb, "button", UI_EMBOSSX, UI_HELV, 0x808080, G.curscreen->mainwin);
	block->flag= UI_BLOCK_LOOP|UI_BLOCK_REDRAW|UI_BLOCK_RET_1;
	block->aspect= 1.0;
	uiSetCurFont(block, block->font);

	x1=mval[0]-150; 
	x2=mval[0]+20; 
	y1=mval[1]-20; 
	y2=mval[1]+20;
	
	uiDefBut(block, NUM|SHO, 0, str,	x1+5,y1+10,125,20, var,(float)min,(float)max, 0, 0, "");
	uiDefBut(block, BUT, 1, "OK",	x1+136,y1+10,25,20, NULL, 0, 0, 0, 0, "");

	ui_boundsblock(block, 5);

	ret= uiDoBlocks(&listb, 0);

	if(ret==UI_RETURN_OK) return 1;
	return 0;
}

short fbutton(float *var, float min, float max, char *str)
{
	uiBlock *block;
	ListBase listb={0, 0};
	short x1,x2,y1,y2;
	short oldmap[4][3],val,mval[2], ret=0;

	if(min>max) min= max;

	getmouseco_sc(mval);
	
	if(mval[0]<150) mval[0]=150;
	if(mval[1]<30) mval[1]=30;
	if(mval[0]>G.curscreen->sizex) mval[0]= G.curscreen->sizex-10;
	if(mval[1]>G.curscreen->sizey) mval[1]= G.curscreen->sizey-10;

	block= uiNewBlock(&listb, "button", UI_EMBOSSX, UI_HELV, 0x808080, G.curscreen->mainwin);
	block->flag= UI_BLOCK_LOOP|UI_BLOCK_REDRAW|UI_BLOCK_RET_1;
	block->aspect= 1.0;
	uiSetCurFont(block, block->font);

	x1=mval[0]-150; 
	x2=mval[0]+20; 
	y1=mval[1]-20; 
	y2=mval[1]+20;
	
	uiDefBut(block, NUM|FLO, 0, str,	x1+5,y1+10,125,20, var, min, max, 0, 0, "");
	uiDefBut(block, BUT, 1, "OK",	x1+136, y1+10, 35, 20, NULL, 0, 0, 0, 0, "");

	ui_boundsblock(block, 2);

	ret= uiDoBlocks(&listb, 0);

	if(ret==UI_RETURN_OK) return 1;
	return 0;
}

int movetolayer_buts(uint *lay)
{
	uiBlock *block;
	ListBase listb={0, 0};
	uint oldlay;
	int bit;
	int dx, dy, a, x1, x2, y1, y2, sizex=160, sizey=30;
	ushort toets;
	short oldmap[4][3], val, mval[2], ret=0;
	char str[12];
	
	if(G.vd->localview) {
		error("Not in localview ");
		return ret;
	}

	getmouseco_sc(mval);
	
	if(mval[0]<sizex/2) mval[0]=sizex/2;
	if(mval[1]<sizey/2) mval[0]=sizey/2;
	if(mval[0]>G.curscreen->sizex -sizex/2) mval[0]= G.curscreen->sizex -sizex/2;
	if(mval[1]>G.curscreen->sizey -sizey/2) mval[1]= G.curscreen->sizey -sizey/2;

	winset(G.curscreen->mainwin);
	
	x1= mval[0]-sizex+10; 
	x2= mval[0]+10; 
	y1= mval[1]-sizey/2; 
	y2= mval[1]+sizey/2;
	
	block= uiNewBlock(&listb, "button", UI_EMBOSSX, UI_HELV, 0x808080, G.curscreen->mainwin);
	block->flag= UI_BLOCK_LOOP|UI_BLOCK_REDRAW|UI_BLOCK_NUMSELECT|UI_BLOCK_ENTER_OK;
	block->aspect= 1.0;
	uiSetCurFont(block, block->font);
	
	dx= (sizex-5)/12;
	dy= sizey/2;
	
	for(a=0; a<10; a++) {
		uiDefBut(block, TOGR|INT|BIT|a, 0, "",	x1+a*dx, y1+dy, dx, dy, lay, 0, 0, 0, 0, "");
		if(a==4) x1+= 5;
	}
	x1-= 5;

	for(a=0; a<10; a++) {
		uiDefBut(block, TOGR|INT|BIT|a+10, 0, "",	x1+a*dx, y1, dx, dy, lay, 0, 0, 0, 0, "");
		if(a==4) x1+= 5;
	}
	x1-= 5;
	
	uiDefBut(block, BUT, 1, "OK", x1+10*dx+10, y1, 3*dx, 2*dy, NULL, 0, 0, 0, 0, "");

	ui_boundsblock(block, 2);

	ret= uiDoBlocks(&listb, 0);

	if(ret==UI_RETURN_OK) return 1;
	return 0;
}

/* ********************** CLEVER_NUMBUTS ******************** */

#define MAXNUMBUTS	24

VarStruct numbuts[MAXNUMBUTS];
void *numbpoin[MAXNUMBUTS];
int numbdata[MAXNUMBUTS];

void draw_numbuts_tip(char *str, int x1, int y1, int x2, int y2)
{
	static char *last=0;	/* avoid ugly updates! */
	int temp;
	
	if(str==last) return;
	last= str;
	if(str==0) return;

	glColor3ub(160, 160, 160); /* MGREY */
	glRecti(x1+4,  y2-36,  x2-4,  y2-16);

	cpack(0x0);

	temp= 0;
	while( fmgetstrwidth(G.fonts, str+temp)>(x2 - x1-24)) temp++;
	fmsetfont(G.fonts);
	glRasterPos2i(x1+16, y2-30);
	fmprstr(str+temp);
}

int do_clever_numbuts(char *name, int tot, int winevent)
{
	ListBase listb= {NULL, NULL};
	uiBlock *block;
	VarStruct *varstr;
	int a, bit, sizex, sizey, x1, x2, y1, y2;
	short mval[2], event;
	char *str;
	
	if(tot<=0 || tot>MAXNUMBUTS) return 0;

	getmouseco_sc(mval);

	/* size */
	sizex= 235;
	sizey= 30+20*(tot+1);
	
	/* midden */
	if(mval[0]<sizex/2) mval[0]=sizex/2;
	if(mval[1]<sizey/2) mval[1]=sizey/2;
	if(mval[0]>G.curscreen->sizex -sizex/2) mval[0]= G.curscreen->sizex -sizex/2;
	if(mval[1]>G.curscreen->sizey -sizey/2) mval[1]= G.curscreen->sizey -sizey/2;

	winset(G.curscreen->mainwin);
	
	x1= mval[0]-sizex/2; 
	x2= mval[0]+sizex/2; 
	y1= mval[1]-sizey/2; 
	y2= mval[1]+sizey/2;
	
	block= uiNewBlock(&listb, "numbuts", UI_EMBOSSX, UI_HELV, 0x808080, G.curscreen->mainwin);
	block->flag= UI_BLOCK_LOOP|UI_BLOCK_REDRAW|UI_BLOCK_RET_1;
	block->aspect= 1.0;
	uiSetCurFont(block, block->font);
	
	/* LET OP: TEX BUTTON UITZONDERING */
	/* WAARSCHUWING: ALLEEN EEN ENKELE BITJES-BUTTON MOGELIJK: ER WORDT OP KOPIEDATA GEWERKT! */

	uiDefBut(block, LABEL, 0, name,	x1+15, y2-35, sizex-60, 19, 0, 1.0, 0.0, 0, 0, ""); 

	if(name[0]=='A' && name[7]=='O') {
		y2 -= 20;
		uiDefBut(block, LABEL, 0, "Rotations in degrees!",	x1+15, y2-35, sizex-60, 19, 0, 0.0, 0.0, 0, 0, "");
	}
	
	varstr= &numbuts[0];
	for(a=0; a<tot; a++, varstr++) {
		if(varstr->type==TEX) {
			uiDefBut(block, TEX, 0,	varstr->name,	x1+15, y2-55-20*a, sizex-60, 19, numbpoin[a], varstr->min, varstr->max, 0, 0, varstr->tip);
		}
		else  {
			uiDefBut(block, varstr->type, 0,	varstr->name,	x1+15, y2-55-20*a, sizex-60, 19, &(numbdata[a]), varstr->min, varstr->max, 100, 0, varstr->tip);
		}
	}

	uiDefBut(block, BUT, 4000, "OK", x1+sizex-40, y2-35-20*a, 25, sizey-50, 0, 0, 0, 0, 0, "OK: Assign Values");

	ui_boundsblock(block, 5);

	event= uiDoBlocks(&listb, 0);

	areawinset(curarea->win);
	
	if(event & UI_RETURN_OK) {
		
		varstr= &numbuts[0];
		for(a=0; a<tot; a++, varstr++) {
			if(varstr->type==TEX);
			else if ELEM( (varstr->type & BUTPOIN), FLO, INT ) memcpy(numbpoin[a], numbdata+a, 4);
			else if((varstr->type & BUTPOIN)==SHO ) *((short *)(numbpoin[a]))= *( (short *)(numbdata+a));
			
			if( strncmp(varstr->name, "Rot", 3)==0 ) {
				float *fp;
				
				fp= numbpoin[a];
				fp[0]= M_PI*fp[0]/180.0;
			}
		}
		
		if(winevent) {
			ScrArea *sa;
		
			sa= G.curscreen->areabase.first;
			while(sa) {
				if(sa->spacetype==curarea->spacetype) addqueue(sa->win, winevent, 1);
				sa= sa->next;
			}
		}
		
		return 1;
	}
	return 0;
}

void add_numbut(int nr, int type, char *str, float min, float max, void *poin, char *tip)
{
	if(nr>=MAXNUMBUTS) return;

	numbuts[nr].type= type;
	strcpy(numbuts[nr].name, str);
	numbuts[nr].min= min;
	numbuts[nr].max= max;
	if(tip) strcpy(numbuts[nr].tip, tip);
	
	/* LET OP: TEX BUTTON UITZONDERING */
	
	numbpoin[nr]= poin;
	
	if ELEM( (type & BUTPOIN), FLO, INT ) memcpy(numbdata+nr, poin, 4);
	if((type & BUTPOIN)==SHO ) *((short *)(numbdata+nr))= *( (short *)poin);
	
	if( strncmp(numbuts[nr].name, "Rot", 3)==0 ) {
		float *fp;
		
		fp= (float *)(numbdata+nr);
		fp[0]= 180.0*fp[0]/M_PI;
	}

}

void clever_numbuts()
{
	Object *ob;
	float lim;
	char str[128];
	
	if(curarea->spacetype==SPACE_VIEW3D) {
		lim= 1000.0*MAX2(1.0, G.vd->grid);

		if(G.obedit==0) {
			ob= OBACT;
			if(ob==0) return;
			
			add_numbut(0, NUM|FLO, "LocX:", -lim, lim, ob->loc, 0);
			add_numbut(1, NUM|FLO, "LocY:", -lim, lim, ob->loc+1, 0);
			add_numbut(2, NUM|FLO, "LocZ:", -lim, lim, ob->loc+2, 0);
			
			add_numbut(3, NUM|FLO, "RotX:", -10.0*lim, 10.0*lim, ob->rot, 0);
			add_numbut(4, NUM|FLO, "RotY:", -10.0*lim, 10.0*lim, ob->rot+1, 0);
			add_numbut(5, NUM|FLO, "RotZ:", -10.0*lim, 10.0*lim, ob->rot+2, 0);
			
			add_numbut(6, NUM|FLO, "SizeX:", -lim, lim, ob->size, 0);
			add_numbut(7, NUM|FLO, "SizeY:", -lim, lim, ob->size+1, 0);
			add_numbut(8, NUM|FLO, "SizeZ:", -lim, lim, ob->size+2, 0);
			
			sprintf(str, "Active Object: %s", ob->id.name+2);
			do_clever_numbuts(str, 9, REDRAW);
		}
		else if(G.obedit->type==OB_MESH) clever_numbuts_mesh();
		else if ELEM(G.obedit->type, OB_CURVE, OB_SURF) clever_numbuts_curve();
	}
	else if(curarea->spacetype==SPACE_IPO) {
		clever_numbuts_ipo();
	}
	else if(curarea->spacetype==SPACE_SEQ) {
		clever_numbuts_seq();
	}
	
	
}


void replace_names_but()
{
	Image *ima= G.main->image.first;
	short len, tot=0;
	char old[64], new[64], temp[80];
	
	strcpy(old, "/");
	strcpy(new, "/");
	
	add_numbut(0, TEX, "Old:", 0, 63, old, 0);
	add_numbut(1, TEX, "New:", 0, 63, new, 0);

	if (do_clever_numbuts("Replace image name", 2, REDRAW) ) {
		
		len= strlen(old);
		
		while(ima) {
			
			if(strncmp(old, ima->name, len)==0) {
				
				strcpy(temp, new);
				strcat(temp, ima->name+len);
				strncpy(ima->name, temp, 79);
				
				if(ima->ibuf) freeImBuf(ima->ibuf);
				ima->ibuf= 0;
				ima->ok= 1;
				
				tot++;
			}
			
			ima= ima->id.next;
		}

		sprintf(temp, "Replaced %d names", tot);
		notice(temp);
	}
	
}

