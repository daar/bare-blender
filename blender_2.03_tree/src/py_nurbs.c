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

#include "blender.h"

#include "py_blender.h"
#include "blender.h"

PyTypeObject NCurve_Type;
PyTypeObject NBPoint_Type;

typedef struct _NBPoint {
  PyObject_VAR_HEAD

  float vec[4];

  int index;
} NBPoint;

static void NBPoint_dealloc(PyObject *self) {
  PyMem_DEL(self);
}

static PyObject *NBPoint_getattr(PyObject *self, char *name) {
  NBPoint *bp = (NBPoint *) self;

  if (STREQ(name, "vec"))
    return newVectorObject(bp->vec, 4);

  else if (STREQ(name, "index"))
    return PyInt_FromLong(bp->index);

  PyErr_SetString(PyExc_AttributeError, name);
  return NULL;
}

static int NBPoint_setattr(PyObject *self, char *name, PyObject *v) {
  int     i;
  NBPoint *bp = (NBPoint *) self;

  if (STREQ(name, "index")) {
    PyArg_Parse(v, "i", &i);
    bp->index = i;
    return 0;
  }

  PyErr_SetString(PyExc_AttributeError, name);
  return -1;
}

static int NBPoint_print (PyObject *self, FILE *fp, int flags) {
  fprintf (fp, "[Ctrlpt]");
  
  return 0;
}

PyTypeObject NBPoint_Type = {
  PyObject_HEAD_INIT(NULL)
  0,				     /*ob_size*/
  "Ctrlpt",			     /*tp_name*/
  sizeof(NBPoint),		     /*tp_basicsize*/
  0,				     /*tp_itemsize*/
  /* methods */
  (destructor)	NBPoint_dealloc,     /*tp_dealloc*/
  (printfunc)	NBPoint_print,       /*tp_print*/
  (getattrfunc)	NBPoint_getattr,     /*tp_getattr*/
  (setattrfunc)	NBPoint_setattr,     /*tp_setattr*/
  0,				     /*tp_compare*/
  0,				     /*tp_repr*/
  0,				     /*tp_as_number*/
  0,				     /*tp_as_sequence*/
  0,				     /*tp_as_mapping*/
  0,				     /*tp_hash*/
};

typedef struct _NCurve {
  PyObject_VAR_HEAD

  PyObject *name;
  PyObject *mats;
  PyObject *bpoints;
  PyObject *knotsu;
  PyObject *knotsv;
/*    BezTriple *bezt; */
  short type, mat_nr;
  short hide, flag;
  short pntsu, pntsv;
  short resolu, resolv;
  short orderu, orderv;
  short flagu, flagv;
} NCurve;

static void NCurve_dealloc(PyObject *self) {
  NCurve *cu = (NCurve *) self;

  Py_DECREF(cu->name);
  Py_DECREF(cu->mats);
  Py_DECREF(cu->bpoints);

  PyMem_DEL(self);
}

static PyObject *NCurve_getattr(PyObject *self, char *name) {
  int    numpts;
  NCurve *cu = (NCurve *) self;

  if (STREQ(name, "name"))
    return py_incr_ret(cu->name);

  else if (STREQ(name, "block_type"))
    return PyString_FromString("Curve");

  else if (STREQ(name, "mats"))
    return py_incr_ret(cu->mats);

  else if (STREQ(name, "ctrlpts"))
    return py_incr_ret(cu->bpoints);

  else if (STREQ(name, "type"))
    return Py_BuildValue("i", cu->type);

  else if (STREQ(name, "mat_nr"))
    return Py_BuildValue("i", cu->mat_nr);

  else if (STREQ(name, "hide"))
    return Py_BuildValue("i", cu->hide);

  else if (STREQ(name, "flag"))
    return Py_BuildValue("i", cu->flag);

  else if (STREQ(name, "pntsu"))
    return Py_BuildValue("i", cu->pntsu);

  else if (STREQ(name, "pntsv"))
    return Py_BuildValue("i", cu->pntsv);

  else if (STREQ(name, "resolu"))
    return Py_BuildValue("i", cu->resolu);

  else if (STREQ(name, "resolv"))
    return Py_BuildValue("i", cu->resolv);

  else if (STREQ(name, "orderu"))
    return Py_BuildValue("i", cu->orderu);

  else if (STREQ(name, "orderv"))
    return Py_BuildValue("i", cu->orderv);

  else if (STREQ(name, "flagu"))
    return Py_BuildValue("i", cu->flagu);

  else if (STREQ(name, "flagv"))
    return Py_BuildValue("i", cu->flagv);

  else if (STREQ(name, "knotsu"))
    return py_incr_ret(cu->knotsu);

  else if (STREQ(name, "knotsv"))
    return py_incr_ret(cu->knotsv);

  PyErr_SetString(PyExc_AttributeError, name);
  return NULL;
}

static int NCurve_setattr(PyObject *self, char *name, PyObject *v) {
  NCurve *cu = (NCurve *) self;

  if (STREQ(name, "ctrlpts")) {
    if (PySequence_Check(v)) {
      Py_DECREF(cu->bpoints);
      cu->bpoints = py_incr_ret(v);
    } else {
      PyErr_SetString(PyExc_AttributeError, "expected a sequence");
      return -1;
    }
  } else {
    PyErr_SetString(PyExc_AttributeError, name);
    return -1;
  }

  return 0;
}

static int NCurve_print (PyObject *self, FILE *fp, int flags) {
  fprintf (fp, "[NURBS]");
  
  return 0;
}

PyTypeObject NCurve_Type = {
  PyObject_HEAD_INIT(NULL)
  0,				     /*ob_size*/
  "NURBS",			     /*tp_name*/
  sizeof(NCurve),		     /*tp_basicsize*/
  0,				     /*tp_itemsize*/
  /* methods */
  (destructor)	NCurve_dealloc,	     /*tp_dealloc*/
  (printfunc)	NCurve_print,        /*tp_print*/
  (getattrfunc)	NCurve_getattr,	     /*tp_getattr*/
  (setattrfunc)	NCurve_setattr,	     /*tp_setattr*/
  0,				     /*tp_compare*/
  0,				     /*tp_repr*/
  0,				     /*tp_as_number*/
  0,				     /*tp_as_sequence*/
  0,				     /*tp_as_mapping*/
  0,				     /*tp_hash*/
};

static NBPoint *bpoint_from_data(NCurve *nc, BPoint *point, int idx)
{
  NBPoint *bp = PyObject_NEW(NBPoint, &NBPoint_Type);

  QUATCOPY (bp->vec, point->vec);

  bp->index = idx;

  return bp;
}

static NBPoint *newNBPointObject(float *vec) {
  NBPoint *bp = PyObject_NEW(NBPoint, &NBPoint_Type);

  QUATCOPY(bp->vec, vec);

  return bp;
}

PyObject *newNCurveObject(Curve *oldcurve)
{
  int      i, numpts;
  float    *oldkp;
  NCurve   *nc;
  Nurb     *nurb;
  BPoint   *oldbp;
  Material *mat;
  PyObject *ob;

  nc = PyObject_NEW(NCurve, &NCurve_Type);

  if (!oldcurve) {
    nc->name    = py_incr_ret(Py_None);
    nc->mats    = PyList_New(0);
    nc->bpoints = PyList_New(0);
    nc->type    = -1;
    nc->mat_nr  = -1;
    nc->hide    = -1;
    nc->flag    = -1;
    nc->pntsu   = -1;
    nc->pntsv   = -1;
    nc->resolu  = -1;
    nc->resolv  = -1;
    nc->orderu  = -1;
    nc->orderv  = -1;
    nc->flagu   = -1;
    nc->flagv   = -1;
    nc->knotsu  = py_incr_ret(Py_None);
    nc->knotsv  = py_incr_ret(Py_None);
  } else {
    nc->name = PyString_FromString(oldcurve->id.name+2);
    nc->mats = PyList_New(oldcurve->totcol);
    for (i = 0; i < oldcurve->totcol;  i++) {
      mat = oldcurve->mat[i];
      if (mat)
	ob = PyString_FromString(mat->id.name+2);
      else
	ob = py_incr_ret(Py_None);

      PyList_SetItem(nc->mats, i, ob);
    }

    nurb = (Nurb *) oldcurve->nurb.first;
    numpts = nurb->pntsu * nurb->pntsv;
    oldbp = nurb->bp;
    nc->bpoints = PyList_New(numpts);
    for (i = 0; i < numpts;  i++) {
      PyList_SetItem(nc->bpoints, i, 
		     (PyObject *) bpoint_from_data(nc, oldbp, i));
      oldbp++;
    }
    nc->type   = nurb->type;
    nc->mat_nr = nurb->mat_nr;
    nc->hide   = nurb->hide;
    nc->flag   = nurb->flag;
    nc->pntsu  = nurb->pntsu;
    nc->pntsv  = nurb->pntsv;
    nc->resolu = nurb->resolu;
    nc->resolv = nurb->resolv;
    nc->orderu = nurb->orderu;
    nc->orderv = nurb->orderv;
    nc->flagu  = nurb->flagu;
    nc->flagv  = nurb->flagv;
    if (nurb->orderu) {
/*        numpts = nurb->orderu * 2 + (nurb->pntsu - nurb->orderu); */
      numpts = KNOTSU(nurb);
      nc->knotsu = newVectorObject(nurb->knotsu, numpts);
    }
    else
      nc->knotsu = py_incr_ret(Py_None);
    if (nurb->orderv) {
/*        numpts = nurb->orderv * 2 + (nurb->pntsv - nurb->orderv); */
      numpts = KNOTSV(nurb);
      nc->knotsv = newVectorObject(nurb->knotsv, numpts);
    }
    else
      nc->knotsv = py_incr_ret(Py_None);
  }

  return (PyObject *) nc;
}

static char Method_Ctrlpt_doc[]=
"([x, y, z, w]) - Get a new controlpoint\n\
\n\
[x, y, z, w] Specify new coordinates";

static PyObject *Method_Ctrlpt(PyObject *self, PyObject *args) {
	float vec[4]= {0.0, 0.0, 0.0, 0.0};
	
	Py_Try(PyArg_ParseTuple(args, "|ffff", 
				&vec[0], &vec[1], &vec[2], &vec[3]));
	
	return (PyObject *) newNBPointObject(vec);
}

static char Method_GetRaw_doc[] =
"([name]) - Get a raw nurb from Blender\n\
\n\
[name] Name of the nurb to be returned\n\
\n\
If name is not specified a new empty nurb is\n\
returned, otherwise Blender returns an existing\n\
nurb.";

static PyObject *Method_GetRaw(PyObject *self, PyObject *args) 
{
  char *name=NULL;
  Curve *oldcurve=NULL;

  Py_Try(PyArg_ParseTuple(args, "|s", &name));

  if(name) {
    oldcurve = (Curve *) find_datablock_from_list((ID *) G.main->curve.first, 
						  name);
    if (!oldcurve) return py_incr_ret(Py_None);
  }

  return newNCurveObject(oldcurve);
}

static struct PyMethodDef NURBS_methods[] = {
  MethodDef(Ctrlpt),
  MethodDef(GetRaw),
  {NULL, NULL}
};

PyObject *init_py_nurbs(void)
{
  return Py_InitModule("Blender.NURBS", NURBS_methods);
}

