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

/* python code for Blender, written by Daniel Dunbar */

/* Version: $Id: py_datablock.c,v 1.8 2000/07/21 09:05:28 nzc Exp $ */

#include "blender.h"
#include "py_blender.h"



ID *find_datablock_from_list(ID *list, char *name) {
	while (list) {
		if(STREQ(name, list->name+2)) break;

		list= list->next;
	}
	
	return list;
}

#define DATABLOCK_GET(uname, docname, list)						\
static char uname##_Get_doc[]=									\
"([name]) - Get " #docname "s from Blender\n"					\
"\n"															\
"[name] The name of the " #docname " to return\n"				\
"\n"															\
"Returns a list of all " #docname "s if name is not specified"; \
																\
static PyObject* uname##_Get (PyObject *self, PyObject *args) {	\
	return py_find_from_list(list, args);								\
}


PyObject *named_enum_get(int val, NamedEnum *enums) {
	while (enums->name) {
		if (enums->num == val) return PyString_FromString(enums->name);
		enums++;
	}	
	PyErr_SetString(PyExc_AttributeError, "Internal error, Unknown enumerated type");
	return NULL;
}

int named_enum_set(char *name, NamedEnum *enums) {
	while (enums->name) {
		if (STREQ(enums->name, name)) 
			return enums->num;
		enums++;
	}	
	
	return -1;
}

static int calc_offset_subsize(int *dlist, int *idx, int *subsize) {
	int n= *dlist;
	
	if (n<=0) {
		*subsize= -n;
		return 0;
	} else {
		int ss;
		int off= calc_offset_subsize(dlist+1, idx+1, &ss);
			
		*subsize= n*ss;
		return off + (*idx)*ss;
	}
}

static int calc_offset(int *dlist, int *idx) {
	int subsize;
	return calc_offset_subsize(dlist, idx, &subsize);
}

static void *get_db_ptr(DataBlockProperty *prop, char *structname, void *struct_ptr) {
	int size, offset= findstruct_offset(structname, prop->struct_name);
	void *ptr= struct_ptr;
	
	if (offset==-1) {
		printf ("Internal error, Invalid prop entry\n");
		return NULL;
	}

	ptr= (void *) (((char *)ptr) + offset);
	
	offset= calc_offset(prop->dlist, prop->idx);	
	ptr= (void *) (((char *)ptr) + offset);
	
	return ptr;
}

PyObject *datablock_getattr(DataBlockProperty *props, char *structname, char *name, void *struct_ptr) {	
	if (STREQ(name, "properties")) {
		PyObject *l= PyList_New(0);
		DataBlockProperty *p= props;
		
		while (p->public_name) {
			PyList_Append(l, PyString_FromString(p->public_name));
			p++;
		}
		
		return l;
	}
	
	while (props->public_name) {
		if (STREQ(name, props->public_name)) {
			void *ptr;
			int val;
			DBPtrToObFP conv_fp;

			if (props->handling==DBP_HANDLING_NONE || 
					props->handling==DBP_HANDLING_NENM) {
				ptr= get_db_ptr(props, structname, struct_ptr);
				if (!ptr) return NULL;
				
			} else if (props->handling==DBP_HANDLING_FUNC) {
				DBGetPtrFP fp= (DBGetPtrFP) props->extra1;
				ptr= fp(struct_ptr, props->struct_name, 0);
				if (!ptr) return NULL;
			}
			
			switch(props->type) {
			case DBP_TYPE_CHA:
				val= *((char	*)ptr);
				if (props->handling==DBP_HANDLING_NENM) 
					return named_enum_get(val, props->extra1);				
				else
					return PyInt_FromLong(val);
			case DBP_TYPE_SHO:
				val= *((short	*)ptr);
				if (props->handling==DBP_HANDLING_NENM) 
					return named_enum_get(val, props->extra1);				
				else
					return PyInt_FromLong(val);
			case DBP_TYPE_INT:
				val= *((int	*)ptr);
				if (props->handling==DBP_HANDLING_NENM) 
					return named_enum_get(val, props->extra1);				
				else
					return PyInt_FromLong(val);
			case DBP_TYPE_FLO:
				return PyFloat_FromDouble	( *((float	*)ptr) );
			case DBP_TYPE_VEC:
				return newVectorObject		( ((float	*)ptr), (int) props->min );
			case DBP_TYPE_FUN:
				conv_fp= (DBPtrToObFP) props->extra2;
				return conv_fp( ptr );
			default:
				PyErr_SetString(PyExc_AttributeError, "Internal error, Unknown prop type");
				return NULL;
			}
		}
		
		props++;
	}
	
	PyErr_SetString(PyExc_AttributeError, name);
	return NULL;		
}

int datablock_setattr(DataBlockProperty *props, char *structname, char *name, void *struct_ptr, PyObject *setto) {
	while (props->public_name) {
		if (STREQ(props->public_name, name)) {
			void *ptr;
			int type;
			DBSetPtrFP conv_fp;
			int clamp= props->min!=props->max;

			int enum_val= -1;
			char	cha_data;
			short	sho_data;
			int		int_data;
			float	flo_data;
			char*	str_data;

			type= props->stype;
			if (type==DBP_TYPE_NON) type= props->type;

			if (props->handling==DBP_HANDLING_NONE) {
				ptr= get_db_ptr(props, structname, struct_ptr);
				if (!ptr) return 0;
				
			} else if (props->handling==DBP_HANDLING_FUNC) {
				if (type!=DBP_TYPE_FUN) {
					DBGetPtrFP fp= (DBGetPtrFP) props->extra1;
					ptr= fp(struct_ptr, props->struct_name, 1);
					if (!ptr) return 0;
				}
			} else if (props->handling==DBP_HANDLING_NENM) {
				char *str;
				if (!PyArg_Parse(setto, "s", &str)) return -1;

				ptr= get_db_ptr(props, structname, struct_ptr);
				if (!ptr) return 0;
				
				enum_val= named_enum_set(str, props->extra1);
				if (enum_val==-1)
					return py_err_ret_int(PyExc_AttributeError, "invalid setting for field");
			}

			switch(type) {
			case DBP_TYPE_CHA:
				if (enum_val==-1) {
					if (!PyArg_Parse(setto, "b", &cha_data)) return -1;
				} else cha_data= (char) enum_val;
				
				if (clamp) CLAMP(cha_data, (char) props->min, (char) props->max);
				*((char		*)ptr)= cha_data;
				return 0;
			case DBP_TYPE_SHO:
				if (enum_val==-1) {
					if (!PyArg_Parse(setto, "h", &sho_data)) return -1;
				} else sho_data= (short) enum_val;

				if (clamp) CLAMP(sho_data, (short) props->min, (short) props->max);
				*((short	*)ptr)= sho_data;
				return 0;
			case DBP_TYPE_INT:
				if (enum_val==-1) {
					if (!PyArg_Parse(setto, "i", &int_data)) return -1;
				} else int_data= (int) enum_val;

				if (clamp) CLAMP(int_data, (int) props->min, (int) props->max);
				*((int		*)ptr)= int_data;
				return 0;
			case DBP_TYPE_FLO:
				if (!PyArg_Parse(setto, "f", &flo_data)) return -1;
				if (clamp) CLAMP(flo_data, (float) props->min, (float) props->max);
				*((float	*)ptr)= flo_data;
				return 0;
			case DBP_TYPE_VEC:
				return py_err_ret_int(PyExc_AttributeError, "cannot directly assign to vector, use slice assignment instead");
			case DBP_TYPE_FUN:
				conv_fp= (DBSetPtrFP) props->extra3;
				if (conv_fp)
					return conv_fp( struct_ptr, props->struct_name, setto );
				else
					return py_err_ret_int(PyExc_AttributeError, "cannot directly assign to item");
			default:
				PyErr_SetString(PyExc_AttributeError, "Internal error, Unknown prop type");
				return -1;
			}
		}
		
		props++;
	}

	PyErr_SetString(PyExc_AttributeError, name);
	return -1;
}

/*********************/
/* Texture Datablocks */
/*
DATABLOCK_GET(Texture, texture, G.main->tex.first)

static struct PyMethodDef Texture_methods[] = {
	{"Get", Texture_Get, 1, Texture_Get_doc},
	{NULL, NULL}
};
*/

/*********************/
/* Camera Datablocks */

DATABLOCK_GET(Camera, camera, G.main->camera.first)

static struct PyMethodDef Camera_methods[] = {
	{"Get", Camera_Get, 1, Camera_Get_doc},
	{NULL, NULL}
};


/***********************/
/* Material Datablocks */

DATABLOCK_GET(Material, material, G.main->mat.first)

static struct PyMethodDef Material_methods[] = {
	{"Get", Material_Get, 1, Material_Get_doc},
	{NULL, NULL}
};


/*******************/
/* Lamp Datablocks */

DATABLOCK_GET(Lamp, lamp, G.main->lamp.first)

static struct PyMethodDef Lamp_methods[] = {
	{"Get", Lamp_Get, 1, Lamp_Get_doc},
	{NULL, NULL}
};


/********************/
/* World Datablocks */

DATABLOCK_GET(World, world, G.main->world.first)

static char World_GetActive_doc[]="() - Returns the active world";
static PyObject *World_GetActive (PyObject *self, PyObject *args)
{
	if (G.scene->world) 
		return add_pyblock(G.scene->world);
	else
		return py_incr_ret(Py_None);	
}

static struct PyMethodDef World_methods[] = {
	{"Get",			World_Get,			1, World_Get_doc},
	{"GetActive",	World_GetActive,	1, World_GetActive_doc},
	{NULL, NULL}
};

/*********************/
/* Object Datablocks */

DATABLOCK_GET(Object, object, G.main->object.first)

static char Object_GetSelected_doc[]=
"() - Returns a list of all selected objects";
static PyObject *Object_GetSelected (PyObject *self, PyObject *args)
{
	PyObject *list;
	Object *tmp;
	
	list= PyList_New(0);
	
	tmp= G.main->object.first;
	while (tmp) {
		if (tmp->flag & SELECT) {
			PyObject *ob= add_pyblock(tmp);
			
			if (!ob) {
				Py_DECREF(list);
				return NULL;
			}
			
			PyList_Append(list, ob);	
		}
			
		tmp= tmp->id.next;
	}
	
	return list;
}

static char Object_Update_doc[]=
"(name) - Updates the specified object during user-transformation\n\
\n\
(name) Name of the object to update\n\
\n\
NOTE: This is an experimental function for combating 'lag'.";
static PyObject *Object_Update (PyObject *self, PyObject *args)
{
	Base *base;
	char *name= NULL;
		
	Py_Try(PyArg_ParseTuple(args, "s", &name));
	
	base= FIRSTBASE;			
	while (base) {
		if (strcmp(name, base->object->id.name+2)==0) 
			base->flag|= BA_WHERE_UPDATE|BA_PARSEL|BA_DISP_UPDATE;
					
		base= base->next;
	}

	Py_INCREF(Py_None);
	return Py_None;	
}

static struct PyMethodDef Object_methods[] = {
	{"Get",			Object_Get,			1, Object_Get_doc},
	{"GetSelected", Object_GetSelected, 1, Object_GetSelected_doc},
	{"Update",		Object_Update,		1, Object_Update_doc},
	{NULL, NULL}
};


/******************/
/* IPO Datablocks */

PyTypeObject PyBezTriple_Type;

typedef struct _PyBezTriple {
	PyObject_VAR_HEAD
	
	BezTriple bzt;
} PyBezTriple;

PyTypeObject PyBezTriple_Type;

void pybzt_dealloc(PyObject *self) {
	PyMem_DEL(self);
}

int pybzt_print(PyObject *self, FILE *fp, int flags) {
	fprintf (fp, "[BezTriple]");
	return 0;
}

NamedEnum bez_triple_flags[]= {
	{"Free", 0},
	{"Auto", 1},
	{"Vect", 2},
	{"Align", 3},
	{NULL}
};

DataBlockProperty BezTriple_Properties[]= {
	{"h1", 	"vec[3][3]", 	DBP_TYPE_VEC, 0, 2.0, 0.0, {0,0}, {3,3,-sizeof(float)}}, 
	{"pt", 	"vec[3][3]", 	DBP_TYPE_VEC, 0, 2.0, 0.0, {1,0}, {3,3,-sizeof(float)}}, 
	{"h2", 	"vec[3][3]", 	DBP_TYPE_VEC, 0, 2.0, 0.0, {2,0}, {3,3,-sizeof(float)}}, 
	
	{"f1", 	"f1", 			DBP_TYPE_CHA, 0, 0.0, 1.0},
	{"f2", 	"f2", 			DBP_TYPE_CHA, 0, 0.0, 1.0},
	{"f3", 	"f3", 			DBP_TYPE_CHA, 0, 0.0, 1.0},

	{"h1t", "h1", 			DBP_TYPE_SHO, 0, 0.0, 0.0, {0}, {0}, DBP_HANDLING_NENM, bez_triple_flags},
	{"h2t", "h2", 			DBP_TYPE_SHO, 0, 0.0, 0.0, {0}, {0}, DBP_HANDLING_NENM, bez_triple_flags},
	
	{NULL}
};

PyObject *pybzt_getattr(PyObject *self, char *name) {
	PyBezTriple *pybzt= (PyBezTriple *) self;

	return datablock_getattr(BezTriple_Properties, "BezTriple", name, &pybzt->bzt);
}

int pybzt_setattr(PyObject *self, char *name, PyObject *ob) {
	PyBezTriple *pybzt= (PyBezTriple *) self;

	return datablock_setattr(BezTriple_Properties, "BezTriple", name, &pybzt->bzt, ob);
}

PyTypeObject PyBezTriple_Type = {
	PyObject_HEAD_INIT(NULL)
	0,								/*ob_size*/
	"BezTriple",					/*tp_name*/
	sizeof(PyBezTriple),			/*tp_basicsize*/
	0,								/*tp_itemsize*/
	/* methods */
	(destructor)	pybzt_dealloc,	/*tp_dealloc*/
	(printfunc)		pybzt_print,	/*tp_print*/
	(getattrfunc)	pybzt_getattr,	/*tp_getattr*/
	(setattrfunc)	pybzt_setattr,	/*tp_setattr*/
	0,								/*tp_compare*/
	0,								/*tp_repr*/
	0,								/*tp_as_number*/
	0,								/*tp_as_sequence*/
	0,								/*tp_as_mapping*/
	0,								/*tp_hash*/	
};

static char pybzt_create_doc[]= "() - Create a new BezTriple object";
PyObject *pybzt_create(PyObject *self, PyObject *args) {
	PyBezTriple *py_bzt= PyObject_NEW(PyBezTriple, &PyBezTriple_Type);
	BezTriple *bzt= &py_bzt->bzt;
	
	Py_Try(PyArg_ParseTuple(args, ""));
	
	memset(&py_bzt->bzt,0,sizeof(py_bzt->bzt));
	
	return (PyObject *) py_bzt;
}

PyObject *pybzt_from_bzt(BezTriple *bzt) {
	PyBezTriple *py_bzt= PyObject_NEW(PyBezTriple, &PyBezTriple_Type);
	
	memcpy(&py_bzt->bzt, bzt, sizeof(*bzt));
	
	return (PyObject *) py_bzt;
}

PyTypeObject PyIpoCurve_Type;

typedef struct _PyIpoCurve {
	PyObject_VAR_HEAD

	IpoCurve *icu;
} PyIpoCurve;

void pyicu_dealloc(PyObject *self) {
	PyMem_DEL(self);
}

int pyicu_print(PyObject *self, FILE *fp, int flags) {
	fprintf (fp, "[IpoCurve]");
	return 0;
}

PyObject *pyicu_getattr(PyObject *self, char *name) {
	PyIpoCurve *py_icu= (PyIpoCurve *) self;
	IpoCurve *icu= py_icu->icu;

	if (STREQ(name, "type")) {
		switch (icu->ipo) {
		case IPO_CONST:	return PyString_FromString("Constant");
		case IPO_LIN:	return PyString_FromString("Linear");
		case IPO_BEZ:	return PyString_FromString("Bezier");			
		}
	} else if (STREQ(name, "extend")) {
		switch (icu->extrap) {
		case IPO_HORIZ:	return PyString_FromString("Constant");
		case IPO_DIR:	return PyString_FromString("Extrapolate");
		case IPO_CYCL:	return PyString_FromString("Cyclic");					
		case IPO_CYCLX:	return PyString_FromString("CyclicX");					
		}
	} else if (STREQ(name, "name")) {
		char icu_name[32]= "";
		
		switch (icu->blocktype) {
		case ID_OB:
			getname_ob_ei(icu->adrcode, icu_name, 0);
			break;
		case ID_MA:
			getname_mat_ei(icu->adrcode, icu_name, 0);
			break;
		case ID_WO:
			getname_world_ei(icu->adrcode, icu_name, 0);
			break;
		case ID_SEQ:
			getname_seq_ei(icu->adrcode, icu_name, 0);
			break;
		case ID_CU:
			getname_cu_ei(icu->adrcode, icu_name, 0);
			break;
		case ID_KE:
			getname_key_ei(icu->adrcode, icu_name, 0);
			break;
		case ID_LA:
			getname_la_ei(icu->adrcode, icu_name, 0);
			break;
		case ID_CA:
			getname_cam_ei(icu->adrcode, icu_name, 0);
			break;
		}
		
		return PyString_FromString(icu_name);
	} else if (STREQ(name, "points")) {
		PyObject *list= PyList_New(icu->totvert);
		BezTriple *bzt= icu->bezt;
		int i;
		
		for (i=0; i<icu->totvert; i++) {
			PyList_SetItem(list, i, pybzt_from_bzt(bzt));
			bzt++;
		}
		
		return list;
	} 
	
	PyErr_SetString(PyExc_AttributeError, name);
	return NULL;
}

int pyicu_setattr(PyObject *self, char *name, PyObject *ob) {
	PyIpoCurve *py_icu= (PyIpoCurve *) self;
	IpoCurve *icu= py_icu->icu;

	if (STREQ(name, "points")) {
		int i, len;
		BezTriple *bzt;
		
		if (!PySequence_Check(ob) || !py_check_sequence_consistency(ob, &PyBezTriple_Type))
			return py_err_ret_int(PyExc_AttributeError, "Expected list of BezTriples");

		len= PySequence_Length(ob);
		
		freeN(icu->bezt);
		icu->totvert= len;
		if (len) icu->bezt= mallocN(len*sizeof(BezTriple), "beztriples");
		
		bzt= icu->bezt;
		for (i=0; i<len; i++) {
			PyBezTriple *pybzt= (PyBezTriple*) PySequence_GetItem(ob, i);
			
			memcpy(bzt, &pybzt->bzt, sizeof(BezTriple));
			bzt++;
			
			Py_DECREF(pybzt);
		}

		/* Twice for auto handles */
		calchandles_ipocurve(icu);
		calchandles_ipocurve(icu);

		boundbox_ipocurve(icu);
		sort_time_ipocurve(icu);
		
		return 0;
	}

	PyErr_SetString(PyExc_AttributeError, name);
	return -1;	
}

PyObject *pyicu_from_icu(IpoCurve *icu) {
	PyIpoCurve *ob= PyObject_NEW(PyIpoCurve, &PyIpoCurve_Type);
	
	ob->icu= icu;
	
	return (PyObject *) ob;
}

PyTypeObject PyIpoCurve_Type = {
	PyObject_HEAD_INIT(NULL)
	0,								/*ob_size*/
	"IpoCurve",						/*tp_name*/
	sizeof(PyIpoCurve),				/*tp_basicsize*/
	0,								/*tp_itemsize*/
	/* methods */
	(destructor)	pyicu_dealloc,	/*tp_dealloc*/
	(printfunc)		pyicu_print,	/*tp_print*/
	(getattrfunc)	pyicu_getattr,	/*tp_getattr*/
	(setattrfunc)	pyicu_setattr,	/*tp_setattr*/
	0,								/*tp_compare*/
	0,								/*tp_repr*/
	0,								/*tp_as_number*/
	0,								/*tp_as_sequence*/
	0,								/*tp_as_mapping*/
	0,								/*tp_hash*/	
};

DATABLOCK_GET(Ipo, ipo, G.main->ipo.first)

#include "ipo.h"

extern PyTypeObject Block_Type;
static char Ipo_Recalc_doc[]=
"(ipo) - Recalculate the ipo and update linked objects\n\
\n\
(ipo) The ipo to recalculate";

PyObject *Ipo_Recalc(PyObject *self, PyObject *args) {
	PyBlock *ob;
	Key *key;
	
	Py_Try(PyArg_ParseTuple(args, "O!", &Block_Type, &ob));
	
	if (!pyblock_is_type(ob,ID_IP))
		return py_err_ret_ob(PyExc_AttributeError, "Expected IPO object");
		
	do_ipo((Ipo *) ob->data);

	/* here we should signal all objects with keys that the ipo changed */
	
	key= G.main->key.first;
	while(key) {
		if(key->ipo == (Ipo *)ob->data) do_spec_key(key);
		key= key->id.next;
	}
	
	return py_incr_ret(Py_None);
}

static char Ipo_Eval_doc[]= 
"(curve, [time]) - Evaluate the value of the curve at some time\n\
\n\
(curve) The curve to evaluate\n\
[time= current frame] The time at which to evaluate the curve";

PyObject *Ipo_Eval(PyObject *self, PyObject *args) {
	PyIpoCurve *ob;
	float time= CFRA;
	
	Py_Try(PyArg_ParseTuple(args, "O!|f", &PyIpoCurve_Type, &ob, &time));
	
	return PyFloat_FromDouble(eval_icu(ob->icu, time));
}

static struct PyMethodDef Ipo_methods[] = {

	{"Get",	Ipo_Get, 1, Ipo_Get_doc},
	{"Eval", Ipo_Eval, 1, Ipo_Eval_doc}, 
	{"Recalc", Ipo_Recalc, 1, Ipo_Recalc_doc}, 
	{"BezTriple", pybzt_create, 1, pybzt_create_doc},
	
	{NULL, NULL}
};

void load_py_datablocks(PyObject *dict) {
#define MODLOAD(name)	PyDict_SetItemString(dict, #name, Py_InitModule("Blender." #name, name##_methods))

	PyIpoCurve_Type.ob_type= &PyType_Type;
	PyBezTriple_Type.ob_type= &PyType_Type;
	
	MODLOAD(Object);
	MODLOAD(Camera);
	MODLOAD(Lamp);
	MODLOAD(Material);
	MODLOAD(World);
	MODLOAD(Ipo);
/* 	MODLOAD(Texture); */
}

