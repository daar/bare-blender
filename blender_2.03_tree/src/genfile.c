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



/*  genfile.c   juni 94     MIXED MODEL
 * 
 *	- pas op: geen beveiling tegen dubbele structen
 *  - struct niet in DNA file: twee hekjes erboven (#<enter>#<enter>)
 * 
 * Version: $Id: genfile.c,v 1.6 2000/07/21 09:05:25 nzc Exp $
 */


#include "blender.h"
#include "screen.h"
#include "file.h"

/* Internal prototypes */
void calculate_structlens(void);

/*  Aanmaken structDNA: alleen wanneer includes wijzigen.
	File Syntax:
		SDNA (4 bytes) (voor fileherkenning)
		NAME (4 bytes)
		<nr> (4 bytes) aantal namen (int)
		<string> 
		<string>
		...
		...
		TYPE (4 bytes)
		<nr> aantal types (int)
		<string>
		<string>
		...
		...
		TLEN (4 bytes)
		<len> (short)
		<len>
		...
		...
		STRC (4 bytes)
		<nr> aantal structen (int)
		<typenr><nr_of_elems> <typenr><namenr> <typenr><namenr> ...
		
	!!Denk aan integer en short aligned schrijven/lezen!!
 
 
	Bij het wegschrijven van files worden namen structen aangegeven
	met type= findstruct_nr(SDNA *, char *), 'type' correspondeert met nummer
	structarray in structDNA.
	
	Voor het moment: complete DNA file appenden achter blenderfile.
	In toekomst nadenken over slimmere methode (alleen gebruikte
	structen?, voorgeprepareerde DNA files? (TOT, OB, MAT )
	
	TOEGESTANE EN GETESTE WIJZIGINGEN IN STRUCTS:
		- type verandering (bij chars naar float wordt door 255 gedeeld)
		- plek in struct (alles kan door elkaar)
		- struct in struct (in struct etc, is recursief)
		- nieuwe elementen toevoegen (standaard op 0)
		- elementen eruit (worden niet meer ingelezen)
		- array's groter/kleiner
		- verschillende typepointers met zelfde naam worden altijd gekopieerd.
	
	(NOG) NIET:
		- float-array (vec[3]) naar struct van floats (vec3f)
	GEDAAN:
		- DNA file in (achter) blender-executable plakken voor upward compatibility
		  Gebruikte methode: het makesdna programma schrijft een c-file met een met
		  spaties gevuld char-array van de juiste lengte. Makesdna maakt er een .o van
		  en vult de spaties met de DNA file.
		- endian compatibility
		- 32 bits en 64 bits pointers
		
	LET OP:
		- uint mag niet in een struct,  gebruik unsigned int. (backwards compatibility vanwege 64 bits code!)
		- structen moeten altijd (intern) 4/8-aligned en short-aligned zijn.
		  de SDNA routine test hierop en print duidelijke errors.
		  DNA files met align errors zijn onbruikbaar!
		- switch_endian doet alleen long long pointers, 
		  zodat ze veilig gecast kunnen worden naar 32 bits
		- casten van 64 naar 32 bits poinetrs: >>3.
 */

struct SDNA cur_sdna={0, 0, }, old_sdna={0, 0, };
int switch_endian= 0;
char *compflags=0;

/* ************************* MAKEN DNA ********************** */


/* duplicate code from makesdna.c */

int arraysize(char *astr, int len)
{
        int a, mul=1;
        char str[100], *cp=0;

        memcpy(str, astr, len+1);

        for(a=0; a<len; a++) {
                if( str[a]== '[' ) {
                        cp= &(str[a+1]);
                }
                else if( str[a]==']' && cp) {
                        str[a]= 0;
                        mul*= atoi(cp);
                }
        }

        return mul;
}


/* ************************* END MAKEN DNA ********************** */
/* ************************* DIV ********************** */

void freestructDNA(struct SDNA *sdna)
{
	if(sdna->data) {
		freeN(sdna->data);
		sdna->data= 0;
		freeN(sdna->names);
		sdna->names= 0;
		freeN(sdna->types);
		sdna->types= 0;
		freeN(sdna->structs);
		sdna->structs= 0;
	}
}

void freestructDNA_all()
{

	freestructDNA(&old_sdna);

	if(cur_sdna.data) {
		freeN(cur_sdna.names);
		cur_sdna.names= 0;
		freeN(cur_sdna.types);
		cur_sdna.types= 0;
		freeN(cur_sdna.structs);
		cur_sdna.structs= 0;
	}

	if(compflags) freeN(compflags);
	compflags= 0;
}

int elementsize(struct SDNA *sdna, short type, short name)
/* aanroepen met nummers uit struct-array */
{
	int mul, namelen, len;
	char *cp;
	
	cp= sdna->names[name];
	len= 0;
	
	namelen= strlen(cp);
	/* is het een pointer of functiepointer? */
	if(cp[0]=='*' || cp[1]=='*') {
		/* heeft de naam een extra lente? (array) */
		mul= 1;
		if( cp[namelen-1]==']') mul= arraysize(cp, namelen);
		
		len= sdna->pointerlen*mul;
	}
	else if( sdna->typelens[type] ) {
		/* heeft de naam een extra lente? (array) */
		mul= 1;
		if( cp[namelen-1]==']') mul= arraysize(cp, namelen);
		
		len= mul*sdna->typelens[type];
		
	}
	
	return len;
}

void printstruct(struct SDNA *sdna, short strnr)
{
	/* geef het structnummer door, is voor debug */
	int b, nr;
	short *sp;
	
	sp= sdna->structs[strnr];
	
	printf("struct %s\n", sdna->types[ sp[0] ]);
	nr= sp[1];
	sp+= 2;
	
	for(b=0; b< nr; b++, sp+= 2) {
		printf("   %s %s\n", sdna->types[sp[0]], sdna->names[sp[1]]);
	}
}


short *findstruct_name(struct SDNA *sdna, char *str)
{
	int a;
	short *sp=0;


	for(a=0; a<sdna->nr_structs; a++) {

		sp= sdna->structs[a];
		
		if(strcmp( sdna->types[ sp[0] ], str )==0) return sp;
	}
	
	return 0;
}

int findstruct_nr(struct SDNA *sdna, char *str)
{
	static int lasta=0;
	int a;
	short *sp=0;

	if(lasta<sdna->nr_structs) {
		sp= sdna->structs[lasta];
		if(strcmp( sdna->types[ sp[0] ], str )==0) return lasta;
	}

	for(a=0; a<sdna->nr_structs; a++) {

		sp= sdna->structs[a];
		
		if(strcmp( sdna->types[ sp[0] ], str )==0) {
			lasta= a;
			return a;
		}
	}
	
	return -1;
}


/* ************************* END DIV ********************** */
/* ************************* LEZEN DNA ********************** */



void init_structDNA(struct SDNA *sdna)
/* in sdna->data staat de data, uit elkaar pulken */
{
	int *data, *verg;
	long nr;
	short *sp;
	static char str[8], *cp;
	
	if(sdna->data==0) {
		bzero(sdna, sizeof(struct SDNA));
		return;
	}
	
	verg= (int *)str;
	data= (int *)sdna->data;

	strcpy(str, "SDNA");
	if( *data == *verg ) {
	
		data++;
		
		/* laad namen array */
		strcpy(str, "NAME");
		if( *data == *verg ) {
			data++;
			
			if(switch_endian) sdna->nr_names= le_int(*data);
			else sdna->nr_names= *data;
			
			data++;
			sdna->names= callocN( sizeof(void *)*sdna->nr_names, "sdnanames");
		}
		else {
			printf("NAME error in SDNA file\n");
			return;
		}
		
		nr= 0;
		cp= (char *)data;
		while(nr<sdna->nr_names) {
			sdna->names[nr]= cp;
			while( *cp) cp++;
			cp++;
			nr++;
		}
		nr= (long)cp;		/* BUS error voorkomen */
		nr= (nr+3) & ~3;
		cp= (char *)nr;
		
		/* laad typenamen array */
		data= (int *)cp;
		strcpy(str, "TYPE");
		if( *data == *verg ) {
			data++;
			
			if(switch_endian) sdna->nr_types= le_int(*data);
			else sdna->nr_types= *data;
			
			data++;
			sdna->types= callocN( sizeof(void *)*sdna->nr_types, "sdnatypes");
		}
		else {
			printf("TYPE error in SDNA file\n");
			return;
		}
		
		nr= 0;
		cp= (char *)data;
		while(nr<sdna->nr_types) {
			sdna->types[nr]= cp;
			
			/* met deze patch kunnen structnamen gewijzigd worden */
			/* alleen gebruiken voor conflicten met systeem-structen (opengl/X) */
			
			if( *cp == 'b') {
				/* struct Screen was already used by X, 'bScreen' replaces the old IrisGL 'Screen' struct */
				if( strcmp("bScreen", cp)==0 ) sdna->types[nr]= cp+1;
			}
			
			while( *cp) cp++;
			cp++;
			nr++;
		}
		nr= (long)cp;		/* BUS error voorkomen */
		nr= (nr+3) & ~3;
		cp= (char *)nr;
		
		/* laad typelen array */
		data= (int *)cp;
		strcpy(str, "TLEN");
		if( *data == *verg ) {
			data++;
			sp= (short *)data;
			sdna->typelens= sp;
			
			if(switch_endian) {
				short a, *spo= sp;
				
				a= sdna->nr_types;
				while(a--) {
					spo[0]= le_short(spo[0]);
					spo++;
				}
			}
			
			sp+= sdna->nr_types;
		}
		else {
			printf("TLEN error in SDNA file\n");
			return;
		}
		if(sdna->nr_types & 1) sp++;	/* BUS error voorkomen */

		/* laad structen array */
		data= (int *)sp;
		strcpy(str, "STRC");
		if( *data == *verg ) {
			data++;
			
			if(switch_endian) sdna->nr_structs= le_int(*data);
			else sdna->nr_structs= *data;
			
			data++;
			sdna->structs= callocN( sizeof(void *)*sdna->nr_types, "sdnatypes");
		}
		else {
			printf("STRC error in SDNA file\n");
			return;
		}
		
		nr= 0;
		sp= (short *)data;
		while(nr<sdna->nr_structs) {
			sdna->structs[nr]= sp;
			
			if(switch_endian) {
				short a;
				
				sp[0]= le_short(sp[0]);
				sp[1]= le_short(sp[1]);
				
				a= sp[1];
				sp+= 2;
				while(a--) {
					sp[0]= le_short(sp[0]);
					sp[1]= le_short(sp[1]);
					sp+= 2;
				}
			}
			else {
				sp+= 2*sp[1]+2;
			}
			
			nr++;
		}
		
		/* finally pointerlen: use struct ListBase to test it, never change the size of it! */
		/* the global pointerlen from readfile.c can't be used here... */
		sp= findstruct_name(sdna, "ListBase");
		
		sdna->pointerlen= sdna->typelens[ sp[0] ]/2;

		if(sp[1]!=2 || (sdna->pointerlen!=4 && sdna->pointerlen!=8)) {
			printf("ListBase struct error! Needs it to calculate pointerize.\n");
			exit(0);
		}
		
	}
}

void read_structDNA(struct SDNA *sdna, char *filename)
{
	int file, filelen;
	
	file= open(filename, O_BINARY|O_RDONLY);
	if(file== -1) {
		printf("Can't read DNA file\n");
	}
	else {
		filelen= filesize(file);
		sdna->datalen= filelen;

		sdna->data= mallocN(filelen, "convert_include");
		read(file, sdna->data, filelen);
		close(file);
		
		init_structDNA(sdna);
	}
}


void findstruct_ext(char *str1, char *str2)
{
	int a, offset;
	short *sp;
	
	/* eerst inlezen DNA file */
	
	read_structDNA(&cur_sdna, "DNA");
	
	sp= findstruct_name(&cur_sdna, str1);
	
	if(sp) {
		a= sp[1];	/* aantal elems */
		sp+= 2;
		offset= 0;
		
		while(a--) {
			if( strcmp( cur_sdna.names[ sp[1] ], str2 )==0 ) {
				printf("type %s offset %d\n", cur_sdna.types[ sp[0] ], offset);
				return;
			}
			offset+= elementsize(&cur_sdna, sp[0], sp[1]);
			
			sp+= 2;
		}
		printf("Element bestaat niet\n");
	}
	else printf("Struct bestaat niet\n");
	
}

int findstruct_offset(char *structname, char *member)
{
	int a, offset;
	short *sp;
	
	sp= findstruct_name(&cur_sdna, structname);
	
	if(sp) {
		a= sp[1];	/* aantal elems */
		sp+= 2;
		offset= 0;
		
		while(a--) {
			if(strcmp(cur_sdna.names[sp[1]], member)==0) return offset;

			offset+= elementsize(&cur_sdna, sp[0], sp[1]);			
			sp+= 2;
		}
	}
	
	return -1;
}

/* ******************** END LEZEN DNA ********************** */
/* ******************* AFHANDELEN DNA ***************** */

int struct_compare(char *stype, void *one, void *two)
/* van struct 'stype' worden one en two vergeleken: pointers detecteren!!! */
{
	int namelen, a, nr, elemcount, elen, firststructtypenr;
	short *spc;
	char *type, *p1, *p2, *name;
	
	if(one==0 || two==0) return 0;
	
	nr= findstruct_nr(&cur_sdna, stype);
	if(nr == -1) return 0;
	
	firststructtypenr= *(cur_sdna.structs[0]);
	spc= cur_sdna.structs[nr];

	elemcount= spc[1];

	spc+= 2;
	p1= one;
	p2= two;

	for(a=0; a<elemcount; a++, spc+=2) {
		
		type= cur_sdna.types[spc[0]];
		name= cur_sdna.names[spc[1]];
		elen= elementsize(&cur_sdna, spc[0], spc[1]);
		namelen= strlen(name);
		
		if(name[0]=='*') {
			if( strncmp(type, "void", 4) ) {
				if( *((long *)p1)==0 && *((long *)p2)==0);
				else if( *((long *)p1) && *((long *)p2));
				else return 0;
			}
		}
		else if(strncmp(type, "char", 4)==0 && name[namelen-1]==']') {
			;
		}
		else {

			/* testen: is type een struct? */
			if(spc[0]>=firststructtypenr) {
				if( struct_compare(type, p1, p2)==0) return 0;
			}
			else {
				if(memcmp(p1, p2, elen)) return 0;
				
				
			}
		}
		
		p1+= elen;
		p2+= elen;
	}
	
	return 1;
}


void recurs_test_compflags(int structnr)
{
	int a, b, typenr, elems;
	short *sp;
	char *cp;
	
	/* loop alle structen af en test of deze struct in andere zit */
	sp= old_sdna.structs[structnr];
	typenr= sp[0];
	
	for(a=0; a<old_sdna.nr_structs; a++) {
		if(a!=structnr && compflags[a]==1) {
			sp= old_sdna.structs[a];
			elems= sp[1];
			sp+= 2;
			for(b=0; b<elems; b++, sp+=2) {
				if(sp[0]==typenr) {
					cp= old_sdna.names[ sp[1] ];
					if(cp[0]!= '*') {
						compflags[a]= 2;
						recurs_test_compflags(a);
					}
				}
			}
		}
	}
	
}

void set_compareflags_structDNA()
{
	/* flag: 0:bestaat niet meer (of nog niet)
	 *       1: is gelijk
	 *       2: is anders
	 */
	int a, b;
	short *spold, *spcur;
	char *str1, *str2;
	
	if(old_sdna.nr_structs==0) {
		printf("error: file without SDNA\n");
		return;
	}
		
	if(compflags) freeN(compflags);
	compflags= callocN(old_sdna.nr_structs, "compflags");

	/* We lopen alle structs in 'old_sdna' af, vergelijken ze met 
	 * de structs in 'cur_sdna'
	 */
	
	for(a=0; a<old_sdna.nr_structs; a++) {
		spold= old_sdna.structs[a];
		
		/* type zoeken in cur */
		spcur= findstruct_name(&cur_sdna, old_sdna.types[spold[0]]);
		
		if(spcur) {
			compflags[a]= 2;
			
			/* lengte en aantal elems vergelijken */
			if( spcur[1] == spold[1]) {
				 if( cur_sdna.typelens[spcur[0]] == old_sdna.typelens[spold[0]] ) {
					 
					 /* evenlang en evenveel elems, nu per type en naam */
					 b= spold[1];
					 spold+= 2;
					 spcur+= 2;
					 while(b > 0) {
						 str1= cur_sdna.types[spcur[0]];
						 str2= old_sdna.types[spold[0]];
						 if(strcmp(str1, str2)!=0) break;

						 str1= cur_sdna.names[spcur[1]];
						 str2= old_sdna.names[spold[1]];
						 if(strcmp(str1, str2)!=0) break;
						 
						 /* naam gelijk, type gelijk, nu nog pointersize, dit geval komt haast nooit voor! */
						 if(str1[0]=='*') {
							 if(old_sdna.pointerlen!=cur_sdna.pointerlen) break;
						 }
						 
						 b--;
						 spold+= 2;
						 spcur+= 2;
					 }
					 if(b==0) compflags[a]= 1;

				 }
			}
			
		}
	}

	/* eerste struct in util.h is struct Link, deze wordt in de compflags overgeslagen (als # 0).
	 * Vuile patch! Nog oplossen....
	 */
	compflags[0]= 1;

	/* Aangezien structen in structen kunnen zitten gaan we recursief
	 * vlaggen zetten als er een struct veranderd is
	 */
	for(a=0; a<old_sdna.nr_structs; a++) {
		if(compflags[a]==2) recurs_test_compflags(a);
	}
	
	for(a=0; a<old_sdna.nr_structs; a++) {
		if(compflags[a]==2) {
			spold= old_sdna.structs[a];
			/* printf("changed: %s\n", old_sdna.types[ spold[0] ]); */
		}
	}
	
	
}

void cast_elem(char *ctype, char *otype, char *name, char *curdata, char *olddata)
{
	double val;
	int arrlen, curlen=1, oldlen=1, ctypenr, otypenr;
	
	arrlen= arraysize(name, strlen(name));
	
	/* otypenr bepalen */
	if(strcmp(otype, "char")==0) otypenr= 0; 
	else if(strcmp(otype, "uchar")==0) otypenr= 1;
	else if(strcmp(otype, "short")==0) otypenr= 2; 
	else if(strcmp(otype, "ushort")==0) otypenr= 3;
	else if(strcmp(otype, "int")==0) otypenr= 4;
	else if(strcmp(otype, "long")==0) otypenr= 5;
	else if(strcmp(otype, "ulong")==0) otypenr= 6;
	else if(strcmp(otype, "float")==0) otypenr= 7;
	else if(strcmp(otype, "double")==0) otypenr= 8;
	else return;
	
	/* ctypenr bepalen */
	if(strcmp(ctype, "char")==0) ctypenr= 0; 
	else if(strcmp(ctype, "uchar")==0) ctypenr= 1;
	else if(strcmp(ctype, "short")==0) ctypenr= 2; 
	else if(strcmp(ctype, "ushort")==0) ctypenr= 3;
	else if(strcmp(ctype, "int")==0) ctypenr= 4;
	else if(strcmp(ctype, "long")==0) ctypenr= 5;
	else if(strcmp(ctype, "ulong")==0) ctypenr= 6;
	else if(strcmp(ctype, "float")==0) ctypenr= 7;
	else if(strcmp(ctype, "double")==0) ctypenr= 8;
	else return;

	/* lengtes bepalen */
	if(otypenr < 2) oldlen= 1;
	else if(otypenr < 4) oldlen= 2;
	else if(otypenr < 8) oldlen= 4;
	else oldlen= 8;

	if(ctypenr < 2) curlen= 1;
	else if(ctypenr < 4) curlen= 2;
	else if(ctypenr < 8) curlen= 4;
	else curlen= 8;
	
	while(arrlen>0) {
		switch(otypenr) {
		case 0:
			val= *olddata; break;
		case 1:
			val= *( (uchar *)olddata); break;
		case 2:
			val= *( (short *)olddata); break;
		case 3:
			val= *( (ushort *)olddata); break;
		case 4:
			val= *( (int *)olddata); break;
		case 5:
			val= *( (int *)olddata); break;
		case 6:
			val= *( (uint *)olddata); break;
		case 7:
			val= *( (float *)olddata); break;
		case 8:
			val= *( (double *)olddata); break;
		}
		
		switch(ctypenr) {
		case 0:
			*curdata= val; break;
		case 1:
			*( (uchar *)curdata)= val; break;
		case 2:
			*( (short *)curdata)= val; break;
		case 3:
			*( (ushort *)curdata)= val; break;
		case 4:
			*( (int *)curdata)= val; break;
		case 5:
			*( (int *)curdata)= val; break;
		case 6:
			*( (uint *)curdata)= val; break;
		case 7:
			if(otypenr<2) val/= 255;
			*( (float *)curdata)= val; break;
		case 8:
			if(otypenr<2) val/= 255;
			*( (double *)curdata)= val; break;
		}

		olddata+= oldlen;
		curdata+= curlen;
		arrlen--;
	}
}

void cast_pointer(int curlen, int oldlen, char *name, char *curdata, char *olddata)
{
#ifdef WIN32
	DWORDLONG lval;
#else
	long long lval;
#endif
	int arrlen;
	
	arrlen= arraysize(name, strlen(name));
	
	while(arrlen>0) {
	
		if(curlen==oldlen) {
			memcpy(curdata, olddata, curlen);
		}
		else if(curlen==4 && oldlen==8) {
#ifdef WIN32			
			lval= *( (DWORDLONG *)olddata );
#else
			lval= *( (long long *)olddata );
#endif
			*((int *)curdata) = lval>>3;		/* is natuurlijk een beetje een gok! */
		}
		else if(curlen==8 && oldlen==4) {
#ifdef WIN32
			 *( (DWORDLONG *)curdata ) = *((int *)olddata);
#else
			 *( (long long *)curdata ) = *((int *)olddata);
#endif
		}
		else {
			/* voor debug */
			printf("errpr: illegal pointersize! \n");
		}
		
		olddata+= oldlen;
		curdata+= curlen;
		arrlen--;

	}
}

int elem_strcmp(char *name, char *oname)
{
	int a=0;
	
	/* strcmp without array part */
	
	while(TRUE) {
		if(name[a] != oname[a]) return 1;
		if(name[a]=='[') break;
		if(name[a]==0) break;
		a++;
	}
	if(name[a] != oname[a]) return 1;
	return 0;
}

char *find_elem(char *type, char *name, short *old, char *olddata, short **sppo)
{
	int a, elemcount, len;
	char *otype, *oname;
	
	/* without arraypart, so names can differ: return old namenr and type */
	
	/* in old staat de oude struct */
	elemcount= old[1];
	old+= 2;
	for(a=0; a<elemcount; a++, old+=2) {
	
		otype= old_sdna.types[old[0]];
		oname= old_sdna.names[old[1]];
		
		len= elementsize(&old_sdna, old[0], old[1]);
		
		if( elem_strcmp(name, oname)==0 ) {	/* naam gelijk */
			if( strcmp(type, otype)==0 ) {	/* type gelijk */
				if(sppo) *sppo= old;
				return olddata;
			}
			
			return 0;
		}
		
		olddata+= len;
	}
	return 0;
}

void reconstruct_elem(char *type, char *name, char *curdata, short *old, char *olddata)
{
	/* regels: testen op NAAM:
			- naam volledig gelijk:
				- type casten
			- naam gedeeltelijk gelijk (array anders)
				- type gelijk: memcpy
				- types casten
	*/	
	int a, elemcount, len, array, oldsize, cursize, mul;
	char *otype, *oname, *cp;
	
	/* is 'name' een array? */
	cp= name;
	array= 0;
	while( *cp && *cp!='[') {
		cp++; array++;
	}
	if( *cp!= '[' ) array= 0;
	
	/* in old staat de oude struct */
	elemcount= old[1];
	old+= 2;
	for(a=0; a<elemcount; a++, old+=2) {
		otype= old_sdna.types[old[0]];
		oname= old_sdna.names[old[1]];
		len= elementsize(&old_sdna, old[0], old[1]);
		
		if( strcmp(name, oname)==0 ) {	/* naam gelijk */
			
			if( name[0]=='*') {		/* pointer afhandelen */
				cast_pointer(cur_sdna.pointerlen, old_sdna.pointerlen, name, curdata, olddata);
			}
			else if( strcmp(type, otype)==0 ) {	/* type gelijk */
				memcpy(curdata, olddata, len);
			}
			else cast_elem(type, otype, name, curdata, olddata);

			return;
		}
		else if(array) {		/* de naam is een array */

			if( strncmp(name, oname, array)==0 ) {			/* basis gelijk */
				
				cursize= arraysize(name, strlen(name));
				oldsize= arraysize(oname, strlen(oname));

				if( name[0]=='*') {		/* pointer afhandelen */
					if(cursize>oldsize) cast_pointer(cur_sdna.pointerlen, old_sdna.pointerlen, oname, curdata, olddata);
					else cast_pointer(cur_sdna.pointerlen, old_sdna.pointerlen, name, curdata, olddata);
				}
				else if(name[0]=='*' || strcmp(type, otype)==0 ) {	/* type gelijk */
					mul= len/oldsize;
					mul*= MIN2(cursize, oldsize);
					memcpy(curdata, olddata, mul);
				}
				else {
					if(cursize>oldsize) cast_elem(type, otype, oname, curdata, olddata);
					else cast_elem(type, otype, name, curdata, olddata);
				}
				return;
			}
		}
		
		olddata+= len;
	}
	
}

void reconstruct_struct(int oldSDNAnr, char *data, int curSDNAnr, char *cur)
{
	/* Recursief!
	 * Per element van cur_struct data lezen uit old_struct.
	 * Als element een struct is, recursief aanroepen.
	 */
	int a, elemcount, elen, eleno, mul, mulo, firststructtypenr;
	short *spo, *spc, *sppo;
	char *name, *nameo, *type, *cpo, *cpc;

	if(oldSDNAnr== -1) return;
	if(curSDNAnr== -1) return;

	if( compflags[oldSDNAnr]==1 ) {		/* bij recurs: testen op gelijk */
	
		spo= old_sdna.structs[oldSDNAnr];
		elen= old_sdna.typelens[ spo[0] ];
		memcpy( cur, data, elen);
		
		return;
	}

	firststructtypenr= *(cur_sdna.structs[0]);

	spo= old_sdna.structs[oldSDNAnr];
	spc= cur_sdna.structs[curSDNAnr];

	elemcount= spc[1];

	spc+= 2;
	cpc= cur;
	for(a=0; a<elemcount; a++, spc+=2) {
		type= cur_sdna.types[spc[0]];
		name= cur_sdna.names[spc[1]];
		
		elen= elementsize(&cur_sdna, spc[0], spc[1]);

		/* testen: is type een struct? */
		if(spc[0]>=firststructtypenr  &&  name[0]!='*') {
		
			/* waar start de oude struct data (is ie er wel?) */
			cpo= find_elem(type, name, spo, data, &sppo);
			
			if(cpo) {
				oldSDNAnr= findstruct_nr(&old_sdna, type);
				curSDNAnr= findstruct_nr(&cur_sdna, type);
				
				/* array! */
				mul= arraysize(name, strlen(name));
				nameo= old_sdna.names[sppo[1]];
				mulo= arraysize(nameo, strlen(nameo));
				
				eleno= elementsize(&old_sdna, sppo[0], sppo[1]);
				
				elen/= mul;
				eleno/= mulo;
				
				while(mul--) {
					reconstruct_struct(oldSDNAnr, cpo, curSDNAnr, cpc);
					cpo+= eleno;
					cpc+= elen;
					
					/* new struct array larger than old */
					mulo--;
					if(mulo<=0) break;
				}
			}
			else cpc+= elen;
		}
		else {

			reconstruct_elem(type, name, cpc, spo, data);
			cpc+= elen;

		}
	}
	
}

void switch_endian_struct(int oldSDNAnr, char *data)
{
	/* Recursief!
	 * Als element een struct is, recursief aanroepen.
	 */
	int a, mul, elemcount, elen, elena, firststructtypenr, curSDNAnr;
	short *spo, *spc, skip;
	char *name, *type, *cpo, *cur, cval;

	if(oldSDNAnr== -1) return;
	firststructtypenr= *(old_sdna.structs[0]);
	
	spo= spc= old_sdna.structs[oldSDNAnr];

	elemcount= spo[1];

	spc+= 2;
	cur= data;
	
	for(a=0; a<elemcount; a++, spc+=2) {
		type= old_sdna.types[spc[0]];
		name= old_sdna.names[spc[1]];
		
		/* elementsize = including arraysize */
		elen= elementsize(&old_sdna, spc[0], spc[1]);

		/* testen: is type een struct? */
		if(spc[0]>=firststructtypenr  &&  name[0]!='*') {
			/* waar start de oude struct data (is ie er wel?) */
			cpo= find_elem(type, name, spo, data, 0);
			if(cpo) {
				oldSDNAnr= findstruct_nr(&old_sdna, type);
				
				mul= arraysize(name, strlen(name));
				elena= elen/mul;

				while(mul--) {
					switch_endian_struct(oldSDNAnr, cpo);
					cpo += elena;
				}
			}
		}
		else {
			
			if( name[0]=='*' ) {
				if(old_sdna.pointerlen==8) {
					
					mul= arraysize(name, strlen(name));
					cpo= cur;
					while(mul--) {
						cval= cpo[0]; cpo[0]= cpo[7]; cpo[7]= cval;
						cval= cpo[1]; cpo[1]= cpo[6]; cpo[6]= cval;
						cval= cpo[2]; cpo[2]= cpo[5]; cpo[5]= cval;
						cval= cpo[3]; cpo[3]= cpo[4]; cpo[4]= cval;
						
						cpo+= 8;
					}
					
				}
			}
			else {
				
				if( spc[0]==2 || spc[0]==3 ) {	/* short-ushort */
					
					/* uitzondering: variable die blocktype/ipowin heet: van ID_ afgeleid */
					skip= 0;
					if(name[0]=='b' && name[1]=='l') {
						if(strcmp(name, "blocktype")==0) skip= 1;
					}
					else if(name[0]=='i' && name[1]=='p') {
						if(strcmp(name, "ipowin")==0) skip= 1;
					}
					
					if(skip==0) {
						mul= arraysize(name, strlen(name));
						cpo= cur;
						while(mul--) {
							cval= cpo[0];
							cpo[0]= cpo[1];
							cpo[1]= cval;
							cpo+= 2;
						}
					}
				}
				else if(spc[0]>3 && spc[0]<8) { /* int-long-ulong-float */

					mul= arraysize(name, strlen(name));
					cpo= cur;
					while(mul--) {
						cval= cpo[0];
						cpo[0]= cpo[3];
						cpo[3]= cval;
						cval= cpo[1];
						cpo[1]= cpo[2];
						cpo[2]= cval;
						cpo+= 4;
					}
				}
			}
		}
		
		cur+= elen;
	}
	
}


void reconstruct(int oldSDNAnr, int blocks, void **data)
{
	int a, curSDNAnr, curlen=0, oldlen;
	short *spo, *spc;
	char *cur, *type, *cpc, *cpo;
	
	/* oldSDNAnr == structnr, we zoeken het corresponderende 'cur' nummer */
	spo= old_sdna.structs[oldSDNAnr];
	type= old_sdna.types[ spo[0] ];
	oldlen= old_sdna.typelens[ spo[0] ];
	curSDNAnr= findstruct_nr(&cur_sdna, type);

	/* data goedzetten en nieuwe calloc doen */
	if(curSDNAnr >= 0) {
		spc= cur_sdna.structs[curSDNAnr];
		curlen= cur_sdna.typelens[ spc[0] ];
	}
	if(curlen==0) {
		freeN(*data);
		*data= 0;
		return;
	}
	cur= callocN( blocks*curlen, "reconstruct");
	
	cpc= cur;
	cpo= *data;
	for(a=0; a<blocks; a++) {
		reconstruct_struct(oldSDNAnr, cpo, curSDNAnr, cpc);
		cpc+= curlen;
		cpo+= oldlen;
	}
	freeN(*data);
	*data= cur;
}

