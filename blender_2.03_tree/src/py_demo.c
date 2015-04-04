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

/*  python.c      MIXED MODEL

 * 
 *  june 99
 * Version: $Id: py_demo.c,v 1.5 2000/08/28 13:21:03 nzc Exp $
 */

#include "blender.h"
#include "py_blender.h"
#include "radio.h"
#include "screen.h"

static PyObject *PyD_RunRadio (PyObject *self, PyObject *args)
{
	int ret;
	
	rad_collect_meshes();
	
	waitcursor(1);
	ret= rad_go();
	waitcursor(0);
	
	return Py_BuildValue("i", ret);
}

static PyObject *PyD_ReadFile (PyObject *self, PyObject *args)
{
	char *file, *lslash;
	
	Py_Try(PyArg_ParseTuple(args, "s", &file));
	
	if (fop_exists(file)) {
		add_readfile_event(file);
	} else {
		char relfile[FILE_MAXDIR+FILE_MAXFILE]="";
		
		strcpy(relfile, G.sce);
		
		lslash= (strrchr(relfile, '/')>strrchr(relfile, '\\'))?strrchr(relfile, '/'):strrchr(relfile, '\\');
		if (lslash) {
			strcpy(lslash+1, file);
		}
		
		if (fop_exists(relfile)) {
			add_readfile_event(relfile);
		} else {
			printf ("Unable to load file <%s> <%s>\n", file, relfile);
		}
	}

	return py_incr_ret(Py_None);
}

static PyObject *PyD_PlayAnim (PyObject *self, PyObject *args)
{
	int ret, from= -1, to= -1;
	
	Py_Try(PyArg_ParseTuple(args, "|ii", &from, &to));
	
	if (from==-1 && to==-1) {
		ret= play_anim(2);
	} else {
		int old_cfra= CFRA, old_efra= EFRA;
		
		CFRA= from;
		if (to!=-1) EFRA= to;
		
		ret= play_anim(2);
		
		CFRA= old_cfra;
		EFRA= old_efra;
	}

	return Py_BuildValue("i", ret);
}

static PyObject *PyD_SetShade (PyObject *self, PyObject *args)
{
	int mode;

	Py_Try(PyArg_ParseTuple(args, "i", &mode));
	
	if (G.vd && mode>0 && mode<5) {
		G.vd->drawtype= mode;
	}

	return py_incr_ret(Py_None);
}

static PyObject *PyD_Sleep (PyObject *self, PyObject *args)
{
	int length;

	Py_Try(PyArg_ParseTuple(args, "i", &length));
	
	usleep(length*1000);

	return py_incr_ret(Py_None);
}

static PyObject *PyD_Render (PyObject *self, PyObject *args)
{
	int ret;
	
	ret= RE_do_renderfg(0);

	return Py_BuildValue("i", ret);
}

static PyObject *PyD_CloseRender (PyObject *self, PyObject *args)
{
	int ret=0;
	
	RE_close_render_display();

	return Py_BuildValue("i", ret);
}

static PyObject *PyD_Redraw (PyObject *self, PyObject *args)
{
	force_draw_all();

	return py_incr_ret(Py_None);
}

static struct PyMethodDef DemoM_methods[] = {
	{"PlayAnim",	PyD_PlayAnim, 1, NULL},
	{"RunRadio",	PyD_RunRadio, 1, NULL},
	{"SetShade",	PyD_SetShade, 1, NULL},
	{"Sleep",		PyD_Sleep, 1, NULL},
	{"ReadFile",	PyD_ReadFile, 1, NULL},
	{"Render",		PyD_Render, 1, NULL},
	{"CloseRender",	PyD_CloseRender, 1, NULL},
	{"Redraw",		PyD_Redraw, 1, NULL},
	{NULL, NULL}
};

PyObject *init_py_demo(void) 
{
	return Py_InitModule("Blender.Demo", DemoM_methods);
}

