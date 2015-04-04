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

 * drawview_ext.h
 * external interface for drawview.c
 *
 * Version: $Id: drawview_ext.h,v 1.2 2000/07/20 13:09:08 nzc Exp $
 */

#ifndef DRAWVIEW_EXT_H
#define DRAWVIEW_EXT_H "$Id: drawview_ext.h,v 1.2 2000/07/20 13:09:08 nzc Exp $"

/* only used in initrender.c */
void drawview3d_render(void);

/* renderfg.c */
void calc_viewborder(void);
void init_gl_stuff(void); /* there are two functions with this name !!! */
	
#endif /* DRAWVIEW_EXT_H */


