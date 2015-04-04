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

/* Version: $Id: py_main.c,v 1.12 2000/08/09 00:32:08 jan Exp $ */

#include "blender.h"
#include "py_blender.h"
#include "screen.h"
#include "ipo.h"

static PyObject *mdict, *gdict;

static PyObject *init_blender(void);

void copy_scriptlink(ScriptLink *scriptlink)
{
	void *tmp;
	
	if (scriptlink->totscript) {
		tmp= scriptlink->scripts;
		scriptlink->scripts= mallocN(sizeof(ID*)*scriptlink->totscript, "scriptlistL");
		memcpy(scriptlink->scripts, tmp, sizeof(ID*)*scriptlink->totscript);

		tmp= scriptlink->flag;
		scriptlink->flag= mallocN(sizeof(short)*scriptlink->totscript, "scriptlistF");
		memcpy(scriptlink->flag, tmp, sizeof(short)*scriptlink->totscript);
	}
}

void free_scriptlink(ScriptLink *slink)
{
	if (slink->totscript) {
		if(slink->flag) freeN(slink->flag);
		if(slink->scripts) freeN(slink->scripts);
	}
}

void clear_bad_scriptlink(ID *id, Text *byebye)
{
	ScriptLink *scriptlink;
	int offset=-1;
	char *structname=NULL;
	int i;

	if (GS(id->name)==ID_OB) structname= "Object";
	else if (GS(id->name)==ID_LA) structname= "Lamp";
	else if (GS(id->name)==ID_CA) structname= "Camera";
	else if (GS(id->name)==ID_MA) structname= "Material";
	else if (GS(id->name)==ID_WO) structname= "World";
	else if (GS(id->name)==ID_SCE) structname= "Scene";
	
	if (!structname) return;
	
	offset= findstruct_offset(structname, "scriptlink");
	
	if (offset<0) return;
	
	scriptlink= (ScriptLink *) (((char *)id) + offset);

	for(i=0; i<scriptlink->totscript; i++)
		if ((Text*)scriptlink->scripts[i] == byebye)
			scriptlink->scripts[i] = NULL;
}

void clear_bad_scriptlist(ListBase *list, Text *byebye)
{
	ID *id;

	id= list->first;
	while (id) {
		clear_bad_scriptlink(id, byebye);
		
		id= id->next;
	}
}

void clear_bad_scriptlinks(Text *byebye)
{
	clear_bad_scriptlist(&G.main->object,	byebye);
	clear_bad_scriptlist(&G.main->lamp,		byebye);
	clear_bad_scriptlist(&G.main->camera,	byebye);
	clear_bad_scriptlist(&G.main->mat,		byebye);
	clear_bad_scriptlist(&G.main->world,	byebye);
	
	clear_bad_scriptlink(&G.scene->id, byebye);
	
	allqueue(REDRAWBUTSSCRIPT, 0);	
}


void start_python (void) 
{
	static int started=0;
	PyObject *mod;
	extern int Py_FrozenFlag;
	extern int Py_NoSiteFlag;
	extern int Py_UseClassExceptionsFlag;
	extern int Py_VerboseFlag;
	extern int Py_DebugFlag;
	
	if (started) return;
	started= 1;
	
	/* This gave too annoying warnings and
	 * no one seemed to use it.	*/
	if (0 && G.f & G_DEBUG) {
		Py_VerboseFlag= 1;
		Py_DebugFlag= 1;		
	} else {
		Py_FrozenFlag= 1;
		Py_NoSiteFlag= 1;
		Py_UseClassExceptionsFlag= 0;
	}
		
	Py_SetProgramName("blender");
	Py_Initialize();
	
	mdict= init_blender();
	
	gdict= PyDict_New();
	PyDict_SetItemString(gdict, "__builtins__", PyEval_GetBuiltins());


}

void end_python(void) {
	int i, len;
	PyObject *dictlist;

	dictlist= PyDict_Items(gdict);
	len= PyList_Size(dictlist);
	
	for (i=0; i<len; i++) {
		PyObject *ob= PyList_GetItem(dictlist, i);
		PyDict_DelItem(gdict, PyTuple_GetItem(ob, 0));
	}
	Py_DECREF(dictlist);
	
	Py_Finalize();
}

int py_check_err(Text *text)
{
	if (PyErr_Occurred()) {
		PyErr_Print();
		text->compiled= NULL;
		
		return 0;
	} else {
		return 1;
	}	
}

static int compile_script (Text *text)
{
	int ret=1;
	char *buf;

	if (text->compiled) return 1;
	
	buf= txt_to_buf(text);
	strcat(buf, "\n");
	
	text->compiled= Py_CompileString(buf, text->id.name+2, Py_file_input);
	freeN(buf);
	
	return py_check_err(text);
}

void txt_do_python (Text *text)
{
	if (!text) return;

	if (compile_script(text)) {
		PyEval_EvalCode(text->compiled, gdict, NULL);
		py_check_err(text);
	}
}

extern PyTypeObject PyIpoCurve_Type, PyBezTriple_Type;
#ifndef IRISGL
extern PyTypeObject Button_Type, Buffer_Type;
#endif
extern PyTypeObject NMesh_Type, NMFace_Type, NMVert_Type, NMCol_Type;
extern PyTypeObject Vector_Type, Matrix_Type;

PyTypeObject Block_Type = {
	PyObject_HEAD_INIT(NULL)
	0,									/*ob_size*/
	"Block",							/*tp_name*/
	sizeof(PyBlock),					/*tp_basicsize*/
	0,									/*tp_itemsize*/
	(destructor)	pyblock_dealloc,	/*tp_dealloc*/
	(printfunc)		pyblock_print,		/*tp_print*/
	(getattrfunc)	pyblock_getattr,	/*tp_getattr*/
	(setattrfunc)	pyblock_setattr,	/*tp_setattr*/
};

static PyObject *get_pyblock_func(void **ptr) {
	ID *id= (ID*) *ptr;
	return add_pyblock(id);
}

extern PyObject *pyicu_from_icu(IpoCurve *);

PyObject *make_icu_list (ListBase *curves) {
	ListBase lb= *curves;
	IpoCurve *icu= (IpoCurve *) lb.first;
	PyObject *list= PyList_New(0);
	
	while (icu) {
		PyList_Append(list, pyicu_from_icu(icu));
		icu= icu->next;
	}
	
	return list;	
}

DataBlockProperty Ipo_Properties[]= {
	{"curves", "curve", DBP_TYPE_FUN, 0, 0.0, 0.0, {0}, {0}, 0, 0, make_icu_list}, 
	{NULL}	
};

/*
NamedEnum TextureTypes[]= {
	{"Clouds",	TEX_CLOUDS}, 
	{"Wood",	TEX_WOOD}, 
	{"Marble",	TEX_MARBLE}, 
	{"Magic",	TEX_MAGIC}, 
	{"Blend",	TEX_BLEND}, 
	{"Stucci",	TEX_STUCCI}, 
	{"Noise",	TEX_NOISE}, 
	{"Image",	TEX_IMAGE}, 
	{"Plugin",	TEX_PLUGIN}, 
	{"Envmap",	TEX_ENVMAP}, 
	{NULL}
};

DataBlockProperty Texture_Properties[]= {
	DBP_NamedEnum("type",	"type",			TextureTypes), 
	DBP_Short("stype",		"stype",		0.0, 0.0,	0),  

	DBP_Float("noisesize",	"noisesize",	0.0, 0.0,	0),  
	DBP_Float("turbulence",	"turbul",		0.0, 0.0,	0), 
	DBP_Float("brightness",	"bright",		0.0, 0.0,	0),  
	DBP_Float("contrast",	"contrast",		0.0, 0.0,	0),  
	DBP_Float("rfac",		"rfac",			0.0, 0.0,	0),  
	DBP_Float("gfac",		"gfac",			0.0, 0.0,	0),  
	DBP_Float("bfac",		"bfac",			0.0, 0.0,	0),  
	DBP_Float("filtersize",	"filtersize",	0.0, 0.0,	0),  

	DBP_Short("noisedepth",	"noisedepth",	0.0, 0.0,	0),  
	DBP_Short("noisetype",	"noisetype",	0.0, 0.0,	0),  

	{NULL}	
};
*/

DataBlockProperty Camera_Properties[]= {
	{"Lens",	"lens",		DBP_TYPE_FLO, 0, 1.0,	250.0}, 
	{"ClSta",	"clipsta",	DBP_TYPE_FLO, 0, 0.0,	100.0}, 
	{"ClEnd",	"clipend",	DBP_TYPE_FLO, 0, 1.0,	5000.0}, 

	{"ipo",		"*ipo",		DBP_TYPE_FUN, 0, 0.0,	0.0, {0}, {0}, 0, 0, get_pyblock_func}, 
	
	{NULL}
};

DataBlockProperty World_Properties[]= {
	{"HorR",	"horr",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"HorG",	"horg",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"HorB",	"horb",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"ZenR",	"zenr",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"ZenG",	"zeng",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"ZenB",	"zenb",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"Expos",	"exposure",	DBP_TYPE_FLO, 0, 0.0,	5.0}, 
	{"MisSta",	"miststa",	DBP_TYPE_FLO, 0, 0.0,	1000.0}, 
	{"MisDi",	"mistdist",	DBP_TYPE_FLO, 0, 0.0,	1000.0}, 
	{"MisHi",	"misthi",	DBP_TYPE_FLO, 0, 0.0,	100.0}, 
	{"StarDi",	"stardist",	DBP_TYPE_FLO, 0, 2.0,	1000.0}, 
	{"StarSi",	"starsize",	DBP_TYPE_FLO, 0, 0.0,	10.0}, 

	{"ipo",		"*ipo",		DBP_TYPE_FUN, 0, 0.0,	0.0, {0}, {0}, 0, 0, get_pyblock_func}, 
	
	{NULL}
};

DataBlockProperty Lamp_Properties[]= {
	{"R",		"r",			DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"G",		"g",			DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"B",		"b",			DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"Energ",	"energy",		DBP_TYPE_FLO, 0, 0.0,	10.0}, 
	{"Dist",	"dist",			DBP_TYPE_FLO, 0, 0.01,	5000.0}, 
	{"SpoSi",	"spotsize",		DBP_TYPE_FLO, 0, 1.0,	180.0}, 
	{"SpoBl",	"spotblend",	DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"HaInt",	"haint",		DBP_TYPE_FLO, 0, 1.0,	5.0}, 
	{"Quad1",	"att1",			DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"Quad2",	"att2",			DBP_TYPE_FLO, 0, 0.0,	1.0}, 

	{"ipo",		"*ipo",		DBP_TYPE_FUN, 0, 0.0,	0.0, {0}, {0}, 0, 0, get_pyblock_func}, 
	
	{NULL}
};

DataBlockProperty Material_Properties[]= {
	{"R",		"r",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"G",		"g",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"B",		"b",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"SpecR",	"specr",	DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"SpecG",	"specg",	DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"SpecB",	"specb",	DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"MirR",	"mirr",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"MirG",	"mirg",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"MirB",	"mirb",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"Ref",		"ref",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"Alpha",	"alpha",	DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"Emit",	"emit",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"Amb",		"amb",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"Spec",	"spec",		DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"SpTra",	"spectra",	DBP_TYPE_FLO, 0, 0.0,	1.0}, 
	{"HaSize",	"hasize",	DBP_TYPE_FLO, 0, 0.0,	10000.0}, 

	{"Mode",	"mode",		DBP_TYPE_INT, 0, 0.0,	0.0},  
	{"Hard",	"har",		DBP_TYPE_SHO, 0, 1.0,	128.0},  

	{"ipo",		"*ipo",		DBP_TYPE_FUN, 0, 0.0,	0.0, {0}, {0}, 0, 0, get_pyblock_func}, 

	{NULL} 
};

#include "ika.h"

static float zero_float= 0.0;

void *Object_getattr(void *vdata, char *name) {
	Object *ob= (Object *) vdata;
	
	if (STREQ(name, "layer")) {
		return &ob->lay;
		
	} else if (strncmp(name, "eff", 3)==0) {
		Ika *ika= ob->data;

		if (ob->type==OB_IKA && ika) {
			if (name[3]=='x') return &ika->effg[0];
			else if (name[3]=='y') return &ika->effg[1];
			else if (name[3]=='z') return &ika->effg[2];
		}
	
		return &zero_float;

	} else if (STREQ(name, "mat")) {
		disable_where_script(1);
		where_is_object(ob);
		disable_where_script(0);
		
		return &ob->obmat;
	}
	
	return py_err_ret_ob(PyExc_AttributeError, name);
}

int Object_setattr(void *vdata, char *name, PyObject *py_ob) {
	Object *ob= (Object *) vdata;

	if (STREQ(name, "layer")) {
		Base *base;
		int ival;
		
		if (!PyArg_Parse(py_ob, "i", &ival)) return -1;
		
		ob->lay= ival;
			
		base= FIRSTBASE;
		while (base) {
			if (base->object == ob) base->lay= ob->lay;
			base= base->next;
		}
		return 0;
	} else if (strncmp(name, "eff", 3)==0) {
		Ika *ika= ob->data;
		float fval;
		
		if (!PyArg_Parse(py_ob, "f", &fval)) return -1;
		
		if (ob->type==OB_IKA && ika) {
			if (name[3]=='x') ika->effg[0]= fval;
			else if (name[3]=='y') ika->effg[1]= fval;
			else if (name[3]=='z') ika->effg[2]= fval;
			
			itterate_ika(ob);
		}
		return 0;
	}

	PyErr_SetString(PyExc_AttributeError, name);
	return -1;
}

DataBlockProperty Object_Properties[]= {
	{"LocX",	"loc[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {0}, {3, -sizeof(float)}}, 
	{"LocY",	"loc[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {1}, {3, -sizeof(float)}}, 
	{"LocZ",	"loc[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {2}, {3, -sizeof(float)}}, 
	{"loc",		"loc[3]",	DBP_TYPE_VEC, 0, 3.0}, 

	{"dLocX",	"dloc[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {0}, {3, -sizeof(float)}}, 
	{"dLocY",	"dloc[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {1}, {3, -sizeof(float)}}, 
	{"dLocZ",	"dloc[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {2}, {3, -sizeof(float)}}, 
	{"dloc",	"dloc[3]",	DBP_TYPE_VEC, 0, 3.0}, 

	{"RotX",	"rot[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {0}, {3, -sizeof(float)}}, 
	{"RotY",	"rot[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {1}, {3, -sizeof(float)}}, 
	{"RotZ",	"rot[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {2}, {3, -sizeof(float)}}, 
	{"rot",		"rot[3]",	DBP_TYPE_VEC, 0, 3.0}, 

	{"dRotX",	"drot[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {0}, {3, -sizeof(float)}}, 
	{"dRotY",	"drot[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {1}, {3, -sizeof(float)}}, 
	{"dRotZ",	"drot[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {2}, {3, -sizeof(float)}}, 
	{"drot",	"drot[3]",	DBP_TYPE_VEC, 0, 3.0}, 

	{"SizeX",	"size[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {0}, {3, -sizeof(float)}}, 
	{"SizeY",	"size[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {1}, {3, -sizeof(float)}}, 
	{"SizeZ",	"size[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {2}, {3, -sizeof(float)}}, 
	{"size",	"size[3]",	DBP_TYPE_VEC, 0, 3.0}, 

	{"dSizeX",	"dsize[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {0}, {3, -sizeof(float)}}, 
	{"dSizeY",	"dsize[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {1}, {3, -sizeof(float)}}, 
	{"dSizeZ",	"dsize[3]",	DBP_TYPE_FLO, 0, 0.0,	0.0, {2}, {3, -sizeof(float)}}, 
	{"dsize",	"dsize[3]",	DBP_TYPE_VEC, 0, 3.0}, 

	{"EffX",	"effx",		DBP_TYPE_FLO, DBP_TYPE_FUN, 0.0,	0.0, {0}, {0}, DBP_HANDLING_FUNC, Object_getattr, 0, Object_setattr}, 
	{"EffY",	"effy",		DBP_TYPE_FLO, DBP_TYPE_FUN, 0.0,	0.0, {0}, {0}, DBP_HANDLING_FUNC, Object_getattr, 0, Object_setattr}, 
	{"EffZ",	"effz",		DBP_TYPE_FLO, DBP_TYPE_FUN, 0.0,	0.0, {0}, {0}, DBP_HANDLING_FUNC, Object_getattr, 0, Object_setattr}, 

	{"Layer",	"layer",	DBP_TYPE_INT, DBP_TYPE_FUN, 0.0,	0.0, {0}, {0}, DBP_HANDLING_FUNC, Object_getattr, 0, Object_setattr}, 

	{"parent",	"*parent",	DBP_TYPE_FUN, 0, 0.0,	0.0, {0}, {0}, 0, 0, get_pyblock_func}, 
	{"track",	"*track",	DBP_TYPE_FUN, 0, 0.0,	0.0, {0}, {0}, 0, 0, get_pyblock_func}, 
	{"data",	"*data",	DBP_TYPE_FUN, 0, 0.0,	0.0, {0}, {0}, 0, 0, get_pyblock_func}, 
	{"ipo",		"*ipo",		DBP_TYPE_FUN, 0, 0.0,	0.0, {0}, {0}, 0, 0, get_pyblock_func}, 

	{"mat",		"mat",		DBP_TYPE_FUN, 0, 0.0, 0.0, {0}, {0}, DBP_HANDLING_FUNC, Object_getattr, newMatrixObject}, 
	
	{NULL}
};

void pyblock_dealloc(PyObject *self) {
	PyMem_DEL(self);
}

int pyblock_print(PyObject *self, FILE *fp, int flags) {
	PyBlock *block= (PyBlock *) self;
	
	fprintf (fp, "[%s %s]", block->type, ((ID*)block->data)->name+2);
	
	return 0;
}

PyObject *pyblock_getattr(PyObject *self, char *name) {
	PyBlock *block= (PyBlock*) self;

	if (STREQ(name, "name"))
		return PyString_FromString((((ID*)block->data)->name)+2);
	else if (STREQ(name, "block_type"))
		return PyString_FromString(block->type);
	
	return datablock_getattr(block->properties, block->type, name, block->data);
}

int pyblock_setattr(PyObject *self, char *name, PyObject *ob) {
	PyBlock *block= (PyBlock*) self;

	if (STREQ(name, "name")) {
		if (!PyArg_Parse(ob, "s", &name)) return -1;

		new_id(block->type_list, (ID*)block->data, name);
		
		return 0;
	}
	
	return datablock_setattr(block->properties, block->type, name, block->data, ob);
}

int pyblock_is_type(PyBlock *block, int type) {
	ID *id= (ID *) block->data;
	return (GS(id->name))==type;
}

PyObject *add_pyblock(void *data) {
	PyBlock *newb;
	ID *id= (ID *) data;
	int idn;
	
	if (!data) return py_incr_ret(Py_None);

	idn= GS(id->name);
	
	if (idn==ID_OB) {
		newb= PyObject_NEW(PyBlock, &Block_Type);
		newb->type= "Object";
		newb->type_list= &G.main->object;
		newb->properties= Object_Properties;

	} else if (idn==ID_ME) {
		/* Special case, should be fixed
		 * by proper high-level mesh access.
		 * 
		 * Later.
		 */
		 
		return newNMeshObject(data);			

	} else if (idn==ID_CU) {
		/* Special case, should be fixed
		 * by proper high-level NURBS access.
		 * 
		 * Later.
		 */
		 
		return newNCurveObject(data);			

	} else if (idn==ID_LA) {
		newb= PyObject_NEW(PyBlock, &Block_Type);
		newb->type= "Lamp";
		newb->type_list= &G.main->lamp;
		newb->properties= Lamp_Properties;

	} else if (idn==ID_CA) {
		newb= PyObject_NEW(PyBlock, &Block_Type);
		newb->type= "Camera";
		newb->type_list= &G.main->camera;
		newb->properties= Camera_Properties;

	} else if (idn==ID_MA) {
		newb= PyObject_NEW(PyBlock, &Block_Type);
		newb->type= "Material";
		newb->type_list= &G.main->mat;
		newb->properties= Material_Properties;
		
	} else if (idn==ID_WO) {
		newb= PyObject_NEW(PyBlock, &Block_Type);
		newb->type= "World";
		newb->type_list= &G.main->world;
		newb->properties= World_Properties;

	} else if (idn==ID_IP) {
		newb= PyObject_NEW(PyBlock, &Block_Type);
		newb->type= "Ipo";
		newb->type_list= &G.main->ipo;
		newb->properties= Ipo_Properties;		

/*
	} else if (idn==ID_TE) {
		newb= PyObject_NEW(PyBlock, &Block_Type);
		newb->type= "Tex";
		newb->type_list= &G.main->tex;
		newb->properties= Texture_Properties;		
*/

	} else return py_err_ret_ob(PyExc_SystemError, "unable to create Block for data");
	
	newb->data= data;
	
	return (PyObject *) newb;
}

PyObject *py_find_from_list(ID *list, PyObject *args) {
	char *name= NULL;
	
	Py_Try(PyArg_ParseTuple(args, "|s", &name));
	
	if (name) {
		while (list) {
			if (strcmp(name, list->name+2)==0) 
				return add_pyblock(list);

			list= list->next;
		}
		return py_incr_ret(Py_None);
		
	} else {
		PyObject *pylist= PyList_New(countlist((ListBase*)&list));
		int i=0;
	
		while (list) {
			PyObject *ob= add_pyblock(list);
			
			if (!ob) {
				Py_DECREF(pylist);
				return NULL;
			}
			
			PyList_SetItem(pylist, i, ob);	
		
			list= list->next; i++;
		}
		return pylist;
	}
}

char *py_event_to_name(short event) {
	switch (event) {
	case SCRIPT_FRAMECHANGED:
		return "FrameChanged";
	case SCRIPT_ONLOAD:
		return "OnLoad";
	case SCRIPT_REDRAW:
		return "Redraw";
	default:
		return "Unknown";
	}
}

void do_pyscript(ID *id, short event) 
{
	ScriptLink *scriptlink;
	int offset;
	PyObject *arg, *evt;
	char *structname=NULL;
	int i;

	if (GS(id->name)==ID_OB) structname= "Object";
	else if (GS(id->name)==ID_LA) structname= "Lamp";
	else if (GS(id->name)==ID_CA) structname= "Camera";
	else if (GS(id->name)==ID_MA) structname= "Material";
	else if (GS(id->name)==ID_WO) structname= "World";
	else if (GS(id->name)==ID_SCE) structname= "Scene";
	else return;
	
	offset= findstruct_offset(structname, "scriptlink");
	if (offset<0) {
		printf ("Internal error, unable to find script link\n");
		return;
	}
	
	scriptlink= (ScriptLink *) (((char *)id) + offset);
	if (!scriptlink->totscript) return;
		
	if (GS(id->name)==ID_SCE)
		arg= py_incr_ret(Py_None);
	else
		arg= add_pyblock(id);
	if (!arg) {
		printf ("Internal error, unable to create PyBlock for script link\n");
		return;
	}
	
	evt= PyString_FromString(py_event_to_name(event));
	
	PyDict_SetItemString(mdict, "link", arg);
	PyDict_SetItemString(mdict, "event", evt);
	PyDict_SetItemString(mdict, "bylink", py_incr_ret(Py_True));
	
	disable_where_script(1);
	for(i=0; i<scriptlink->totscript; i++)	{
		if (scriptlink->flag[i]==event && scriptlink->scripts[i]) {
				
			if (compile_script((Text*) scriptlink->scripts[i])) {
				PyEval_EvalCode(((Text*) scriptlink->scripts[i])->compiled, gdict, NULL);

				if(PyErr_Occurred()) {
					PyErr_Print();
					((Text*) scriptlink->scripts[i])->compiled= NULL;
				}
			}
		}
	}
	disable_where_script(0);	

	if(PyDict_GetItemString(mdict, "link")) PyDict_DelItemString(mdict, "link");
	if(PyDict_GetItemString(mdict, "event")) PyDict_DelItemString(mdict, "event");
	
	if(PyErr_Occurred()) PyErr_Print();
	
	PyDict_SetItemString(mdict, "bylink", py_incr_ret(Py_False));
	
}

void do_all_scriptlist(ListBase *list, short event)
{
	ID *id;
	
	id= list->first;
	while (id) {
		do_pyscript (id, event);
		
		id= id->next;
	}
	
}

void do_all_scripts(short event) 
{
	do_all_scriptlist(&G.main->object, event);
	do_all_scriptlist(&G.main->lamp, event);
	do_all_scriptlist(&G.main->camera, event);
	do_all_scriptlist(&G.main->mat, event);
	do_all_scriptlist(&G.main->world, event);
	
	do_pyscript(&G.scene->id, event);
}

/*****************************/
/*  Main interface routines  */
/*****************************/

PyObject *py_incr_ret(PyObject *ob) {
	Py_INCREF(ob);
	
	return ob;
}

PyObject *py_err_ret_ob(PyObject *type, char *err) {
	PyErr_SetString(type, err);
	return NULL;
}

int py_err_ret_int(PyObject *type, char *err) {
	PyErr_SetString(type, err);
	return -1;
}

int py_check_sequence_consistency(PyObject *seq, PyTypeObject *against)
{
	PyObject *ob;
	int len= PySequence_Length(seq);
	int i;
	
	for (i=0; i<len; i++) {
		ob= PySequence_GetItem(seq, i);
		if (ob->ob_type != against) {
			Py_DECREF(ob);
			return 0;
		}
		Py_DECREF(ob);
	}
	return 1;
}

static char Method_Redraw_doc[]= "() - Force a redraw of all 3d windows";
static PyObject *Method_Redraw(PyObject *self, PyObject *args) 
{
	ScrArea *tempsa, *sa;
	int queue= 0;
	
	Py_Try(PyArg_ParseTuple(args, "|i", &queue));
	
	if (!during_script()) {
		if (queue) {
			allqueue(REDRAWVIEW3D, 0);
		} else {
			tempsa= curarea;
			sa= G.curscreen->areabase.first;
			while(sa) {
				if (sa->spacetype== SPACE_VIEW3D) {
					areawinset(sa->win);
					sa->windraw();
				}

				sa= sa->next;
			}
			if(curarea!=tempsa) areawinset(tempsa->win);

			screen_swapbuffers();
		}
	}

	return py_incr_ret(Py_None);
}

#define BP_CURFRAME 1
#define BP_CURTIME  2
#define BP_FILENAME 3

static char Method_Set_doc[]=
"(request, data) - Update settings in Blender\n\
\n\
(request) A string indentifying the setting to change\n\
	'curframe'	- Sets the current frame using the number in data";

static PyObject *Method_Set (PyObject *self, PyObject *args)
{
	char *name;
	PyObject *arg;
	
	Py_Try(PyArg_ParseTuple(args, "sO", &name, &arg));

	if (STREQ(name, "curframe")) {
		int framenum;
		
		Py_Try(PyArg_Parse(arg, "i", &framenum));
		
		CFRA= framenum;
		
		do_global_buttons(B_NEWFRAME);
		
	} else {
		return py_err_ret_ob(PyExc_AttributeError, "bad request identifier");
	}

	return py_incr_ret(Py_None);
}

static char Method_Get_doc[]=
"(request) - Retrieve settings from Blender\n\
\n\
(request) A string indentifying the data to be returned\n\
	'curframe'	- Returns the current animation frame\n\
	'curtime'	- Returns the current animation time\n\
	'staframe'	- Returns the start frame of the animation\n\
	'endframe'	- Returns the end frame of the animation\n\
	'filename'	- Returns the name of the last file read or written\n\
	'version'	- Returns the Blender version number";

static PyObject *Method_Get (PyObject *self, PyObject *args)
{	
	PyObject *ob;

	Py_Try(PyArg_ParseTuple(args, "O", &ob));

	if (PyInt_Check(ob) && G.version<180) {
		int item= PyInt_AsLong(ob);
		
		switch (item) {
		case BP_CURFRAME:
			return PyInt_FromLong(CFRA);

		case BP_CURTIME:
			return PyFloat_FromDouble(frame_to_float(CFRA));

		case BP_FILENAME:
			return PyString_FromString(G.sce);
		
		default:
			break;
		}
	} else if (PyString_Check(ob)) {
		char *str= PyString_AsString(ob);
		
		if (STREQ(str, "curframe")) 
			return PyInt_FromLong(CFRA);
			
		else if (STREQ(str, "curtime"))
			return PyFloat_FromDouble(frame_to_float(CFRA));
			
		else if (STREQ(str, "staframe")) 
			return PyInt_FromLong(SFRA);
			
		else if (STREQ(str, "endframe")) 
			return PyInt_FromLong(EFRA);
			
		else if (STREQ(str, "filename"))
			return PyString_FromString(G.sce);
			
		else if (STREQ(str, "version"))
			return PyInt_FromLong(G.version);
	} else {
		return py_err_ret_ob(PyExc_AttributeError, "expected string argument");
	}

	return py_err_ret_ob(PyExc_AttributeError, "bad request identifier");
}

static struct PyMethodDef Blender_methods[] = {
	MethodDef(Redraw), 
	MethodDef(Get),
	MethodDef(Set), 
	{NULL, NULL}
};

static struct PyMethodDef Null_methods[] = {
	{NULL, NULL}
};

static PyObject *init_blender(void)
{
	PyObject *mod, *dict, *tmod, *tdict;

	Block_Type.ob_type = &PyType_Type;
	
	mod= Py_InitModule("Blender", Blender_methods);

	dict= PyModule_GetDict(mod);
	PyDict_SetItemString(dict, "bylink", py_incr_ret(Py_False));

		/* Blender object modules */
	load_py_datablocks(dict);

	PyDict_SetItemString(dict, "NMesh",		init_py_nmesh());

	PyDict_SetItemString(dict, "NURBS",		init_py_nurbs());

	PyDict_SetItemString(dict, "Draw",		init_py_draw());
#ifndef IRISGL
	PyDict_SetItemString(dict, "BGL",		init_py_bgl());
#endif

	init_py_vector();
	init_py_matrix();

	PyDict_SetItemString(dict, "Demo",		init_py_demo());
	PyDict_SetItemString(dict, "Interface", init_py_interface());

	/* Auto depreciation ;) */
	if (G.version<180) {
		tmod= Py_InitModule("Blender.Const", Null_methods);
		PyDict_SetItemString(dict, "Const", tmod);
	
		tdict= PyModule_GetDict(tmod);

		PyDict_SetItemString(tdict, "BP_CURFRAME", PyInt_FromLong(BP_CURFRAME));
		PyDict_SetItemString(tdict, "BP_CURTIME", PyInt_FromLong(BP_CURTIME));

		PyDict_SetItemString(tdict, "CURFRAME", PyInt_FromLong(BP_CURFRAME));
		PyDict_SetItemString(tdict, "CURTIME",	PyInt_FromLong(BP_CURTIME));
		PyDict_SetItemString(tdict, "FILENAME", PyInt_FromLong(BP_FILENAME));
	}

	tmod= Py_InitModule("Blender.Types", Null_methods);
	PyDict_SetItemString(dict, "Types", tmod);
	
	tdict= PyModule_GetDict(tmod);

	PyDict_SetItemString(tdict, "IpoCurve", (PyObject *)&PyIpoCurve_Type);
	PyDict_SetItemString(tdict, "BezTriple", (PyObject *)&PyBezTriple_Type);

	PyDict_SetItemString(tdict, "ButtonType", (PyObject *)&Button_Type);
	#ifndef IRISGL
	PyDict_SetItemString(tdict, "BufferType", (PyObject *)&Buffer_Type);
	#endif
	PyDict_SetItemString(tdict, "NMeshType", (PyObject *)&NMesh_Type);
	PyDict_SetItemString(tdict, "NMFaceType", (PyObject *)&NMFace_Type);
	PyDict_SetItemString(tdict, "NMVertType", (PyObject *)&NMVert_Type);
	PyDict_SetItemString(tdict, "NMColType", (PyObject *)&NMCol_Type);

	PyDict_SetItemString(tdict, "BlockType", (PyObject *)&Block_Type);
	
	/* Setup external types */
	PyDict_SetItemString(tdict, "VectorType", (PyObject *)&Vector_Type);
	PyDict_SetItemString(tdict, "MatrixType", (PyObject *)&Matrix_Type);

	return dict;
}


/* Hack for simple python stuff in GBlender prototype */
/* Zr sends his apologies for all his ugly code */

typedef struct actvar {
	struct actvar *next, *prev;
	Object *ob;			/* bij lokale variable */
	char *name;
	short *poin;		/* staat op ->var of op life */
	short nr, var, flag, rt;	
} actvar;

int py_eval_string_ret_int(char *str, actvar *vars, int numvars) {
	PyObject *result, *dict;
	int ret= -1;
	
	dict= PyDict_New();
	
	while (numvars--) {
		PyDict_SetItemString(dict,vars->name,PyInt_FromLong(vars->var));
		vars++;
	}
	
	result= PyRun_String(str,Py_eval_input,gdict,dict);

	if (!result) PyErr_Print();
	else {
		if (PyNumber_Check(result))
			ret= PyInt_AsLong(result);
	
		Py_DECREF(result);
	}

	Py_DECREF(dict);
	
	return ret;
}


