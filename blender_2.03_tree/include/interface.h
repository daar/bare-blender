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
 * Version: $Id: interface.h,v 1.16 2000/08/13 19:22:38 ton Exp $
 */


#ifndef INTERFACE_H
#define INTERFACE_H

/* general defines */

#define UI_MAX_DRAW_STR	180
#define UI_MAX_NAME_STR	64
#define UI_ARRAY	20

/* block->font, for now: bold = medium+1 */
#define UI_HELV			0
#define UI_HELVB		1


#define BUTBW			0
#define BUTGREY			1
#define BUTGREEN		2
#define BUTBLUE			3
#define BUTSALMON		4
#define MIDGREY			5
#define BUTPURPLE		6
#define BUTYELLOW		7
#define REDALERT		8
#define BUTRUST			9
#define BUTWHITE		10
#define BUTDBLUE		11

/* return from uiDoBlock */
#define UI_CONT				0
#define UI_NOTHING			1
#define UI_RETURN_CANCEL	2
#define UI_RETURN_OK		4
#define UI_RETURN_OUT		8
#define UI_RETURN			14

/* uiBut->flag */
#define UI_SELECT		1
#define UI_MOUSE_OVER	2
#define UI_ACTIVE		4
#define UI_HAS_ICON		8
#define UI_TEXT_LEFT	16
#define UI_NO_BACK		32

/* uiBlock->flag */
#define UI_BLOCK_LOOP		1
#define UI_BLOCK_REDRAW		2
#define UI_BLOCK_RET_1		4
#define UI_BLOCK_BUSY		8
#define UI_BLOCK_NUMSELECT	16
#define UI_BLOCK_ENTER_OK	32
#define UI_BLOCK_TEST_ACTIVE	64

/* uiBlock->dt */
#define UI_EMBOSSX		0
#define UI_EMBOSSW		1
#define UI_EMBOSSN		2
#define UI_EMBOSSF		3
#define UI_EMBOSSM		4

/* uiBlock->direction */
#define UI_TOP		0
#define UI_DOWN		1
#define UI_LEFT		2
#define UI_RIGHT	3

/* uiBlock->autofill */
#define UI_BLOCK_COLLUMNS	1
#define UI_BLOCK_ROWS		2

typedef struct uiEvent {
	short mval[2];
	short qual, val;
	int event;
} uiEvent;

typedef struct uiIconImage {
	short xim, yim;
	unsigned int *rect;
	short xofs, yofs;
} uiIconImage;


typedef struct uiCol
{
	uint medium, pen, pen_sel;
	uint white, light, hilite, grey, dark;
} uiCol;


typedef struct uiFont
{
	void *xl, *large, *medium, *small;
} uiFont;


typedef struct uiLinkLine {				/* only for draw/edit */
	struct uiLinkLine *next, *prev;

	short flag, pad;
	
	struct uiBut *from, *to;	
} uiLinkLine;

typedef struct uiLink
{
	void **poin;		/* pointer to original pointer */
	void ***ppoin;		/* pointer to original pointer-array */
	short *totlink;		/* if pointer-array, here is the total */
	
	short maxlink, pad;
	short fromcode, tocode;
	
	ListBase lines;

} uiLink;

typedef struct uiBut
{
	struct uiBut *next, *prev;
	short type, pointype, bit, bitnr, retval, flag, strwidth;
	short ofs, pos, pad;
	uint paper;
	
	char *str;
	char strdata[UI_MAX_NAME_STR];
	char drawstr[UI_MAX_DRAW_STR];
	
	float x1, y1, x2, y2;

	char *poin;
	float min, max;
	float a1, a2, rt[4];
	float aspect;
	void (*func)();
	void (*butfunc)(struct uiBut *);
	struct uiBlock *(*blockfunc)();
	void (*embossfunc)(uiCol *, float, float, float, float, float, int);

	uiLink *link;
	
	char *tip, *lockstr;

	uiCol *col;
	void *font;
	uiIconImage *icon;

	short lock, win;
	short iconxadd, iconx, icony;
	
} uiBut ;


typedef struct uiBlock {
	struct uiBlock *next, *prev;
	
	ListBase buttons;
	
	char name[UI_MAX_NAME_STR];
	
	float winmat[4][4];
	
	float minx, miny, maxx, maxy;
	float aspect;
	void (*func)();
	
	short col, font;	/* indices */
	void *curfont;
	
	short autofill, flag, win, winq, direction, dt;
	uint paper;
	
} uiBlock;



extern void uiInit();
extern void uiAutoBlock(uiBlock *block, float minx, float miny, float maxx, float maxy, int flag);
extern void uiDefFont(uint index, void *xl, void *large, void *medium, void *small);
extern void uiDrawBlock(uiBlock *uib);
extern void uiDefCol(uint index, uint medium, uint pen, uint pen_sel);
extern uiBlock *uiNewBlock(ListBase *lb, char *name, short dt, short font, uint paper, short win);
extern void uiFreeBlocks(ListBase *lb);
extern void uiFreeBut(uiBut *but);
extern void uiFreeBlock(uiBlock *block);
extern uiBut *uiDefBut(uiBlock *block, int retval, int nr, char *str, short x1, short y1, short x2, short y2, void *poin, float min, float max, float a1, float a2,  char *tip);
extern void uiSetCurFont(uiBlock *block, int index);
extern double uiGetButVal(uiBut *but);
extern int uiDoBlocks(ListBase *lb, int event);
extern void uiIsButSel(uiBut *but);
extern void uiSetButVal(uiBut *but, double value);
extern void uiCheckBut(uiBut *but);
extern void uiGetMouse(short *adr);
extern void uiSetButLock(int val, char *lockstr);
extern void uiClearButLock();
extern void uiDrawMenuBox(uint paper, float minx, float miny, float maxx, float maxy);
extern void uiSetButLink(uiBut *but, void **poin, void ***ppoin, short *tot, int from, int to);
extern void uiComposeLinks(uiBlock *block);
extern void uiEmbossX(uiCol *bc, float asp, float x1, float y1, float x2, float y2, int sel);
extern void uiEmbossW(uiCol *bc, float asp, float x1, float y1, float x2, float y2, int sel);
extern void uiEmboss(float x1, float y1, float x2, float y2, int sel);
extern void uiGetNameMenu(char *name, char *menu, int value);
extern uiBlock *uiGetBlock(char *name, ScrArea *sa);


#endif

