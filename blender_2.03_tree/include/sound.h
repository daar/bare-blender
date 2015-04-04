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

/* game.h    july 2000

 * 
 * 
 * Version: $Id: sound.h,v 1.6 2000/08/09 16:01:50 frank Exp $
 */

#ifndef SOUND_H
#define SOUND_H






/* ****************************************************** */

#define SND_WAV		0;
#define SND_AIFF	1;


typedef struct bSound {
	ID id;
	short type, bits;
	short channels, pad;
	char name[160];	
	void *data;
	int len, rate;
	PackedFile * packedfile;
	unsigned int alindex; /* officialy Aluint */
	int pad2;
} bSound;


typedef struct SpaceSound {
	struct SpaceSound *next, *prev;
	int spacetype, pad;
	View2D v2d;
	
	bSound *sound;
	short mode, sndnr;
	short xof, yof;
	short flag, lock;
	int pad2;
} SpaceSound;




/* sound.c */
extern void free_sound(bSound *sound);
extern bSound *add_sound(char *name);


#endif /* SOUND_H */

