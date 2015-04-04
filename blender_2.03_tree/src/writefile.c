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

/*  writefile.c       MIXED MODEL

 * 
 *  jan feb maart 95
 *  Version: $Id: writefile.c,v 1.21 2000/09/25 22:02:55 ton Exp $
 * 
 * 
 *	FILEFORMAAT: IFF-achtige structuur  (niet meer IFF compatible!)
 
	start file:
		BLENDER_V100	12 bytes  (versie 1.00)
						V = big endian, v = little endian
						_ = 4 byte pointer, - = 8 byte pointer
						
	datablokken:		zie ook struct BHead
		<bh.code>			4 chars
		<bh.len>			int,  len data achter BHead
		<bh.old>			void,  oude pointer
		<bh.SDNAnr>			int
		<bh.nr>				int, bij array: aantal structs

		data			
		...
		...
	
	Vrijwel alle data in blender zijn structs. Elke struct krijgt een BHead header mee.
	Met BHead kan de struktuur weer worden gelinkt en wordt met StructDNA vergeleken.
	
	SCHRIJVEN
	
	Voorkeur volgorde schrijven: (waarschijnlijk mag ook door elkaar, maar waarom zou je? )
	In ieder geval MOET indirekte data ACHTER LibBlock
	
	(Locale data)
	- voor elk LibBlock
		- schrijf LibBlock
		- schrijf bijhorende direkte data
	(Externe data)
	- per library
		- schrijf library block
		- per LibBlock
			- schrijf ID LibBlock
	- schrijf FileGlobal (een selectie uit globale data )
	- schrijf SDNA
	- schrijf USER als aangegeven (~/.B.blend)
		
 */

#include "blender.h"
#include "file.h"
#include "graphics.h"
#include "screen.h"
#include "sequence.h"
#include "effect.h"
#include "ika.h"
#include "oops.h"
#include "imasel.h"
#include "text.h"
#include "game.h"
#include "sound.h"
#include "group.h"

#include <fcntl.h>

/* Modules used */
#include "render.h"

/* *******  MYWRITE ********* */

char *writebuf;
int mywcount, mywtot, mywfile, noBlog=0;

void mywrite(void *adr, int len)
{

	if(len<=0) return;
	mywtot+= len;
	if(len>50000) {
		if(mywcount) {
			write(mywfile, writebuf, mywcount);
			mywcount= 0;
		}
		write(mywfile, adr, len);
		return;
	}
	if(len+mywcount>99999) {
		write(mywfile, writebuf, mywcount);
		mywcount= 0;
	}
	memcpy(writebuf+mywcount, adr, len);
	mywcount+= len;
}

void bgnwrite(file)
int file;
{
	mywfile= file;
	writebuf= (char *)mallocN(100000,"bgnwrite");

	mywcount= 0;
	mywtot= 0;
}

void endwrite()
{
	if(mywcount) {
		write(mywfile, writebuf, mywcount);
	}
	freeN(writebuf);

}

void flushwrite()
{
	if(mywcount) {
		write(mywfile, writebuf, mywcount);
		mywcount= 0;
	}
}

/* ********  END MYWRITE ******** */
/* ********  DIV ******** */

void writeBlog()
{
	int file;
	char name[FILE_MAXDIR+FILE_MAXFILE], *home;
	
	if(noBlog) return;
	
	home = gethome();
	if (home) {
		make_file_string (name, home, ".Blog");

		file= open(name, O_BINARY|O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (file >= 0) {
			write(file, G.sce, strlen(G.sce));
			close(file);
		}
	}
}

void readBlog()
{
	FILE *fp;
	int file, len;
	int end;
	char name[FILE_MAXDIR+FILE_MAXFILE], *home;

	home = gethome();
	if (home) {
		make_file_string (name, home, ".Blog");

		file= open(name, O_BINARY|O_RDONLY);
		if (file >= 0) {
			len = read(file, G.sce, sizeof(G.sce));
			close(file);
			if (len > 0) G.sce[len] = 0;
		}

		make_file_string (name, home, ".Bfs");
		
		fp= fopen(name, "r");
		if(fp==NULL) {
			return;
		}

#ifdef WIN32
		/* Add the drive names to the listing */
		{
			DWORDLONG tmp;
			char tmps[4];
			int i;
			
			tmp= GetLogicalDrives();
			
			for (i=0; i < 26; i++) {
				if ((tmp>>i) & 1) {
					tmps[0]='a'+i;
					tmps[1]=':';
					tmps[2]='\\';
					tmps[3]=0;
					
					addfilename_to_fsmenu(tmps);
				}
			}
			
		}

#endif
		
		end= 1;
		while(end>0) {
			end= fscanf(fp, "%s", name);
			if(end<=0) break;
			addfilename_to_fsmenu(name);
		}
		
		fclose(fp);
	}
}


/* ********** WRITE FILE ****************** */

void writestruct(int filecode, char *structname, int nr, void *adr)
{
	BHead bh;
	short *sp;
	
	if(adr==0 || nr==0) return;

	/* BHead vullen met data */
	bh.code= filecode;
	bh.old= adr;
	bh.nr= nr;

	bh.SDNAnr= findstruct_nr(&cur_sdna, structname);
	if(bh.SDNAnr== -1) {
		printf("error: can't find SDNA code %s\n", structname);
		return;
	}
	sp= cur_sdna.structs[bh.SDNAnr];
	
	bh.len= nr*cur_sdna.typelens[sp[0]];

	if(bh.len==0) return;
		
	mywrite(&bh, sizeof(BHead));
	mywrite(adr, bh.len);
	
}

void writedata(int filecode, int len, void *adr)	/* geen struct */
{
	BHead bh;
	
	if(adr==0) return;
	if(len==0) return;

	len+= 3;
	len-= ( len % 4);

	/* BHead vullen met data */
	bh.code= filecode;
	bh.old= adr;
	bh.nr= 1;
	bh.SDNAnr= 0;	
	bh.len= len;
	
	mywrite(&bh, sizeof(BHead));
	if(len) mywrite(adr, len);
	
}

void write_scriptlink(ScriptLink *slink)
{
	writedata(DATA, sizeof(void *)*slink->totscript, slink->scripts);	
	writedata(DATA, sizeof(short)*slink->totscript, slink->flag);	
}

void write_renderinfo()		/* alleen voor renderdaemon */
{
	Scene *sce;
	int data[8];
	
	sce= G.main->scene.first;
	while(sce) {
		if(sce->id.lib==0  && ( sce==G.scene || (sce->r.scemode & R_BG_RENDER)) ) {
			data[0]= sce->r.sfra;
			data[1]= sce->r.efra;
			
			strncpy((char *)(data+2), sce->id.name+2, 23);
			
			writedata(REND, 32, data);
		}
		sce= sce->id.next;
	}
}

void write_userdef()
{
	char name[FILE_MAXDIR+FILE_MAXFILE], *home;

	home = gethome();
	if (home) {
		make_file_string(name, home, ".B.blend");
		
		if(strcmp(G.sce, name)==0) {
			writestruct(USER, "UserDef", 1, &U);
		}
	}
}

void write_effects(ListBase *lb)
{
	Effect *eff;
	
	eff= lb->first;
	while(eff) {
		
		switch(eff->type) {
		case EFF_BUILD:
			writestruct(DATA, "BuildEff", 1, eff);
			break;	
		case EFF_PARTICLE:
			writestruct(DATA, "PartEff", 1, eff);
			break;	
		case EFF_WAVE:
			writestruct(DATA, "WaveEff", 1, eff);
			break;	
		default:
			writedata(DATA, alloc_len(eff), eff);
		}
		
		eff= eff->next;
	}
}

void write_properties(ListBase *lb)
{
	bProperty *prop;
	
	prop= lb->first;
	while(prop) {
		writestruct(DATA, "bProperty", 1, prop);

		if(prop->poin && prop->poin != &prop->data) 
			writedata(DATA, alloc_len(prop->poin), prop->poin);

		prop= prop->next;
	}
}

void write_sensors(ListBase *lb)
{
	bSensor *sens;
	
	sens= lb->first;
	while(sens) {
		writestruct(DATA, "bSensor", 1, sens);
		
		writedata(DATA, sizeof(void *)*sens->totlinks, sens->links);
		
		switch(sens->type) {
		case SENS_NEAR:
			writestruct(DATA, "bNearSensor", 1, sens->data);
			break;
		case SENS_MOUSE:
			writestruct(DATA, "bMouseSensor", 1, sens->data);
			break;
		case SENS_TOUCH:
			writestruct(DATA, "bTouchSensor", 1, sens->data);
			break;
		case SENS_KEYBOARD:
			writestruct(DATA, "bKeyboardSensor", 1, sens->data);
			break;
		case SENS_PROPERTY:
			writestruct(DATA, "bPropertySensor", 1, sens->data);
			break;
		case SENS_COLLISION:
			writestruct(DATA, "bCollisionSensor", 1, sens->data);
			break;
		case SENS_RADAR:
			writestruct(DATA, "bRadarSensor", 1, sens->data);
			break;
		}

		sens= sens->next;
	}
}

void write_controllers(ListBase *lb)
{
	bController *cont;
	
	cont= lb->first;
	while(cont) {
		writestruct(DATA, "bController", 1, cont);

		writedata(DATA, sizeof(void *)*cont->totlinks, cont->links);

		switch(cont->type) {
		case CONT_EXPRESSION:
			writestruct(DATA, "bExpressionCont", 1, cont->data);
			break;
		case CONT_PYTHON:
			writestruct(DATA, "bPythonCont", 1, cont->data);
			break;
		}

		cont= cont->next;
	}
}

void write_actuators(ListBase *lb)
{
	bActuator *act;
	
	act= lb->first;
	while(act) {
		writestruct(DATA, "bActuator", 1, act);

		switch(act->type) {
		case ACT_SOUND:
			writestruct(DATA, "bSoundActuator", 1, act->data);
			break;
		case ACT_OBJECT:
			writestruct(DATA, "bObjectActuator", 1, act->data);
			break;
		case ACT_IPO:
			writestruct(DATA, "bIpoActuator", 1, act->data);
			break;
		case ACT_PROPERTY:
			writestruct(DATA, "bPropertyActuator", 1, act->data);
			break;
		case ACT_CAMERA:
			writestruct(DATA, "bCameraActuator", 1, act->data);
			break;
		case ACT_CONSTRAINT:
			writestruct(DATA, "bConstraintActuator", 1, act->data);
			break;
		case ACT_EDIT_OBJECT:
			writestruct(DATA, "bEditObjectActuator", 1, act->data);
			break;
		case ACT_SCENE:
			writestruct(DATA, "bSceneActuator", 1, act->data);
			break;
		case ACT_GROUP:
			writestruct(DATA, "bGroupActuator", 1, act->data);
			break;
		}

		act= act->next;
	}
}

void write_objects(ListBase *idbase)
{
	Object *ob;
	
	ob= idbase->first;
	while(ob) {
		if(ob->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_OB, "Object", 1, ob);
			
			/* alle direkte data */
			writedata(DATA, sizeof(void *)*ob->totcol, ob->mat);
			write_effects(&ob->effect);
			write_properties(&ob->prop);
			write_sensors(&ob->sensors);
			write_controllers(&ob->controllers);
			write_actuators(&ob->actuators);
			write_scriptlink(&ob->scriptlink);
		}
		ob= ob->id.next;
	}
}

void write_vfonts(ListBase *idbase)
{
	VFont *vf;
	PackedFile * pf;
	
	vf= idbase->first;
	while(vf) {
		if(vf->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_VF, "VFont", 1, vf);
		
			/* alle direkte data */
			
			if (vf->packedfile) {
				pf = vf->packedfile;
				writestruct(DATA, "PackedFile", 1, pf);
				writedata(DATA, pf->size, pf->data);
			}
		}
		
		vf= vf->id.next;
	}
}

void write_ipos(ListBase *idbase)
{
	Ipo *ipo;
	IpoCurve *icu;
	
	ipo= idbase->first;
	while(ipo) {
		if(ipo->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_IP, "Ipo", 1, ipo);
		
			/* alle direkte data */
			icu= ipo->curve.first;
			while(icu) {
				writestruct(DATA, "IpoCurve", 1, icu);
				icu= icu->next;
			}

			icu= ipo->curve.first;
			while(icu) {
				if(icu->bezt)  writestruct(DATA, "BezTriple", icu->totvert, icu->bezt);
				if(icu->bp)  writestruct(DATA, "BPoint", icu->totvert, icu->bp);
				icu= icu->next;
			}
		}
		
		ipo= ipo->id.next;
	}
}

void write_keys(ListBase *idbase)
{
	Key *key;
	KeyBlock *kb;
	
	key= idbase->first;
	while(key) {
		if(key->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_KE, "Key", 1, key);
		
			/* alle direkte data */
			kb= key->block.first;
			while(kb) {
				writestruct(DATA, "KeyBlock", 1, kb);
				if(kb->data) writedata(DATA, kb->totelem*key->elemsize, kb->data);
				kb= kb->next;
			}
		}
		
		key= key->id.next;
	}
}

void write_cameras(ListBase *idbase)
{
	Camera *cam;
	
	cam= idbase->first;
	while(cam) {
		if(cam->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_CA, "Camera", 1, cam);
		
			/* alle direkte data */
			write_scriptlink(&cam->scriptlink);
		}
		
		cam= cam->id.next;
	}
}

void write_mballs(ListBase *idbase)
{
	MetaBall *mb;
	MetaElem *ml;
	
	mb= idbase->first;
	while(mb) {
		if(mb->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_MB, "MetaBall", 1, mb);
			
			/* alle direkte data */
			writedata(DATA, sizeof(void *)*mb->totcol, mb->mat);
			
			ml= mb->elems.first;
			while(ml) {
				writestruct(DATA, "MetaElem", 1, ml);
				ml= ml->next;
			}
		}
		mb= mb->id.next;
	}
}

void write_curves(ListBase *idbase)
{
	Curve *cu;
	Nurb *nu;
	
	cu= idbase->first;
	while(cu) {
		if(cu->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_CU, "Curve", 1, cu);
			
			/* alle direkte data */
			writedata(DATA, sizeof(void *)*cu->totcol, cu->mat);
			
			if(cu->vfont) {
				writedata(DATA, cu->len+1, cu->str);
			}
			else {
				/* is ook volgorde van inlezen */
				nu= cu->nurb.first;
				while(nu) {
					writestruct(DATA, "Nurb", 1, nu);
					nu= nu->next;
				}
				nu= cu->nurb.first;
				while(nu) {
					if( (nu->type & 7)==CU_BEZIER) 
						writestruct(DATA, "BezTriple", nu->pntsu, nu->bezt);
					else {
						writestruct(DATA, "BPoint", nu->pntsu*nu->pntsv, nu->bp);
						if(nu->knotsu) writedata(DATA, KNOTSU(nu)*sizeof(float), nu->knotsu);
						if(nu->knotsv) writedata(DATA, KNOTSV(nu)*sizeof(float), nu->knotsv);
					}
					nu= nu->next;
				}
			}
		}
		cu= cu->id.next;
	}
}

void write_meshs(ListBase *idbase)
{
	Mesh *mesh;
	
	mesh= idbase->first;
	while(mesh) {
		if(mesh->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_ME, "Mesh", 1, mesh);
			
			/* alle direkte data */
			writedata(DATA, sizeof(void *)*mesh->totcol, mesh->mat);
			writestruct(DATA, "MVert", mesh->totvert, mesh->mvert);
			writestruct(DATA, "MFace", mesh->totface, mesh->mface);
			writestruct(DATA, "TFace", mesh->totface, mesh->tface);
			writestruct(DATA, "MCol", 4*mesh->totface, mesh->mcol);
			writestruct(DATA, "MSticky", mesh->totvert, mesh->msticky);
		}
		mesh= mesh->id.next;
	}
}

void write_images(ListBase *idbase)
{
	Image *ima;
	PackedFile * pf;
	
	ima= idbase->first;
	while(ima) {
		if(ima->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_IM, "Image", 1, ima);

			if (ima->packedfile) {
				pf = ima->packedfile;
				writestruct(DATA, "PackedFile", 1, pf);
				writedata(DATA, pf->size, pf->data);
			}
		}
		ima= ima->id.next;
	}
}

void write_textures(ListBase *idbase)
{
	Tex *tex;
	
	tex= idbase->first;
	while(tex) {
		if(tex->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_TE, "Tex", 1, tex);
			
			/* alle direkte data */
			if(tex->plugin) writestruct(DATA, "PluginTex", 1, tex->plugin);
			if(tex->coba) writestruct(DATA, "ColorBand", 1, tex->coba);
			if(tex->env) writestruct(DATA, "EnvMap", 1, tex->env);
		}
		tex= tex->id.next;
	}
}

void write_materials(ListBase *idbase)
{
	Material *ma;
	int a;
	
	ma= idbase->first;
	while(ma) {
		if(ma->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_MA, "Material", 1, ma);
			
			for(a=0; a<8; a++) {
				if(ma->mtex[a]) writestruct(DATA, "MTex", 1, ma->mtex[a]);
			}

			write_scriptlink(&ma->scriptlink);
		}
		ma= ma->id.next;
	}
}

void write_worlds(ListBase *idbase)
{
	World *wrld;
	int a;
	
	wrld= idbase->first;
	while(wrld) {
		if(wrld->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_WO, "World", 1, wrld);

			for(a=0; a<8; a++) {
				if(wrld->mtex[a]) writestruct(DATA, "MTex", 1, wrld->mtex[a]);
			}

			write_scriptlink(&wrld->scriptlink);
		}
		wrld= wrld->id.next;
	}
}

void write_lamps(ListBase *idbase)
{
	Lamp *la;
	int a;
	
	la= idbase->first;
	while(la) {
		if(la->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_LA, "Lamp", 1, la);
		
			/* alle direkte data */
			for(a=0; a<8; a++) {
				if(la->mtex[a]) writestruct(DATA, "MTex", 1, la->mtex[a]);
			}

			write_scriptlink(&la->scriptlink);
		}
		la= la->id.next;
	}
}

void write_lattices(ListBase *idbase)
{
	Lattice *lt;
	
	lt= idbase->first;
	while(lt) {
		if(lt->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_LT, "Lattice", 1, lt);
		
			/* alle direkte data */
			writestruct(DATA, "BPoint", lt->pntsu*lt->pntsv*lt->pntsw, lt->def);
		}
		lt= lt->id.next;
	}
}

void write_ikas(ListBase *idbase)
{
	Ika *ika;
	Limb *li;
	
	ika= idbase->first;
	while(ika) {
		if(ika->id.us>0) {
			/* schrijf LibData */
			writestruct(ID_IK, "Ika", 1, ika);
			
			/* alle direkte data */
			li= ika->limbbase.first;
			while(li) {
				writestruct(DATA, "Limb", 1, li);
				li= li->next;
			}
			
			writestruct(DATA, "Deform", ika->totdef, ika->def);
		}
		ika= ika->id.next;
	}
}

void write_scenes(ListBase *scebase)
{
	Scene *sce;
	Base *base;
	Editing *ed;
	Sequence *seq;
	Strip *strip;
	
	sce= scebase->first;
	while(sce) {
		/* schrijf LibData */
		writestruct(ID_SCE, "Scene", 1, sce);
		
		/* alle direkte data */
		base= sce->base.first;
		while(base) {
			writestruct(DATA, "Base", 1, base);
			base= base->next;
		}
		
		writestruct(DATA, "Radio", 1, sce->radio);
		writestruct(DATA, "FreeCamera", 1, sce->fcam);
		
		ed= sce->ed;
		if(ed) {
			writestruct(DATA, "Editing", 1, ed);

			/* ook schrijfflags op nul */
			WHILE_SEQ(&ed->seqbase) {
				if(seq->strip) seq->strip->done= 0;
				writestruct(DATA, "Sequence", 1, seq);
			}
			END_SEQ
			
			WHILE_SEQ(&ed->seqbase) {
				if(seq->strip && seq->strip->done==0) {
					/* strip wegschrijven met done op 0 ivm readfile */
					
					if(seq->plugin) writestruct(DATA, "PluginSeq", 1, seq->plugin);
					
					strip= seq->strip;
					writestruct(DATA, "Strip", 1, strip);
					
					if(seq->type==SEQ_IMAGE) 
						writestruct(DATA, "StripElem", strip->len, strip->stripdata);
					else if(seq->type==SEQ_MOVIE)
						writestruct(DATA, "StripElem", 1, strip->stripdata);
						
					strip->done= 1;
				}
			}
			END_SEQ
		}

		write_scriptlink(&sce->scriptlink);
		
		sce= sce->id.next;
	}
}

void write_screens(ListBase *scrbase)
{
	bScreen *sc;
	ScrArea *sa;
	ScrVert *sv;
	ScrEdge *se;
	View3D *v3d;
	Oops *oops, *oopsn;
	
	sc= scrbase->first;
	while(sc) {
		/* schrijf LibData */
		writestruct(ID_SCR, "Screen", 1, sc);
		
		/* alle direkte data */
		sv= sc->vertbase.first;
		while(sv) {
			writestruct(DATA, "ScrVert", 1, sv);
			sv= sv->next;
		}

		se= sc->edgebase.first;
		while(se) {
			writestruct(DATA, "ScrEdge", 1, se);
			se= se->next;
		}

		sa= sc->areabase.first;
		while(sa) {
			writestruct(DATA, "ScrArea", 1, sa);
			
			v3d= sa->spacedata.first; /* v3d als algemeen voorbeeld */
			while(v3d) {
				if(v3d->spacetype==SPACE_VIEW3D) {
					writestruct(DATA, "View3D", 1, v3d);
					if(v3d->bgpic) writestruct(DATA, "BGpic", 1, v3d->bgpic);
					if(v3d->localvd) writestruct(DATA, "View3D", 1, v3d->localvd);
				}
				else if(v3d->spacetype==SPACE_IPO) {
					writestruct(DATA, "SpaceIpo", 1, v3d);
				}
				else if(v3d->spacetype==SPACE_BUTS) {
					writestruct(DATA, "SpaceButs", 1, v3d);
				}
				else if(v3d->spacetype==SPACE_FILE) {
					writestruct(DATA, "SpaceFile", 1, v3d);
				}
				else if(v3d->spacetype==SPACE_SEQ) {
					writestruct(DATA, "SpaceSeq", 1, v3d);
				}
				else if(v3d->spacetype==SPACE_OOPS) {
					SpaceOops *so= (SpaceOops *)v3d;
					
					/* cleanup */
					oops= so->oops.first;
					while(oops) {
						oopsn= oops->next;
						if(oops->id==0) {
							remlink(&so->oops, oops);
							free_oops(oops);
						}
						oops= oopsn;
					}

					/* NA de cleanup, ivm listbase! */					
					writestruct(DATA, "SpaceOops", 1, so);
					
					oops= so->oops.first;
					while(oops) {
						writestruct(DATA, "Oops", 1, oops);
						oops= oops->next;
					}
				}
				else if(v3d->spacetype==SPACE_IMAGE) {
					writestruct(DATA, "SpaceImage", 1, v3d);
				}
				else if(v3d->spacetype==SPACE_IMASEL) {
					writestruct(DATA, "SpaceImaSel", 1, v3d);
				}
				else if(v3d->spacetype==SPACE_PAINT) {
					writestruct(DATA, "SpacePaint", 1, v3d);
				}
				else if(v3d->spacetype==SPACE_TEXT) {
					writestruct(DATA, "SpaceText", 1, v3d);
				}
				else if(v3d->spacetype==SPACE_SOUND) {
					writestruct(DATA, "SpaceSound", 1, v3d);
				}
				v3d= v3d->next;
			}
			
			sa= sa->next;
		}

		sc= sc->id.next;
	}
}

void write_libraries(Main *main)
{
	ListBase *lbarray[30];
	ID *id;
	int a, tot, foundone;

	while(main) {

		a=tot= set_listbasepointers(main, lbarray);

		/* test: wordt lib nog gebruikt */
		foundone= 0;
		while(tot--) {
			id= lbarray[tot]->first;
			while(id) {
				if(id->us>0 && (id->flag & LIB_EXTERN)) {
					foundone= 1;
					break;
				}
				id= id->next;
			}
			if(foundone) break;
		}
		
		if(foundone) {	

			writestruct(ID_LI, "Library", 1, main->curlib);
	
			while(a--) {
				id= lbarray[a]->first;
				while(id) {
					if(id->us>0 && (id->flag & LIB_EXTERN)) {
						
						writestruct(ID_ID, "ID", 1, id);
					}
					id= id->next;
				}
			}
		}
		
		main= main->next;
	}
}

void write_texts(ListBase *idbase)
{
	Text *text;
	TextLine *tmp;
	
	text= idbase->first;
	while(text) {
		if ( (text->flags & TXT_ISMEM) && (text->flags & TXT_ISEXT)) text->flags &= ~TXT_ISEXT;
		
		/* write LibData */
		writestruct(ID_TXT, "Text", 1, text);
		if(text->name) writedata(DATA, strlen(text->name)+1, text->name);

		if(!(text->flags & TXT_ISEXT)) {			
			/* now write the text data, in two steps for optimization in the readfunction */
			tmp= text->lines.first;
			while (tmp) {
				writestruct(DATA, "TextLine", 1, tmp);
				tmp= tmp->next;
			}

			tmp= text->lines.first;
			while (tmp) {
				writedata(DATA, tmp->len+1, tmp->line);
				tmp= tmp->next;
			}
		}
		text= text->id.next;
	}
}

void write_sounds(ListBase *idbase)
{
	bSound *sound;
	PackedFile * pf;

	sound= idbase->first;
	while(sound) {
		if(sound->id.us>0) {
			/* write LibData */
			writestruct(ID_SO, "bSound", 1, sound);
	
			if (sound->packedfile) {
				pf = sound->packedfile;
				writestruct(DATA, "PackedFile", 1, pf);
				writedata(DATA, pf->size, pf->data);
			}
		}
		sound= sound->id.next;
	}
}

void write_groups(ListBase *idbase)
{
	Group *group;
	GroupKey *gk;
	GroupObject *go;
	ObjectKey *ok;
	
	group= idbase->first;
	while(group) {
		if(group->id.us>0) {
			/* write LibData */
			writestruct(ID_GR, "Group", 1, group);
			
			gk= group->gkey.first;
			while(gk) {
				writestruct(DATA, "GroupKey", 1, gk);
				gk= gk->next;
			}

			go= group->gobject.first;
			while(go) {
				writestruct(DATA, "GroupObject", 1, go);
				go= go->next;
			}
			go= group->gobject.first;
			while(go) {
				ok= go->okey.first;
				while(ok) {
					writestruct(DATA, "ObjectKey", 1, ok);
					ok= ok->next;
				}
				go= go->next;
			}
			
		}
		group= group->id.next;
	}
}

void write_global()
{
	FileGlobal fg;
	
	fg.curscreen= G.curscreen;
	fg.displaymode= R.displaymode;
	fg.winpos= R.winpos;
	fg.fileflags = G.fileflags;
	
	writestruct(GLOB, "FileGlobal", 1, &fg);

}

void do_history(char *name)
{
	int hisnr= U.versions, len;
	char tempname1[FILE_MAXDIR+FILE_MAXFILE], tempname2[FILE_MAXDIR+FILE_MAXFILE];
	
	if(U.versions==0 || noBlog) return;
		
	len= strlen(name);
	if(len<2) return;
		
	while(  hisnr > 1) {
		sprintf(tempname1, "%s%d", name, hisnr-1);
		sprintf(tempname2, "%s%d", name, hisnr);

		if(fop_rename(tempname1, tempname2))
			error("Unable to make version backup");
			
		hisnr--;
	}
		
	/* lijkt dubbelop: maar deze is nodig als hisnr==1 */
	sprintf(tempname1, "%s%d", name, hisnr);

	if(fop_rename(name, tempname1))
		error("Unable to make version backup");
}

void write_file(char *dir)
{
	extern char versionfstr[];
	Library *li;
	int file, len, fout=0;
	char di[FILE_MAXDIR], tempname[FILE_MAXDIR+FILE_MAXFILE];

	len= strlen(dir);
	if(len==0) return;

	strcpy(di, dir);

	if(testextensie(di,".blend")==0) strcat(di,".blend");

	/* test op libraries */
	li= G.main->library.first;
	while(li) {
		if(strcmp(li->name, dir)==0) {
			error("Cannot overwrite used library");
			return;
		}
		li= li->id.next;
	}
	
	file= open(di,O_BINARY+O_RDONLY);
	close(file);
	if(file>-1) {
		if(!saveover(di))
			return; 
	}
	
	if( (G.f & G_DISABLE_OK)==0) {	/* schrijf tempfile, niet met U.tempdir vergelijken */
		if(G.obedit) {
			exit_editmode(0);	/* 0 = geen freedata */
		}
	}

	/* BEVEILIGING */
	strcpy(tempname, di);
	strcat(tempname, "@"); 
	
	file= open(tempname,O_BINARY+O_WRONLY+O_CREAT+O_TRUNC, 0666);
	if(file== -1) {
		errorstr("Can't write file", di, 0);
		return;
	}

	waitcursor(1);
	
	G.save_over = TRUE;
	
	// if auto_packing is on, now is the time to pack all files...

	if (G.fileflags & B_PACKFILE)
	{
		packAll();
	}
	
	bgnwrite(file);
	strcpy(G.sce, di);
	strcpy(G.main->name, di);	/* is gegarandeerd current file */

	/* let op G.version! (in blender.c) */
	mywrite(versionfstr, 12);

	split_main();
/*----------------*/						
	
	write_renderinfo();
	
	write_screens(&G.main->screen);
	write_scenes(&G.main->scene);
	write_objects(&G.main->object);
	write_meshs(&G.main->mesh);
	write_curves(&G.main->curve);
	write_mballs(&G.main->mball);
	write_materials(&G.main->mat);
	write_textures(&G.main->tex);
	write_images(&G.main->image);
	write_cameras(&G.main->camera);
	write_lamps(&G.main->lamp);
	write_lattices(&G.main->latt);
	write_ikas(&G.main->ika);
	write_vfonts(&G.main->vfont);
	write_ipos(&G.main->ipo);
	write_keys(&G.main->key);
	write_worlds(&G.main->world);
	write_texts(&G.main->text);
	write_libraries(G.main->next);
	write_sounds(&G.main->sound);
	write_groups(&G.main->group);
	write_global();
	write_userdef();
	
		/* dna als laatste i.v.m. (nog te schrijven) test op welke gebruikt zijn */
	writedata(DNA1, cur_sdna.datalen, cur_sdna.data);
	
	endwrite();
/*----------------*/						
	join_main();

	/* testen of alles goed is gelopen */
	len= ENDB;
	write(file, &len, 4);
	len= 0;
	if(write(file, &len,4)!=4) {
		error("Not enough diskspace");
		fout= 1;
	}

	close(file);

	/* EINDE BEVEILIGING */
	if(!fout) {
		
		do_history(di);	/* doet alleen renames, geen delete */
		
		if(fop_rename(tempname, di) < 0) {
			error("Can't change old file. File saved with @");
		}
		
		writeBlog();
	}
	else remove(tempname);
	
	waitcursor(0);
}

int write_homefile()
{
	char *home, tstr[FILE_MAXDIR+FILE_MAXFILE], scestr[FILE_MAXDIR+FILE_MAXFILE];
	int save_over;
	
	home = gethome();
	if (home) {
		make_file_string(tstr, home, ".B.blend");
		
		noBlog= 1;
		strcpy(scestr, G.sce);	/* even bewaren */
		save_over = G.save_over;
		write_file(tstr);
		G.save_over = save_over;
		strcpy(G.sce, scestr);
		noBlog= 0;
		return 1;

	}
	return 0;
}

void write_autosave()
{
	char scestr[FILE_MAXDIR+FILE_MAXFILE], tstr[FILE_MAXDIR+FILE_MAXFILE], tmp2[32];
	int save_over;
	
	noBlog= 1;		
	strcpy(scestr, G.sce);	/* even bewaren */
	G.f |= G_DISABLE_OK;

	sprintf(tmp2, "%d", abs(getpid()));

	make_file_string (tstr, U.tempdir, tmp2);

	save_over = G.save_over;
	write_file(tstr);
	G.save_over = save_over;

	G.f &= ~G_DISABLE_OK;
	strcpy(G.sce, scestr);
	noBlog= 0;
}

void delete_autosave()
{
	int file;
	char tstr[FILE_MAXDIR+FILE_MAXFILE], str[FILE_MAXDIR+FILE_MAXFILE], tmpfile[FILE_MAXFILE];
	
	sprintf(tmpfile, "%d.blend", abs(getpid()));
	
	make_file_string(tstr, U.tempdir, tmpfile);

	file= open(tstr, O_BINARY|O_RDONLY);
	if(file>0) {
		close(file);
		
		make_file_string(str, U.tempdir, "quit.blend");
		fop_move (tstr, str);
	}
}

/* ********************** PSX ******************************** */

#define COORDFIX	4096.0

short le_floatangshort(float ftemp)
{
	int temp;
	short new;
	char *rt=(char *)&temp, *rtn=(char *)&new;
	
	temp= ffloor(4096.0*ftemp/(2.0*M_PI));
	if(temp > 0) temp= temp % 4096;
	else temp= - ( (abs(temp) % 4096));
	
	rtn[0]= rt[3];
	rtn[1]= rt[2];

	return new;
}

short le_short(short temp)
{
	short new;
	char *rt=(char *)&temp, *rtn=(char *)&new;

	rtn[0]= rt[1];
	rtn[1]= rt[0];

	return new;
}


int le_int(int temp)
{
	int new;
	char *rt=(char *)&temp, *rtn=(char *)&new;

	rtn[0]= rt[3];
	rtn[1]= rt[2];
	rtn[2]= rt[1];
	rtn[3]= rt[0];

	return new;
}

int le_coordint(float ftemp)
{
	int temp;

	ftemp*= COORDFIX;
	temp= ffloor(ftemp+.5);

	return le_int(temp);
}


