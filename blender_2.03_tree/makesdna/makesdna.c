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




/* makesdna.c */

#include <stdio.h>
#include <stdlib.h>

char *includefiles[] = {
    "util.h",
    "blender.h",
    "screen.h",
    "file.h",
    "sequence.h",
    "effect_types.h",
    "ika.h",
    "oops.h",
    "imasel.h",
    "game.h",
    "old_game.h",
    "sound.h",
    "group.h",
	
	// empty string to indicate end of includefiles
	""
};

#ifdef WIN32
typedef unsigned int uint;
#endif

#ifndef WIN32
#include <strings.h>
#include <unistd.h>
#endif

#include <fcntl.h>
#include "file.h"
#include "util.h"

int maxdata= 100000, maxnr= 5000;
int nr_names=0;
int nr_types=0;
int nr_structs=0;
char **names, *namedata;		/* op adres names[a] staat string a */
char **types, *typedata;		/* op adres types[a] staat string a */
short *typelens;				/* op typelens[a] staat de lengte van type a */
short **structs, *structdata;	/* op sp= structs[a] staat eerste adres structdefinitie
								   sp[0] is typenummer
								   sp[1] is aantal elementen
								   sp[2] sp[3] is typenr,  namenr (enz) */



/* stubs */

int filesize(int file)
{
        struct stat buf;

        if (file <= 0) return (-1);
        fstat(file, &buf);
        return (buf.st_size);
}

/* ************************* MAKEN DNA ********************** */

int add_type(char *str, int len)
{
	int nr;
	char *cp;
	
	if(str[0]==0) return -1;
	
	/* zoek typearray door */
	for(nr=0; nr<nr_types; nr++) {
		if(strcmp(str, types[nr])==0) {
			if(len) typelens[nr]= len;
			return nr;
		}
	}
	
	/* nieuw type appenden */
	if(nr_types==0) cp= typedata;
	else {
		cp= types[nr_types-1]+strlen(types[nr_types-1])+1;
	}
	strcpy(cp, str);
	types[nr_types]= cp;
	typelens[nr_types]= len;
	
	if(nr_types>=maxnr) {
		printf("too many types\n");
		return nr_types-1;;
	}
	nr_types++;
	
	return nr_types-1;
}

int add_name(char *str)
{
	int nr;
	char *cp;
	
	if(str[0]==0) return -1;
	
	/* zoek name array door */
	for(nr=0; nr<nr_names; nr++) {
		if(strcmp(str, names[nr])==0) {
			return nr;
		}
	}
	
	/* nieuw type appenden */
	if(nr_names==0) cp= namedata;
	else {
		cp= names[nr_names-1]+strlen(names[nr_names-1])+1;
	}
	strcpy(cp, str);
	names[nr_names]= cp;
	
	if(nr_names>=maxnr) {
		printf("too many names\n");
		return nr_names-1;
	}
	nr_names++;
	
	return nr_names-1;
}

short *add_struct(int namecode)
{
	int len;
	short *sp;

	if(nr_structs==0) {
		structs[0]= structdata;
	}
	else {
		sp= structs[nr_structs-1];
		len= sp[1];
		structs[nr_structs]= sp+ 2*len+2;
	}
	
	sp= structs[nr_structs];
	sp[0]= namecode;
	
	if(nr_structs>=maxnr) {
		printf("too many structs\n");
		return sp;
	}
	nr_structs++;
	
	return sp;
}

int preprocess_include(char *maindata, int len)
{
	int a, newlen, comment;
	char *cp, *temp, *md;
	
	temp= mallocN(len, "preprocess_include");
	memcpy(temp, maindata, len);
	
	/* alle enters/tabs/etc vervangen door spaties */
	cp= temp;
	a= len;
	while(a--) {
		if( *cp<32 || *cp>128 ) *cp= 32;
		cp++;
	}
	
	/* data uit temp naar maindata kopieeren, verwijder commentaar en dubbele spaties */
	cp= temp;
	md= maindata;
	newlen= 0;
	comment= 0;
	a= len;
	while(a--) {
		
		if(cp[0]=='/' && cp[1]=='*') {
			comment= 1;
			cp[0]=cp[1]= 32;
		}
		if(cp[0]=='*' && cp[1]=='/') {
			comment= 0;
			cp[0]=cp[1]= 32;
		}

		/* niet kopieeren als: */
		if(comment);
		else if( cp[0]==' ' && cp[1]==' ' );
		else if( cp[-1]=='*' && cp[0]==' ' );	/* pointers met spatie */
		else {
			md[0]= cp[0];
			md++;
			newlen++;
		}
		cp++;
	}
	
	freeN(temp);
	return newlen;
}

void convert_include(char *filename)
{
	/* lees includefile, sla structen over die op regel ervoor '#' hebben.
	   sla alle data op in tijdelijke arrays.
	*/
	int file, filelen, count, overslaan, slen, type, name, strct;
	short *structpoin, *sp;
	char *maindata, *mainend, *md, *md1;


	file= open(filename, O_BINARY|O_RDONLY);
	if(file== -1) {
		printf("Can't read file %s\n", filename);
	}
	else {
		filelen= filesize(file);
		
		md= maindata= mallocN(filelen, "convert_include");
		read(file, maindata, filelen);
		close(file);

		filelen= preprocess_include(maindata, filelen);
		mainend= maindata+filelen-1;

		/* we zoeken naar '{' en dan terug naar 'struct' */
		count= 0;
		overslaan= 0;
		while(count<filelen) {
			
			/* code voor struct overslaan: twee hekjes. (voor spatie zorgt preprocess) */
			if(md[0]=='#' && md[1]==' ' && md[2]=='#') {
				overslaan= 1;
			}
			
			if(md[0]=='{') {
				md[0]= 0;
				if(overslaan) {
					overslaan= 0;
				}
				else {
					if(md[-1]==' ') md[-1]= 0;
					md1= md-2;
					while( *md1!=32) md1--;		/* naar begin woord */
					md1++;
					
					/* structnaam te pakken, als... */
					if( strncmp(md1-7, "struct", 6)==0 ) {
						
						strct= add_type(md1, 0);
						structpoin= add_struct(strct);
						sp= structpoin+2;
						
						/* eerst overal keurige strings van maken */
						md1= md+1;
						while(*md1 != '}') {
							if( ((long)md1) > ((long)mainend) ) break;
							
							if(*md1==',' || *md1==' ') *md1= 0;
							md1++;
						}
						
						/* types en namen lezen tot eerste karakter niet '}' */
						md1= md+1;
						while( *md1 != '}' ) {
							if( ((long)md1) > ((long)mainend) ) break;
							
							/* als er 'struct' of 'unsigned' staat, overslaan */
							if(*md1) {
								if( strncmp(md1, "struct", 6)==0 ) md1+= 7;
								if( strncmp(md1, "unsigned", 6)==0 ) md1+= 9;
								
								/* type te pakken! */
								type= add_type(md1, 0);
								
								md1+= strlen(md1);
								
								/* doorlezen tot ';' */
								while( *md1 != ';' ) {
									if( ((long)md1) > ((long)mainend) ) break;
									
									if(*md1) {
										/* name te pakken */
										slen= strlen(md1);
										if( md1[slen-1]==';' ) {
											md1[slen-1]= 0;

											name= add_name(md1);
											sp[0]= type;
											sp[1]= name;
											structpoin[1]++;
											sp+= 2;
											
											md1+= slen;
											break;
										}
										
										name= add_name(md1);
										sp[0]= type;
										sp[1]= name;
										structpoin[1]++;
										sp+= 2;
										
										md1+= slen;
									}
									md1++;
								}
							}
							md1++;
						}
					}
				}
			}
			count++;
			md++;
		}
		
		freeN(maindata);	
	}	
}

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

void calculate_structlens()
{
	int a, b, len, unknown= nr_structs, lastunknown, structtype, type, mul, namelen;
	short *sp, *structpoin;
	char *cp;
		
	while(unknown) {
		lastunknown= unknown;
		unknown= 0;
		
		/* loop alle structen af... */
		for(a=0; a<nr_structs; a++) {
			structpoin= structs[a];
			structtype= structpoin[0];

			/* als lengte nog niet bekend */
			if(typelens[structtype]==0) {
				
				sp= structpoin+2;
				len= 0;
				
				/* loop alle elementen in struct af */
				for(b=0; b<structpoin[1]; b++, sp+=2) {
					type= sp[0];
					cp= names[sp[1]];
					
					namelen= strlen(cp);
					/* is het een pointer of functiepointer? */
					if(cp[0]=='*' || cp[1]=='*') {
						/* heeft de naam een extra lengte? (array) */
						mul= 1;
						if( cp[namelen-1]==']') mul= arraysize(cp, namelen);
						
						/* 4-8 aligned/ */
						if(sizeof(void *) == 4) {
							if (len % 4) {
								printf("Align pointer error in struct: %s %s\n", types[structtype], cp);
							}
						}
						else {
							if (len % 8) {
								printf("Align pointer error in struct: %s %s\n", types[structtype], cp);
							}
						}
						len += sizeof(void *) * mul;
					}
					else if( typelens[type] ) {
						/* heeft de naam een extra lente? (array) */
						mul= 1;
						if( cp[namelen-1]==']') mul= arraysize(cp, namelen);
						
						/* 2-4 aligned/ */
						if(typelens[type]>3 && (len % 4) ) {
							printf("Align 4 error in struct: %s %s\n", types[structtype], cp);
						}
						else if(typelens[type]==2 && (len % 2) ) {
							printf("Align 2 error in struct: %s %s\n", types[structtype], cp);
						}

						len+= mul*typelens[type];
						
					}
					else {
						len= 0;
						break;
					}
				}
				
				if(len==0) unknown++;
				else {
					typelens[structtype]= len;
				}
			}
		}
		
		if(unknown==lastunknown) break;
	}
	
	if(unknown) {
		printf("error: still %d structs unknown\n", unknown);
		
		for(a=0; a<nr_structs; a++) {
			structpoin= structs[a];
			structtype= structpoin[0];

			/* lengte nog niet bekend */
			if(typelens[structtype]==0) {
				printf("  %s\n", types[structtype]);
			}
		}
	}
}

#define MAX_DNA_LINE_LENGTH 20

void dna_write(FILE *file, void *pntr, int size)
{
	static int linelength = 0;
	int i;
	char *data;

	data = (char *) pntr;
	
	for (i = 0 ; i < size ; i++)
	{
		fprintf(file, "%d,", data[i]);
		linelength++;
		if (linelength >= MAX_DNA_LINE_LENGTH) {
			fprintf(file, "\n");
			linelength = 0;
		}
	}
}

void make_structDNA(FILE *file)
{
	int len, i;
	short *sp;
	char str[40], *cp;
	int firststruct;

	/* de allerlangst bekende struct is 50k, 100k is ruimte genoeg! */
	namedata= callocN(maxdata, "namedata");
	typedata= callocN(maxdata, "typedata");
	structdata= callocN(maxdata, "structdata");
	
	/* maximaal 5000 variablen, vast voldoende? */
	names= callocN(sizeof(char *)*maxnr, "names");
	types= callocN(sizeof(char *)*maxnr, "types");
	typelens= callocN(sizeof(short)*maxnr, "typelens");
	structs= callocN(sizeof(short)*maxnr, "structs");
	
	/* inserten alle bekende types */
	/* let op: uint komt niet voor! gebruik in structen unsigned int */
	add_type("char", 1);	/* 0 */
	add_type("uchar", 1);	/* 1 */
	add_type("short", 2);	/* 2 */
	add_type("ushort", 2);	/* 3 */
	add_type("int", 4);		/* 4 */
	add_type("long", 4);	/* 5 */
	add_type("ulong", 4);	/* 6 */
	add_type("float", 4);	/* 7 */
	add_type("double", 8);	/* 8 */
	add_type("void", 0);	/* 9 */

	// the defines above shouldn't be output in the padding file...
	firststruct = nr_types;
	
	/* eerste struct in util.h is struct Link, deze wordt in de compflags overgeslagen (als # 0).
	 * Vuile patch! Nog oplossen....
	 */

#ifdef IRISGL

#ifdef NINCLUDES
	#define BASE_HEADER "../include/"
#else
	#ifdef MIPS1
		#define BASE_HEADER "./"
	#else
		#define BASE_HEADER "../"
	#endif
#endif

#else

#define BASE_HEADER "../include/"

#endif

	// add all include files defined in the global array
	for (i = 0; strlen(includefiles[i]); i++) {
		sprintf(str, "%s%s", BASE_HEADER, includefiles[i]);
		convert_include(str);
	}

	calculate_structlens();

	/* DIT DEEL VOOR DEBUG */
	/*
	printf("nr_names %d nr_types %d nr_structs %d\n", nr_names, nr_types, nr_structs);
	for(a=0; a<nr_names; a++) { 
		printf(" %s \n", names[a]);
	}
	printf("\n");
	
	sp= typelens;
	for(a=0; a<nr_types; a++, sp++) { 
		printf(" %s %d\n", types[a], *sp);
	}
	printf("\n");
	
	for(a=0; a<nr_structs; a++) {
		sp= structs[a];
		printf(" struct %s elems: %d \n", types[sp[0]], sp[1]);
		elem= sp[1];
		sp+= 2;
		for(b=0; b< elem; b++, sp+= 2) {
			printf("   %s %s\n", types[sp[0]], names[sp[1]]);
		}
	}
	*/
	
	/* file schrijven */
	
	if(nr_names==0 || nr_structs==0);
	else {
		strcpy(str, "SDNA");
		dna_write(file, str, 4);
		
		/* SCHRIJF NAMEN */
		strcpy(str, "NAME");
		dna_write(file, str, 4);
		len= nr_names;
		dna_write(file, &len, 4);
		
		/* lengte berekenen datablok met strings */
		cp= names[nr_names-1];
		cp+= strlen(names[nr_names-1]) + 1;			/* +1: nul-terminator */
		len= (long)cp - (long)(names[0]);
		len= (len+3) & ~3;
		dna_write(file, names[0], len);
		
		/* SCHRIJF TYPES */
		strcpy(str, "TYPE");
		dna_write(file, str, 4);
		len= nr_types;
		dna_write(file, &len, 4);
	
		/* lengte berekenen datablok */
		cp= types[nr_types-1];
		cp+= strlen(types[nr_types-1]) + 1;		/* +1: nul-terminator */
		len= (long)cp - (long)(types[0]);
		len= (len+3) & ~3;
		
		dna_write(file, types[0], len);
		
		/* SCHRIJF TYPELENGTES */
		strcpy(str, "TLEN");
		dna_write(file, str, 4);
		
		len= 2*nr_types;
		if(nr_types & 1) len+= 2;
		dna_write(file, typelens, len);
		
		/* SCHRIJF STRUCTEN */
		strcpy(str, "STRC");
		dna_write(file, str, 4);
		len= nr_structs;
		dna_write(file, &len, 4);
	
		/* lengte berekenen datablok */
		sp= structs[nr_structs-1];
		sp+= 2+ 2*( sp[1] );
		len= (long)sp - (long)(structs[0]);
		len= (len+3) & ~3;
		
		dna_write(file, structs[0], len);
	
		/* dna padding test */
		/*
		{
			FILE *fp;
			int a;
			
			fp= fopen("padding.c", "w");
			if(fp==NULL);
			else {

				// add all include files defined in the global array
				for (i = 0; strlen(includefiles[i]); i++) {
					fprintf(fp, "#include \"%s\"\n", includefiles[i]);
				}

				fprintf(fp, "main(){\n");
				sp = typelens;
				sp += firststruct;
				for(a=firststruct; a<nr_types; a++, sp++) { 
					fprintf(fp, "\tprintf(\" ");
					fprintf(fp, "%%d %s %d ", types[a], *sp);
					fprintf(fp, "\\n\",  sizeof(struct %s) - %d);\n", types[a], *sp);
				}
				fprintf(fp, "}\n");
				fclose(fp);
			}
		}
		*/
	}
	
	
	freeN(namedata);
	freeN(typedata);
	freeN(structdata);
	freeN(names);
	freeN(types);
	freeN(typelens);
	freeN(structs);
}

/* ************************* END MAKEN DNA ********************** */


int main(int argc, char ** argv)
{
	FILE *file;
	int return_status = 0;

	if (argc != 2) {
		printf("Usage: %s outfile.c\n", argv[0]);
		return_status = 1;
	} else {
		file = fopen(argv[1], "w");
		if (!file) {
			printf ("Unable to open file: %s\n", argv[1]);
			return_status = 1;
		} else {
			fprintf (file, "char DNAstr[]= {\n");
			make_structDNA (file);

			fprintf(file, "};\n");
			fprintf(file, "int DNAlen= sizeof(DNAstr);\n");
	
			fclose(file);
		}
	}

	return(return_status);
}

