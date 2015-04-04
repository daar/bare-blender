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

 * edgeRender.h
 *
 * Version: $Id: edgeRender.h,v 1.2 2000/09/21 13:03:59 nzc Exp $
 */

#ifndef EDGERENDER_H
#define EDGERENDER_H "$Id: edgeRender.h,v 1.2 2000/09/21 13:03:59 nzc Exp $"

/* maybe not the best place for these ... */
#define RE_EGDERENDER_COMPATIBILITY_MODE_ON  1
#define RE_EGDERENDER_COMPATIBILITY_MODE_OFF 1

/* ------------------------------------------------------------------------- */
/**
 * Add edges to <targetbuf>, which is of size <iw> by <ih>. Use <osanr>
 * samples, and intensity <i>. <compat> indicates an extra shift in the
 * image, for backwards compatibility with the old renderpipe. <mode>
 * indicates which edges should be considered.
 */
void addEdges(char * targetbuf, int iw, int ih,
			  int osanr, short int i, int compat, int mode);

/* ------------------------------------------------------------------------- */

#endif /* EDGERENDER_H */

