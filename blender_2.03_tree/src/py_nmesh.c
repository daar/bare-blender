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
 * Version: $Id: py_nmesh.c,v 1.6 2000/08/09 00:32:08 jan Exp $
 */

#include "blender.h"
#include "py_blender.h"
#include "screen.h"

PyTypeObject NMesh_Type;
PyTypeObject NMFace_Type;
PyTypeObject NMVert_Type;
PyTypeObject NMCol_Type;

#define NMesh_Check(v)       ((v)->ob_type == &NMesh_Type)
#define NMFace_Check(v)      ((v)->ob_type == &NMFace_Type)
#define NMVert_Check(v)      ((v)->ob_type == &NMVert_Type)
#define NMCol_Check(v)       ((v)->ob_type == &NMCol_Type)

/*****************************/
/*	    Mesh Color Object    */
/*****************************/

typedef struct _NMCol {
	PyObject_VAR_HEAD

	unsigned char r, g, b, a;
} NMCol;

static void NMCol_dealloc(PyObject *self) {
	PyMem_DEL(self);
}

static NMCol *newcol (char r, char g, char b, char a) {
	NMCol *mc= (NMCol *) PyObject_NEW(NMCol, &NMCol_Type);
	
	mc->r= r;
	mc->g= g;
	mc->b= b;
	mc->a= a;

	return mc;	
}

static char Method_Col_doc[]=
"([r, g, b, a]) - Get a new mesh color\n\
\n\
[r=255, g=255, b=255, a=255] Specify the color components";

static PyObject *Method_Col(PyObject *self, PyObject *args) {
	NMCol *mc;
	int r=255, g=255, b=255, a=255;
	
	Py_Try(PyArg_ParseTuple(args, "|iiii", &r, &g, &b, &a));
	
	return (PyObject *) newcol(r, g, b, a);
}

static PyObject *NMCol_getattr(PyObject *self, char *name) {
	NMCol *mc= (NMCol *) self;
		
	if (strcmp(name, "r")==0) return Py_BuildValue("i", mc->r);
	else if (strcmp(name, "g")==0) return Py_BuildValue("i", mc->g);
	else if (strcmp(name, "b")==0) return Py_BuildValue("i", mc->b);
	else if (strcmp(name, "a")==0) return Py_BuildValue("i", mc->a);

	PyErr_SetString(PyExc_AttributeError, name);
	return NULL;
}

static int NMCol_setattr(PyObject *self, char *name, PyObject *v) {
	NMCol *mc= (NMCol *) self;
	int ival;
	
	if(!PyArg_Parse(v, "i", &ival)) return -1;
	
	CLAMP(ival, 0, 255);
	
	if (strcmp(name, "r")==0) mc->r= ival;
	else if (strcmp(name, "g")==0) mc->g= ival;
	else if (strcmp(name, "b")==0) mc->b= ival;
	else if (strcmp(name, "a")==0) mc->a= ival;
	else return -1;
	
	return 0;
}

static int NMCol_print (PyObject *self, FILE *fp, int flags) {
	NMCol *mc= (NMCol *) self;
		
	fprintf (fp, "[NMCol - <%d, %d, %d, %d>]", mc->r, mc->g, mc->b, mc->a);
	
	return 0;
}

PyTypeObject NMCol_Type = {
	PyObject_HEAD_INIT(NULL)
	0,								/*ob_size*/
	"NMCol",						/*tp_name*/
	sizeof(NMCol),					/*tp_basicsize*/
	0,								/*tp_itemsize*/
	/* methods */
	(destructor) NMCol_dealloc,		/*tp_dealloc*/
	(printfunc) NMCol_print,		/*tp_print*/
	(getattrfunc) NMCol_getattr,	/*tp_getattr*/
	(setattrfunc) NMCol_setattr,	/*tp_setattr*/
	0,								/*tp_compare*/
	0,								/*tp_repr*/
	0,								/*tp_as_number*/
	0,								/*tp_as_sequence*/
	0,								/*tp_as_mapping*/
	0,								/*tp_hash*/
};

/*****************************/
/*    NMesh Python Object    */
/*****************************/

typedef struct _NMFace {
	PyObject_VAR_HEAD
	
	PyObject *v;
	PyObject *col;
	char mat_nr, smooth;
} NMFace;

static void NMFace_dealloc(PyObject *self) {
	NMFace *mf= (NMFace *) self;
	
	Py_DECREF(mf->v);
	Py_DECREF(mf->col);
	
	PyMem_DEL(self);

}

static NMFace *newface(void) {
	NMFace *mf= PyObject_NEW(NMFace, &NMFace_Type);

	mf->v= PyList_New(0);
	mf->col= PyList_New(0);

	mf->smooth= 0;
	mf->mat_nr= 0;
	
	return mf;
}

static char Method_Face_doc[]=
"() - Get a new face";
static PyObject *Method_Face(PyObject *self, PyObject *args) {
	Py_Try(PyArg_ParseTuple(args, ""));
	
	return (PyObject *) newface();
}

static PyObject *NMFace_getattr(PyObject *self, char *name) {
	PyObject *list;
	NMFace *mf= (NMFace *) self;
		
	if(strcmp(name, "v")==0) 
		return Py_BuildValue("O", mf->v);
	else if (strcmp(name, "col")==0) 
		return Py_BuildValue("O", mf->col);
	else if (strcmp(name, "mat")==0) 
		return Py_BuildValue("i", mf->mat_nr);
	else if (strcmp(name, "smooth")==0)
		return Py_BuildValue("i", mf->smooth);
		
	PyErr_SetString(PyExc_AttributeError, name);
	return NULL;
}

static int NMFace_setattr(PyObject *self, char *name, PyObject *v) {
	NMFace *mf= (NMFace *) self;
	int ival;
	
	if (STREQ(name, "v")) {
		if(PySequence_Check(v)) {
			Py_DECREF(mf->v);
			mf->v= py_incr_ret(v);

			return 0;
		}
	} else if (STREQ(name, "col")) {
		if(PySequence_Check(v)) {
			Py_DECREF(mf->col);
			mf->col= py_incr_ret(v);

			return 0;
		}
	} else if (STREQ(name, "mat")) {
		PyArg_Parse(v, "i", &ival);

		mf->mat_nr= ival;
		
		return 0;
	} else if (STREQ(name, "smooth")) {
		PyArg_Parse(v, "i", &ival);

		mf->smooth= ival?1:0;
		
		return 0;
	}
	
	PyErr_SetString(PyExc_AttributeError, name);
	return -1;
}

static int NMFace_print (PyObject *self, FILE *fp, int flags) {
	NMFace *mf= (NMFace *) self;
		
	fprintf (fp, "[NMFace]");
	
	return 0;
}

PyTypeObject NMFace_Type = {
	PyObject_HEAD_INIT(NULL)
	0,							/*ob_size*/
	"NMFace",						/*tp_name*/
	sizeof(NMFace),			/*tp_basicsize*/
	0,							/*tp_itemsize*/
	/* methods */
	(destructor) NMFace_dealloc,	/*tp_dealloc*/
	(printfunc) NMFace_print,		/*tp_print*/
	(getattrfunc) NMFace_getattr,	/*tp_getattr*/
	(setattrfunc) NMFace_setattr,/*tp_setattr*/
	0,							/*tp_compare*/
	0,							/*tp_repr*/
	0,							/*tp_as_number*/
	0,							/*tp_as_sequence*/
	0,							/*tp_as_mapping*/
	0,							/*tp_hash*/
};

typedef struct _NMVert {
	PyObject_VAR_HEAD

	float co[3];
	float no[3];
	float uvco[3];
	
	int index;
} NMVert;

static void NMVert_dealloc(PyObject *self) {
	PyMem_DEL(self);
}

static NMVert *newvert(float *co) {
	NMVert *mv= PyObject_NEW(NMVert, &NMVert_Type);

	VECCOPY(mv->co, co);
	mv->no[0]= mv->no[1]= mv->no[2]= 0.0;
	mv->uvco[0]= mv->uvco[1]= mv->uvco[2]= 0.0;
	
	return mv;
}

static char Method_Vert_doc[]=
"([x, y, z]) - Get a new vertice\n\
\n\
[x, y, z] Specify new coordinates";

static PyObject *Method_Vert(PyObject *self, PyObject *args) {
	float co[3]= {0.0, 0.0, 0.0};
	
	Py_Try(PyArg_ParseTuple(args, "|fff", &co[0], &co[1], &co[2]));
	
	return (PyObject *) newvert(co);
}

static PyObject *NMVert_getattr(PyObject *self, char *name) {
	PyObject *list;
	NMVert *mv= (NMVert *) self;

	if (STREQ(name, "co")) return newVectorObject(mv->co, 3);
	else if (STREQ(name, "no")) return newVectorObject(mv->no, 3);		
	else if (STREQ(name, "uvco")) return newVectorObject(mv->uvco, 3);		
	else if (STREQ(name, "index")) return PyInt_FromLong(mv->index);		
	
	PyErr_SetString(PyExc_AttributeError, name);
	return NULL;
}

static int NMVert_setattr(PyObject *self, char *name, PyObject *v) {
	NMVert *mv= (NMVert *) self;
	int i;
	
	if (STREQ(name,"index")) {
		PyArg_Parse(v, "i", &i);
		mv->index= i;
		return 0;
	}
	
	PyErr_SetString(PyExc_AttributeError, name);
	return -1;
}

static int NMVert_print (PyObject *self, FILE *fp, int flags) {
	NMVert *mv= (NMVert *) self;

	fprintf (fp, "[NMVert]");
	
	return 0;
}

PyTypeObject NMVert_Type = {
	PyObject_HEAD_INIT(NULL)
	0,							/*ob_size*/
	"NMVert",						/*tp_name*/
	sizeof(NMVert),			/*tp_basicsize*/
	0,							/*tp_itemsize*/
	/* methods */
	(destructor) NMVert_dealloc,	/*tp_dealloc*/
	(printfunc) NMVert_print,		/*tp_print*/
	(getattrfunc) NMVert_getattr,	/*tp_getattr*/
	(setattrfunc) NMVert_setattr,/*tp_setattr*/
	0,							/*tp_compare*/
	0,							/*tp_repr*/
	0,							/*tp_as_number*/
	0,							/*tp_as_sequence*/
	0,							/*tp_as_mapping*/
	0,							/*tp_hash*/
};

typedef struct _NMesh {
	PyObject_VAR_HEAD

	PyObject *name;
	PyObject *mats;
	PyObject *verts;
	PyObject *faces;
	
	char flags;
#define NMESH_HASMCOL	1<<0
#define NMESH_HASUVCO	1<<1

} NMesh;

static void NMesh_dealloc(PyObject *self) {
	NMesh *me= (NMesh *) self;

	Py_DECREF(me->name);
	Py_DECREF(me->verts);
	Py_DECREF(me->faces);
	
	PyMem_DEL(self);
}

static PyObject *NMesh_getattr(PyObject *self, char *name) {
	NMesh *me= (NMesh *) self;
	
	if (STREQ(name, "name")) 
		return py_incr_ret(me->name);
	
	else if (STREQ(name, "block_type"))
	  return PyString_FromString("NMesh");

	else if (STREQ(name, "mats"))
		return py_incr_ret(me->mats);

	else if (STREQ(name, "verts"))
		return py_incr_ret(me->verts);
		
	else if (STREQ(name, "faces"))
		return py_incr_ret(me->faces);
	
	else if (STREQ(name, "has_col")) {
		if (me->flags & NMESH_HASMCOL)
			return py_incr_ret(Py_True);
		else
			return py_incr_ret(Py_False);
		
	} else if (STREQ(name, "has_uvco")) {
		if (me->flags & NMESH_HASUVCO)
			return py_incr_ret(Py_True);
		else
			return py_incr_ret(Py_False);
	}

	PyErr_SetString(PyExc_AttributeError, name);
	return NULL;
}

static int NMesh_setattr(PyObject *self, char *name, PyObject *v) {
	NMesh *me= (NMesh *) self;
	PyObject *ob;
	
	if (STREQ2(name, "has_col", "has_uvco")) {
		int flag, ival;
		
		if(STREQ(name, "has_col")) flag= NMESH_HASMCOL;
		else flag= NMESH_HASUVCO;
		
		PyArg_Parse(v, "i", &ival);

		if(ival) me->flags |= flag;
		else me->flags &= ~flag;
		
	} else if (STREQ3(name, "verts", "faces", "mats")) {
		if(PySequence_Check(v)) {
			if(STREQ(name, "mats")) {
				Py_DECREF(me->mats);
				me->mats= py_incr_ret(v);
			} else if (STREQ(name, "verts")) {
				Py_DECREF(me->verts);
				me->verts= py_incr_ret(v);
			} else {
				Py_DECREF(me->faces);
				me->faces= py_incr_ret(v);				
			}
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

static int NMesh_print (PyObject *self, FILE *fp, int flags) {
	fprintf (fp, "[NMesh]");
	
	return 0;
}

PyTypeObject NMesh_Type = {
	PyObject_HEAD_INIT(NULL)
	0,								/*ob_size*/
	"NMesh",						/*tp_name*/
	sizeof(NMesh),					/*tp_basicsize*/
	0,								/*tp_itemsize*/
	/* methods */
	(destructor)	NMesh_dealloc,	/*tp_dealloc*/
	(printfunc)		NMesh_print,	/*tp_print*/
	(getattrfunc)	NMesh_getattr,	/*tp_getattr*/
	(setattrfunc)	NMesh_setattr,	/*tp_setattr*/
	0,								/*tp_compare*/
	0,								/*tp_repr*/
	0,								/*tp_as_number*/
	0,								/*tp_as_sequence*/
	0,								/*tp_as_mapping*/
	0,								/*tp_hash*/
};

static NMFace *nmface_from_data(NMesh *mesh, MFace *face, MCol *col, int idx) 
{
	NMFace *mf= PyObject_NEW(NMFace, &NMFace_Type);
	int len;

	if(face->v4) len= 4;
	else if(face->v3) len= 3;
	else len= 2;

	mf->v= PyList_New(len);
	
	if (len>0) {
		PyList_SetItem(mf->v, 0, py_incr_ret(PyList_GetItem(mesh->verts, face->v1)));
		if (len>1) {
			PyList_SetItem(mf->v, 1, py_incr_ret(PyList_GetItem(mesh->verts, face->v2)));
			if (len>2) {
				PyList_SetItem(mf->v, 2, py_incr_ret(PyList_GetItem(mesh->verts, face->v3)));
				if (len>3) {
					PyList_SetItem(mf->v, 3, py_incr_ret(PyList_GetItem(mesh->verts, face->v4)));
				}
			}
		}
	}
	
	mf->mat_nr= face->mat_nr;
	mf->smooth= face->flag&ME_SMOOTH;
	
	if (col) {
		int i;
		
		mf->col= PyList_New(4);
				
		for(i=0; i<4; i++, col++)
			PyList_SetItem(mf->col, i, (PyObject *) newcol(col->b, col->g, col->r, col->a));
	} else
		mf->col= PyList_New(0);
		
	return mf;
}

static NMVert *nmvert_from_data(NMesh *me, MVert *vert, MSticky *st, int idx)
{			
	NMVert *mv= PyObject_NEW(NMVert, &NMVert_Type);
			
	VECCOPY (mv->co, vert->co);
		
	mv->no[0]= vert->no[0]/32767.0;
	mv->no[1]= vert->no[1]/32767.0;
	mv->no[2]= vert->no[2]/32767.0;
	
	if (st) {
		mv->uvco[0]= st->co[0];
		mv->uvco[1]= st->co[1];
		mv->uvco[2]= 0.0;
		
	} else mv->uvco[0]= mv->uvco[1]= mv->uvco[2]= 0.0;

	mv->index= idx;
			
	return mv;
}

PyObject *newNMeshObject(Mesh *oldmesh) 
{
	NMesh *me;
	NMVert *mv;
	NMCol *mc;
	MFace *oldmf;
	MVert *oldmv;
	MSticky *oldst;
	MCol *oldmc;
	int i;
	
	me= PyObject_NEW(NMesh, &NMesh_Type);
	me->flags= 0;
	
	if (!oldmesh) {
		me->name= py_incr_ret(Py_None);
		me->mats= PyList_New(0);
		me->verts= PyList_New(0);
		me->faces= PyList_New(0);
	} else {
		me->name= PyString_FromString(oldmesh->id.name+2);
		
		oldmv= oldmesh->mvert;
		oldst= oldmesh->msticky;
		if (oldst) me->flags |= NMESH_HASUVCO;

		me->verts= PyList_New(oldmesh->totvert);
		for (i=0; i<oldmesh->totvert; i++) {
			PyList_SetItem(me->verts, i, (PyObject *) nmvert_from_data(me, oldmv, oldst, i));	
			
			oldmv++;
			if (oldst) oldst++;
		}

		oldmf= oldmesh->mface;
		oldmc= oldmesh->mcol;
		if (oldmc) me->flags |= NMESH_HASMCOL;

		me->faces= PyList_New(oldmesh->totface);
		for (i=0; i<oldmesh->totface; i++) {
			PyList_SetItem(me->faces, i, (PyObject *) nmface_from_data(me, oldmf, oldmc, i));
			
			oldmf++;
			if (oldmc) oldmc+= 4;
		}
	
		me->mats= PyList_New(oldmesh->totcol);

		for (i=0; i<oldmesh->totcol; i++) {
			Material *mat= oldmesh->mat[i];
			PyObject *ob;
			
			if (mat)
				ob= PyString_FromString(mat->id.name+2);
			else
				ob= py_incr_ret(Py_None);

			PyList_SetItem(me->mats, i, ob);
		}
	}
	
	return (PyObject *) me;	
}

static char Method_GetRaw_doc[]=
"([name]) - Get a raw mesh from Blender\n\
\n\
[name] Name of the mesh to be returned\n\
\n\
If name is not specified a new empty mesh is\n\
returned, otherwise Blender returns an existing\n\
mesh.";

static PyObject *Method_GetRaw(PyObject *self, PyObject *args) 
{
	char *name=NULL;
	Mesh *oldmesh=NULL;
	
	Py_Try(PyArg_ParseTuple(args, "|s", &name));	

	if(name) {
		oldmesh= (Mesh *) find_datablock_from_list((ID *) G.main->mesh.first, name);

		if (!oldmesh) return py_incr_ret(Py_None);
	}
	
	return newNMeshObject(oldmesh);
}

static void mvert_from_data(MVert *mv, MSticky *st, NMVert *from) 
{
	VECCOPY (mv->co, from->co);
	mv->no[0]= from->no[0]*32767.0;
	mv->no[1]= from->no[1]*32767.0;
	mv->no[2]= from->no[2]*32767.0;
		
	mv->flag= 0;
	mv->mat_nr= 0;

	if (st) {
		st->co[0]= from->uvco[0];
		st->co[1]= from->uvco[1];
	}
}

static void mface_from_data(MFace *mf, MCol *col, NMFace *from)
{
	NMVert *nmv;
	
	int i= PyList_Size(from->v);
	if(i>=1) {
		nmv= (NMVert *) PyList_GetItem(from->v, 0);
		if (NMVert_Check(nmv) && nmv->index!=-1) mf->v1= nmv->index;
		else mf->v1= 0;
	}
	if(i>=2) {
		nmv= (NMVert *) PyList_GetItem(from->v, 1);
		if (NMVert_Check(nmv) && nmv->index!=-1) mf->v2= nmv->index;
		else mf->v2= 0;
	}
	if(i>=3) {
		nmv= (NMVert *) PyList_GetItem(from->v, 2);
		if (NMVert_Check(nmv) && nmv->index!=-1) mf->v3= nmv->index;
		else mf->v3= 0;
	}
	if(i>=4) {
		nmv= (NMVert *) PyList_GetItem(from->v, 3);
		if (NMVert_Check(nmv) && nmv->index!=-1) mf->v4= nmv->index;
		else mf->v4= 0;
	}

	test_index_mface(mf, i);
					
	mf->puno= 0;
	mf->mat_nr= from->mat_nr;
	mf->edcode= 0;
	if (from->smooth) 
		mf->flag= ME_SMOOTH;
	else
		mf->flag= 0;
	
	if (col) {
		int len= PySequence_Length(from->col);
		
		if(len>4) len= 4;
		
		for (i=0; i<len; i++, col++) {
			NMCol *mc= (NMCol *) PySequence_GetItem(from->col, i);
			if(!NMCol_Check(mc)) {
				Py_DECREF(mc);
				continue;
			}
			
					
			col->b= mc->r;
			col->g= mc->g;
			col->r= mc->b;
			col->a= mc->a;

			Py_DECREF(mc);
		}
	}
}

static char Method_PutRaw_doc[]=
"(mesh, [name, renormal]) - Return a raw mesh to Blender\n\
\n\
(mesh) The NMesh object to store\n\
[name] The mesh to replace\n\
[renormal=1] Flag to control vertex normal recalculation\n\
\n\
If the name of a mesh to replace is not given a new\n\
object is created and returned.";

static PyObject *Method_PutRaw(PyObject *self, PyObject *args) 
{
	char *name= NULL;
	Mesh *nmesh= NULL;
	Object *ob= NULL;
	NMesh *me;
	NMCol *mc;
	MFace *newmf;
	MVert *newmv;
	MSticky *newst;
	MCol *newmc;
	int i, j, len;
	int recalc_normals= 1;
	
	Py_Try(PyArg_ParseTuple(args, "O!|si", &NMesh_Type, &me, &name, &recalc_normals));
	
	if (!PySequence_Check(me->verts))
		return py_err_ret_ob(PyExc_AttributeError, "nmesh vertices are not a sequence");
	if (!PySequence_Check(me->faces))
		return py_err_ret_ob(PyExc_AttributeError, "nmesh faces are not a sequence");
	if (!PySequence_Check(me->mats))
		return py_err_ret_ob(PyExc_AttributeError, "nmesh materials are not a sequence");

	if (!py_check_sequence_consistency(me->verts, &NMVert_Type))
		return py_err_ret_ob(PyExc_AttributeError, "nmesh vertices must be NMVerts");
	if (!py_check_sequence_consistency(me->faces, &NMFace_Type))
		return py_err_ret_ob(PyExc_AttributeError, "nmesh faces must be NMFaces");
	
	if (name) 
		nmesh= (Mesh *) find_datablock_from_list((ID *) G.main->mesh.first, name);
	
	if(!nmesh || nmesh->id.us==0) {
		ob= add_object(OB_MESH);

		if (nmesh)
			set_mesh(ob, nmesh);
		else
			nmesh= (Mesh *) ob->data;
		
		ob->loc[0]= ob->loc[1]= ob->loc[2]= 0.0;
		ob->rot[0]= ob->rot[1]= ob->rot[2]= 0.0;
		ob->size[0]= ob->size[1]= ob->size[2]= 1.0;
	}
	if(name) new_id(&G.main->mesh, &nmesh->id, name);
	
	freedisplist(&nmesh->disp);
	
	unlink_mesh(nmesh);
	
	if(nmesh->mvert) freeN(nmesh->mvert);
	if(nmesh->mface) freeN(nmesh->mface);
	if(nmesh->mcol) freeN(nmesh->mcol);
	if(nmesh->msticky) freeN(nmesh->msticky);
	if(nmesh->mat) freeN(nmesh->mat);

	nmesh->mvert= NULL;
	nmesh->mface= NULL;
	nmesh->mcol= NULL;
	nmesh->msticky= NULL;
	nmesh->mat= NULL;

	nmesh->totcol= PySequence_Length(me->mats);
	if (nmesh->totcol) {
		nmesh->mat= callocN(sizeof(Material*)*nmesh->totcol, "mesh mats");
		
		for (i= 0; i<nmesh->totcol; i++) {
			PyObject *ob= PySequence_GetItem(me->mats, i);
			
			if (PyString_Check(ob)) {
				Material *ma= (Material *) find_datablock_from_list((ID *) G.main->mat.first, PyString_AsString(ob));
				
				if (ma) ma->id.us++;
				nmesh->mat[i]= ma;
			}
			
			Py_DECREF(ob);
		}
	}
	
	nmesh->totvert= PySequence_Length(me->verts);
	if (nmesh->totvert) {
		if (me->flags&NMESH_HASUVCO)
			nmesh->msticky= callocN(sizeof(MSticky)*nmesh->totvert, "msticky");

		nmesh->mvert= callocN(sizeof(MVert)*nmesh->totvert, "mverts");
	}

	if (nmesh->totvert)
		nmesh->totface= PySequence_Length(me->faces);
	else
		nmesh->totface= 0;

	if (nmesh->totface) {
		if (me->flags&NMESH_HASMCOL)
			nmesh->mcol= callocN(4*sizeof(MCol)*nmesh->totface, "mcol");
			
		nmesh->mface= callocN(sizeof(MFace)*nmesh->totface, "mfaces");
	}

	/* This stuff here is to tag all the vertices referenced
	 * by faces, then untag the vertices which are actually
	 * in the vert list. Any vertices untagged will be ignored
	 * by the mface_from_data function. It comes from my
	 * screwed up decision to not make faces only store the
	 * index. - Zr
	 */
	for (i=0; i<nmesh->totface; i++) {
		NMFace *mf= (NMFace *) PySequence_GetItem(me->faces, i);
			
		j= PySequence_Length(mf->v);
		while (j--) {
			NMVert *mv= (NMVert *) PySequence_GetItem(mf->v, j);
			if (NMVert_Check(mv)) mv->index= -1;
			Py_DECREF(mv);
		}
		
		Py_DECREF(mf);
	}
	
	for (i=0; i<nmesh->totvert; i++) {
		NMVert *mv= (NMVert *) PySequence_GetItem(me->verts, i);
		mv->index= i;
		Py_DECREF(mv);
	}	
	
	newmv= nmesh->mvert;
	newst= nmesh->msticky;
	for (i=0; i<nmesh->totvert; i++) {
		PyObject *mv=  PySequence_GetItem(me->verts, i);
		mvert_from_data(newmv, newst, (NMVert *)mv);
		Py_DECREF(mv);
		
		newmv++;
		if (newst) newst++;
	}

	newmc= nmesh->mcol;
	newmf= nmesh->mface;
	for (i=0; i<nmesh->totface; i++) {
		PyObject *mf= PySequence_GetItem(me->faces, i);
		mface_from_data(newmf, newmc, (NMFace *) mf);
		Py_DECREF(mf);
			
		newmf++;
		if (newmc) newmc++;
	}
	
	if(recalc_normals)
		vertexnormals_mesh(nmesh, 0);

	edge_drawflags_mesh(nmesh);
	tex_space_mesh(nmesh);
	
	if (!during_script())
		allqueue(REDRAWVIEW3D, 0);
	
	if (ob)
		return add_pyblock(ob);
	else
		return py_incr_ret(Py_None);
}

static struct PyMethodDef NMeshM_methods[] = {
	MethodDef(Col),
	MethodDef(Vert),
	MethodDef(Face),
	MethodDef(GetRaw),
	MethodDef(PutRaw),
	{NULL, NULL}
};

PyObject *init_py_nmesh(void) 
{
	NMesh_Type.ob_type= &PyType_Type;	
	NMVert_Type.ob_type= &PyType_Type;	
	NMFace_Type.ob_type= &PyType_Type;	
	NMCol_Type.ob_type= &PyType_Type;	
	
	return Py_InitModule("Blender.NMesh", NMeshM_methods);
}

