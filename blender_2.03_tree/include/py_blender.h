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

/* Python for PC uses a hand made config.h.

 * All other systems use a generic config.h. I've
 * made a copy of that config.h here and put the same
 * #ifdef PY_CONFIG.H around it, so it won't get included
 * after PC/config.h has been included
 * Version: $Id: py_blender.h,v 1.7 2000/08/09 14:45:24 ton Exp $
 */ 

#ifdef WIN32
	#include "../../extern/python/distribution/PC/config.h"
#endif /* WIN32 */

#include "../../extern/python/distribution/Include/Python.h"


#define Py_Try(x) {if((!(x))) return NULL;}
#define Py_AddConsti(dict, name) PyDict_SetItemString(dict, #name, PyInt_FromLong(name))
#define Py_AddConstf(dict, name) PyDict_SetItemString(dict, #name, PyFloat_FromDouble(name))
#define MethodDef(func) {#func, Method_##func, METH_VARARGS, Method_##func##_doc}

typedef struct _NamedEnum {
	char *name;
	int num;
} NamedEnum;

typedef struct _DataBlockProperty {
	char *public_name;
	char *struct_name;

	int type;
#define DBP_TYPE_CHA	1	/* Char item				*/
#define DBP_TYPE_SHO	2	/* Short item				*/
#define DBP_TYPE_INT	3	/* Int item					*/
#define DBP_TYPE_FLO	4	/* Float item				*/
#define DBP_TYPE_VEC	5	/* Float vector object		*/
#define DBP_TYPE_FUN	6	/* Extra2 hold function to convert ptr->ob
								extra3 holds function to convert ob->ptr */

	int stype;
#define DBP_TYPE_NON	0
	
	float min;				/* Minimum allowed value	*/
	float max;				/* Maximum allowed value	*/
	
	int idx[4];
	int dlist[4];

	int handling;
#define DBP_HANDLING_NONE	0	/* No special handling required			*/
#define DBP_HANDLING_FUNC	1	/* Extra1 is used to retrieve ptr		*/
#define DBP_HANDLING_NENM	2	/* Extra1 holds named enum to resolve 
									values from/to. */
	
	void *extra1;
	void *extra2;
	void *extra3;
} DataBlockProperty;

typedef void *	(*DBGetPtrFP) (void *struct_ptr, char *name, int forsetting);
typedef PyObject *	(*DBPtrToObFP) (void **ptr);
typedef int		(*DBSetPtrFP) (void *struct_ptr, char *name, PyObject *ob);



PyObject *datablock_getattr(DataBlockProperty *props, char *structname, char *name, void *struct_ptr);
int datablock_setattr(DataBlockProperty *props, char *structname, char *name, void *struct_ptr, PyObject *setto);

void pyblock_dealloc(PyObject *self);
int pyblock_print(PyObject *self, FILE *fp, int flags);
PyObject *pyblock_getattr(PyObject *self, char *name);
int pyblock_setattr(PyObject *self, char *name, PyObject *ob);


PyObject *newVectorObject(float *vec, int size);
PyObject *newMatrixObject(float *vec);

void init_py_vector(void);
void init_py_matrix(void);

void load_py_datablocks(PyObject *dict);

PyObject *init_py_mesh(void);
PyObject *init_py_nmesh(void);
PyObject *init_py_interface(void);
PyObject *init_py_draw(void);
PyObject *init_py_bgl(void);

PyObject *init_py_demo(void);

PyObject *py_incr_ret(PyObject *ob);
int py_check_sequence_consistency(PyObject *seq, PyTypeObject *against);

PyObject *py_err_ret_ob(PyObject *type, char *err);
int py_err_ret_int(PyObject *type, char *err);

ID *find_datablock_from_list(ID *list, char *name);

typedef struct {
	PyObject_HEAD
	void		*data;
	char		*type;
	ListBase	*type_list;
	DataBlockProperty *properties;
} PyBlock;

extern int pyblock_is_type(PyBlock *block, int type);
extern PyObject *add_pyblock(void *data);
extern PyObject *py_find_from_list(ID *list, PyObject *args);

extern PyObject *newNMeshObject(Mesh *me);
extern PyObject *newNCurveObject(Curve *oldcurve);
extern PyObject *init_py_nurbs(void);

