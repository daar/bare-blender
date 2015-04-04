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

/* Version: $Id: packedFile.c,v 1.9 2000/08/28 12:23:46 nzc Exp $ */


#include "blender.h"
#include "file.h"
#include "sound.h"
#include "packedFile.h"
#include "render.h"

int seekPackedFile(PackedFile * pf, int offset, int whence)
{
	int oldseek = -1, seek;

	if (pf) {
		oldseek = pf->seek;
		switch(whence) {
		case SEEK_CUR:
			seek = oldseek + offset;
			break;
		case SEEK_END:
			seek = pf->size + offset;
			break;
		case SEEK_SET:
			seek = offset;
			break;
		default:
			oldseek = -1;
		}
		if (seek < 0) {
			seek = 0;
		} else if (seek > pf->size) {
			seek = pf->size;
		}
		pf->seek = seek;
	}

	return(oldseek);
}
	
void rewindPackedFile(PackedFile * pf)
{
	seekPackedFile(pf, 0, SEEK_SET);
}

int readPackedFile(PackedFile * pf, void * data, int size)
{ 
	if ((pf != NULL) && (size >= 0) && (data != NULL)) {
		if (size + pf->seek > pf->size) {
			size = pf->size - pf->seek;
		}

		if (size > 0) {
			memcpy(data, ((char *) pf->data) + pf->seek, size);
		} else {
			size = 0;
		}

		pf->seek += size;
	} else {
		size = -1;
	}

	return(size);
}

int countPackedFiles()
{
	int count = 0;
	Image * ima;
	VFont * vf;
	bSound * bs;
	
	// let's check if there are packed files...
	ima = G.main->image.first;
	while (ima) {
		if (ima->packedfile) {
			count++;
		}
		ima= ima->id.next;
	}

	vf = G.main->vfont.first;
	while (vf) {
		if (vf->packedfile) {
			count++;
		}
		vf = vf->id.next;
	}

	bs = G.main->sound.first;
	while (bs) {
		if (bs->packedfile) {
			count++;
		}
		bs = bs->id.next;
	}

	return(count);
}

void freePackedFile(PackedFile * pf)
{
	if (pf) {
		freeN(pf->data);
		freeN(pf);
	} else {
		printf("freePackedFile: Trying to free a NULL pointer\n");
	}
}
	
PackedFile * newPackedFile(char * filename)
{
	PackedFile * pf = NULL;
	int file, filelen;
	char name[FILE_MAXDIR+FILE_MAXFILE];
	void * data;
	
	waitcursor(1);
	
	// convert relative filenames to absolute filenames
	
	strcpy(name, filename);
	convertstringcode(name);
	
	// open the file
	// and create a PackedFile structure

	file= open(name, O_BINARY|O_RDONLY);
	if (file <= 0) {
		// errorstr("Can't open file", name, 0);
	} else {
		filelen = filesize(file);
		pf = CLN(PackedFile);

		if (filelen == 0) {
			// mallocN complains about mallocN(0, "bla");
			// we don't care....
			data = mallocN(1, "packFile");
		} else {
			data = mallocN(filelen, "packFile");
		}
		
		// initialize pf
		
		pf->size = filelen;
		pf->data = data;
		
		// read the data
		
		if (read(file, data, filelen) != filelen) {
			// errorstr("Can't read file", name, 0);
			freePackedFile(pf);
			pf = NULL;
		}
		close(file);
	}

	waitcursor(0);
		
	return (pf);
}

void packAll()
{
	Image *ima;
	VFont *vf;
	bSound *bs;
	
	ima = G.main->image.first;
	while (ima) {
		if (ima->packedfile == NULL) {
			ima->packedfile = newPackedFile(ima->name);
		}
		ima= ima->id.next;
	}
	
	vf = G.main->vfont.first;
	while (vf) {
		if (vf->packedfile == NULL) {
			vf->packedfile = newPackedFile(vf->name);
		}
		vf = vf->id.next;
	}


	bs = G.main->sound.first;
	while (bs) {
		if (bs->packedfile == NULL) {
			bs->packedfile = newPackedFile(bs->name);
		}
		bs = bs->id.next;
	}

	allqueue(REDRAWINFO, 0);
}


/*

// attempt to create a function that generates an unique filename
// this will work when all funtions in fileops.c understand relative filenames...

char * find_new_name(char * name)
{
	char tempname[FILE_MAXDIR + FILE_MAXFILE];
	char * newname;
	
	if (fop_exists(name)) {
		for (number = 1; number <= 999; number++) {
			sprintf(tempname, "%s.%03d", name, number);
			if (! fop_exists(tempname)) {
				break;
			}
		}
	}
	
	newname = mallocN(strlen(tempname) + 1, "find_new_name");
	strcpy(newname, tempname);
	
	return(newname);
}
	
*/

int writePackedFile(char * filename, PackedFile *pf)
{
	int file, number, remove_tmp = FALSE;
	int ret_value = RET_OK;
	char name[FILE_MAXDIR + FILE_MAXFILE];
	char tempname[FILE_MAXDIR + FILE_MAXFILE];
	void * data;
	
	waitcursor(1);
	
	strcpy(name, filename);
	convertstringcode(name);
	
	if (fop_exists(name)) {
		for (number = 1; number <= 999; number++) {
			sprintf(tempname, "%s.%03d_", name, number);
			if (! fop_exists(tempname)) {
				if (fop_copy(name, tempname) == RET_OK) {
					remove_tmp = TRUE;
				}
				break;
			}
		}
	}
	
	// make sure the path to the file exists...
	RE_make_existing_file(name);
	
	file = open(name, O_BINARY + O_WRONLY + O_CREAT + O_TRUNC, 0666);
	if (file >= 0) {
		if (write(file, pf->data, pf->size) != pf->size) {
			errorstr("Error writing file:", name, 0);
			ret_value = RET_ERROR;
		}
		close(file);
	} else {
		errorstr("Error creating file:", name, 0);
		ret_value = RET_ERROR;
	}
	
	if (remove_tmp) {
		if (ret_value == RET_ERROR) {
			if (fop_rename(tempname, name) == RET_ERROR) {
				errorstr("Error restoring tempfile. Check files:", tempname, name);
			}
		} else {
			if (fop_delete(tempname, 0, 0) == RET_ERROR) {
				errorstr("Error deleting", tempname, "(ignored)");
			}
		}
	}
	
	waitcursor(0);

	return (ret_value);
}
	
/* 

This function compares a packed file to a 'real' file.
It returns an integer indicating if:

PF_EQUAL		- the packed file and original file are identical
PF_DIFFERENT	- the packed file and original file differ
PF_NOFILE		- the original file doens't exist

*/

int checkPackedFile(char * filename, PackedFile * pf)
{
	struct stat st;
	int ret_val, i, len, file;
	char buf[4096];
	char name[FILE_MAXDIR + FILE_MAXFILE];
	void * data;
	
	strcpy(name, filename);
	convertstringcode(name);
	
	if (stat(name, &st)) {
		ret_val = PF_NOFILE;
	} else if (st.st_size != pf->size) {
		ret_val = PF_DIFFERS;
	} else {
		// we'll have to compare the two...
		
		file = open(name, O_BINARY | O_RDONLY);
		if (file < 0) {
			ret_val = PF_NOFILE;
		} else {
			ret_val = PF_EQUAL;
			
			for (i = 0; i < pf->size; i += sizeof(buf)) {
				len = pf->size - i;
				if (len > sizeof(buf)) {
					len = sizeof(buf);
				}
				
				if (read(file, buf, len) != len) {
					// read error ...
					ret_val = PF_DIFFERS;
					break;
				} else {
					if (memcmp(buf, ((char *)pf->data) + i, len)) {
						ret_val = PF_DIFFERS;
						break;
					}
				}
			}
		}
	}
	
	return(ret_val);
}

/*

unpackFile() looks at the existing files (abs_name, local_name) and a packed file.
If how == PF_ASK it offers the user a couple of options what to do with the packed file.

It returns a char * to the existing file name / new file name or NULL when
there was an error or when the user desides to cancel the operation.

*/

char * unpackFile(char * abs_name, char * local_name, PackedFile * pf, int how)
{
	char menu[6 * (FILE_MAXDIR + FILE_MAXFILE + 100)];
	char line[FILE_MAXDIR + FILE_MAXFILE + 100];
	char * newname = NULL, * temp = NULL;
	
	// char newabs[FILE_MAXDIR + FILE_MAXFILE];
	// char newlocal[FILE_MAXDIR + FILE_MAXFILE];
	
	if (pf != NULL) {
		if (how == PF_ASK) {
			strcpy(menu, "UnPack file%t");
			
			if (strcmp(abs_name, local_name)) {
				switch (checkPackedFile(local_name, pf)) {
					case PF_NOFILE:
						sprintf(line, "|Create %s%%x%d", local_name, PF_WRITE_LOCAL);
						strcat(menu, line);
						break;
					case PF_EQUAL:
						sprintf(line, "|Use %s (identical)%%x%d", local_name, PF_USE_LOCAL);
						strcat(menu, line);
						break;
					case PF_DIFFERS:
						sprintf(line, "|Use %s (differs)%%x%d", local_name, PF_USE_LOCAL);
						strcat(menu, line);
						sprintf(line, "|Overwrite %s%%x%d", local_name, PF_WRITE_LOCAL);
						strcat(menu, line);
						break;
				}
				// sprintf(line, "|%%x%d", PF_INVALID);
				// strcat(menu, line);
			}
			
			switch (checkPackedFile(abs_name, pf)) {
				case PF_NOFILE:
					sprintf(line, "|Create %s%%x%d", abs_name, PF_WRITE_ORIGINAL);
					strcat(menu, line);
					break;
				case PF_EQUAL:
					sprintf(line, "|Use %s (identical)%%x%d", abs_name, PF_USE_ORIGINAL);
					strcat(menu, line);
					break;
				case PF_DIFFERS:
					sprintf(line, "|Use %s (differs)%%x%d", abs_name, PF_USE_ORIGINAL);
					strcat(menu, line);
					sprintf(line, "|Overwrite %s%%x%d", abs_name, PF_WRITE_ORIGINAL);
					strcat(menu, line);
					break;
			}
			
			how = pupmenu(menu);
		}
		
		switch (how) {
			case -1:
			case PF_KEEP:
				break;
			case PF_USE_LOCAL:
				// if file exists use it
				if (fop_exists(local_name)) {
					temp = local_name;
					break;
				}
				// else fall through and create it
			case PF_WRITE_LOCAL:
				if (writePackedFile(local_name, pf) == RET_OK) {
					temp = local_name;
				}
				break;
			case PF_USE_ORIGINAL:
				// if file exists use it
				if (fop_exists(abs_name)) {
					temp = abs_name;
					break;
				}
				// else fall through and create it
			case PF_WRITE_ORIGINAL:
				if (writePackedFile(abs_name, pf) == RET_OK) {
					temp = abs_name;
				}
				break;
			default:
				printf("unpackFile: unknown return_value %d\n", how);
				break;
		}
		
		if (temp) {
			newname = mallocN(strlen(temp) + 1, "unpack_file newname");
			strcpy(newname, temp);
		}
	}
	
	return (newname);
}


int unpackVFont(VFont * vfont, int how)
{
	char localname[FILE_MAXDIR + FILE_MAXFILE], fi[FILE_MAXFILE];
	char * newname;
	int ret_value = RET_ERROR;
	
	if (vfont != NULL) {
		strcpy(localname, vfont->name);
		splitdirstring(localname, fi);
		
		sprintf(localname, "//fonts/%s", fi);
		
		newname = unpackFile(vfont->name, localname, vfont->packedfile, how);
		if (newname != NULL) {
			ret_value = RET_OK;
			freePackedFile(vfont->packedfile);
			vfont->packedfile = 0;
			strcpy(vfont->name, newname);
			freeN(newname);
			reload_vfont(vfont);
		}
	}
	
	return (ret_value);
}

int unpackSound(bSound * bs, int how)
{
	char localname[FILE_MAXDIR + FILE_MAXFILE];
	char * newname;
	int ret_value = RET_ERROR;
	
	if (bs != NULL) {
		sprintf(localname, "//sounds/%s", bs->id.name + 2);
		
		newname = unpackFile(bs->name, localname, bs->packedfile, how);
		if (newname != NULL) {
			ret_value = RET_OK;
			freePackedFile(bs->packedfile);
			bs->packedfile = 0;
			strcpy(bs->name, newname);
			freeN(newname);
		}
	}
	
	return(ret_value);
}

int unpackImage(Image * ima, int how)
{
	char localname[FILE_MAXDIR + FILE_MAXFILE];
	char * newname;
	int ret_value = RET_ERROR;
	
	if (ima != NULL) {
		sprintf(localname, "//textures/%s", ima->id.name + 2);
		
		newname = unpackFile(ima->name, localname, ima->packedfile, how);
		if (newname != NULL) {
			ret_value = RET_OK;
			freePackedFile(ima->packedfile);
			ima->packedfile = 0;
			strcpy(ima->name, newname);
			freeN(newname);
			free_image_buffers(ima);
		}
	}
	
	return(ret_value);
}

void unpackAll(int how)
{
	Image *ima;
	VFont *vf;
	bSound *bs;
		
	ima = G.main->image.first;
	while (ima) {
		if (ima->packedfile) {
			unpackImage(ima, how);
		}
		ima= ima->id.next;
	}
	
	vf = G.main->vfont.first;
	while (vf) {
		if (vf->packedfile) {
			unpackVFont(vf, how);
		}
		vf = vf->id.next;
	}

	bs = G.main->sound.first;
	while (bs) {
		if (bs->packedfile) {
			unpackSound(bs, how);
		}
		bs = bs->id.next;
	}
}

