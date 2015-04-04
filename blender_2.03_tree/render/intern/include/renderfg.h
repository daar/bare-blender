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

 * renderfg_ext.h
 *
 * Version: $Id: renderfg.h,v 1.4 2000/09/08 16:41:20 nzc Exp $
 */

#ifndef RENDERFG_EXT_H
#define RENDERFG_EXT_H "$Id: renderfg.h,v 1.4 2000/09/08 16:41:20 nzc Exp $"

/**
 * Clear the active render context and swap display buffers, if
 * needed. 
 */
void clear_render_display(void);
void display_scanline(View3D *vd, uint *rect, int starty, int endy);

/**
 * Set up a rendering context under the relevant windowing system.  
 */
void init_render_display();
/**
 * Prints framenumber, number of vertices, faces and lamps, memory
 * usage, and duration of the last frame calculation in the header of
 * the blender-window.
 * @param mode Number to display as sample.
 */
void printrenderinfo(int mode);
void redraw_render_display();

#endif

