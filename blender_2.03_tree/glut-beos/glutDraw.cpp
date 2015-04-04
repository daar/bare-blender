/***********************************************************
 *	Copyright (C) 1997, Be Inc.  All rights reserved.
 *
 *  FILE:	glutDraw.cpp
 *
 *	DESCRIPTION:	here it is, the BeOS GLUT event loop
 ***********************************************************/

/***********************************************************
 *	Headers
 ***********************************************************/
#include <stdio.h>
#include <GL/glut.h>
#include "glutint.h"
#include "glutState.h"
#include "glutBlocker.h"

void glutDrawInit(void)
{

}

static void sys_coord(int *x, int *y) 
{
	*y= gState.currentWindow->m_height - *y;
	
	gState.currentWindow->Window()->Lock();
	*y= *y - (int)gState.currentWindow->Frame().top;
	gState.currentWindow->Window()->Unlock();
}

void glutInvertLine(int x1, int y1, int x2, int y2)
{
	sys_coord(&x1, &y1);
	sys_coord(&x2, &y2);

	if(gState.currentWindow) {
		BPoint p1, p2;

		gState.currentWindow->Window()->Lock();
		gState.currentWindow->SetDrawingMode(B_OP_INVERT);
		p1.x= x1; p1.y= y1;
		p2.x= x2; p2.y= y2;
		gState.currentWindow->StrokeLine(p1, p2, B_SOLID_HIGH);
		
		gState.currentWindow->SetDrawingMode(B_OP_COPY);
		gState.currentWindow->Window()->Unlock();
	}
}

void glutDrawLine(int x1, int y1, int x2, int y2)
{
	sys_coord(&x1, &y1);
	sys_coord(&x2, &y2);
	
	if(gState.currentWindow) {
		BPoint p1, p2;

		p1.x= x1; p1.y= y1;
		p2.x= x2; p2.y= y2;

		gState.currentWindow->Window()->Lock();
		gState.currentWindow->StrokeLine(p1, p2, B_SOLID_HIGH);	
		gState.currentWindow->Window()->Unlock();
	}
}

void glutDrawRectFilled(int x1, int y1, int x2, int y2)
{
	sys_coord(&x1, &y1);
	sys_coord(&x2, &y2);

	if(gState.currentWindow) {
		BRect rt(x1, y1, x2, y2);
		
		if(rt.left > rt.right) {
			rt.left= x2; rt.right= x1;
		}
		if(rt.top > rt.bottom) {
			rt.top= y2; rt.bottom= y1;
		}
		
		gState.currentWindow->Window()->Lock();
		gState.currentWindow->FillRect(rt, B_SOLID_HIGH);
		gState.currentWindow->Window()->Unlock();
	}
}

void glutDrawRect(int x1, int y1, int x2, int y2)
{
	sys_coord(&x1, &y1);
	sys_coord(&x2, &y2);

	if(gState.currentWindow) {
		BRect rt(x1,y1,x2,y2);

		if(rt.left > rt.right) {
			rt.left= x2; rt.right= x1;
		}
		if(rt.top > rt.bottom) {
			rt.top= y2; rt.bottom= y1;
		}

		gState.currentWindow->Window()->Lock();
		gState.currentWindow->StrokeRect(rt, B_SOLID_HIGH);
		gState.currentWindow->Window()->Unlock();
	}
}

static void __glutDrawSetColor(unsigned char a, unsigned char b, unsigned char c)
{
	if(gState.currentWindow) {
		gState.currentWindow->Window()->Lock();
		gState.currentWindow->SetHighColor(a, b, c, 0);
		gState.currentWindow->Window()->Unlock();
	}
}

void glutDrawSetColor(int col)
{
	if(col==GLUTCOLWHITE) __glutDrawSetColor(240, 240, 240);
	else if(col==GLUTCOLDGREY) __glutDrawSetColor(80, 80, 80);
	else if(col==GLUTCOLLGREY) __glutDrawSetColor(210, 210, 210);
	else if(col==GLUTCOLGREY) __glutDrawSetColor(160, 160, 160);
	else if(col==GLUTCOLBLACK) __glutDrawSetColor(0, 0, 0);
	else printf ("ERROR: Bad color code passed to DrawSetColor\n");
}

void glutDrawUpdate(void)
{

}
