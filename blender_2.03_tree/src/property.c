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



/*  property.c   june 2000
 * 
 *  ton roosendaal
 * Version: $Id: property.c,v 1.10 2000/09/28 21:31:26 ton Exp $
 */


#include "blender.h"
#include "game.h"


void free_property(bProperty *prop)
{
	
	if(prop->poin && prop->poin != &prop->data) freeN(prop->poin);
	freeN(prop);
	
}

void free_properties(ListBase *lb)
{
	bProperty *prop;
	
	while(prop= lb->first) {
		remlink(lb, prop);
		free_property(prop);
	}
}

bProperty *copy_property(bProperty *prop)
{
	bProperty *propn;
	
	propn= dupallocN(prop);
	if(prop->poin && prop->poin != &prop->data) {
		propn->poin= dupallocN(prop->poin);
	}
	else propn->poin= &propn->data;
	
	return propn;
}

void copy_properties(ListBase *lbn, ListBase *lbo)
{
	bProperty *prop, *propn;
	
	lbn->first= lbn->last= 0;
	prop= lbo->first;
	while(prop) {
		propn= copy_property(prop);
		addtail(lbn, propn);
		prop= prop->next;
	}
	
	
}

void init_property(bProperty *prop)
{
	/* also use when property changes type */
	
	if(prop->poin && prop->poin != &prop->data) freeN(prop->poin);
	prop->poin= 0;
	
	prop->otype= prop->type;
	prop->data= 0;
	
	switch(prop->type) {
	case PROP_BOOL:
		prop->poin= &prop->data;
		break;
	case PROP_INT:
		prop->poin= &prop->data;
		break;
	case PROP_FLOAT:
		prop->poin= &prop->data;
		break;
	case PROP_STRING:
		prop->poin= callocN(MAX_PROPSTRING, "property string");
		break;
	case PROP_TIME:
		prop->poin= &prop->data;
		break;
	}
}

bProperty *new_property(int type)
{
	bProperty *prop;

	prop= callocN(sizeof(bProperty), "property");
	prop->type= type;

	init_property(prop);
	
	strcpy(prop->name, "prop");
	make_unique_prop_names(prop->name);

	return prop;
}

bProperty *get_property(Object *ob, char *name)
{
	bProperty *prop;
	
	prop= ob->prop.first;
	while(prop) {
		if( strcmp(prop->name, name)==0 ) return prop;
		prop= prop->next;
	}
	return NULL;
}

/* negative: prop is smaller
 * positive: prop is larger
 */
int compare_property(bProperty *prop, char *str)
{
	extern int Gdfra;		/* sector.c */
	int value;
	float fvalue, ftest;
	
	switch(prop->type) {
	case PROP_BOOL:
		if(strcasecmp(str, "true")==0) {
			if(prop->data==1) return 0;
			else return 1;
		}
		else if(strcasecmp(str, "false")==0) {
			if(prop->data==0) return 0;
			else return 1;
		}
		/* no break, do prop_int too! */
		
	case PROP_INT:
		return prop->data - atoi(str);
		break;
	case PROP_FLOAT:
		fvalue= *((float *)&prop->data);
		ftest= atof(str);
		if( fvalue > ftest) return 1;
		else if( fvalue < ftest) return -1;
		return 0;
		break;
	case PROP_STRING:
		return strcmp(prop->poin, str);
		break;
	case PROP_TIME:
		value= (Gdfra - prop->data)/2;
		value-= atoi(str);
		return value;
		break;
	}
	
	return 0;
}

void set_property(bProperty *prop, char *str)
{
	extern int Gdfra;		/* sector.c */

	switch(prop->type) {
	case PROP_BOOL:
		if(strcasecmp(str, "true")==0) prop->data= 1;
		else if(strcasecmp(str, "false")==0) prop->data= 0;
		else prop->data= (atoi(str)!=0);
		break;
	case PROP_INT:
		prop->data= atoi(str);
		break;
	case PROP_FLOAT:
		*((float *)&prop->data)= atof(str);
		break;
	case PROP_STRING:
		strcpy(prop->poin, str);
		break;
	case PROP_TIME:
		prop->data= Gdfra -2*atoi(str);
		break;
	}
	
}

void add_property(bProperty *prop, char *str)
{
	extern int Gdfra;		/* sector.c */

	switch(prop->type) {
	case PROP_BOOL:
	case PROP_INT:
		prop->data+= atoi(str);
		break;
	case PROP_FLOAT:
		*((float *)&prop->data)+= atof(str);
		break;
	case PROP_STRING:
		/* strcpy(prop->poin, str); */
		break;
	case PROP_TIME:
		prop->data+= 2*atoi(str);
		break;
	}
}

/* reads value of property, sets it in chars in str */
void set_property_valstr(bProperty *prop, char *str)
{
	extern int Gdfra;		/* sector.c */

	if(str == NULL) return;

	switch(prop->type) {
	case PROP_BOOL:
	case PROP_INT:
		sprintf(str, "%d", prop->data);
		break;
	case PROP_FLOAT:
		sprintf(str, "%f", *((float *)&prop->data));
		break;
	case PROP_STRING:
		strncpy(str, prop->poin, 31);
		break;
	case PROP_TIME:
		sprintf(str, "%d", (Gdfra - prop->data)/2);
		break;
	}
}

void cp_property(bProperty *prop1, bProperty *prop2)
{
	char str[128];

	set_property_valstr(prop2, str);

	set_property(prop1, str);
}

