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

#ifndef __MYDEVICE_H__

#define __MYDEVICE_H__

/* standard glut defines plus some extra */

/*
 * 
 *   mouse / timer / window: tot 0x020
 *   eigen codes: 0x4...
 * 
 * Version: $Id: mydevice.h,v 1.3 2000/07/25 08:53:07 nzc Exp $
 */



/* MOUSE : 0x00x */

#define LEFTMOUSE	0x001	
#define MIDDLEMOUSE	0x002	
#define RIGHTMOUSE	0x003	
#define MOUSEX		0x004	
#define MOUSEY		0x005	

/* timers */

#define TIMER0		0x006	
#define TIMER1		0x007	
#define TIMER2		0x008	
#define TIMER3		0x009	

/* SYSTEM : 0x01x */

#define KEYBD			0x010	/* keyboard */
#define RAWKEYBD		0x011	/* raw keyboard for keyboard manager */
#define REDRAW			0x012	/* used by port manager to signal redraws */
#define	INPUTCHANGE		0x013	/* input connected or disconnected */
#define	QFULL			0x014	/* queue was filled */
#define WINFREEZE		0x015	/* user wants process in this win to shut up */
#define WINTHAW			0x016	/* user wants process in this win to go again */
#define WINCLOSE		0x017	/* window close */
#define WINQUIT			0x018	/* signal from user that app is to go away */
#define Q_FIRSTTIME		0x019	/* on startup */

/* standard keyboard */

#define AKEY		'a'
#define BKEY		'b'
#define CKEY		'c'
#define DKEY		'd'
#define EKEY		'e'
#define FKEY		'f'
#define GKEY		'g'
#define HKEY		'h'
#define IKEY		'i'
#define JKEY		'j'
#define KKEY		'k'
#define LKEY		'l'
#define MKEY		'm'
#define NKEY		'n'
#define OKEY		'o'
#define PKEY		'p'
#define QKEY		'q'
#define RKEY		'r'
#define SKEY		's'
#define TKEY		't'
#define UKEY		'u'
#define VKEY		'v'
#define WKEY		'w'
#define XKEY		'x'
#define YKEY		'y'
#define ZKEY		'z'

#define ZEROKEY		'0'
#define ONEKEY		'1'
#define TWOKEY		'2'
#define THREEKEY	'3'
#define FOURKEY		'4'
#define FIVEKEY		'5'
#define SIXKEY		'6'
#define SEVENKEY	'7'
#define EIGHTKEY	'8'
#define NINEKEY		'9'

#define CAPSLOCKKEY		211

#define LEFTCTRLKEY		212
#define LEFTALTKEY 		213
#define	RIGHTALTKEY 	214
#define	RIGHTCTRLKEY 	215
#define RIGHTSHIFTKEY	216
#define LEFTSHIFTKEY	217

#define ESCKEY			218
#define TABKEY			219
#define RETKEY			220
#define SPACEKEY		221
#define LINEFEEDKEY		222
#define BACKSPACEKEY	223
#define DELKEY			224
#define SEMICOLONKEY	225
#define PERIODKEY		226
#define COMMAKEY		227
#define QUOTEKEY		228
#define ACCENTGRAVEKEY	229
#define MINUSKEY		230
#define VIRGULEKEY		231
#define SLASHKEY		232
#define BACKSLASHKEY	233
#define EQUALKEY		234
#define LEFTBRACKETKEY	235
#define RIGHTBRACKETKEY	236

#define LEFTARROWKEY	137
#define DOWNARROWKEY	138
#define RIGHTARROWKEY	139
#define UPARROWKEY		140

#define PAD2		150
#define PAD4		151
#define PAD6		152
#define PAD8		153

#define PAD1		154
#define PAD3		155
#define PAD5		156
#define PAD7		157
#define PAD9		158

#define PADPERIOD		199
#define	PADVIRGULEKEY 	159
#define PADASTERKEY 	160


#define PAD0		161
#define PADMINUS		162
#define PADENTER		163
#define PADPLUSKEY 		164


#define	F1KEY 		300
#define	F2KEY 		301
#define	F3KEY 		302
#define	F4KEY 		303
#define	F5KEY 		304
#define	F6KEY 		305
#define	F7KEY 		306
#define	F8KEY 		307
#define	F9KEY 		308
#define	F10KEY		309
#define	F11KEY		310
#define	F12KEY		312

#define	PAUSEKEY	165
#define	INSERTKEY	166
#define	HOMEKEY 	167
#define	PAGEUPKEY 	168
#define	PAGEDOWNKEY	169
#define	ENDKEY		170

#endif	/* !__MYDEVICE_H__ */

