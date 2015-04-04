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

/*  readfile.c   juni 94     MIXED MODEL

 * 
 *  jan 95
 *  maart 95
 *  Version: $Id: readfile.c,v 1.47 2000/09/24 20:22:10 ton Exp $
 * 
 *   ZWAK PUNT: newadres berekening en meerdere files: oplossen
 *   bijvoorbeeld door per file aparte newadresarrays te maken? 
 *   
 * 
 * 
	LEZEN
	
	- Bestaande Library (Main) pushen of vrijgeven
	- Nieuwe Main alloceren
	- file inlezen
	- lees SDNA
	- per LibBlock
		- lees LibBlock
		- als Library
			- nieuwe Main maken
			- ID's eraan hangen
		- else 
			- lees bijhorende direkte data
			- link direkte data (intern en aan LibBlock)
	- lees FileGlobal
	- lees USER data, als aangegeven (~/.B.blend)
	- file vrijgeven
	- per Library met Scene (per Main)
		- file inlezen
		- lees SDNA
		- alle LibBlocks uit Scene opzoeken en ID's aan Main hagen
			- als extern LibBlock
				- zoek Main's af
					- is al ingelezen:
					- nog niet ingelezen
					- of nieuwe Main maken
		- per LibBlock
			- recursief dieper lezen
			- lees bijhorende direkte data
			- link direkte data (intern en aan LibBlock)
		- file vrijgeven
	- per Library met nog niet gelezen LibBlocks
		- file inlezen
		- lees SDNA
		- per LibBlock
			- recursief dieper lezen
			- lees bijhorende direkte data
			- link direkte data (intern en aan LibBlock)
		- file vrijgeven
	- alle Main's samenvoegen
	- alle LibBlocks linken en indirekte pointers naar libblocks
	- FileGlobal goedzetten en pointers naar Global kopieeren
	
 *
 *
 *
 */



#include "blender.h"
#include "graphics.h"
#include "file.h"
#include "screen.h"
#include "render.h"
#include "sequence.h"
#include "effect.h"
#include "ika.h"
#include "oops.h"
#include "imasel.h"
#include "datatoc.h"
#include "text.h"
#include "sound.h"
#include "game.h"
#include "group.h"

/* own include */
#include "readfile_ext.h"

#ifndef WIN32
#include <sys/param.h>
#endif
#include <fcntl.h>

typedef struct OldNew {
	void *old, *newp;
	int nr;
} OldNew;

extern char *compflags;		/* genfile.c */
extern struct SDNA cur_sdna;
void read_libraries();

OldNew *datablocks=0;
OldNew *libblocks=0;
OldNew *globblocks=0;
int datacount= 0, maxdatacount=1024;
int globcount= 0, maxglobcount=1024;
int libcount= 0, maxlibcount=1024;
int pointerlen, installed_blend=0;

extern int switch_endian;	/* genfile.c */

void *disable_newlibadr;

/* ******************************************* */

extern char bprogname[]; /* usiblender.c */

char *gethome(void) {
	
	#ifdef __BeOS
		return "/boot/home/";		/* BeOS 4.5: doubleclick at icon doesnt give home env */

	#elif !defined(__WIN32)
		return getenv("HOME");

	#else /* Windows */
		char * ret;
		ret= getenv("HOME");
		if(ret) {
			if (G.f&G_DEBUG) printf ("HOME == %s\n", ret);
			if (fop_exists(ret)) return ret;
			if (G.f&G_DEBUG) printf ("Unable to find home at: %s\n", ret);
		}
		
		ret= getenv("WINDOWS");		
		if (ret) {
			if (G.f&G_DEBUG) printf ("WINDOWS == %s\n", ret);
			if(fop_exists(ret)) return ret;
			if (G.f&G_DEBUG) printf ("Unable to find home at: %s\n", ret);
		}

		ret= getenv("WINDIR");	
		if(ret) {
			if (G.f&G_DEBUG) printf ("WINDIR == %s\n", ret);
			if(fop_exists(ret)) return ret;
			if (G.f&G_DEBUG) printf ("Unable to find home at: %s\n", ret);
		}
		
		if (G.f&G_DEBUG) printf ("Unable to find home\n");
		return "C:\\Temp";	
	#endif
}

/* ************ DIV ****************** */

int testextensie(char *str, char *ext)
{
	short a, b;
	int retval;

	a= strlen(str);
	b= strlen(ext);

	if(a==0 || b==0 || b>=a) {
		retval = 0;
	} else if (strcasecmp(ext, str + a - b)) {
		retval = 0;	
	} else {
		retval = 1;
	}

	return (retval);
}

int convertstringcode(char *str)
{
	int len, ret= 0;
	char *slash, temp[FILE_MAXDIR+FILE_MAXFILE];

	if (str[0] == '/' && str[1] == '/') {
		strcpy(temp, G.sce);
		
		/* Find the last slash */
		slash= (strrchr(temp, '/')>strrchr(temp, '\\'))?strrchr(temp, '/'):strrchr(temp, '\\');
		if (slash) {
			strcpy(slash+1, str+2);
			strcpy(str, temp);
		}
		else {
			/* pad zonder slash! */
			strcpy(temp, str+2);
			strcpy(str, temp);
		}
		ret= 1;
	}
	len= strlen(str);
	if(len && str[len-1]=='#') {
		sprintf(str+len-1, "%04d", CFRA);
	}
	return ret;
}

void makestringcode(char *str)
{
	char *slash, len, temp[FILE_MAXDIR];

	strcpy(temp, G.sce);

	/* Find the last slash */
	slash= (strrchr(temp, '/')>strrchr(temp, '\\'))?strrchr(temp, '/'):strrchr(temp, '\\');
	if(slash) {
		*(slash+1)= 0;
		len= strlen(temp);
		if(len) {
			if(strncmp(str, temp, len)==0) {
				temp[0]= '/';
				temp[1]= '/';
				strcpy(temp+2, str+len);
				strcpy(str, temp);
			}
		}
	}
}

void splitdirstring(char *di,char *fi)
{
	char *slash;
	
	/* Find the last slash */
	slash= (strrchr(di, '/')>strrchr(di, '\\'))?strrchr(di, '/'):strrchr(di, '\\');
	
	if (slash) {
		strcpy(fi, slash+1);
		*(slash+1)=0;
	}
}


/* ************** OLD POINTERS ******************* */

/* three systems:
   - newadr() add_data_adr: for direct data
   - newglobadr() add_glob_adr: for global data
   - newlibadr()  addl_lob_adr: for lib data
*/

void add_data_adr(void *old, void *new)
/* met dynamische malloc
 * (0, 1) doorgeven herinitialiseert en geeft ongebruikte blokken vrij
 * (0, 0) doorgeven geeft alles vrij 
 */
{
	OldNew *temp;
	int a;
	
	if(old==0) {	/* ongebruikte vrijgeven */
		temp= datablocks;
		for(a=0; a<datacount; a++, temp++) {
			if(temp->nr==0 && temp->newp) freeN(temp->newp);
		}
		if(new==0 && datablocks) {
			freeN(datablocks);
			datablocks= 0;
			maxdatacount= 1024;
		}
		datacount= 0;
	}
	else {
		if(datablocks==0) {
			datablocks= mallocN(maxdatacount*sizeof(OldNew), "OldNew");
		}
		else if(datacount==maxdatacount) {
			maxdatacount*= 2;
			temp= mallocN(maxdatacount*sizeof(OldNew), "OldNew");
			memcpy(temp, datablocks, maxdatacount*sizeof(OldNew)/2);
			freeN(datablocks);
			datablocks= temp;
		}
		
		temp= datablocks+datacount;
		temp->old= old;
		temp->newp= new;
		temp->nr= 0;
		
		datacount++;
	}
}

void add_glob_adr(void *old, void *new)
/* met dynamische malloc
 * (0, 1) doorgeven herinitialiseert en geeft ongebruikte blokken vrij
 * (0, 0) doorgeven geeft alles vrij 
 */
{
	OldNew *temp;

	if(old==0) {  /* free globblocks */
		temp= globblocks;

		if(new==0 && globblocks) {
			freeN(globblocks);
			globblocks= 0;
			maxglobcount= 1024;
		}
		globcount= 0;
	}
	else {
		if(globblocks==0) {
			globblocks= mallocN(maxglobcount*sizeof(OldNew), "OldNew");
		}
		else if(globcount==maxglobcount) {
			maxglobcount*= 2;
			temp= mallocN(maxglobcount*sizeof(OldNew), "OldNew");
			memcpy(temp, globblocks, maxglobcount*sizeof(OldNew)/2);
			freeN(globblocks);
			globblocks= temp;
		}
		
		temp= globblocks+globcount;
		temp->old= old;
		temp->newp= new;
		temp->nr= 0;
		
		globcount++;
	}
}

void add_lib_adr(void *old, void *new)
/* met dynamische malloc
 * (0, 0) doorgeven geeft alles vrij
 * Zet aantal users al op 1!!!
 */
{
	OldNew *temp=0;
	
	/* onderscheid tussen twee gevallen:
	 * 
	 * 1. lib obj's in locale scene, ob->parent		(old is uit library)
	 * 2. lib obj's in locale scene: base->object   (old is uit locale scene)
	 * 
	 */
	
	if(disable_newlibadr) {
		disable_newlibadr= new;
		
	}
	
	if(old==0) {	/* alles vrijgeven */
		if(libblocks) freeN(libblocks);
		libblocks= 0;
		maxlibcount= 1024;
		libcount= 0;
	}
	else {
		if(libblocks==0) {
			libblocks= mallocN(maxlibcount*sizeof(OldNew), "OldNew");
		}
		else if(libcount==maxlibcount) {
			maxlibcount*= 2;
			temp= mallocN(maxlibcount*sizeof(OldNew), "OldNew");
			memcpy(temp, libblocks, maxlibcount*sizeof(OldNew)/2);
			freeN(libblocks);
			libblocks= temp;
		}
		
		temp= libblocks+libcount;
		temp->old= old;
		temp->newp= new;
		temp->nr= 1;
		
		libcount++;
	}
	
}

void *newadr(void *adr)		/* only direct databocks */
{
	static int lastone= 0;
	struct OldNew *onew;

	if(adr) {
		/* op goed geluk: eerst het volgende blok doen */
		if(lastone<datacount-1) {
			lastone++;
			onew= datablocks+lastone;
			if(onew->old==adr) {
				onew->nr++;
				return onew->newp;
			}
		}
		
		lastone= 0;
		onew= datablocks;
		while(lastone<datacount) {
			if(onew->old==adr) {
				onew->nr++;
				return onew->newp;
			}
			onew++;
			lastone++;
		}
	}
	
	return 0;
}

void *newglobadr(void *adr)		/* direct datablocks with global linking */
{
	static int lastone= 0;
	struct OldNew *onew;

	if(adr) {
		/* op goed geluk: eerst het volgende blok doen */
		if(lastone<globcount-1) {
			lastone++;
			onew= globblocks+lastone;
			if(onew->old==adr) {
				onew->nr++;
				return onew->newp;
			}
		}
		
		lastone= 0;
		onew= globblocks;
		while(lastone<globcount) {
			if(onew->old==adr) {
				onew->nr++;
				return onew->newp;
			}
			onew++;
			lastone++;
		}
	}
	
	return 0;
}

void *newlibadr(void *lib, void *adr)		/* only lib data */
{
	static int lastone= 0;
	struct OldNew *onew;
	ID *id;
	
	/* ook testen op lib: lib==id->lib) */
	
	if(adr) {
	
		/* op goed geluk: eerst het volgende blok doen */
		/* als je dit weglaat gaat newcarou.blend mis */
		if(lastone<libcount-1) {
			lastone++;
			onew= libblocks+lastone;
			if(onew->old==adr) {
				id= onew->newp;
				if(id==NULL) return NULL;
				else if(id->lib==0 && lib);
				else {
					onew->nr++;
					return onew->newp;
				}
			}
		}
		
		lastone= 0;
		onew= libblocks;
		while(lastone<libcount) {
			if(onew->old==adr) {
				id= onew->newp;
				
				if(id && id->lib==0 && lib);
				else {
					onew->nr++;
					return onew->newp;
				}
			}
			onew++;
			lastone++;
		}

	}

	return 0;
}

void *newlibadr_us(void *lib, void *adr)		/* hoogt usernummer op */
{
	ID *id;
	
	id= newlibadr(lib, adr);
	if(id) {
		id->us++;
	}
	
	return id;
}

void *newlibadr_us_type(short type, void *adr)		/* alleen Lib datablokken */
{
	static int lastone= 0;
	struct OldNew *onew;
	ID *id;
	
	if(adr) {
	
		/* op goed geluk: eerst het volgende blok doen */
		/* als je dit weglaat gaat newcarou.blend mis */

		if(lastone<libcount-1) {
			lastone++;
			onew= libblocks+lastone;
			if(onew->old==adr) {
				id= onew->newp;
				if( GS(id->name) == type ) {
					id->us++;
					onew->nr++;
					return id;
				}
			}
		}
		
		lastone= 0;
		onew= libblocks;
		while(lastone<libcount) {
			if(onew->old==adr) {
				id= onew->newp;
				
				if( GS(id->name) == type ) {
					id->us++;
					onew->nr++;
					return id;
				}
			}
			onew++;
			lastone++;
		}

	}

	return 0;
}

void change_libadr(void *old, void *new)
{
	struct OldNew *onew;
	ID *id;
	int lastone= 0;
	
	onew= libblocks;
	while(lastone<libcount) {
		
		id= onew->newp;
		if(id && id->lib) {
			if(onew->newp==old) {
				onew->newp= new;
				
				/* geen return; blijkbaar kunnen er meer zijn? */
			}
		}
		
		onew++;
		lastone++;
	}
	
}



/* ********** END OUDE POINTERS ****************** */
/* ********** READ FILE ****************** */

void switch_endian_structs(BHead *bhead)
{
	int oldlen, blocks;
	short *spo;
	char *data;
	
	blocks= bhead->nr;
	data= (char *)(bhead+1);
	
	spo= old_sdna.structs[bhead->SDNAnr];
	oldlen= old_sdna.typelens[ spo[0] ];

	while(blocks--) {
		switch_endian_struct(bhead->SDNAnr, data);
		data+= oldlen;
	}
}

void read_struct(BHead *bh)
{
	void *temp= 0;
	
	if(bh->len) {
		
		if(bh->SDNAnr && switch_endian) switch_endian_structs(bh);

		if(compflags[bh->SDNAnr]) {		/* flag==0: bestaat niet meer */
			
			/* dit hele ingewikkelde spul is de naam van de struct, let op: old_sdna! */
			temp= mallocN(bh->len, old_sdna.types[ *old_sdna.structs[bh->SDNAnr] ]);
			
			/* temp= mallocN(bh->len, "read_struct"); */
			memcpy(temp, (bh+1), bh->len);
			
			if(compflags[bh->SDNAnr]==2) reconstruct(bh->SDNAnr, bh->nr, &temp);
		}
	}
	
	add_data_adr(bh->old, temp);
}

void newstructname(BHead *bh)
{
	/* rijtje hardcoded uitzonderingen */
	short *sp;
	
	sp= old_sdna.structs[bh->SDNAnr];
	if( strcmp("Screen", old_sdna.types[sp[0]])==0 ) {
		bh->SDNAnr= findstruct_nr(&old_sdna, "bScreen");
	}
}


void *read_libstruct(BHead *bh)
{
	void *temp= 0;

	if(bh->len) {
	
		if(switch_endian) switch_endian_structs(bh);
		
		if(compflags[bh->SDNAnr]) {		/* flag==0: bestaat niet meer */

			temp= mallocN(bh->len, "read_libstruct");
			memcpy(temp, (bh+1), bh->len);

			if(compflags[bh->SDNAnr]==2) reconstruct(bh->SDNAnr, bh->nr, &temp);
		}
	}

	add_lib_adr(bh->old, temp);

	return temp;
}

void read_struct_expl(BHead *bh, void **data)	
		/* dubbele pointer ivm reconstruct */
{

	if(bh->len) {

		if(bh->SDNAnr && switch_endian) switch_endian_structs(bh);
		
		if(compflags[bh->SDNAnr]) {		/* flag==0: bestaat niet meer */
			
			*data= mallocN(bh->len, "read_struct_expl");
			memcpy(*data, (bh+1), bh->len);
			
			if(compflags[bh->SDNAnr]==2) reconstruct(bh->SDNAnr, bh->nr, data);
		}
	}
}

void link_list(ListBase *lb)		/* alleen direkte data */
{
	Link *ln, *prev;
	
	if(lb->first==0) return;

	lb->first= newadr(lb->first);
	
	ln= lb->first;
	prev= 0;
	while(ln) {
		
		ln->next= newadr(ln->next);
		ln->prev= prev;
		
		prev= ln;
		ln= ln->next;
	}
	
	lb->last= prev;
}

void link_glob_list(ListBase *lb)		/* for glob data */
{
	Link *ln, *prev;
	void *poin;
    
	if(lb->first==0) return;
	
    poin= newadr(lb->first);
	if(lb->first) add_glob_adr(lb->first, poin);
	lb->first= poin;
	
	ln= lb->first;
	prev= 0;
	while(ln) {
		
        poin= newadr(ln->next);
        if(ln->next) add_glob_adr(ln->next, poin);
		ln->next= poin;
		ln->prev= prev;
		
		prev= ln;
		ln= ln->next;
	}
	
	lb->last= prev;
}

void test_pointer_array(void **mat)
{
#ifdef WIN32
	DWORDLONG *lpoin, *lmat;
#else
	long long *lpoin, *lmat;
#endif
	int len, *ipoin, *imat;

	/* mat is still 'old_sdna' */
	if(*mat) {
		len= alloc_len(*mat)/old_sdna.pointerlen;
		
		if(old_sdna.pointerlen==8 && cur_sdna.pointerlen==4) {
			ipoin=imat= mallocN( len*4, "newmatar");
			lpoin= *mat;

			while(len-- > 0) {
				if(switch_endian) SWITCH_LONGINT(*lpoin);
				*ipoin= (*lpoin)>>3;
				ipoin++;
				lpoin++;
			}
			freeN(*mat);
			*mat= imat;
		}
		
		if(old_sdna.pointerlen==4 && cur_sdna.pointerlen==8) {
			lpoin=lmat= mallocN( len*8, "newmatar");
			ipoin= *mat;

			while(len-- > 0) {
				*lpoin= *ipoin;
				ipoin++;
				lpoin++;
			}
			freeN(*mat);
			*mat= lmat;
		}
	}
}

/* ************ READ scriptlinks *************** */

void lib_link_scriptlink(ID *id, ScriptLink *slink)
{
	int i;

	for(i=0; i<slink->totscript; i++) {
		slink->scripts[i]= newlibadr(id->lib, slink->scripts[i]);
	}
}		

void direct_link_scriptlink(ScriptLink *slink)
{
	slink->scripts= newadr(slink->scripts);	
	slink->flag= newadr(slink->flag);	

	if(switch_endian) {
		int a;
		
		for(a=0; a<slink->totscript; a++) {
			SWITCH_SHORT(slink->flag[a]);
		}
	}
	
}

/* ************ READ IKA ***************** */

void lib_link_ika(Main *main)
{
	Ika *ika;
	int a;
	Deform *def;
	
	ika= main->ika.first;
	while(ika) {
		if(ika->id.flag & LIB_NEEDLINK) {
			
			ika->parent= newlibadr(ika->id.lib, ika->parent);
			
			a= ika->totdef;
			def= ika->def;
			while(a--) {
				def->ob=  newlibadr(ika->id.lib, def->ob);
				def++;
			}
			
			ika->id.flag -= LIB_NEEDLINK;
		}
		ika= ika->id.next;
	}
	
}

void direct_link_ika(Ika *ika)
{
	
	link_list(&ika->limbbase);

	ika->def= newadr(ika->def);

	/* afvangen fout uit V.138 en ouder */
	if(ika->def==0) ika->totdef= 0;
}


/* ************ READ CAMERA ***************** */

void lib_link_camera(Main *main)
{
	Camera *ca;
	
	ca= main->camera.first;
	while(ca) {
		if(ca->id.flag & LIB_NEEDLINK) {
			
			ca->ipo= newlibadr_us(ca->id.lib, ca->ipo);

			lib_link_scriptlink(&ca->id, &ca->scriptlink);
			
			ca->id.flag -= LIB_NEEDLINK;
		}
		ca= ca->id.next;
	}
	
}

void direct_link_camera(Camera *ca)
{
	direct_link_scriptlink(&ca->scriptlink);
}


	
/* ************ READ LATTICE ***************** */

void lib_link_latt(Main *main)
{
	Lattice *lt;
	
	lt= main->latt.first;
	while(lt) {
		if(lt->id.flag & LIB_NEEDLINK) {
			
			lt->ipo= newlibadr_us(lt->id.lib, lt->ipo);
			lt->key= newlibadr_us(lt->id.lib, lt->key);
			
			lt->id.flag -= LIB_NEEDLINK;
		}
		lt= lt->id.next;
	}
	
}

void direct_link_latt(Lattice *lt)
{
	lt->def= newadr(lt->def);

}

/* ************ READ LAMP ***************** */

void lib_link_lamp(Main *main)
{
	Lamp *la;
	MTex *mtex;
	int a;
	
	la= main->lamp.first;
	while(la) {
		if(la->id.flag & LIB_NEEDLINK) {

			for(a=0; a<8; a++) {
				mtex= la->mtex[a];
				if(mtex) {
					mtex->tex= newlibadr_us(la->id.lib, mtex->tex);
					mtex->object= newlibadr(la->id.lib, mtex->object);
				}
			}
		
			la->ipo= newlibadr_us(la->id.lib, la->ipo);
			
			lib_link_scriptlink(&la->id, &la->scriptlink);
			
			la->id.flag -= LIB_NEEDLINK;
		}
		la= la->id.next;
	}
}

void direct_link_lamp(Lamp *la)
{
	int a;

	direct_link_scriptlink(&la->scriptlink);
		
	for(a=0; a<8; a++) {
		la->mtex[a]= newadr(la->mtex[a]);
	}
}

/* ************ READ keys ***************** */

void lib_link_key(Main *main)
{
	Key *key;
	
	key= main->key.first;
	while(key) {
		if(key->id.flag & LIB_NEEDLINK) {
			
			key->ipo= newlibadr_us(key->id.lib, key->ipo);
			key->from= newlibadr(key->id.lib, key->from);
			
			key->id.flag -= LIB_NEEDLINK;
		}
		key= key->id.next;
	}
	
}

void direct_link_key(Key *key)
{
	KeyBlock *kb;
	
	link_list(&(key->block));
	
	key->refkey= newadr(key->refkey);
	
	kb= key->block.first;
	while(kb) {
		
		kb->data= newadr(kb->data);
		
		if(switch_endian) switch_endian_keyblock(key, kb);
			
		kb= kb->next;
	}
}

/* ************ READ mball ***************** */

void lib_link_mball(Main *main)
{
	MetaBall *mb;
	int a;
	
	mb= main->mball.first;
	while(mb) {
		if(mb->id.flag & LIB_NEEDLINK) {
			
			for(a=0; a<mb->totcol; a++) mb->mat[a]= newlibadr_us(mb->id.lib, mb->mat[a]);

			mb->ipo= newlibadr_us(mb->id.lib, mb->ipo);

			mb->id.flag -= LIB_NEEDLINK;
		}
		mb= mb->id.next;
	}
	
}

void direct_link_mball(MetaBall *mb)
{

	mb->mat= newadr(mb->mat);
	test_pointer_array((void **)&mb->mat);

	link_list(&(mb->elems));
	
	mb->disp.first= mb->disp.last= 0;
	
	mb->bb= 0;

}

/* ************ READ WORLD ***************** */

void lib_link_world(Main *main)
{
	World *wrld;
	MTex *mtex;
	int a;
	
	wrld= main->world.first;
	while(wrld) {
		if(wrld->id.flag & LIB_NEEDLINK) {
		
			wrld->ipo= newlibadr_us(wrld->id.lib, wrld->ipo);
			
			for(a=0; a<8; a++) {
				mtex= wrld->mtex[a];
				if(mtex) {
					mtex->tex= newlibadr_us(wrld->id.lib, mtex->tex);
					mtex->object= newlibadr(wrld->id.lib, mtex->object);
				}
			}

			lib_link_scriptlink(&wrld->id, &wrld->scriptlink);
			
			wrld->id.flag -= LIB_NEEDLINK;
		}
		wrld= wrld->id.next;
	}
	
}

void direct_link_world(World *wrld)
{
	int a;

	direct_link_scriptlink(&wrld->scriptlink);
	
	for(a=0; a<8; a++) {
		wrld->mtex[a]= newadr(wrld->mtex[a]);
	}
}


/* ************ READ IPO ***************** */

void lib_link_ipo(Main *main)
{
	Ipo *ipo;
	
	ipo= main->ipo.first;
	while(ipo) {
		if(ipo->id.flag & LIB_NEEDLINK) {
			
			ipo->id.flag -= LIB_NEEDLINK;
		}
		ipo= ipo->id.next;
	}
	
}

void direct_link_ipo(Ipo *ipo)
{
	IpoCurve *icu;
	
	link_list(&(ipo->curve));
	icu= ipo->curve.first;
	while(icu) {
		icu->bezt= newadr(icu->bezt);
		icu->bp= newadr(icu->bp);
		icu= icu->next;
	}
}

/* ************ READ VFONT ***************** */

void direct_link_vfont(VFont *vf)
{
	vf->data= callocN(sizeof(VFontData), "vfontdata");
	vf->packedfile = newadr(vf->packedfile);
	if (vf->packedfile) {
		vf->packedfile->data = newadr(vf->packedfile->data);
	}

	vf->flag= 1;	/* NEEDLOAD */
}

/* ************ READ TEXT ****************** */

void lib_link_text(Main *main)
{
	Text *text;
	
	text= main->text.first;
	while(text) {
		if(text->id.flag & LIB_NEEDLINK) {
			text->id.flag -= LIB_NEEDLINK;
		}
		text= text->id.next;
	}	
}

void print_text(Text *text)
{
	TextLine *line;
	
	printf ("-- Text: %s (%d)--\n", text->id.name, text->nlines);
	line= text->lines.first;
	while (line) {
		printf ("%x %x %x %d: <%s>\n", line->prev, line, line->next, line->len, line->line);
		line= line->next;
	}
	
	printf ("Main cursor: %x %d\n", text->curl, text->curc);
	printf ("Selection cursor: %x %d\n", text->sell, text->selc);
}

void direct_link_text(Text *text)
{
	TextLine *ln;
	
	text->name= newadr(text->name);
	
	text->undo_pos= -1;
	text->undo_len= TXT_INIT_UNDO;
	text->undo_buf= mallocN(text->undo_len, "undo buf");
	
	text->compiled= NULL;
		
	if(text->flags & TXT_ISEXT) {
		reopen_text(text);
	} else {

		if(text->lines.first==0) return;
	
		link_list(&text->lines);
	
		text->curl= newadr(text->curl);
		text->sell= newadr(text->sell);
	
		ln= text->lines.first;
		while(ln) {
			ln->line= newadr(ln->line);
	
			if (ln->len != strlen(ln->line)) {
				printf ("Error loading text, line lengths differ\n");
				ln->len = strlen(ln->line);
			}
	
			ln= ln->next;
		}
			
		text->flags |= TXT_ISTMP;
	}
/* 	print_text (text); */
	
	text->id.us= 1;
}

/* ************ READ IMAGE ***************** */

void lib_link_image(Main *main)
{
	Image *ima;
	
	ima= main->image.first;
	while (ima) {
		if(ima->id.flag & LIB_NEEDLINK) {
			
			ima->id.flag -= LIB_NEEDLINK;
		}
		ima= ima->id.next;
	}
	
}

void direct_link_image(Image *ima)
{
	int a;
	
	ima->ibuf= 0;
	ima->anim= 0;
	for(a=0; a<MAXMIPMAP; a++) ima->mipmap[a]= 0;
	ima->repbind= 0;
	ima->bindcode= 0;

		
	ima->packedfile = newadr(ima->packedfile);
	if (ima->packedfile) {
		ima->packedfile->data = newadr(ima->packedfile->data);
	}

		
	ima->ok= 1;
	
}


/* ************ READ CURVE ***************** */

void lib_link_curve(Main *main)
{
	Curve *cu;
	int a;
	
	cu= main->curve.first;
	while(cu) {
		if(cu->id.flag & LIB_NEEDLINK) {
		
			for(a=0; a<cu->totcol; a++) cu->mat[a]= newlibadr_us(cu->id.lib, cu->mat[a]);

			cu->bevobj= newlibadr(cu->id.lib, cu->bevobj);
			cu->textoncurve= newlibadr(cu->id.lib, cu->textoncurve);
			cu->vfont= newlibadr_us(cu->id.lib, cu->vfont);

			cu->ipo= newlibadr_us(cu->id.lib, cu->ipo);
			cu->key= newlibadr_us(cu->id.lib, cu->key);
			
			cu->id.flag -= LIB_NEEDLINK;
		}
		cu= cu->id.next;
	}
	
}

void direct_link_curve(Curve *cu)
{
	Nurb *nu;
	
	cu->mat= newadr(cu->mat);
	test_pointer_array((void **)&cu->mat);
	cu->str= newadr(cu->str);

	if(cu->vfont==0) link_list(&(cu->nurb));
	else {
		cu->nurb.first=cu->nurb.last= 0;
	}
	
	cu->bev.first=cu->bev.last= 0;
	cu->disp.first=cu->disp.last= 0;
	cu->path= 0;
	
	nu= cu->nurb.first;
	while(nu) {
		nu->bezt= newadr(nu->bezt);
		nu->bp= newadr(nu->bp);
		nu->knotsu= newadr(nu->knotsu);
		nu->knotsv= newadr(nu->knotsv);

		if(switch_endian) {
			switch_endian_knots(nu);
		}
		
		nu= nu->next;
	}
	cu->bb= 0;
}

/* ************ READ TEX ***************** */

void lib_link_texture(Main *main)
{
	Tex *tex;
	
	tex= main->tex.first;
	while(tex) {
		if(tex->id.flag & LIB_NEEDLINK) {
		
			tex->ima= newlibadr_us(tex->id.lib, tex->ima);
			tex->ipo= newlibadr_us(tex->id.lib, tex->ipo);
			if(tex->env) tex->env->object= newlibadr(tex->id.lib, tex->env->object);
			
			tex->id.flag -= LIB_NEEDLINK;
		}
		tex= tex->id.next;
	}
	
}

void direct_link_texture(Tex *tex)
{
	tex->plugin= newadr(tex->plugin);
	if(tex->plugin) {
		tex->plugin->handle= 0;
		open_plugin_tex(tex->plugin);
	}
	tex->coba= newadr(tex->coba);
	tex->env= newadr(tex->env);
	if(tex->env) {
		tex->env->ima= 0;
		bzero(tex->env->cube, 6*sizeof(void *));
		tex->env->ok= 0;
	}
}



/* ************ READ MATERIAL ***************** */

void lib_link_material(Main *main)
{
	Material *ma;
	MTex *mtex;
	int a;
	
	ma= main->mat.first;
	while(ma) {
		if(ma->id.flag & LIB_NEEDLINK) {
		
			ma->ipo= newlibadr_us(ma->id.lib, ma->ipo);
			
			for(a=0; a<8; a++) {
				mtex= ma->mtex[a];
				if(mtex) {
					mtex->tex= newlibadr_us(ma->id.lib, mtex->tex);
					mtex->object= newlibadr(ma->id.lib, mtex->object);
				}
			}

			lib_link_scriptlink(&ma->id, &ma->scriptlink);
			
			ma->id.flag -= LIB_NEEDLINK;
		}
		ma= ma->id.next;
	}
	
}

void direct_link_material(Material *ma)
{
	int a;
	
	direct_link_scriptlink(&ma->scriptlink);
	
	for(a=0; a<8; a++) {
		ma->mtex[a]= newadr(ma->mtex[a]);
	}
	ma->ren= 0;	/* mag niet blijven hangen, maarja */
}

/* ************ READ MESH ***************** */

void lib_link_mesh(Main *main)
{
	Mesh *me;
	TFace *tface;
	Image *ima;
	int a;
	
	me= main->mesh.first;
	while(me) {
		if(me->id.flag & LIB_NEEDLINK) {
		
			for(a=0; a<me->totcol; a++) me->mat[a]= newlibadr_us(me->id.lib, me->mat[a]);
			me->ipo= newlibadr_us(me->id.lib, me->ipo);
			me->key= newlibadr_us(me->id.lib, me->key);
			me->texcomesh= newlibadr_us(me->id.lib, me->texcomesh);
			
			if(me->tface) {
				a= me->totface;
				tface= me->tface;
				while(a--) {
					tface->tpage= newlibadr(me->id.lib, tface->tpage);

					if(tface->tpage) {
						ima= tface->tpage;
						if(ima->id.us==0) ima->id.us= 1;
					}
					/* was already swapped in genfile, should be! */
					if(switch_endian) {
						SWITCH_INT(tface->col[0]);
						SWITCH_INT(tface->col[1]);
						SWITCH_INT(tface->col[2]);
						SWITCH_INT(tface->col[3]);
					}
					
					tface++;
				}
			}
			me->id.flag -= LIB_NEEDLINK;
		}
		me= me->id.next;
	}
}

void direct_link_mesh(Mesh *mesh)
{
	mesh->mat= newadr(mesh->mat);
	test_pointer_array((void **)&mesh->mat);
	mesh->mvert= newadr(mesh->mvert);
	mesh->mface= newadr(mesh->mface);
	mesh->tface= newadr(mesh->tface);
	mesh->mcol= newadr(mesh->mcol);
	mesh->msticky= newadr(mesh->msticky);
	
	mesh->disp.first= mesh->disp.last= 0;
	mesh->bb= 0;
	mesh->oc= 0;
	mesh->dface= 0;
	mesh->orco= 0;
}

/* ************ READ OBJECT ***************** */


#include "old_game.h"

void link_sector_or_life(Object *ob)
{

	if(ob->type==OLD_OB_SECTOR) {
		Sector *se;

		se= ob->data;
		if(se) {
			se->texmesh= newlibadr_us(ob->id.lib, se->texmesh);
			if(se->texmesh) {
				ob->data= se->texmesh;
				ob->type= OB_MESH;
				ob->gameflag= OB_SECTOR;
				if(ob->mat) freeN(ob->mat);
				ob->mat= NULL;
				ob->totcol= 0;
			}
		}
	}
	else {
		Life *lf;

		lf= ob->data;
		if(lf) {
			lf->texmesh= newlibadr_us(ob->id.lib, lf->texmesh);
			ob->inertia= lf->axsize;
			if(lf->texmesh) {
				ob->data= lf->texmesh;
				ob->type= OB_MESH;
				if(ob->mat) freeN(ob->mat);
				ob->mat= NULL;
				ob->totcol= 0;
			}
		}
	}
}


void lib_link_object(Main *main)
{
	Object *ob;
	bSensor *sens;
	bController *cont;
	bActuator *act;
	void *poin;
	int warn=0, a;
	
	ob= main->object.first;
	while(ob) {
		if(ob->id.flag & LIB_NEEDLINK) {
	
			ob->parent= newlibadr(ob->id.lib, ob->parent);
			ob->track= newlibadr(ob->id.lib, ob->track);
			ob->ipo= newlibadr_us(ob->id.lib, ob->ipo);
			
			poin= ob->data;
		
			ob->data= newlibadr_us(ob->id.lib, ob->data);
			
			if(ob->type==OLD_OB_LIFE || ob->type==OLD_OB_SECTOR) 
				link_sector_or_life(ob);
			
			if(ob->data==NULL && poin!=NULL) {
				ob->type= OB_EMPTY;
				warn= 1;
				if(ob->id.lib) printf("Can't find obdata of %s lib %s\n", ob->id.name+2, ob->id.lib->name);
				else printf("Object %s lost data. Lib:%x\n", ob->id.name+2, ob->id.lib);
			}
			for(a=0; a<ob->totcol; a++) ob->mat[a]= newlibadr_us(ob->id.lib, ob->mat[a]);
			
			ob->id.flag -= LIB_NEEDLINK;
			/* dit stond er eerst: weggehaald omdat de fie give_base_to... er niet meer is */
			/* if(ob->id.us) ob->id.flag -= LIB_NEEDLINK; */
			/* als us==0 wordt verderop nog een base gemaakt */
			
			sens= ob->sensors.first;
			while(sens) {
				for(a=0; a<sens->totlinks; a++) {
					sens->links[a]= newglobadr(sens->links[a]);
				}
				if(sens->type==SENS_TOUCH) {
					bTouchSensor *ts= sens->data;
					ts->ma= newlibadr(ob->id.lib, ts->ma);
				}
				sens= sens->next;
			}
			
			cont= ob->controllers.first;
			while(cont) {
				for(a=0; a<cont->totlinks; a++) {
					cont->links[a]= newglobadr(cont->links[a]);
				}
				if(cont->type==CONT_PYTHON) {
					bPythonCont *pc= cont->data;
					pc->text= newlibadr(ob->id.lib, pc->text);
				}
				cont->slinks= NULL;
				cont->totslinks= 0;
				
				cont= cont->next;
			}
			
			act= ob->actuators.first;
			while(act) {
				if(act->type==ACT_SOUND) {
					bSoundActuator *sa= act->data;
					sa->sound= newlibadr_us(ob->id.lib, sa->sound);
				}
				else if(act->type==ACT_CAMERA) {
					bCameraActuator *ca= act->data;
					ca->ob= newlibadr(ob->id.lib, ca->ob);
				}
					/* leave this one, it's obsolete but necessary to read for conversion */
				else if(act->type==ACT_ADD_OBJECT) {
					bAddObjectActuator *eoa= act->data;
					if(eoa) eoa->ob= newlibadr(ob->id.lib, eoa->ob);
				}
				else if(act->type==ACT_EDIT_OBJECT) {
					bEditObjectActuator *eoa= act->data;
					if(eoa==NULL) {
						init_actuator(act);
					}
					eoa->ob= newlibadr(ob->id.lib, eoa->ob);
					eoa->me= newlibadr(ob->id.lib, eoa->me);
				}
				else if(act->type==ACT_SCENE) {
					bSceneActuator *sa= act->data;
					sa->camera= newlibadr(ob->id.lib, sa->camera);
					sa->scene= newlibadr(ob->id.lib, sa->scene);
				}
				else if(act->type==ACT_PROPERTY) {
					bPropertyActuator *pa= act->data;
					pa->ob= newlibadr(ob->id.lib, pa->ob);
				}
				act= act->next;
			}
			
			lib_link_scriptlink(&ob->id, &ob->scriptlink);
		}
		ob= ob->id.next;
	}
	
	if(warn) error("WARNING IN CONSOLE");
}

void direct_link_object(Object *ob)
{
	PartEff *paf;
	bProperty *prop;
	bSensor *sens;
	bController *cont;
	bActuator *act;
	
	ob->disp.first=ob->disp.last= 0;

	direct_link_scriptlink(&ob->scriptlink);

	ob->mat= newadr(ob->mat);
	test_pointer_array((void **)&ob->mat);
	link_list(&ob->effect);
	paf= ob->effect.first;
	while(paf) {
		if(paf->type==EFF_PARTICLE) {
			paf->keys= 0;
		}
		if(paf->type==EFF_WAVE) {
			
		}
		paf= paf->next;
	}

	link_list(&ob->network);
	
	link_list(&ob->prop);
	prop= ob->prop.first;
	while(prop) {
		prop->poin= newadr(prop->poin);
		if(prop->poin==0) prop->poin= &prop->data;
		prop= prop->next;
	}

	link_list(&ob->sensors);
	sens= ob->sensors.first;
	while(sens) {
		sens->data= newadr(sens->data);	
		sens->links= newadr(sens->links);
		test_pointer_array((void **)&sens->links);
		sens= sens->next;
	}

	link_glob_list(&ob->controllers);
	cont= ob->controllers.first;
	while(cont) {
		cont->data= newadr(cont->data);
		cont->links= newadr(cont->links);
		test_pointer_array((void **)&cont->links);
		cont= cont->next;
	}

	link_glob_list(&ob->actuators);
	act= ob->actuators.first;
	while(act) {
		act->data= newadr(act->data);
		act= act->next;
	}

	ob->bb= 0;
}

/* ************ READ SCENE ***************** */

void lib_link_scene(Main *main)
{
	Scene *sce;
	Base *base, *next;
	Editing *ed;
	Sequence *seq;
	
	sce= main->scene.first;
	while(sce) {
		if(sce->id.flag & LIB_NEEDLINK) {
			sce->id.us= 1;
			sce->camera= newlibadr(sce->id.lib, sce->camera);
			sce->world= newlibadr_us(sce->id.lib, sce->world);
			sce->set= newlibadr(sce->id.lib, sce->set);
			sce->ima= newlibadr_us(sce->id.lib, sce->ima);
			sce->group= newlibadr_us(sce->id.lib, sce->group);
			
			base= sce->base.first;
			while(base) {
				next= base->next;
				
				/* base->object= newlibadr_us(sce->id.lib, base->object); */

				base->object= newlibadr_us_type(ID_OB, base->object);
				
				if(base->object==0) {
					printf("LIB ERROR: base removed\n");
					remlink(&sce->base, base);
					if(base==sce->basact) sce->basact= 0;
					freeN(base);
				}
				base= next;
			}
			
			ed= sce->ed;
			if(ed) {
				WHILE_SEQ(ed->seqbasep) {
					if(seq->ipo) seq->ipo= newlibadr_us(sce->id.lib, seq->ipo);
					if(seq->scene) seq->scene= newlibadr(sce->id.lib, seq->scene);
					seq->anim= 0;
				}
				END_SEQ
			}
			sce->id.flag -= LIB_NEEDLINK;
		}
		
		lib_link_scriptlink(&sce->id, &sce->scriptlink);
		
		sce= sce->id.next;
	}
}

void link_recurs_seq(ListBase *lb)
{
	Sequence *seq;

	link_list( lb );
	seq= lb->first;
	while(seq) {
		if(seq->seqbase.first) link_recurs_seq( &seq->seqbase );
		seq= seq->next;
	}
}

void direct_link_scene(Scene *sce)
{
	Editing *ed;
	Sequence *seq;
	StripElem *se;
	int a;
	
	link_list( &(sce->base) );

	sce->basact= newadr(sce->basact);

	sce->radio= newadr(sce->radio);
	sce->fcam= newadr(sce->fcam);
	
	if(sce->ed) {
		ed= sce->ed= newadr(sce->ed);
		
		ed->metastack.first= ed->metastack.last= 0;
		
		/* recursief sequenties linken, ook lb wordt goedgezet */
		link_recurs_seq(&ed->seqbase);
		
		ed->seqbasep= &ed->seqbase;
		
		WHILE_SEQ(ed->seqbasep) {
			
			seq->seq1= newadr(seq->seq1);
			seq->seq2= newadr(seq->seq2);
			seq->seq3= newadr(seq->seq3);
			/* eigenlijk een patch: na invoering drie-seq effects */
			if(seq->seq3==0) seq->seq3= seq->seq2;
			
			seq->curelem= 0;
			
			seq->plugin= newadr(seq->plugin);
			if(seq->plugin) open_plugin_seq(seq->plugin, seq->name+2);
	
			seq->strip= newadr(seq->strip);
			if(seq->strip && seq->strip->done==0) {
				seq->strip->done= 1;
				
				/* standaard: strips van effecten/meta's worden niet weggeschreven, wel malloccen */
				
				if(seq->type==SEQ_IMAGE) {
					seq->strip->stripdata= newadr(seq->strip->stripdata);
					se= seq->strip->stripdata;
					if(se) {
						for(a=0; a<seq->strip->len; a++, se++) {
							se->ok= 1;
							se->ibuf= 0;
						}
					}
				}
				else if(seq->type==SEQ_MOVIE) {
					/* alleen eerste stripelem zit in file */
					se= newadr(seq->strip->stripdata);
					
					if(se) {
						seq->strip->stripdata= callocN(seq->len*sizeof(StripElem), "stripelem");
						*seq->strip->stripdata= *se;
						freeN(se);
						
						se= seq->strip->stripdata;
					
						for(a=0; a<seq->strip->len; a++, se++) {
							se->ok= 1;
							se->ibuf= 0;
							se->nr= a + 1;
						}
					}
				}
				else if(seq->len>0) 
					seq->strip->stripdata= callocN(seq->len*sizeof(StripElem), "stripelem");

			}
		}
		END_SEQ
	}
	
	direct_link_scriptlink(&sce->scriptlink);
}

/* ************ READ SCREEN ***************** */

void lib_link_screen(Main *main)
{
	bScreen *sc;
	ScrArea *sa;
	View3D *v3d;
	SpaceIpo *si;
	SpaceButs *buts;
	SpaceFile *sfile;
	Oops *oops;
	
	sc= main->screen.first;
	while(sc) {
		if(sc->id.flag & LIB_NEEDLINK) {
			sc->id.us= 1;
			sc->scene= newlibadr(sc->id.lib, sc->scene);
			
			sa= sc->areabase.first;
			while(sa) {
				
				sa->full= newlibadr(sc->id.lib, sa->full);
	
				v3d= sa->spacedata.first;	/* v3d als voorbeeld */
				while(v3d) {
					if(v3d->spacetype==SPACE_VIEW3D) {

						v3d->camera= newlibadr(sc->id.lib, v3d->camera);
						
						if(v3d->bgpic) {
							v3d->bgpic->ima= newlibadr_us(sc->id.lib, v3d->bgpic->ima);
							v3d->bgpic->tex= newlibadr_us(sc->id.lib, v3d->bgpic->tex);
							v3d->bgpic->rect= 0;
						}
						if(v3d->localvd) {
							v3d->localvd->camera= newlibadr(sc->id.lib, v3d->localvd->camera);
						}
					}
					else if(v3d->spacetype==SPACE_IPO) {
						si= (SpaceIpo *)v3d;
						si->editipo= 0;
						si->from= 0;
						si->ipokey.first= si->ipokey.last= 0;
						si->ipo= newlibadr(sc->id.lib, si->ipo);
					}
					else if(v3d->spacetype==SPACE_BUTS) {
						buts= (SpaceButs *)v3d;
						buts->rect= 0;
						buts->lockpoin= 0;
						if(main->versionfile<132) set_rects_butspace(buts);
					}
					else if(v3d->spacetype==SPACE_FILE) {
						sfile= (SpaceFile *)v3d;
						
						sfile->filelist= 0;
						sfile->libfiledata= 0;
						sfile->returnfunc= 0;
					}
					else if(v3d->spacetype==SPACE_IMASEL) {
						check_imasel_copy((SpaceImaSel *)v3d);
					}
					else if(v3d->spacetype==SPACE_PAINT) {
					}
					else if(v3d->spacetype==SPACE_IMAGE) {
						SpaceImage *sima= (SpaceImage *)v3d;
						
						sima->image= newlibadr_us(sc->id.lib, sima->image);
					}
					else if(v3d->spacetype==SPACE_TEXT) {
						SpaceText *st= (SpaceText *)v3d;
				
						st->text= newlibadr(sc->id.lib, st->text);
						
						st->py_draw= NULL;
						st->py_event= NULL;
						st->py_button= NULL;
					}
					else if(v3d->spacetype==SPACE_OOPS) {
						SpaceOops *so= (SpaceOops *)v3d;
						
						/* patch als deze in oude files zit */
						if(so->v2d.cur.xmin==so->v2d.cur.xmax) {
							init_v2d_oops(&so->v2d);		
						}
						oops= so->oops.first;
						while(oops) {
							oops->id= newlibadr(0, oops->id);
							oops= oops->next;
						}
						so->lockpoin= 0;
					}
					else if(v3d->spacetype==SPACE_SOUND) {
						SpaceSound *ssound= (SpaceSound *)v3d;
						
						ssound->sound= newlibadr_us(sc->id.lib, ssound->sound);
					}
					v3d= v3d->next;
				}
				sa= sa->next;
			}
			sc->id.flag -= LIB_NEEDLINK;
		}
		sc= sc->id.next;
	}
}

void direct_link_screen(bScreen *sc)
{
	ScrArea *sa;
	ScrVert *sv;
	ScrEdge *se;
	View3D *v3d;
	SpaceOops *soops;
	Oops *oops;

	link_list( &(sc->vertbase) );
	link_list( &(sc->edgebase) );
	link_list( &(sc->areabase) );
	sc->winakt= 0;

	/* edges */
	se= sc->edgebase.first;
	while(se) {
		se->v1= newadr(se->v1);
		se->v2= newadr(se->v2);
		if( (long)se->v1 > (long)se->v2) {
			sv= se->v1;
			se->v1= se->v2;
			se->v2= sv;
		}
		
		if(se->v1==NULL) {
			printf("error reading screen... file corrupt\n");
			se->v1= se->v2;
		}
		se= se->next;
	}

	/* areas */
	sa= sc->areabase.first;
	while(sa) {
	
		link_list( &(sa->spacedata) );

		v3d= sa->spacedata.first;
		while(v3d) {
			if(v3d->spacetype==SPACE_VIEW3D) {
				
				v3d->bgpic= newadr(v3d->bgpic);
				v3d->localvd= newadr(v3d->localvd);
			}
			else if(v3d->spacetype==SPACE_OOPS) {
				soops= (SpaceOops *)v3d;
				link_list( &(soops->oops) );
				oops= soops->oops.first;
				while(oops) {
					oops->link.first= oops->link.last= 0;
					oops= oops->next;
				}
			}
			v3d= v3d->next;
		}
		
		sa->v1= newadr(sa->v1);
		sa->v2= newadr(sa->v2);
		sa->v3= newadr(sa->v3);
		sa->v4= newadr(sa->v4);
		
		sa->win= sa->headwin= 0;
		sa->cursor= CURSOR_STD;
		sa->headqueue= sa->hq= sa->winqueue= sa->wq= NULL;

		set_func_space(sa);	/* space.c */
		
		sa->uiblocks.first= sa->uiblocks.last= NULL;
		
		sa= sa->next;
	}

	/* vertices en offset */
	test_scale_screen(sc);
} 

/* ********** READ LIBRARY *************** */


void direct_link_library(Library *lib)
{
	Main *newmain;
	
	/* nieuwe main */
	newmain= callocN(sizeof(Main), "directlink");
	addtail(&G.mainbase, newmain);
	newmain->curlib= lib;

}

void lib_link_library(Main *main)
{
	Library *lib;
	
	lib= main->library.first;
	while(lib) {
		lib->id.us= 1;
		lib= lib->id.next;
	}

}

/* ************** READ SOUND ******************* */

void direct_link_sound(bSound *sound)
{
	sound->data= NULL;
	sound->alindex = -1;

	sound->packedfile = newadr(sound->packedfile);
	if (sound->packedfile) {
		sound->packedfile->data = newadr(sound->packedfile->data);
	}
}

void lib_link_sound(Main *main)
{
	bSound *sound;
	
	sound= main->sound.first;
	while(sound) {
		if(sound->id.flag & LIB_NEEDLINK) {
			sound->id.flag -= LIB_NEEDLINK;
		}
		sound= sound->id.next;
	}
}

/* ***************** READ GROUP *************** */

void direct_link_group(Group *group)
{
	GroupObject *go;
	ObjectKey *ok;
	
	link_list(&group->gobject);
	link_list(&group->gkey);
	group->active= newadr(group->active);
	
	go= group->gobject.first;
	while(go) {
		link_list(&go->okey);
		ok= go->okey.first;
		while(ok) {
			ok->gkey= newadr(ok->gkey);
			ok= ok->next;
		}
		go= go->next;
	}
	
}

void lib_link_group(Main *main)
{
	Group *group= main->group.first;
	GroupObject *go;
	ObjectKey *ok;
	
	while(group) {
		if(group->id.flag & LIB_NEEDLINK) {
			group->id.flag -= LIB_NEEDLINK;
	
			go= group->gobject.first;
			while(go) {
				go->ob= newlibadr(group->id.lib, go->ob);
				ok= go->okey.first;
				while(ok) {
					ok->parent= newlibadr(group->id.lib, ok->parent);
					ok->track= newlibadr(group->id.lib, ok->track);	
					ok->ipo= newlibadr_us(group->id.lib, ok->ipo);	
					ok= ok->next;
				}
				go= go->next;
			}
		}
		group= group->id.next;
	}
}

/* ************** ALG & MAIN ******************** */

int read_libblock(Main *main, BHead *bhead, int flag)
{
	/* deze routine leest libblock en direkte data. Met linkfunkties
	 * alles aan elkaar hangen.
	 */
	
	ID *id;
	ListBase *lb;
	int skipdata;
	char *fd;

	if(bhead->code==ID_ID) {
		id= (ID *)(bhead + 1);
		lb= wich_libbase(main, GS(id->name));
		/* printf("idid %s oldp %x\n", id->name, bhead->old); */
	}
	else {
		lb= wich_libbase(main, bhead->code);
	}

	if(lb==0) {
		
		/* PRINT2(s, d, ((char *)&(bhead->code))+2, bhead->len); */
		
		/* temporal: read the libstruct to fetch the old sector and life of the prototype */
		/* see link_sector_or_life(); */
		read_libstruct(bhead);
		
		return bhead->len+sizeof(BHead);
	}
	
	fd= (char *)bhead;
	
	/* libblock inlezen */
	id= read_libstruct(bhead);
	addtail(lb, id);
	/* eerste acht bits wissen */
	id->flag= (id->flag & 0xFF00) | flag | LIB_NEEDLINK;
	id->lib= main->curlib;
	if(id->flag & LIB_FAKEUSER) id->us= 1;
	else id->us= 0;
	
	/* deze mag niet door de direct_link molen: is alleen het ID deel */
	if(bhead->code==ID_ID) {
		skipdata= bhead->len+sizeof(BHead);
		return skipdata;	
	}

	skipdata= bhead->len+sizeof(BHead);
	fd+= bhead->len+sizeof(BHead);
	bhead= (BHead *)fd;

	
	/* alle data inlezen */
	while(bhead->code==DATA) {
		
		read_struct(bhead);
		
		skipdata+= bhead->len+sizeof(BHead);
		fd+= bhead->len+sizeof(BHead);
		bhead= (BHead *)fd;		
	}

	/* pointers directe data goedzetten */
	switch( GS(id->name) ) {
		case ID_SCR:
			direct_link_screen((bScreen *)id);
			break;
		case ID_SCE:
			direct_link_scene((Scene *)id);
			break;
		case ID_OB:
			direct_link_object((Object *)id);
			break;
		case ID_ME:
			direct_link_mesh((Mesh *)id);
			break;
		case ID_CU:
			direct_link_curve((Curve *)id);
			break;
		case ID_MB:
			direct_link_mball((MetaBall *)id);
			break;
		case ID_MA:
			direct_link_material((Material *)id);
			break;
		case ID_TE:
			direct_link_texture((Tex *)id);
			break;
		case ID_IM:
			direct_link_image((Image *)id);
			break;
		case ID_LA:
			direct_link_lamp((Lamp *)id);
			break;
		case ID_VF:
			direct_link_vfont((VFont *)id);
			break;
		case ID_TXT:
			direct_link_text((Text *)id);
			break;
		case ID_IP:
			direct_link_ipo((Ipo *)id);
			break;
		case ID_KE:
			direct_link_key((Key *)id);
			break;
		case ID_LT:
			direct_link_latt((Lattice *)id);
			break;
		case ID_IK:
			direct_link_ika((Ika *)id);
			break;
		case ID_WO:
			direct_link_world((World *)id);
			break;
		case ID_LI:
			direct_link_library((Library *)id);
			break;
		case ID_CA:
			direct_link_camera((Camera *)id);
			break;
		case ID_SO:
			direct_link_sound((bSound *)id);
			break;
		case ID_GR:
			direct_link_group((Group *)id);
			break;
	}
	
	/* vrijgeven, herinitialiseren */
	add_data_adr(0, (void *)1);	
	
	return skipdata;
}

void link_global(FileGlobal *fg)
{

	R.displaymode= fg->displaymode;
	R.winpos= fg->winpos;
	G.fileflags = fg->fileflags;
	
	G.curscreen= newlibadr(0, fg->curscreen);
	if(G.curscreen==0) G.curscreen= G.main->screen.first;
	G.scene= G.curscreen->scene;
	G.obedit= 0;
	G.buts=0;
	G.v2d= 0;
	G.vd= 0;
	G.soops= 0;
	G.sima= 0;
	G.sipo= 0;
	
}

void vcol_to_fcol(Mesh *me)
{
	MFace *mface;
	uint *mcol, *mcoln, *mcolmain;
	int a;

	if(me->totface==0 || me->mcol==0) return;
	
	mcoln= mcolmain= mallocN(4*sizeof(int)*me->totface, "mcoln");
	mcol = (uint *)me->mcol;
	mface= me->mface;
	for(a=me->totface; a>0; a--, mface++) {
		mcoln[0]= mcol[mface->v1];
		mcoln[1]= mcol[mface->v2];
		mcoln[2]= mcol[mface->v3];
		mcoln[3]= mcol[mface->v4];
		mcoln+= 4;
	}
	freeN(me->mcol);
	me->mcol= (MCol *)mcolmain;
}

void do_versions(Main *main)
{
	/* PAS OP: pointers van libdata zijn nog niet omgezet */
	Tex *tex;
	Object *ob;
	Material *ma;
	Scene *sce;
	int a, b;

	if(main->versionfile == 100) {

		/* tex->extend en tex->imageflag veranderd: */
		
		tex= main->tex.first;
		while(tex) {
			if(tex->id.flag & LIB_NEEDLINK) {
				
				if(tex->extend==0) {
					if(tex->xrepeat || tex->yrepeat) tex->extend= TEX_REPEAT;
					else {
						tex->extend= TEX_EXTEND;
						tex->xrepeat= tex->yrepeat= 1;
					}
				}
	
				if(tex->imaflag & TEX_ANIM5) {
					tex->imaflag |= TEX_MORKPATCH;
					tex->imaflag |= TEX_ANTIALI;
				}
			}
			tex= tex->id.next;
		}
	}
	if(main->versionfile <= 101) {
		/* frame mapping */
		sce= main->scene.first;
		while(sce) {
			sce->r.framapto= 100;
			sce->r.images= 100;
			sce->r.framelen= 1.0;
			sce= sce->id.next;
		}
	}
	if(main->versionfile <= 102) {
		/* init halo's op 1.0 */
		ma= main->mat.first;
		while(ma) {
			ma->add= 1.0;
			ma= ma->id.next;
		}
	}
	if(main->versionfile <= 103) {
		/* nieuwe variabele in object: colbits */
		ob= main->object.first;
		while(ob) {
			ob->colbits= 0;
			if(ob->totcol) {
				for(a=0; a<ob->totcol; a++) {
					if(ob->mat[a]) ob->colbits |= (1<<a);
				}
			}
			ob= ob->id.next;
		}
	}
	if(main->versionfile <= 104) {
		/* de timeoffs zit op betere plek */
		ob= main->object.first;
		while(ob) {
			if(ob->transflag & 1) {
				ob->transflag -= 1;
				ob->ipoflag |= OB_OFFS_OB;
			}
			ob= ob->id.next;
		}
	}
	if(main->versionfile <= 105) {
		ob= main->object.first;
		while(ob) {
			ob->dupon= 1; ob->dupoff= 0;
			ob->dupsta= 1; ob->dupend= 100;
			ob= ob->id.next;
		}
	}
	if(main->versionfile <= 106) {
		Mesh *me;
		/* mcol is veranderd */

		me= main->mesh.first;
		while(me) {
			if(me->mcol) vcol_to_fcol(me);
			me= me->id.next;
		}
		
	}
	if(main->versionfile <= 107) {
		sce= main->scene.first;
		while(sce) {
			sce->r.mode |= R_GAMMA;
			sce= sce->id.next;
		}		
		ob= main->object.first;
		while(ob) {
			ob->ipoflag |= OB_OFFS_PARENT;
			if(ob->dt==0) ob->dt= 3;
			ob= ob->id.next;
		}
		
	}
	if(main->versionfile <= 109) {
		bScreen *sc;
		ScrArea *sa;
		View3D *vd;
		
		/* nieuwe variabele: gridlines */
		
		sc= main->screen.first;
		while(sc) {
			
			sa= sc->areabase.first;
			while(sa) {
	
				vd= sa->spacedata.first;
				while(vd) {
					if(vd->spacetype==SPACE_VIEW3D) {
						if(vd->gridlines==0) vd->gridlines= 20;
					}
					vd= vd->next;
				}
				sa= sa->next;
			}
			sc= sc->id.next;
		}
		
		}
	if(main->versionfile <= 112) {
		Mesh *me= main->mesh.first;
		while(me) {
			me->cubemapsize= 1.0;
			me= me->id.next;
		}
	}
	if(main->versionfile <= 113) {
		ma= main->mat.first;
		while(ma) {
			if(ma->flaresize==0.0) ma->flaresize= 1.0;
			ma->subsize= 1.0;
			ma->flareboost= 1.0;
			ma= ma->id.next;
		}
	}
	if(main->versionfile <= 114) {
		Mesh *me= main->mesh.first;
		MFace *mface;
		int a;
		
		/* edge drawflags veranderd */
		
		while(me) {
			a= me->totface;
			mface= me->mface;
			while(a--) {
				if(mface->edcode & 16) {
					mface->edcode -= 16;
					mface->edcode |= ME_V3V1;
				}
				mface++;
			}
			me= me->id.next;
		}
	}

	/* eentje overgeslagen voor bug in freeware versie */
	
	if(main->versionfile <= 121) {
		/* O2 versie gemaakt. */
	}
	if(main->versionfile <= 122) {
		/* dithering gaat soms af (backbuf, pas sinds 121) */
		/* relatieve paden hersteld */
		/* sequences: endframe van seq wordt op betere plek geprint */
	}
	if(main->versionfile <= 123) {
		/* nog een paar O2 foutjes: keylines in ipo window */
		/* vertices halo object (O2) nu ook goed */
		/* zoomwin: ook op O2 */
		/* bug eruit: schaduw render in ortho */
	}
	if(main->versionfile <= 124) {
		/* inventor lezer */
		/* key kleur 24 bits beveiligd */
		/* schrijf plaatje: je kun niet naderhand 24bits naar 32 omzetten */
	}
	if(main->versionfile <= 125) {
		/* bug vanwege compileer fout (makefile/.h dependency)*/
	}
	if(main->versionfile <= 126) {
		/* overdraw text beter (clever numbuts) */
		/* bug uit inventor lezer: node ambientColor werd niet herkend */
		/* bugje uit toolbox: clear loc= alt-g */
	}

	if(main->versionfile <= 130) {
		/* openGL en GLUT */

		bScreen *sc;
		ScrArea *sa;
		
		sc= main->screen.first;
		while(sc) {
			
			sa= sc->areabase.first;
			while(sa) {
				if(sa->cursor==0) sa->cursor= CURSOR_STD;
				sa= sa->next;
			}
			sc= sc->id.next;
		}
	}
	if(main->versionfile <=131) {
		/* jpeq quality button */
		/* anim5 and blacksmith demo */
		/* foutje uit transp zbuf: te vroege afbreek */
		/* geen paarse code meer als imap onvindbaar is meer */
		/* locx werd niet geprint: string overflow! */
		/* unieke namen: werkte niet */
		/* toolbox menu: ook alt en ctrl keys */
	}
	if(main->versionfile <=132) {
		/* strings in Userdef: eroverheen! */
		/* betere overdraw implementatie (numbuts) */
		/* snapmenu redraw */
		/* warp met 1 vertex */
	}
	if(main->versionfile <=133) {
		/* bug uit 'make edge face' (array overflow */
		/* volledig X getekende menu's */
		/* storage.c terug */
	}
	if(main->versionfile <=134) {
		/* Play (flipbook) restored */
		/* Timecursor restored */
		/* Debug option -d; prints a lot of info in console */
		/* Text Object. Accentcodes fixed: ALT+BACKSPACE */
		/* Cursor was sometimes wrong after reading files */
		/* Texspace draw error: dashed lines */
		/* Draw Schematic View now with icons in Objects */
		/* Ortho camera: zbuffer improved. Near/far still not OK */
		/* Text Object. Character pound= alt-l */
		/* In editmode and 'set', draw error fixed. */
		/* Scanline display during rendering had dropouts */
		/* Sometimes - after render - frontbuffer drawing wasn't disabled */
		/* Sometimes the render window got black and Blender 'hanged'. */
		/* Better 'active window' implementation. */
		/* Automatic name was too critical, more intuitive now */
		Tex *tex;

		tex= main->tex.first;
		while(tex) {
			if(tex->rfac==0.0 && tex->gfac==0.0 && tex->bfac==0.0) {
				tex->rfac= 1.0;
				tex->gfac= 1.0;
				tex->bfac= 1.0;
				tex->filtersize= 1.0;
			}
			tex= tex->id.next;
		}

	}
	if(main->versionfile <=135) {
		/* 'Windows' key resistant */
		/* Preview-render: RGB flip (material, lamp, world) */
		/* Fileselect draw error: 2nd time no redraw! */
		/* Names error: names were not unique automatically */
		/* Metaball display error: because of previous */
		/* CTRL and ALT and SHIFT keys sometimes were locked */
	}
	if(main->versionfile <=136) {
		/* Files incompatibility Colorband PC-SGI solved */
		/* RightMouse selecting was blocked after border-select */
		/* Border select: print size */
		/* Inventor: reads some 2.0 syntaxes too. Under development */
		/* Shift/Ctrl/Alt release events got lost while moving view */
		/* Particles draw (size) error fixed */
		/* Display type 'DispView' works */
		/* Metaballs convert to Mesh, normals error fixed. */
	}
	if(main->versionfile <=137) {
		
	}
	if(main->versionfile <=138) {
		/* fixed: z buffer draw and Mesh with no materials: coredump! */
		/* bug removed from calculation 3D Bevel Objects */
		/* view translation in perspective fixed */
		/* Drawing with ortho camera fixed */
		/* timing error FreeBSD version fixed */
		/* Mesa 3.0 included in static version */
		/* New: LeftMouse+RightMouse allowed at numerber-button to type in values */
		/* Vertex paint bug fixed */
		/* New: ALT+(1, 2, 3...) for layers 11, 12, 13... */
	}
	if(main->versionfile <=140) {
		Tex *tex;
		/* r-g-b-fac in texure */
		
		tex= main->tex.first;
		while(tex) {
			if(tex->rfac==0.0 && tex->gfac==0.0 && tex->bfac==0.0) {
				tex->rfac= 1.0;
				tex->gfac= 1.0;
				tex->bfac= 1.0;
				tex->filtersize= 1.0;
			}
			tex= tex->id.next;
		}
	}
	if(main->versionfile <=153) {

		sce= main->scene.first;
		while(sce) {
			if(sce->r.blurfac==0.0) sce->r.blurfac= 1.0;
			sce= sce->id.next;
		}
	}
	if(main->versionfile <=163) {

		sce= main->scene.first;
		while(sce) {
			if(sce->r.frs_sec==0) sce->r.frs_sec= 25;
			sce= sce->id.next;
		}
	}
	if(main->versionfile <=164) {
		Mesh *me= main->mesh.first;
		
		while(me) {
			me->smoothresh= 30;
			me= me->id.next;
		}
	}

	if(main->versionfile <=165) {
		Mesh *me= main->mesh.first;
		TFace *tface;
		Ika *ika= main->ika.first;
		Deform *def;
		int nr;
		char *cp;
		
		while(ika) {
			ika->xyconstraint= .5;
			
			def= ika->def;
			nr= ika->totdef;
			while(nr--) {
				if(def->fac==0.0) def->fac= 1.0;
				def++;
			}
			ika= ika->id.next;
		}
		
		while(me) {
			if(me->tface) {
				nr= me->totface;
				tface= me->tface;
				while(nr--) {
					cp= (char *)&tface->col[0];
					if(cp[1]>126) cp[1]= 255; else cp[1]*=2;
					if(cp[2]>126) cp[2]= 255; else cp[2]*=2;
					if(cp[3]>126) cp[3]= 255; else cp[3]*=2;
					cp= (char *)&tface->col[1];
					if(cp[1]>126) cp[1]= 255; else cp[1]*=2;
					if(cp[2]>126) cp[2]= 255; else cp[2]*=2;
					if(cp[3]>126) cp[3]= 255; else cp[3]*=2;
					cp= (char *)&tface->col[2];
					if(cp[1]>126) cp[1]= 255; else cp[1]*=2;
					if(cp[2]>126) cp[2]= 255; else cp[2]*=2;
					if(cp[3]>126) cp[3]= 255; else cp[3]*=2;
					cp= (char *)&tface->col[3];
					if(cp[1]>126) cp[1]= 255; else cp[1]*=2;
					if(cp[2]>126) cp[2]= 255; else cp[2]*=2;
					if(cp[3]>126) cp[3]= 255; else cp[3]*=2;
					
					tface++;
				}
			}
			me= me->id.next;
		}
	}

	if(main->versionfile <=169) {
		Mesh *me= main->mesh.first;
		
		while(me) {
			if(me->subdiv==0) me->subdiv= 4;
			me= me->id.next;
		}
	}
	
	if(main->versionfile <= 169) {
		bScreen *sc= main->screen.first;
		ScrArea *sa;
		SpaceIpo *sipo;
		while(sc) {
			
			sa= sc->areabase.first;
			while(sa) {
	
				sipo= sa->spacedata.first;
				while(sipo) {
					if(sipo->spacetype==SPACE_IPO) {
						sipo->v2d.max[0]= 15000.0;
					}
					sipo= sipo->next;
				}
				sa= sa->next;
			}
			sc= sc->id.next;
		}
	}

	if(main->versionfile <=170) {
		Object *ob= main->object.first;
		PartEff *paf;
	
		while(ob) {
			paf = give_parteff(ob);
			if( paf ) {
				if(paf->staticstep==0) paf->staticstep= 5;
			}
			ob= ob->id.next;
		}
	}

	if(main->versionfile <=171) {
		bScreen *sc= main->screen.first;
		ScrArea *sa;
		SpaceText *st;
		
		while(sc) {
			
			sa= sc->areabase.first;
			while(sa) {
	
				st= sa->spacedata.first;
				while(st) {
					if(st->spacetype==SPACE_TEXT) {
						if(st->font_id>1) {
							st->font_id= 0;
							st->lheight= 13;
						}
					}
					st= st->next;
				}
				sa= sa->next;
			}
			sc= sc->id.next;
		}
	}
	
	if(main->versionfile <=173) {
		Mesh *me= main->mesh.first;
		
		while(me) {
			if(me->tface) {
				TFace *tface= me->tface;
				
				for(a=0; a<me->totface; a++, tface++) {
					for(b=0; b<4; b++) {
						tface->uv[b][0]/= 32767.0;
						tface->uv[b][1]/= 32767.0;
					}
				}
			}
			me= me->id.next;
		}
	}

	if(main->versionfile <=191) {
		SpaceButs *buts;
		ScrArea *sa;
		bScreen *sc= G.main->screen.first;
		Object *ob= G.main->object.first;
		ma = G.main->mat.first;
		
		/* let faces have default add factor of 0.0 */
		while(ma) {
		  if (!(ma->mode & MA_HALO)) ma->add = 0.0;
		  ma = ma->id.next;
		}

		while(ob) {
			ob->mass= 1.0;
			ob->damping= 0.1;
			ob->quat[1]= 1.0;		
			ob= ob->id.next;
		}

		while(sc) {
			sa= sc->areabase.first;
			while(sa) {
				buts= sa->spacedata.first;
				while(buts) {
					if(buts->spacetype==SPACE_BUTS) {
						buts->scaflag= BUTS_SENS_LINK|BUTS_SENS_ACT|BUTS_CONT_ACT|BUTS_ACT_ACT|BUTS_ACT_LINK;
					}
					buts= buts->next;
				}
				sa= sa->next;
			}
			sc= sc->id.next;
		}
		
		strcpy(U.plugtexdir, U.textudir);
		strcpy(U.sounddir, "/");
	}
	
	if(main->versionfile <=193) {
		Object *ob= G.main->object.first;
		
		while(ob) {
			ob->inertia= 1.0;
			ob->rdamping= 0.1;
			ob= ob->id.next;
		}
	}

	if(main->versionfile <=196) {
		Mesh *me= G.main->mesh.first;
		
		while(me) {
			if(me->tface) {
				TFace *tface= me->tface;
				
				for(a=0; a<me->totface; a++, tface++) {
					for(b=0; b<4; b++) {
						tface->mode |= TF_DYNAMIC;
						tface->mode &= ~TF_INVISIBLE;
					}
				}
			}
			me= me->id.next;
		}
	}
		
	if(main->versionfile <=200) {
		Object *ob= G.main->object.first;
		
		while(ob) {
			ob->scaflag= ob->gameflag & (64+128+256+512+1024+2048);
			ob->gameflag &= ~(128+256+512+1024+2048);	/* 64 is do_fh */
			ob= ob->id.next;
		}
	}

	if(main->versionfile <=201) {
		Object *ob= G.main->object.first;
		bProperty *prop;
		bActuator *act;
		bIpoActuator *ia;
		bEditObjectActuator *eoa;
		bAddObjectActuator *aoa;
		
		/* add-object and end-object are joined to edit-object actuator */
		
		while(ob) {
			act= ob->actuators.first;
			while(act) {
				if(act->type==ACT_IPO) {
					ia= act->data;
					prop= get_property(ob, ia->name);
					if(prop) {
						ia->type= ACT_IPO_FROM_PROP;
					}
				}
				else if(act->type==ACT_ADD_OBJECT) {
					aoa= act->data;
					eoa= callocN(sizeof(bEditObjectActuator), "edit ob act");
					eoa->type= ACT_EDOB_ADD_OBJECT;
					eoa->ob= aoa->ob;
					eoa->time= aoa->time;
					freeN(aoa);
					act->data= eoa;
					act->type= act->otype= ACT_EDIT_OBJECT;
				}
				else if(act->type==ACT_END_OBJECT) {
					eoa= callocN(sizeof(bEditObjectActuator), "edit ob act");
					eoa->type= ACT_EDOB_END_OBJECT;
					act->data= eoa;
					act->type= act->otype= ACT_EDIT_OBJECT;
				}
				act= act->next;
			}
			ob= ob->id.next;
		}
	}

	if(main->versionfile <=202) {
		Object *ob= G.main->object.first;
		bActuator *act;
		bObjectActuator *oa;
		
		/* add-object and end-object are joined to edit-object actuator */
		
		while(ob) {
			act= ob->actuators.first;
			while(act) {
				if(act->type==ACT_OBJECT) {
					oa= act->data;
					oa->flag &= ~(ACT_TORQUE_LOCAL|ACT_DROT_LOCAL);		/* this actuator didn't do local/glob rot before */
				}
				act= act->next;
			}
			ob= ob->id.next;
		}
	}

	/* onder in blender.c de nummers wijzigen! */

}

void lib_link_all(Main *main)
{


	lib_link_screen(main);
	lib_link_scene(main);
	lib_link_object(main);
	lib_link_curve(main);
	lib_link_mball(main);
	lib_link_material(main);
	lib_link_texture(main);
	lib_link_image(main);
	lib_link_ipo(main);
	lib_link_key(main);
	lib_link_world(main);
	lib_link_lamp(main);
	lib_link_latt(main);
	lib_link_ika(main);
	lib_link_camera(main);
	lib_link_sound(main);
	lib_link_group(main);

	// No lib_link_vfont ???
	
	lib_link_mesh(main);	/* als laatste: tpage images met users op nul */

	lib_link_library(main);	/* alleen users goedzetten */
}


int read_file_dna(char *filedata, int filelen)
{
	BHead *bhead;
	int afbreek=0;
	char *fd;
	
	freestructDNA(&old_sdna);

	fd= filedata;
	while(filelen>0 && afbreek!=3) {
		
		bhead= (BHead *)fd;

		if(bhead->code==ENDB) afbreek+= 2;
		else if(bhead->code==DNA1) {

			old_sdna.data= mallocN(bhead->len, "sdna");
			memcpy(old_sdna.data, fd+sizeof(BHead), bhead->len);
			afbreek+= 1;
		}
		
		fd+= bhead->len+sizeof(BHead);
		filelen -= bhead->len+sizeof(BHead);
	}

	if(afbreek==3) {
		init_structDNA(&old_sdna);
		set_compareflags_structDNA();
		return 1;
	}
	else {
		if(old_sdna.data) freeN(old_sdna.data);
		old_sdna.data= 0;
		error("File not complete");
		return 0;
	}
}

void convert_dos_newlines(char *filedata, int filelen)
{
	char *out= filedata, *s= filedata;
	char c;
	
	while (filelen--) {
		c= *s++;
		
		if (!(c==13 && *s=='\n'))
			*out++= c;
	}
}

void reconstruct_bheads(char **filedata, int *filelen)
{
#ifdef WIN32
	DWORDLONG old;
#else
	long long old;
#endif
	BHead8 *bhead8;
	BHead4 *bhead4;
	int totbh=0, len, filelenn;
	char *fd, *fdn, *filedatan;
	
	if(pointerlen==4) {
		/* count bheads */
		len= *filelen;
		fd= *filedata;
		
		while(len>0) {
			
			bhead4= (BHead4 *)fd;
			totbh++;
			
			if(bhead4->code==ENDB) break;
			
			fd+= bhead4->len+sizeof(BHead4);
			len -= bhead4->len+sizeof(BHead4);
		}
		
		filelenn= *filelen + 4*totbh;
	}
	else {
		/* count bheads */
		len= *filelen;
		fd= *filedata;
		
		while(len>0) {
			
			bhead8= (BHead8 *)fd;
			totbh++;
			
			if(bhead8->code==ENDB) break;
			
			fd+= bhead8->len+sizeof(BHead8);
			len -= bhead8->len+sizeof(BHead8);
		}
		/* + 4: ENDB */
		filelenn= *filelen - 4*totbh + 4;
	}
	
	fdn= filedatan= mallocN(filelenn, "filedatan");
	fd= *filedata;
	
	if(pointerlen==4) {
	
		len= *filelen;
		while(len>0) {
		
			bhead4= (BHead4 *)fd;
			bhead8= (BHead8 *)fdn;
		
			bhead8->code= bhead4->code;
			bhead8->len= bhead4->len;

			if(bhead8->code==ENDB) break;
			
			bhead8->old= bhead4->old;
			bhead8->SDNAnr= bhead4->SDNAnr;
			bhead8->nr= bhead4->nr;
			
			if(bhead4->len) memcpy(bhead8+1, bhead4+1, bhead4->len);
			
			fd+= bhead4->len+sizeof(BHead4);
			fdn+= bhead8->len+sizeof(BHead8);
			len -= bhead4->len+sizeof(BHead4);
		}
	}
	else {
	
		len= *filelen;
		while(len>0) {
		
			bhead4= (BHead4 *)fdn;
			bhead8= (BHead8 *)fd;
		
			bhead4->code= bhead8->code;
			bhead4->len= bhead8->len;

			if(bhead4->code==ENDB) break;
			
			if(switch_endian) {
				SWITCH_LONGINT(bhead8->old);
			}
			
			/* this patch is to avoid a long long being read from not-eight aligned positions
			   is necessary on SGI with -n32 compiling */
			memcpy(&old, &bhead8->old, 8);
			bhead4->old= old>>3;
			
			/* bhead4->old= bhead8->old>>3; */
			
			bhead4->SDNAnr= bhead8->SDNAnr;
			bhead4->nr= bhead8->nr;
			if(bhead8->len) memcpy(bhead4+1, bhead8+1, bhead8->len);
			
			fdn+= bhead4->len+sizeof(BHead4);
			fd+= bhead8->len+sizeof(BHead8);
			len -= bhead8->len+sizeof(BHead8);
		}
	}

	freeN(*filedata);
	*filedata= filedatan;
	*filelen= filelenn;
}

void switch_endian_bhead4(char *filedata, int filelen)
{
	BHead4 *bhead;
	char *fd, str[5];
	
	str[4]= 0;
	
	fd= filedata;
	while(filelen>0) {
		
		bhead= (BHead4 *)fd;

		/* de ID_.. codes */
		if((bhead->code & 0xFFFF)==0) bhead->code >>=16;

		if(bhead->code==ENDB) break;

		SWITCH_INT(bhead->len);
		SWITCH_INT(bhead->SDNAnr);
		SWITCH_INT(bhead->nr);

		fd+= bhead->len+sizeof(BHead4);
		filelen -= bhead->len+sizeof(BHead4);
	}
}

void switch_endian_bhead8(char *filedata, int filelen)
{
	BHead8 *bhead;
	char *fd, str[5];
	
	str[4]= 0;
	
	fd= filedata;
	while(filelen>0) {
		
		bhead= (BHead8 *)fd;

		/* de ID_.. codes */
		if((bhead->code & 0xFFFF)==0) bhead->code >>=16;
		if(bhead->code==ENDB) break;

		SWITCH_INT(bhead->len);
		SWITCH_INT(bhead->SDNAnr);
		SWITCH_INT(bhead->nr);

		fd+= bhead->len+sizeof(BHead8);
		filelen -= bhead->len+sizeof(BHead8);
	}
}

char *openblenderfile(char *name, int *filelen)
{
	int len, file;	
	char *filedata, str[16];
	
	len= strlen(name);
	if(len<6) return 0;
	if( name[len-1]=='/' || name[len-1]=='\\') return 0;

	if( strstr(name, ".ble") || strstr(name, ".BLE")) {
		file= open(name, O_BINARY|O_RDONLY);
		if (file < 0) {
			errorstr("Can't find file", name, 0);
			return 0;
		}
		
		*filelen= filesize(file);

		read(file, str, 12);
		
		if(strncmp(str, "BLENDER", 7)!=0) {
			close(file);
			errorstr("Not a Blender file: ", name, 0);
			return 0;
		}
		
		/* long pointer test */
		if(str[7]=='_') pointerlen= 4;
		else pointerlen= 8;
		
		/* endian test */
		switch_endian= 0;
		
		if( str[8]=='V' && G.order==L_ENDIAN) {
			switch_endian= 1;
		}
		else if( str[8]=='v' && G.order==B_ENDIAN) {
			switch_endian= 1;
		}
		
		str[12]= 0;
		G.versionfile= atoi(str+9);
		
		/* hele file inlezen */
		(*filelen) -= 12;
		filedata= mallocN(*filelen + 12, "filedata");	/* + 12: anders errors bij Little endina files. waarom??? */
		read(file, filedata, *filelen);
		close(file);
		
		/* reconstruct_bheads doet ook de pointer endian switchen */
		if(switch_endian) {
			if(pointerlen==4)
				switch_endian_bhead4(filedata, *filelen);
			else 
				switch_endian_bhead8(filedata, *filelen);
		}

		if(pointerlen!=sizeof(void *)) reconstruct_bheads(&filedata, filelen);

		
		/* op zoek naar SDNA */
		if( read_file_dna(filedata, *filelen) ) {
			return filedata;
		}
		else {
			/* most probable case: a windows browser converted newline codes */
			convert_dos_newlines(filedata, *filelen);

			if( read_file_dna(filedata, *filelen) ) {
				return filedata;
			}
			else {
				freeN(filedata);
				return 0;
			}
		}
	}
	else {
		read_exotic(name);
	}
	
	return 0;
}

void read_file(char *dir)
{
	BHead *bhead;
	Main *main;
	Object *ob;
	Curve *cu;
	FileGlobal *fg;
	UserDef *user;
	VFont *vf;
	int ok, len, filelen, skipdata;
	char *filedata, *fd;
	
	waitcursor(1);

	filedata= openblenderfile(dir, &filelen);

	if (filedata) {
		G.save_over = TRUE;
		
		strcpy(G.sce, dir);
		strcpy(G.main->name, dir);	/* is gegarandeerd current file */
		
		/* er is maar 1 Main, dus alleen inhoud vrijgeven */
		
		freeAllRad();
		free_main(0, G.main);
		G.curscreen= 0;
		freefastshade();	/* anders oude lampgegevens */
		G.main->versionfile= G.versionfile;	/* kan per main verschillen */
		if(G.obedit) {
			freeNurblist(&editNurb);
			free_editmesh();
			free_editText();
		}
		G.f &= ~(G_VERTEXPAINT + G_FACESELECT);

		/* alle data inlezen: */
		fd= filedata;
		ok= 0;
		main= G.main;
		
		while(filelen>0) {
			
			bhead= (BHead *)fd;
			
			switch(bhead->code) {
			case GLOB:
				read_struct_expl(bhead, (void **)&fg);
				skipdata= bhead->len+sizeof(BHead);
				break;
			case DATA:
				skipdata= bhead->len+sizeof(BHead);
				break;
			case DNA1:
				skipdata= bhead->len+sizeof(BHead);
				break;
			case USER:
				read_struct_expl(bhead, (void **)&user);
				U= *user;
				freeN(user);
				skipdata= bhead->len+sizeof(BHead);
				break;
			case TEST:
				skipdata= bhead->len+sizeof(BHead);
				break;
			case REND:
				skipdata= bhead->len+sizeof(BHead);
				break;
			case ENDB:
				ok= 1;
				skipdata= 8;
				break;
			case ID_LI:
				skipdata= read_libblock(G.main, bhead, LIB_LOCAL);
				main= G.mainbase.last;
				break;
			case ID_ID:
				skipdata= read_libblock(main, bhead, LIB_READ+LIB_EXTERN);
				break;
			default:
				skipdata= read_libblock(G.main, bhead, LIB_LOCAL);
			}
			
			if(ok) break;

			fd+= skipdata;
			filelen-= skipdata;
		}

		freeN(filedata);

		do_versions(G.main);	/* voor read_libraries */
		read_libraries();
		
		/* LibData linken */
		
		lib_link_all(G.main);
		link_global(fg);	/* als laatste */


		/* losslingerende blokken vrijgeven */
		add_data_adr(0, 0);
		add_lib_adr(0, 0);
		add_glob_adr(0, 0);
        
		/* VECTORFONTS */
		vf= G.main->vfont.first;
		
		/* afvangen: .Bfont van ander systeem */
		if (vf) {
			len= strlen(vf->name);

			if (len > 5 && strcmp(vf->name+len-5, "Bfont")==0) {
				char tempname[FILE_MAXDIR + FILE_MAXFILE];
				
				// ignore this rule if font exists....
				// first expand the name to an abolute filename...
				
				strcpy(tempname, vf->name);
				convertstringcode(tempname);
				if (vf->packedfile == NULL && !fop_exists(tempname)) {
					sprintf(vf->name, "%s/.Bfont", gethome());
				}
			}
		}
		while(vf) {
			reload_vfont(vf);
			vf= vf->id.next;
		}
		
		/* weinig DispListen, wel text_to_curve */
		ob= G.main->object.first;
		while(ob) {
			if(ob->type==OB_FONT) {
				cu= ob->data;
				if(cu->nurb.first==0) text_to_curve(ob, 0);
			}
			else if(ob->type==OB_MESH) {
				makeDispList(ob);
				if(ob->effect.first) object_wave(ob);
			}

			ob= ob->id.next;
		}
		
		freeN(fg);
		if(G.background==0) {
			setscreen(G.curscreen);
			countall();
		}
		set_scene_bg(G.scene);	/* baseflags */
		
		reset_autosave();
		clear_obact_names();	/* voor add object */
		set_obact_names(OBACT);
	}
	
	if(G.background==0) waitcursor(0);

	/* do_pyscript may not be executed if there is no valid file, and    */
	/* we run in background. 'normal' mode handles the error differently */
	if ( ( (!G.background) || filedata ) 
		 && (G.f & G_SCENESCRIPT)) {
		extern char ext_load_str[256];
		
		if (ext_load_str[0]) force_draw_all();

		do_pyscript(G.scene, SCRIPT_ONLOAD);
	}	
}

void inst_file(char *name, char *data, int size) {
	char fname[MAXPATHLEN];
	FILE *fp;

	make_file_string(fname, gethome(), name);

	fp= fopen(fname, "wb");
	if (fp==NULL) {
		printf("Could not install %s\n", fname);
		printf("Check permissions on %s.\n", gethome());
		exit(1);
	}

	fwrite(data, size, 1, fp);
	
	fclose(fp);
}

int read_homefile()
{
	int file;
	char *str;
	char *home, tstr[FILE_MAXDIR+FILE_MAXFILE], scestr[FILE_MAXDIR];

	home = gethome();
	if (home) {
		
		/* a vectorfont can't be read from memory... so has to be saved in $HOME */
		
		make_file_string (tstr, home, ".Bfont");
		file= open(tstr, O_BINARY|O_RDONLY);
		if (file < 0) inst_file(".Bfont", datatoc_Bfont, datatoc_Bfont_size);
		else close(file);

		make_file_string (tstr, home, ".Bfs");		
		file= open(tstr, O_BINARY|O_RDONLY);
		if (file < 0) inst_file(".Bfs", datatoc_Bfs, datatoc_Bfs_size);
		else close(file);

		make_file_string (tstr, home, ".B.blend");
		file= open(tstr, O_BINARY|O_RDONLY);
		if (file < 0) {
			inst_file(".B.blend", datatoc_B_blend, datatoc_B_blend_size);
			installed_blend= 1;
			file= open(tstr, O_BINARY|O_RDONLY);
		}
		if(file >=0 ) {
			/* bestand gevonden */
			close(file);
			
			strcpy(scestr, G.sce);	/* even bewaren */
			read_file(tstr);
			strcpy(G.sce, scestr);
			
			G.save_over = FALSE;
			
			/* nog wat afhandelen? */
			disable_capslock(U.flag & NO_CAPSLOCK);
			
			/* holobutton */
			if(strcmp(G.scene->r.ftype, "*@&#")==0) G.special1= G_HOLO;
			
			if(U.tempdir[0]=='/' && U.tempdir[1]==0) {
				str= getenv("TEMP");
				if(str) strcpy(U.tempdir, str);
				else strcpy(U.tempdir, "/tmp/");
			}
			
			return 1;
		}
	}
	return 0;
}


void read_autosavefile()
{
	char tstr[FILE_MAXDIR], scestr[FILE_MAXDIR], tmp2[32];
	int save_over;
	

	strcpy(scestr, G.sce);	/* even bewaren */
	
	sprintf(tmp2, "%d.blend", abs(getpid()));
	make_file_string(tstr, U.tempdir, tmp2);

	save_over = G.save_over;
	read_file(tstr);
	G.save_over = save_over;
	strcpy(G.sce, scestr);
			
}


/* ************* APPEND LIBRARY ************** */

BHead *bheadlib;

BHead *find_bhead(void *old, char *filedata)
{
	BHead *bhead;
	int afbreek=0;
	char *fd;
	
	bheadlib= 0;
	if(old==0) return 0;
	
	fd= filedata;
	while(afbreek==0) {
		
		bhead= (BHead *)fd;

		if(bhead->code==ID_LI) {
			bheadlib= bhead;
		}
		if(bhead->code==ENDB) afbreek= 1;
		else if(bhead->old==old) return bhead;
		
		fd+= bhead->len+sizeof(BHead);
	}
	return 0;
}

ID *is_yet_read(Main *main, BHead *bhead)
{
	ListBase *lb;
	ID *idtest, *id;
	
	idtest= (ID *)(bhead +1);
	lb= wich_libbase(main, GS(idtest->name));
	if(lb) {
		id= lb->first;
		while(id) {
			if( strcmp(id->name, idtest->name)==0 ) return id;
			id= id->next;
		}
	}
	return 0;
}

void expand_doit(Main *main, char *filedata, void *old)
{
	BHead *bhead;
	Library *lib;
	ID *id;

	bhead= find_bhead(old, filedata);
	if(bhead) {

		/* andere library? */
		
		if(bhead->code==ID_ID) {
			if(bheadlib) {
				lib= (Library *)(bheadlib+1);
				main= find_main(lib->name);

				id= is_yet_read(main, bhead);
		
				if(id==0) {
					read_libblock(main, bhead, LIB_READ+LIB_INDIRECT);
					printf("expand: other lib %s\n", lib->name);
				}
				else {
					printf("expand: already linked: %s lib: %s\n", id->name, lib->name);
					
					/* if(id->lib==0) add_lib_adr(bhead->old, id); */
					/* bovenstaand is niet nodig! Kan toch niet misgaan? */
					add_lib_adr(bhead->old, id);
					
				}
			}
		}
		else {
			id= is_yet_read(main, bhead);
			if(id==0) {
				id= (ID *)(bhead+1);
				read_libblock(main, bhead, LIB_TESTIND);
			}
			else {
				/* printf("expand: al ingelezen %s\n", id->name); */
				add_lib_adr(bhead->old, id);
			}
		}
	}
}

void expand_key(Main *main, char *filedata, Key *key)
{
	expand_doit(main, filedata, key->ipo);
}


void expand_texture(Main *main, char *filedata, Tex *tex)
{
	expand_doit(main, filedata, tex->ima);
}

void expand_material(Main *main, char *filedata, Material *ma)
{
	int a;
	
	for(a=0; a<8; a++) {
		if(ma->mtex[a]) {
			expand_doit(main, filedata, ma->mtex[a]->tex);
			expand_doit(main, filedata, ma->mtex[a]->object);
		}
	}
	expand_doit(main, filedata, ma->ipo);
}

void expand_lamp(Main *main, char *filedata, Lamp *la)
{
	int a;
	
	for(a=0; a<8; a++) {
		if(la->mtex[a]) {
			expand_doit(main, filedata, la->mtex[a]->tex);
			expand_doit(main, filedata, la->mtex[a]->object);
		}
	}
	expand_doit(main, filedata, la->ipo);
}

void expand_lattice(Main *main, char *filedata, Lattice *lt)
{
	expand_doit(main, filedata, lt->ipo);
	expand_doit(main, filedata, lt->key);
}


void expand_world(Main *main, char *filedata, World *wrld)
{
	int a;
	
	for(a=0; a<8; a++) {
		if(wrld->mtex[a]) {
			expand_doit(main, filedata, wrld->mtex[a]->tex);
			expand_doit(main, filedata, wrld->mtex[a]->object);
		}
	}
	expand_doit(main, filedata, wrld->ipo);
}


void expand_mball(Main *main, char *filedata, MetaBall *mb)
{
	int a;
	
	for(a=0; a<mb->totcol; a++) {
		expand_doit(main, filedata, mb->mat[a]);
	}
}

void expand_curve(Main *main, char *filedata, Curve *cu)
{
	int a;
	
	for(a=0; a<cu->totcol; a++) {
		expand_doit(main, filedata, cu->mat[a]);
	}
	expand_doit(main, filedata, cu->vfont);
	expand_doit(main, filedata, cu->key);
	expand_doit(main, filedata, cu->ipo);
	expand_doit(main, filedata, cu->bevobj);
	expand_doit(main, filedata, cu->textoncurve);
}

void expand_mesh(Main *main, char *filedata, Mesh *me)
{
	int a;
	TFace *tface;
	
	for(a=0; a<me->totcol; a++) {
		expand_doit(main, filedata, me->mat[a]);
	}
	
	expand_doit(main, filedata, me->key);
	expand_doit(main, filedata, me->texcomesh);
	
	if(me->tface) {
		tface= me->tface;
		a= me->totface;
		while(a--) {
			if(tface->tpage) expand_doit(main, filedata, tface->tpage);
			tface++;
		}
	}
}

void expand_object(Main *main, char *filedata, Object *ob)
{
	int a;
	
	expand_doit(main, filedata, ob->data);
	expand_doit(main, filedata, ob->ipo);
	for(a=0; a<ob->totcol; a++) {
		expand_doit(main, filedata, ob->mat[a]);
	}
}

void expand_scene(Main *main, char *filedata, Scene *sce)
{
	Base *base;
	
	base= sce->base.first;
	while(base) {
		expand_doit(main, filedata, base->object);
		base= base->next;
	}
	expand_doit(main, filedata, sce->camera);
	expand_doit(main, filedata, sce->world);
	
}

void expand_camera(Main *main, char *filedata, Camera *ca)
{

	expand_doit(main, filedata, ca->ipo);
}

void expand_text(Main *main, char *filedata, Text *txt)
{
	
}

void expand_main(Main *main, char *filedata)
{
	ListBase *lbarray[30];
	ID *id;
	int a, doit= 1;
	
	if(filedata==0) return;
	
	while(doit) {
		doit= 0;
		
		a= set_listbasepointers(main, lbarray);
		while(a--) {
			id= lbarray[a]->first;

			while(id) {
				if(id->flag & LIB_TEST) {
					
					switch(GS(id->name)) {
						
					case ID_OB:
						expand_object(main, filedata, (Object *)id);
						break;
					case ID_ME:
						expand_mesh(main, filedata, (Mesh *)id);
						break;
					case ID_CU:
						expand_curve(main, filedata, (Curve *)id);
						break;
					case ID_MB:
						expand_mball(main, filedata, (MetaBall *)id);
						break;
					case ID_SCE:
						expand_scene(main, filedata, (Scene *)id);
						break;
					case ID_MA:
						expand_material(main, filedata, (Material *)id);
						break;
					case ID_TE:
						expand_texture(main, filedata, (Tex *)id);
						break;
					case ID_WO:
						expand_world(main, filedata, (World *)id);
						break;
					case ID_LT:
						expand_lattice(main, filedata, (Lattice *)id);
						break;
					case ID_LA:
						expand_lamp(main, filedata, (Lamp *)id);
						break;
					case ID_KE:
						expand_key(main, filedata, (Key *)id);
						break;
					case ID_CA:
						expand_camera(main, filedata, (Camera *)id);
						break;
					case ID_TXT:
						expand_text(main, filedata, (Text *)id);
						break;
					case ID_SO:
						break;
					}

					doit= 1;
					id->flag -= LIB_TEST;
					
				}
				id= id->next;
			}
		}
	}
}

void give_base_to_objects(Scene *sce, ListBase *lb)
{
	Object *ob;
	Base *base;
	
	/* alle objects die LIB_EXTERN en LIB_NEEDLINK zijn, een base geven */
	ob= lb->first;
	while(ob) {

		if(ob->id.us==0) {

			if(ob->id.flag & LIB_NEEDLINK) {
			
				ob->id.flag -= LIB_NEEDLINK;
				
				if( ob->id.flag & LIB_INDIRECT ) {
					
					base= callocN( sizeof(Base), "add_ext_base");
					addtail(&(sce->base), base);
					base->lay= ob->lay;
					base->object= ob;
					ob->id.us= 1;
					
					ob->id.flag -= LIB_INDIRECT;
					ob->id.flag |= LIB_EXTERN;

				}
			}
		}
		ob= ob->id.next;
	}
}


void append_named_part(SpaceFile *sfile, Main *main, char *name, int idcode)
{
	Object *ob;
	Base *base;
	BHead *bhead;
	ID *id;
	int afbreek=0;
	char *fd;
	
	fd= sfile->libfiledata;
	while(afbreek==0) {
		
		bhead= (BHead *)fd;
		
		if(bhead->code==ENDB) afbreek= 1;
		else if(bhead->code==idcode) {
			id= (ID *)(bhead+1);
			if(strcmp(id->name+2, name)==0) {
				
				id= is_yet_read(main, bhead);
				if(id==0) {
					read_libblock(main, bhead, LIB_TESTEXT);
				}
				else {
					printf("append: already linked\n");
					add_lib_adr(bhead->old, id);
					if(id->flag & LIB_INDIRECT) {
						id->flag -= LIB_INDIRECT;
						id->flag |= LIB_EXTERN;
					}
				}
				
				if(idcode==ID_OB) {	/* los object: base geven */
					base= callocN( sizeof(Base), "app_nam_part");
					addtail(&(G.scene->base), base);
					
					if(id==0) ob= main->object.last;
					else ob= (Object *)id;
					
					base->lay= ob->lay;
					base->object= ob;
					ob->id.us++;
				}
				afbreek= 1;
			}
		}
		
		fd+= bhead->len+sizeof(BHead);
	}
}

void append_id_part(char *filedata, Main *main, ID *id)
{
	BHead *bhead;
	ID *idread;
	int afbreek=0;
	char *fd;
	
	fd= filedata;
	while(afbreek==0) {
		
		bhead= (BHead *)fd;
		
		if(bhead->code==ENDB) afbreek= 1;
		else if(bhead->code== GS(id->name)) {
			idread= (ID *)(bhead+1);
			
			if(strcmp(id->name, idread->name)==0) {
				id->flag -= LIB_READ;
				id->flag |= LIB_TEST;
				read_libblock(main, bhead, id->flag);
				afbreek= 1;
			}
		}
		
		fd+= bhead->len+sizeof(BHead);
	}
}


void library_append(SpaceFile *sfile)	/* append aan G.scene */
{
	Main *mainptr;
	VFont *vf;
	Object *ob;
	Curve *cu;
	int a, totsel=0, idcode;
	char dir[FILE_MAXDIR], group[32];
	
	/* is er sprake van een library? */
	if( is_a_library(sfile, dir, group)==0 ) {
		error("Not a library");
		return;
	}
	if(sfile->libfiledata==0) {
		error("library not loaded");
		return;
	}
	if(group[0]==0) {
		error("Nothing indicated");
		return;
	}
	if(strcmp(G.main->name, dir)==0) {
		error("Cannot use current file as library");
		return;
	}
	
	/* zijn er geselecteerde files? */
	for(a=0; a<sfile->totfile; a++) {
		if(sfile->filelist[a].flags & ACTIVE) {
			totsel++;
		}
	}
	
	if(totsel==0) {
		/* is de aangegeven file in de filelist? */
		if(sfile->file[0]) {
			for(a=0; a<sfile->totfile; a++) {
				if( strcmp(sfile->filelist[a].relname, sfile->file)==0) break;
			}
			if(a==sfile->totfile) {
				error("Wrong indicated name");
				return;
			}
		}
		else {
			error("Nothing indicated");
			return;
		}
	}
	/* nu hebben OF geselecteerde, OF 1 aangegeven file */
	
	/* mains maken */
	split_main();
	
	/* welke moeten wij hebben? */
	mainptr= find_main(dir);

	idcode= groupname_to_code(group);
	
	if(totsel==0) {
		append_named_part(sfile, mainptr, sfile->file, idcode);
	}
	else {
		for(a=0; a<sfile->totfile; a++) {
			if(sfile->filelist[a].flags & ACTIVE) {
				append_named_part(sfile, mainptr, sfile->filelist[a].relname, idcode);
			}
		}
	}
	
	/* de main consistent maken */
	expand_main(mainptr, sfile->libfiledata);
	
	/* als expand nog andere libs gevonden heeft: */
	read_libraries();
	
	lib_link_all(G.main);

	/* losse objects aan G.scene hangen deze hebben nog een linkflag
	   moet na lib_link ivm gelinkte scenes (ob->us==0) */

	/* indirecte objects kunnen geen kwaad */
	/* als je deze terugzet, denk aan de 'need_link' flag: doe een find naar 'give_base_to' */
	/* give_base_to_objects(G.scene, &(G.main->object)); */



	/* VECTORFONTS */
	vf= G.main->vfont.first;
	while(vf) {
		if(vf->id.lib && vf->flag==1) reload_vfont(vf);
		vf= vf->id.next;
	}
	
	/* DISPLISTEN */
	ob= G.main->object.first;
	set_displist_onlyzero(1);
	while(ob) {
		if(ob->id.lib) {
			if(ob->type==OB_FONT) {
				cu= ob->data;
				if(cu->nurb.first==0) text_to_curve(ob, 0);
			}
			makeDispList(ob);
		}
		else if(ob->type==OB_MESH && ob->parent && ob->parent->type==OB_LATTICE ) makeDispList(ob);
		
		ob= ob->id.next;
	}
	set_displist_onlyzero(0);

	/* losslingerende blokken vrijgeven */
	add_data_adr(0, 0);
	add_lib_adr(0, 0);
	
	/* in sfile->dir staat de HELE libnaam */
	strcpy(G.lib, sfile->dir);
	
		if((sfile->flag & FILE_LINK)==0) all_local();

	/* voorlopige patch om te voorkomen dat de switch_endian 2x gebeurt */
	if(switch_endian) {
		if(sfile->libfiledata) {
			freeN(sfile->libfiledata);
			sfile->libfiledata= 0;
		}
	}
}

/* ************* READ LIBRARY ************** */

void read_libraries()
{
	Main *mainptr;
	ID *id, *idn;
	ListBase *lbarray[30];
	int a, doit=1, tot, filelen;
	char *filedata;
	
	while(doit) {
		doit= 0;
		
		/* test 1: inlezen libdata */
		mainptr= G.main->next;
		
		while(mainptr) {

			disable_newlibadr= (void *)1;
		
			tot= count_mainblocks_flag(mainptr, LIB_READ);
			if(tot) {
				filedata= mainptr->curlib->filedata;
				if(filedata==0) {
					printf("read lib %s\n", mainptr->curlib->name);
					filedata= openblenderfile(mainptr->curlib->name, &filelen);
					mainptr->curlib->filedata= filedata;
				}
				if(filedata) {
					doit= 1;
					a= set_listbasepointers(mainptr, lbarray);
					while(a--) {
						id= lbarray[a]->first;
						while(id) {
							idn= id->next;
							if(id->flag & LIB_READ) {
								remlink(lbarray[a], id);
								
								append_id_part(filedata, mainptr, id);
								/* printf("change adr: %s %x\n", id->name, disable_newlibadr); */
								if(disable_newlibadr==(void *)1) {
									printf("LIB ERROR: can't find %s\n", id->name);
									change_libadr(id, 0);
								}
								else change_libadr(id, disable_newlibadr);
								
								freeN(id);
							}
							id= idn;
						}
					}
				}
				else printf("ERROR: can't find lib %s \n", mainptr->curlib->name);
			}
			disable_newlibadr= 0;

			expand_main(mainptr, mainptr->curlib->filedata);
			
			mainptr= mainptr->next;
		}
	}
	mainptr= G.main->next;
	while(mainptr) {
		if(mainptr->curlib->filedata) freeN(mainptr->curlib->filedata);
		mainptr->curlib->filedata= 0;
		
		/* test of er libblocken niet zijn gelezen */
		a= set_listbasepointers(mainptr, lbarray);
		while(a--) {
			id= lbarray[a]->first;
			while(id) {
				idn= id->next;
				if(id->flag & LIB_READ) {
					remlink(lbarray[a], id);
										
					printf("LIB ERROR: can't find %s\n", id->name);
					change_libadr(id, 0);
					
					freeN(id);
				}
				id= idn;
			}
		}
		
		/* sommige mains moeten nog worden ingelezen, dan is versionfile nog nul! */
		if(mainptr->versionfile) do_versions(mainptr);

		mainptr= mainptr->next;
	}
	
	join_main();
}


