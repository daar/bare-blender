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

/* Version: $Id: packedFile.h,v 1.2 2000/07/25 08:53:07 nzc Exp $ */


#ifndef PACKEDFILE_H
#define PACKEDFILE_H

#include "blender.h"
#include "file.h"

extern void freePackedFile(PackedFile * pf);
extern PackedFile * newPackedFile(char * filename);
extern void packAll();
extern void unpackAll();
// extern int checkPackedFile(char * filename, PackedFile * pf);
//extern char * unpackFile(char * abs_name, char * local_name, PackedFile * pf, int choice);
extern int unpackVFont(VFont *, int how);
extern int unpackImage(Image *, int how);

enum PF_FileStatus
{
	PF_EQUAL = 0,
	PF_DIFFERS,
	PF_NOFILE,
			
	PF_WRITE_ORIGINAL,
	PF_WRITE_LOCAL,
	PF_USE_LOCAL,
	PF_USE_ORIGINAL,
	PF_KEEP,
	PF_NOOP,
			
	PF_ASK
};


#endif /* PACKEDFILE_H */

