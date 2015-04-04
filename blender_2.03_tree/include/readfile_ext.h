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

/*

 * readfile.c
 *
 * Version: $Id: readfile_ext.h,v 1.2 2000/08/25 15:27:33 nzc Exp $
 */

#ifndef READFILE_EXT_H
#define READFILE_EXT_H "$Id: readfile_ext.h,v 1.2 2000/08/25 15:27:33 nzc Exp $"

#include "blender.h"

/* unsorted */

extern	  char* gethome(void);
extern    void add_data_adr(void *, void *);
extern    void add_lib_adr(void *, void *);
extern    void change_libadr(void *, void *);
extern    int convertstringcode(char *str);
extern    void inst_file(char *filename, char *data, int size);
extern    void makestringcode(char *str);
extern    void *mallocNN(int len, char *str);
extern    void *newadr(void *adr);		/* alleen direkte datablokken */
extern    void *newlibadr(void *lib, void *adr);		/* alleen Lib datablokken */
extern    void *newlibadr_us(void *lib, void *adr);		/* hoogt usernummer op */
extern    void *newlibadr_us_type(short type, void *adr);		/* alleen Lib datablokken */
extern    char *openblenderfile(char *name, int *filelen);
extern    void read_autosavefile(void);
extern    void read_file(char *dir);
extern    int read_file_dna(char *filedata, int filelen);
extern    int read_homefile(void);
extern    void read_libraries(void);

void splitdirstring(char *di,char *fi);

extern    void switch_endian_bheads(char *filedata, int filelen);
extern    int testextensie(char *str, char *ext);
extern    void vcol_to_fcol(Mesh *me);

#endif /* READFILE_EXT_H */








