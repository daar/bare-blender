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

 * mywindow_ext.h
 * external interface for mywindow.c
 *
 * Version: $Id: mywindow_ext.h,v 1.1 2000/07/03 10:16:29 nzc Exp $
 */

#ifndef MYWINDOW_EXT_H
#define MYWINDOW_EXT_H "$Id: mywindow_ext.h,v 1.1 2000/07/03 10:16:29 nzc Exp $"

/* these two only used in initrender.c */
int mywin_inmenu(void);
void mywin_getmenu_rect(int *x, int *y, int *sx, int *sy);

#endif /* MYWINDOW_EXT_H */


