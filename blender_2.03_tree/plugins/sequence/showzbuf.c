/* 
 * Copyright (c) 1999, Not a Number / NeoGeo b.v. 
 * 
 */

#include "plugin.h"

/* ******************** GLOBAL VARIABLES ***************** */


char name[24]= "showzbuf";

/* structure for buttons, 
 *  butcode      name           default  min  max  0
 */

VarStruct varstr[]= {
	{NUMSLI|FLO,	"width ",		1.0,	0.0, 10.0, "This button is obsolete!"}
};

/* The cast struct is for input in the main doit function
   Varstr and Cast must have the same variables in the same order */ 

typedef struct Cast {
	float width;
} Cast;

/* cfra: the current frame */

float cfra;

void plugin_seq_doit(Cast *, float, float, int, int, ImBuf *, ImBuf *, ImBuf *, ImBuf *);

/* ******************** Fixed functions ***************** */

int plugin_seq_getversion(void) 
{
	return B_PLUGIN_VERSION;
}

void plugin_but_changed(int but) 
{
}

void plugin_init()
{
}

void plugin_getinfo(PluginInfo *info)
{
	info->name= name;
	info->nvars= sizeof(varstr)/sizeof(VarStruct);
	info->cfra= &cfra;

	info->varstr= varstr;

	info->init= plugin_init;
	info->seq_doit= (SeqDoit) plugin_seq_doit;
	info->callback= plugin_but_changed;
}

/* ************************************************************
	Show Zbuffer
	
	Demonstration of usage of the 32 bits zbuffer input.
	remember: z-values are not linear...
	
	Z values are only displayed when the input is a Scene-strip
	or when images were saved in IRIZ format.
	
   ************************************************************ */


void plugin_seq_doit(Cast *cast, float facf0, float facf1, int sx, int sy, ImBuf *ibuf1, ImBuf *ibuf2, ImBuf *out, ImBuf *use)
{
	int x, y, a;
	int *rectz;	
	char *rectc;
	
	if(ibuf1) {
		if(ibuf1->zbuf==0) {
			printf("no zbuf\n");
			return;
		}
		
		a= ibuf1->x*ibuf1->y;
		rectz= ibuf1->zbuf;
		rectc= (char *)out->rect;
		
		while(a--) {
			rectc[3]= 255;
			rectc[0]= rectc[1]= rectc[2]= (rectz[0]>>18);
			rectc+= 4;
			rectz++;
		}
	}

}

