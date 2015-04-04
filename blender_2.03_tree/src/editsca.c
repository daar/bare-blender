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



/*  editsca.c   june 2000
 *  
 *  sensor/controller/actuator editing code
 * 
 * 
 *  ton roosendaal
 * Version: $Id: editsca.c,v 1.49 2000/09/28 21:31:26 ton Exp $ 
 */


#include "blender.h"
#include "graphics.h"
#include "interface.h"
#include "sound.h"
#include "game.h"

#define B_DIFF			1
#define B_ADD_PROP	2701
#define B_CHANGE_PROP	2702

#define B_ADD_SENS		2703
#define B_CHANGE_SENS	2704
#define B_DEL_SENS		2705

#define B_ADD_CONT		2706
#define B_CHANGE_CONT	2707
#define B_DEL_CONT		2708

#define B_ADD_ACT		2709
#define B_CHANGE_ACT	2710
#define B_DEL_ACT		2711

#define B_SOUNDACT_BROWSE	2712
#define B_SETSECTOR			2713
#define B_SETPROP			2714
#define B_SETACTOR			2715
#define B_SETMAINACTOR		2716
#define B_SETDYNA			2717

/* internals */
ID **get_selected_and_linked_obs(short *, short);



void del_property(int event)
{
	bProperty *prop;
	Object *ob;
	int a=0;
		
	ob= OBACT;
	if(ob==NULL) return;

	prop= ob->prop.first;
	while(prop) {
		if(a==event) {
			remlink(&ob->prop, prop);
			free_property(prop);
			break;
		}
		a++;
		prop= prop->next;
	}
	allqueue(REDRAWBUTSGAME, 0);
	
}

static int vergname(const void *v1, const void *v2)
{
	char **x1, **x2;
	
	x1= (char **)v1;
	x2= (char **)v2;
	
	return strcmp(*x1, *x2);
}

void make_unique_prop_names(char *str)
{
	Object *ob;
	bProperty *prop;
	bSensor *sens;
	bController *cont;
	bActuator *act;
	ID *id, **idar;
	short a, obcount, propcount=0, nr;
	char **names;
	
	/* this function is called by a Button, and gives the current
	 * stringpointer as an argument, this is the one that can change
	 */

	idar= get_selected_and_linked_obs(&obcount, BUTS_SENS_SEL|BUTS_SENS_ACT|BUTS_ACT_SEL|BUTS_ACT_ACT|BUTS_CONT_SEL|BUTS_CONT_ACT);
	
	/* for each object, make properties and sca names unique */
	
	/* count total names */
	for(a=0; a<obcount; a++) {
		ob= (Object *)idar[a];
		propcount+= countlist(&ob->prop);
		propcount+= countlist(&ob->sensors);
		propcount+= countlist(&ob->controllers);
		propcount+= countlist(&ob->actuators);
	}	
	if(propcount==0) {
		if(idar) freeN(idar);
		return;
	}
	
	/* make names array for sorting */
	names= callocN(propcount*sizeof(void *), "names");

	/* count total names */
	nr= 0;
	for(a=0; a<obcount; a++) {
		ob= (Object *)idar[a];
		prop= ob->prop.first;
		while(prop) {
			names[nr++]= prop->name;
			prop= prop->next;
		}
		sens= ob->sensors.first;
		while(sens) {
			names[nr++]= sens->name;
			sens= sens->next;
		}
		cont= ob->controllers.first;
		while(cont) {
			names[nr++]= cont->name;
			cont= cont->next;
		}
		act= ob->actuators.first;
		while(act) {
			names[nr++]= act->name;
			act= act->next;
		}
	}

	qsort(names, propcount, sizeof(void *), vergname);
	
	/* now we check for double names, and change them */
	
	for(nr=0; nr<propcount; nr++) {
		if(names[nr]!=str && strcmp( names[nr], str )==0 ) {
			newname(str, +1);
		}
	}
	
	freeN(idar);
	freeN(names);
}


void sca_move_sensor(uiBut *but)
{
	int val;
	Base *base;
	bSensor *sens;
	
	val= pupmenu("Move up%x1|Move down %x2");
	
	if(val>0) {
		/* now find out which object has this ... */
		base= FIRSTBASE;
		while(base) {
		
			sens= base->object->sensors.first;
			while(sens) {
				if(sens == (bSensor *)but->poin) break;
				sens= sens->next;
			}
			
			if(sens) {
				if( val==1 && sens->prev) {
					remlink(&base->object->sensors, sens);
					insertlinkbefore(&base->object->sensors, sens->prev, sens);
				}
				else if( val==2 && sens->next) {
					remlink(&base->object->sensors, sens);
					insertlink(&base->object->sensors, sens->next, sens);
				}
				allqueue(REDRAWBUTSGAME, 0);
				break;
			}
			
			base= base->next;
		}
	}
}

void sca_move_controller(uiBut *but)
{
	int val;
	Base *base;
	bController *cont;
	
	val= pupmenu("Move up%x1|Move down %x2");
	
	if(val>0) {
		/* now find out which object has this ... */
		base= FIRSTBASE;
		while(base) {
		
			cont= base->object->controllers.first;
			while(cont) {
				if(cont == (bController *)but->poin) break;
				cont= cont->next;
			}
			
			if(cont) {
				if( val==1 && cont->prev) {
					remlink(&base->object->controllers, cont);
					insertlinkbefore(&base->object->controllers, cont->prev, cont);
				}
				else if( val==2 && cont->next) {
					remlink(&base->object->controllers, cont);
					insertlink(&base->object->controllers, cont->next, cont);
				}
				allqueue(REDRAWBUTSGAME, 0);
				break;
			}
			
			base= base->next;
		}
	}
}
void sca_move_actuator(uiBut *but)
{
	int val;
	Base *base;
	bActuator *act;
	
	val= pupmenu("Move up%x1|Move down %x2");
	
	if(val>0) {
		/* now find out which object has this ... */
		base= FIRSTBASE;
		while(base) {
		
			act= base->object->actuators.first;
			while(act) {
				if(act == (bActuator *)but->poin) break;
				act= act->next;
			}
			
			if(act) {
				if( val==1 && act->prev) {
					remlink(&base->object->actuators, act);
					insertlinkbefore(&base->object->actuators, act->prev, act);
				}
				else if( val==2 && act->next) {
					remlink(&base->object->actuators, act);
					insertlink(&base->object->actuators, act->next, act);
				}
				allqueue(REDRAWBUTSGAME, 0);
				break;
			}
			
			base= base->next;
		}
	}
}

void do_gamebuts(ushort event)
{
	bProperty *prop;
	bSensor *sens;
	bController *cont;
	bActuator *act;
	Base *base;
	Object *ob;
	int val, didit;
	
	ob= OBACT;
	if(ob==0) return;
	
	switch(event) {
		
	case B_ADD_PROP:
		prop= new_property(PROP_FLOAT);
		addtail(&ob->prop, prop);
		allqueue(REDRAWBUTSGAME, 0);
		break;
	
	case B_CHANGE_PROP:
		prop= ob->prop.first;
		while(prop) {
			if(prop->type!=prop->otype) init_property(prop);
			prop= prop->next;
		}
		allqueue(REDRAWBUTSGAME, 0);
		break;
	
	case B_ADD_SENS:
		base= FIRSTBASE;
		while(base) {
			if(base->object->scaflag & OB_ADDSENS) {
				base->object->scaflag &= ~OB_ADDSENS;
				sens= new_sensor(SENS_ALWAYS);
				addtail(&(base->object->sensors), sens);
				base->object->scaflag |= OB_SHOWSENS;
			}
			base= base->next;
		}
		
		allqueue(REDRAWBUTSGAME, 0);
		break;

	case B_CHANGE_SENS:
		base= FIRSTBASE;
		while(base) {
			sens= base->object->sensors.first;
			while(sens) {
				if(sens->type != sens->otype) {
					init_sensor(sens);
					sens->otype= sens->type;
					break;
				}
				sens= sens->next;
			}
			base= base->next;
		}
		allqueue(REDRAWBUTSGAME, 0);
		break;
	
	case B_DEL_SENS:
		base= FIRSTBASE;
		while(base) {
			sens= base->object->sensors.first;
			while(sens) {
				if(sens->flag & SENS_DEL) {
					remlink(&(base->object->sensors), sens);
					free_sensor(sens);
					break;
				}
				sens= sens->next;
			}
			base= base->next;
		}
		allqueue(REDRAWBUTSGAME, 0);
		break;
	
	case B_ADD_CONT:
		base= FIRSTBASE;
		while(base) {
			if(base->object->scaflag & OB_ADDCONT) {
				base->object->scaflag &= ~OB_ADDCONT;
				cont= new_controller(CONT_LOGIC_AND);
				base->object->scaflag |= OB_SHOWCONT;
				addtail(&(base->object->controllers), cont);
			}
			base= base->next;
		}
		allqueue(REDRAWBUTSGAME, 0);
		break;

	case B_CHANGE_CONT:
		base= FIRSTBASE;
		while(base) {
			cont= base->object->controllers.first;
			while(cont) {
				if(cont->type != cont->otype) {
					init_controller(cont);
					cont->otype= cont->type;
					break;
				}
				cont= cont->next;
			}
			base= base->next;
		}
		allqueue(REDRAWBUTSGAME, 0);
		break;
	

	case B_DEL_CONT:
		base= FIRSTBASE;
		while(base) {
			cont= base->object->controllers.first;
			while(cont) {
				if(cont->flag & CONT_DEL) {
					remlink(&(base->object->controllers), cont);
					free_controller(cont);
					break;
				}
				cont= cont->next;
			}
			base= base->next;
		}
		allqueue(REDRAWBUTSGAME, 0);
		break;
	
	case B_ADD_ACT:
		base= FIRSTBASE;
		while(base) {
			if(base->object->scaflag & OB_ADDACT) {
				base->object->scaflag &= ~OB_ADDACT;
				act= new_actuator(ACT_OBJECT);
				addtail(&(base->object->actuators), act);
				base->object->scaflag |= OB_SHOWACT;
			}
			base= base->next;
		}
		allqueue(REDRAWBUTSGAME, 0);
		break;

	case B_CHANGE_ACT:
		base= FIRSTBASE;
		while(base) {
			act= base->object->actuators.first;
			while(act) {
				if(act->type != act->otype) {
					init_actuator(act);
					act->otype= act->type;
					break;
				}
				act= act->next;
			}
			base= base->next;
		}
		allqueue(REDRAWBUTSGAME, 0);
		break;

	case B_DEL_ACT:
		base= FIRSTBASE;
		while(base) {
			act= base->object->actuators.first;
			while(act) {
				if(act->flag & ACT_DEL) {
					remlink(&(base->object->actuators), act);
					free_actuator(act);
					break;
				}
				act= act->next;
			}
			base= base->next;
		}
		allqueue(REDRAWBUTSGAME, 0);
		break;
	
	case B_SOUNDACT_BROWSE:
		/* since we don't know which... */
		didit= 0;
		base= FIRSTBASE;
		while(base) {
			act= base->object->actuators.first;
			while(act) {
				if(act->type==ACT_SOUND) {
					bSoundActuator *sa= act->data;
					if(sa->sndnr) {
						bSound *sound= G.main->sound.first;
						int nr= 1;

						while(sound) {
							if(nr==sa->sndnr) break;
							nr++;
							sound= sound->id.next;
						}
						
						if(sa->sound) sa->sound->id.us--;
						sa->sound= sound;
						if(sound) sound->id.us++;
						
						sa->sndnr= 0;
						didit= 1;
					}
				}
				act= act->next;
			}
			if(didit) break;
			base= base->next;
		}
		allqueue(REDRAWBUTSGAME, 0);
		allqueue(REDRAWSOUND, 0);

		break;

	case B_SETSECTOR:
		/* check for inconsistant types */
		ob->gameflag &= ~(OB_PROP|OB_MAINACTOR|OB_DYNAMIC|OB_ACTOR);
		ob->dtx |= OB_BOUNDBOX;
		allqueue(REDRAWBUTSGAME, 0);
		allqueue(REDRAWVIEW3D, 0);
		break;
		
	case B_SETPROP:
		/* check for inconsistant types */
		ob->gameflag &= ~(OB_SECTOR|OB_MAINACTOR|OB_DYNAMIC|OB_ACTOR);
		allqueue(REDRAWBUTSGAME, 0);
		allqueue(REDRAWVIEW3D, 0);
		break;

	case B_SETACTOR:
	case B_SETDYNA:
	case B_SETMAINACTOR:
		ob->gameflag &= ~(OB_SECTOR|OB_PROP);
		allqueue(REDRAWBUTSGAME, 0);
		allqueue(REDRAWVIEW3D, 0);
		break;

	}
}


char *sensor_name(int type)
{
	switch (type) {
	case SENS_ALWAYS:
		return "Always";
	case SENS_TOUCH:
		return "Touch";
	case SENS_NEAR:
		return "Near";
	case SENS_KEYBOARD:
		return "Keyboard";
	case SENS_PROPERTY:
		return "Property";
	case SENS_MOUSE:
		return "Mouse";
	case SENS_COLLISION:
		return "Collision";
	case SENS_RADAR:
		return "Radar";
	}
	return "unknown";
}

char *sensor_pup()
{
	return "Sensors %t|Always %x0|Keyboard %x3|Touch %x1|Collision %x6|Near %x2|Radar %x7|Property %x4";
}

char *controller_name(int type)
{
	switch (type) {
	case CONT_LOGIC_AND:
		return "AND";
	case CONT_LOGIC_OR:
		return "OR";
	case CONT_EXPRESSION:
		return "Expression";
	case CONT_PYTHON:
		return "Python";
	}
	return "unknown";
}

char *controller_pup()
{
	return "Controllers   %t|AND %x0|OR %x1|Expression %x2|Python %x3";
}

char *actuator_name(int type)
{
	switch (type) {
	case ACT_OBJECT:
		return "Object";
	case ACT_IPO:
		return "Ipo";
	case ACT_LAMP:
		return "Lamp";
	case ACT_CAMERA:
		return "Camera";
	case ACT_MATERIAL:
		return "Material";
	case ACT_SOUND:
		return "Sound";
	case ACT_PROPERTY:
		return "Property";
	case ACT_EDIT_OBJECT:
		return "Edit Object";
	case ACT_CONSTRAINT:
		return "Constraint";
	case ACT_SCENE:
		return "Scene";
	case ACT_GROUP:
		return "Group";
	}
	return "unknown";
}

char *actuator_pup()
{
	return "Actuators  %t|Object %x0|Constraint %x9|Ipo %x1|Camera %x3|Sound %x5|Property %x6|Edit Object %x10|Scene %x11|Group %x12";
}

void set_sca_ob(Object *ob)
{
	bSensor *sens;
	bController *cont;
	bActuator *act;

	cont= ob->controllers.first;
	while(cont) {
		cont->mynew= (bController *)ob;
		cont= cont->next;
	}
	act= ob->actuators.first;
	while(act) {
		act->mynew= (bActuator *)ob;
		act= act->next;
	}
}

ID **get_selected_and_linked_obs(short *count, short scavisflag)
{
	Base *base;
	Object *ob, *obt;
	ID **idar;
	bSensor *sens;
	bController *cont;
	bActuator *act;
	uint lay;
	int a, nr, doit;
	
	/* we need a sorted object list */
	/* set scavisflags flags in Objects to indicate these should be evaluated */
	/* also hide ob pointers in ->new entries of controllerss/actuators */
	
	*count= 0;
	
	if(G.scene==NULL) return NULL;
	
	ob= G.main->object.first;
	while(ob) {
		ob->scavisflag= 0;
		set_sca_ob(ob);
		ob= ob->id.next;
	}
	
	if(G.vd) lay= G.vd->lay;
	else lay= G.scene->lay;
	
	base= FIRSTBASE;
	while(base) {
		if(base->lay & lay) {
			if(base->flag & SELECT) {
				if(scavisflag & BUTS_SENS_SEL) base->object->scavisflag |= OB_VIS_SENS;
				if(scavisflag & BUTS_CONT_SEL) base->object->scavisflag |= OB_VIS_CONT;
				if(scavisflag & BUTS_ACT_SEL) base->object->scavisflag |= OB_VIS_ACT;
			}
		}
		base= base->next;
	}

	if(OBACT) {
		if(scavisflag & BUTS_SENS_ACT) OBACT->scavisflag |= OB_VIS_SENS;
		if(scavisflag & BUTS_CONT_ACT) OBACT->scavisflag |= OB_VIS_CONT;
		if(scavisflag & BUTS_ACT_ACT) OBACT->scavisflag |= OB_VIS_ACT;
	}
	
	if(scavisflag & (BUTS_SENS_LINK|BUTS_CONT_LINK|BUTS_ACT_LINK)) {
		doit= 1;
		while(doit) {
			doit= 0;
			
			ob= G.main->object.first;
			while(ob) {
			
				/* 1st case: select sensor when controller selected */
				if((scavisflag & BUTS_SENS_LINK) && (ob->scavisflag & OB_VIS_SENS)==0) {
					sens= ob->sensors.first;
					while(sens) {
						for(a=0; a<sens->totlinks; a++) {
							if(sens->links[a]) {
								obt= (Object *)sens->links[a]->mynew;
								if(obt && (obt->scavisflag & OB_VIS_CONT)) {
									doit= 1;
									ob->scavisflag |= OB_VIS_SENS;
									break;
								}
							}
						}
						if(doit) break;
						sens= sens->next;
					}
				}
				
				/* 2nd case: select cont when act selected */
				if((scavisflag & BUTS_CONT_LINK)  && (ob->scavisflag & OB_VIS_CONT)==0) {
					cont= ob->controllers.first;
					while(cont) {
						for(a=0; a<cont->totlinks; a++) {
							if(cont->links[a]) {
								obt= (Object *)cont->links[a]->mynew;
								if(obt && (obt->scavisflag & OB_VIS_ACT)) {
									doit= 1;
									ob->scavisflag |= OB_VIS_CONT;
									break;
								}
							}
						}
						if(doit) break;
						cont= cont->next;
					}
				}
				
				/* 3rd case: select controller when sensor selected */
				if((scavisflag & BUTS_CONT_LINK) && (ob->scavisflag & OB_VIS_SENS)) {
					sens= ob->sensors.first;
					while(sens) {
						for(a=0; a<sens->totlinks; a++) {
							if(sens->links[a]) {
								obt= (Object *)sens->links[a]->mynew;
								if(obt && (obt->scavisflag & OB_VIS_CONT)==0) {
									doit= 1;
									obt->scavisflag |= OB_VIS_CONT;
								}
							}
						}
						sens= sens->next;
					}
				}
				
				/* 4th case: select actuator when controller selected */
				if( (scavisflag & BUTS_ACT_LINK)  && (ob->scavisflag & OB_VIS_CONT)) {
					cont= ob->controllers.first;
					while(cont) {
						for(a=0; a<cont->totlinks; a++) {
							if(cont->links[a]) {
								obt= (Object *)cont->links[a]->mynew;
								if(obt && (obt->scavisflag & OB_VIS_ACT)==0) {
									doit= 1;
									obt->scavisflag |= OB_VIS_ACT;
								}
							}
						}
						cont= cont->next;
					}
					
				}
				ob= ob->id.next;
			}
		}
	} 
	
	/* now we count */
	ob= G.main->object.first;
	while(ob) {
		if(ob->id.lib==NULL) {
			if( ob->scavisflag ) (*count)++;
		}
		ob= ob->id.next;
	}

	if(*count==0) return NULL;
	if(*count>24) *count= 24;		/* temporal */
	
	idar= callocN( (*count)*sizeof(void *), "idar");
	
	ob= G.main->object.first;
	nr= 0;
	while(ob) {
		if(ob->id.lib==NULL) {
			if( ob->scavisflag ) {
				idar[nr]= (ID *)ob;
				nr++;
			}
		}
		if(nr>=24) break;
		ob= ob->id.next;
	}
	
	/* just to be sure... these were set in set_sca_done_ob() */
	clear_sca_new_poins();
	
	return idar;
}


void set_key_event(uiBut *but)
{
	int loop=1;
	ushort event;
	short val;
	char *str;
	
	strcpy(but->str, "Press any key");
	uiCheckBut(but);
	uiDrawBut(but);
	
	while(loop) {
		
		event= extern_qread(&val);
		if(event && val && event!=MOUSEY && event!=MOUSEX && event!=KEYBD) {
			str= key_event_to_string(event);
			if(str[0]) {
				strcpy(but->drawstr, str);
				uiSetButVal(but, (double)event);
			}
			else {
				uiSetButVal(but, 0.0);
				strcpy(but->str, "");
			}
			loop= 0;
			uiCheckBut(but);
		}	
	}
}

uint get_col_sensor(int type, int medium)
{
	extern uiCol UIcol[];
	
	if(medium) {
		switch(type) {
		case SENS_ALWAYS:
			return(UIcol[BUTGREY].medium);
		case SENS_TOUCH:
			return(UIcol[BUTGREEN].medium);
		case SENS_COLLISION:
			return(UIcol[BUTYELLOW].medium);
		case SENS_NEAR:
			return(UIcol[BUTPURPLE].medium);
		case SENS_KEYBOARD:
			return(UIcol[BUTYELLOW].medium);
		case SENS_PROPERTY:
			return(UIcol[BUTBLUE].medium);
		case SENS_MOUSE:
			return(UIcol[BUTSALMON].medium);
		case SENS_RADAR:
			return(UIcol[BUTPURPLE].medium);
		default:
			return(UIcol[BUTGREY].medium);
		}
	}
	else {
		switch(type) {
		case SENS_ALWAYS:
			return(UIcol[BUTGREY].grey);
		case SENS_TOUCH:
			return(UIcol[BUTGREEN].grey);
		case SENS_COLLISION:
			return(UIcol[BUTYELLOW].grey);
		case SENS_NEAR:
			return(UIcol[BUTPURPLE].grey);
		case SENS_KEYBOARD:
			return(UIcol[BUTYELLOW].grey);
		case SENS_PROPERTY:
			return(UIcol[BUTBLUE].grey);
		case SENS_MOUSE:
			return(UIcol[BUTSALMON].grey);
		case SENS_RADAR:
			return(UIcol[BUTPURPLE].grey);
		default:
			return(UIcol[BUTGREY].grey);
		}
	}
}

short draw_sensorbuttons(bSensor *sens, uiBlock *block, short xco, short yco, short width)
{
	extern uiCol UIcol[];
	extern void test_matpoin_but();
	extern void test_obpoin_but();
	bNearSensor *ns=NULL;
	bTouchSensor *ts=NULL;
	bKeyboardSensor *ks=NULL;
	bPropertySensor *ps=NULL;
	bMouseSensor *ms=NULL;
	bCollisionSensor *cs=NULL;
	bRadarSensor *rs=NULL;
	uiBut *but;
	uint col;
	short ysize;
	char *str;
	
	/* yco is at the top of the rect, draw downwards */
	
	block->dt= UI_EMBOSSM;
	
	col= get_col_sensor(sens->type, 0);
	cpack(col);
	
	switch (sens->type) {
	case SENS_ALWAYS:
		ysize= 24;

		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		block->col= BUTGREEN;
		uiDefBut(block, TOG|SHO, 1, "ICON 0 2 3", xco, yco-22, 30, 19, &sens->pulse, 1.0, 0.0, 0, 0, "Set pulse mode");
		uiDefBut(block, NUM|SHO, 1, "f:", xco+30, yco-22, 60, 19, &sens->freq, 0.0, 250.0, 0, 0, "Frequency of pulse (in 1/50 sec)");

		yco-= ysize;
		
		break;
	case SENS_TOUCH:
		ysize= 28;

		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		ts= sens->data;
		
			/* uiDefBut(block, TEX, 1, "Property:",	xco,yco-22,width, 19, &ts->name, 0, 31, 0, 0, "Only look for Objects with this property"); */
		but= uiDefBut(block, IDPOIN, 1, "MA:",	xco,yco-22,width, 19, &ts->ma, 0, 31, 0, 0, "Only look for floors with this Material");
		but->func= (test_matpoin_but);
			/* uiDefBut(block, NUM|FLO, 1, "Margin:",	xco+width/2,yco-44,width/2, 19, &ts->dist, 0.0, 10.0, 100, 0, "Extra margin (distance) for larger sensitivity"); */

		yco-= ysize;
		break;

	case SENS_COLLISION:
		ysize= 28;

		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		cs= sens->data;
		
		uiDefBut(block, TEX, 1, "Property:",	xco+10,yco-24,width-90, 19, &cs->name, 0, 31, 0, 0, "Only look for Objects with this property");
		uiDefBut(block, NUM|SHO, 1, "Damp:",	xco+10+width-90,yco-24, 70, 19, &cs->damp, 0, 250, 0, 0, "For 'damp' time don't detect another collision");

		yco-= ysize;
		break;
		
	case SENS_NEAR:
		ysize= 54;

		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		ns= sens->data;

		uiDefBut(block, TEX, 1, "Property:",	10+xco,yco-22, width-20, 19, &ns->name, 0, 31, 0, 0, "Only look for Objects with this property");
		uiDefBut(block, NUM|FLO, 1, "Dist",		10+xco, yco-44, (width-22)/2, 19, &ns->dist, 0.0, 100.0, 100, 0, "Trigger distance");
		uiDefBut(block, NUM|FLO, 1, "Reset",	10+xco+(width-22)/2, yco-44, (width-22)/2, 19, &ns->resetdist, 0.0, 100.0, 100, 0, "Reset distance");
		yco-= ysize;
		break;

	case SENS_RADAR:
		ysize= 54;

		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		rs= sens->data;

		uiDefBut(block, TEX, 1, "Property:",	10+xco,yco-22, width-20, 19, &rs->name, 0, 31, 0, 0, "Only look for Objects with this property");
		uiDefBut(block, NUM|FLO, 1, "Angle",	10+xco, yco-44, (width-22)/2, 19, &rs->angle, 0.0, 180.0, 10, 0, "View angle");
		uiDefBut(block, ROW|SHO, 1, "X",		10+xco+(width-22)/2, yco-44, (width-22)/6, 19, &rs->axis, 2.0, 0.0, 0, 0, "");
		uiDefBut(block, ROW|SHO, 1, "Y",		10+xco+(width-22)/2 + (width-22)/6, yco-44, (width-22)/6, 19, &rs->axis, 2.0, 1.0, 0, 0, "");
		uiDefBut(block, ROW|SHO, 1, "Z",		10+xco+(width-22)/2 + 2*(width-22)/6, yco-44, (width-22)/6, 19, &rs->axis, 2.0, 2.0, 0, 0, "");
		yco-= ysize;
		break;

	case SENS_KEYBOARD:
		ysize= 46;

		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		ks= sens->data;
		
		str= key_event_to_string(ks->key);
		but= uiDefBut(block, BUT|SHO, 0, str, xco+40, yco-24, (width)/2, 19, &ks->key, 0.0, 100.0, 100, 0, "Key code");
		but->butfunc= set_key_event;
		str= key_event_to_string(ks->qual);
		but= uiDefBut(block, BUT|SHO, 0, str, xco+40, yco-44, (width-50)/2, 19, &ks->qual, 0.0, 100.0, 100, 0, "Modifier key code");
		but->butfunc= set_key_event;
		str= key_event_to_string(ks->qual2);
		but= uiDefBut(block, BUT|SHO, 0, str, xco+40+(width-50)/2, yco-44, (width-50)/2, 19, &ks->qual2, 0.0, 100.0, 100, 0, "Second modifier key code");
		but->butfunc= set_key_event;
		
		uiDefBut(block, LABEL, 0, "Key",	  xco, yco-24, 40, 19, NULL, 0, 0, 0, 0, "");
		uiDefBut(block, LABEL, 0, "Hold",	  xco, yco-44, 40, 19, NULL, 0, 0, 0, 0, "");
		block->col= BUTPURPLE;
		uiDefBut(block, TOG|SHO|BIT|0, 0, "All keys",	  xco+40+(width/2), yco-24, (width/2)-50, 19, &ks->type, 0, 0, 0, 0, "");

		yco-= ysize;
		break;
		
	case SENS_PROPERTY:
		ysize= 68;

		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		ps= sens->data;
		
		str= "Type %t|Equal %x0|Not Equal %x1|Interval %x2|Changed %x3";
		uiDefBut(block, MENU|INT, 1, str,			xco+30,yco-24,width-60, 19, &ps->type, 0, 31, 0, 0, "Type");
         
		if (ps->type != SENS_PROP_EXPRESSION)
		{
			uiDefBut(block, TEX, 1, "Prop: ",			xco+30,yco-44,width-60, 19, ps->name, 0, 31, 0, 0, "Property name");
		}
		
        if(ps->type == SENS_PROP_INTERVAL) {
			uiDefBut(block, TEX, 1, "Min: ",		xco,yco-64,width/2, 19, ps->value, 0, 31, 0, 0, "test for value");
			uiDefBut(block, TEX, 1, "Max: ",		xco+width/2,yco-64,width/2, 19, ps->maxvalue, 0, 31, 0, 0, "test for max value");
        }
        else if(ps->type == SENS_PROP_CHANGED);
		else {
			uiDefBut(block, TEX, 1, "Value: ",		xco+30,yco-64,width-60, 19, ps->value, 0, 31, 0, 0, "test for value");
        }
				
		yco-= ysize;
		break;
		
	case SENS_MOUSE:
		ysize= 28;

		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		ms= sens->data;

		uiDefBut(block, ROW|SHO, 1, "Left", xco, yco-24, (width)/3, 19, &ms->type, 2.0, LEFTMOUSE, 0, 0, "Trigger on LeftMouse");
		uiDefBut(block, ROW|SHO, 1, "Middle", xco+width/3, yco-24, (width)/3, 19, &ms->type, 2.0, MIDDLEMOUSE, 0, 0, "Trigger on MiddleMouse");
		uiDefBut(block, ROW|SHO, 1, "Right", xco+2*width/3, yco-24, (width)/3, 19, &ms->type, 2.0, RIGHTMOUSE, 0, 0, "Trigger on RightMouse");

		yco-= ysize;
		break;
	}
	
	block->dt= UI_EMBOSSM;
	block->col= BUTGREY;

	return yco-4;
}

short draw_controllerbuttons(bController *cont, uiBlock *block, short xco, short yco, short width)
{
	extern uiCol UIcol[];
	extern void test_scriptpoin_but();
	uiBut *but;
	bExpressionCont *ec;
	bPythonCont *pc;
	short ysize, col;
	
	block->dt= UI_EMBOSSM;
	
	switch (cont->type) {
	case CONT_EXPRESSION:
		col= BUTGREY;
		ysize= 24;

		cpack(UIcol[col].grey);
		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		ec= cont->data;	
		uiDefBut(block, LABEL, 1, "Not yet...", xco,yco-24,80, 19, NULL, 0, 0, 0, 0, "");
		/* uiDefBut(block, BUT, 1, "Variables", xco,yco-24,80, 19, NULL, 0, 0, 0, 0, "Available variables for expression"); */
		/* uiDefBut(block, TEX, 1, "Exp:",		xco,yco-44,width-22, 19, ec->str, 0, 127, 0, 0, "Expression"); */
		
		yco-= ysize;
		break;
	case CONT_PYTHON:
		col= BUTGREEN;
		ysize= 28;
		
		if(cont->data==NULL) init_controller(cont);
		pc= cont->data;
		
		cpack(UIcol[col].grey);
		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);

		uiDefBut(block, LABEL, 1, "Not yet...", xco,yco-24,80, 19, NULL, 0, 0, 0, 0, "");
		/* but= uiDefBut(block, IDPOIN, 1, "TEXT:", xco+45,yco-24,width-90, 19, &pc->text, 0, 0, 0, 0, ""); */
		/* but->func= test_scriptpoin_but; */
		
		yco-= ysize;
		break;
		
	default:
		col= BUTGREY;
		ysize= 4;

		cpack(UIcol[col].grey);
		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		yco-= ysize;
	}
	
	block->dt= UI_EMBOSSM;
	block->col= col;

	return yco;
}

uint get_col_actuator(int type, int medium)
{
	extern uiCol UIcol[];

	if(medium) {
		switch(type) {
		case ACT_OBJECT:
			return(UIcol[BUTGREEN].medium);
		case ACT_IPO:
			return(UIcol[BUTYELLOW].medium);
		case ACT_PROPERTY:
			return(UIcol[BUTBLUE].medium);
		case ACT_SOUND:
			return(UIcol[BUTSALMON].medium);
		case ACT_CAMERA:
			return(UIcol[BUTGREY].medium);
		case ACT_EDIT_OBJECT:
			return(UIcol[BUTPURPLE].medium);
		case ACT_CONSTRAINT:
			return(UIcol[BUTRUST].hilite);
		case ACT_GROUP:
			return(UIcol[BUTYELLOW].medium);
		default:
			return(UIcol[BUTGREY].medium);
		}
	}
	else {
		switch(type) {
		case ACT_OBJECT:
			return(UIcol[BUTGREEN].grey);
		case ACT_IPO:
			return(UIcol[BUTYELLOW].grey);
		case ACT_PROPERTY:
			return(UIcol[BUTBLUE].grey);
		case ACT_SOUND:
			return(UIcol[BUTSALMON].grey);
		case ACT_CAMERA:
			return(UIcol[BUTGREY].grey);
		case ACT_EDIT_OBJECT:
			return(UIcol[BUTPURPLE].grey);
		case ACT_CONSTRAINT:
			return(UIcol[BUTRUST].medium);
		case ACT_GROUP:
			return(UIcol[BUTYELLOW].grey);
		default:
			return(UIcol[BUTGREY].grey);
		}
	}
}


short draw_actuatorbuttons(bActuator *act, uiBlock *block, short xco, short yco, short width)
{
	extern uiCol UIcol[];
	extern void test_obpoin_but();
	extern void test_meshpoin_but();
	extern void test_scenepoin_but();
	bSoundActuator *sa=NULL;
	bObjectActuator *oa=NULL;
	bIpoActuator *ia=NULL;
	bPropertyActuator *pa= NULL;
	bCameraActuator *ca= NULL;
	bEditObjectActuator *eoa= NULL;
	bConstraintActuator *coa= NULL;
	bSceneActuator *sca= NULL;
	bGroupActuator *ga= NULL;
	uiBut *but;
	float *fp;
	uint col;
	short ysize, wval;
	char *str;
	
	/* yco is at the top of the rect, draw downwards */
	
	block->dt= UI_EMBOSSM;
	col= get_col_actuator(act->type, 0);
	cpack(col);
	
	switch (act->type) {
	case ACT_OBJECT:
		ysize= 90;

		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		oa= act->data;
		wval= (width-60)/3;
		
		block->col= BUTGREY;
		
		uiDefBut(block, LABEL, 0, "Force",	xco, yco-24, 55, 19, NULL, 0, 0, 0, 0, "");
		uiDefBut(block, NUM|FLO, 0, "",		xco+45, yco-24, wval, 19, oa->forceloc, -100.0, 100.0, 10, 0, "");
		uiDefBut(block, NUM|FLO, 0, "",		xco+45+wval, yco-24, wval, 19, oa->forceloc+1, -100.0, 100.0, 10, 0, "");
		uiDefBut(block, NUM|FLO, 0, "",		xco+45+2*wval, yco-24, wval, 19, oa->forceloc+2, -100.0, 100.0, 10, 0, "");
		
		uiDefBut(block, LABEL, 0, "Torque", xco, yco-44, 55, 19, NULL, 0, 0, 0, 0, "");
		uiDefBut(block, NUM|FLO, 0, "",		xco+45, yco-44, wval, 19, oa->forcerot, -100.0, 100.0, 10, 0, "");
		uiDefBut(block, NUM|FLO, 0, "",		xco+45+wval, yco-44, wval, 19, oa->forcerot+1, -100.0, 100.0, 10, 0, "");
		uiDefBut(block, NUM|FLO, 0, "",		xco+45+2*wval, yco-44, wval, 19, oa->forcerot+2, -100.0, 100.0, 10, 0, "");

		uiDefBut(block, LABEL, 0, "dLoc",	xco, yco-70, 45, 19, NULL, 0, 0, 0, 0, "");
		uiDefBut(block, NUM|FLO, 0, "",		xco+45, yco-70, wval, 19, oa->dloc, -100.0, 100.0, 10, 0, "");
		uiDefBut(block, NUM|FLO, 0, "",		xco+45+wval, yco-70, wval, 19, oa->dloc+1, -100.0, 100.0, 10, 0, "");
		uiDefBut(block, NUM|FLO, 0, "",		xco+45+2*wval, yco-70, wval, 19, oa->dloc+2, -100.0, 100.0, 10, 0, "");

		uiDefBut(block, LABEL, 0, "dRot",	xco, yco-90, 45, 19, NULL, 0, 0, 0, 0, "");
		uiDefBut(block, NUM|FLO, 0, "",		xco+45, yco-90, wval, 19, oa->drot, -100.0, 100.0, 10, 0, "");
		uiDefBut(block, NUM|FLO, 0, "",		xco+45+wval, yco-90, wval, 19, oa->drot+1, -100.0, 100.0, 10, 0, "");
		uiDefBut(block, NUM|FLO, 0, "",		xco+45+2*wval, yco-90, wval, 19, oa->drot+2, -100.0, 100.0, 10, 0, "");
		
		block->col= BUTGREEN;
		uiDefBut(block, TOG|INT|BIT|0, 0, "L",		xco+45+3*wval, yco-24, 15, 19, &oa->flag, 0.0, 0.0, 0, 0, "Local transformation");
		uiDefBut(block, TOG|INT|BIT|1, 0, "L",		xco+45+3*wval, yco-44, 15, 19, &oa->flag, 0.0, 0.0, 0, 0, "Local transformation");
		uiDefBut(block, TOG|INT|BIT|2, 0, "L",		xco+45+3*wval, yco-70, 15, 19, &oa->flag, 0.0, 0.0, 0, 0, "Local transformation");
		uiDefBut(block, TOG|INT|BIT|3, 0, "L",		xco+45+3*wval, yco-90, 15, 19, &oa->flag, 0.0, 0.0, 0, 0, "Local transformation");
		
		yco-= ysize;
		break;
		
	case ACT_IPO:
		ia= act->data;

		if(ia->type==ACT_IPO_KEY2KEY) ysize= 72; 
		else ysize= 52;

		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		str= "Ipo types   %t|Play %x0|Ping Pong %x1|Flipper %x2|Loop stop %x3|Loop End %x4|Key 2 Key %x5|Property %x6";

		uiDefBut(block, MENU|SHO, 1, str,		xco+20, yco-24, width-40, 19, &ia->type, 0, 0, 0, 0, "");
		if(ia->type==ACT_IPO_KEY2KEY) {
			block->col= BUTGREEN;

			uiDefBut(block, TOG|SHO|BIT|0, 0, "Prev", xco+20, yco-44, (width-40)/3, 19, &ia->flag, 0, 0, 0, 0, "Play backwards");
			uiDefBut(block, TOG|SHO|BIT|1, 0, "Cycl", xco+20+(width-40)/3, yco-44, (width-40)/3, 19, &ia->flag, 0, 0, 0, 0, "Play cyclic");
			uiDefBut(block, TOG|SHO|BIT|3, 0, "Hold", xco+20+2*(width-40)/3, yco-44, (width-40)/3, 19, &ia->flag, 0, 0, 0, 0, "Keep playing while activated");
			
			uiDefBut(block, TEX, 0, "Prop: ",		xco+20, yco-66, width-40, 19, ia->name, 0.0, 31.0, 0, 0, "Set property to key position");
		}
		else if(ia->type==ACT_IPO_FROM_PROP) {
			uiDefBut(block, TEX, 0, "Prop: ",		xco+20, yco-44, width-40, 19, ia->name, 0.0, 31.0, 0, 0, "Use this property to define the Ipo position");
		}
		else {
			uiDefBut(block, NUM|SHO, 0, "Sta",		xco+20, yco-44, (width-90)/2, 19, &ia->sta, 0.0, 2500.0, 0, 0, "Start frame");
			uiDefBut(block, NUM|SHO, 0, "End",		xco+20+(width-90)/2, yco-44, (width-90)/2, 19, &ia->end, 0.0, 2500.0, 0, 0, "End frame");
			
			block->col= BUTGREEN;
			uiDefBut(block, TOG|SHO|BIT|0, 0, "Force", xco+width-70, yco-44, 50, 19, &ia->flag, 0, 0, 0, 0, "Convert Ipo to force");
		}
		yco-= ysize;
		break;
		
	case ACT_PROPERTY:
		ysize= 68;
        
		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		pa= act->data;

		str= "Type   %t|Assign   %x0|Add %x1|Copy %x2";
		uiDefBut(block, MENU|INT, 1, str,		xco+30,yco-24,width-60, 19, &pa->type, 0, 31, 0, 0, "Type");
        
		uiDefBut(block, TEX, 1, "Prop: ",		xco+30,yco-44,width-60, 19, pa->name, 0, 31, 0, 0, "Property name");

		if(pa->type==ACT_PROP_COPY) {
			but= uiDefBut(block, IDPOIN, 0, "OB:",	xco+10, yco-64, (width-20)/2, 19, &(pa->ob), 0, 0, 0, 0, "Copy from this Object");
			but->func= (test_obpoin_but);
	        uiDefBut(block, TEX, 1, "Prop: ",		xco+10+(width-20)/2, yco-64, (width-20)/2, 19, pa->value, 0, 31, 0, 0, "Copy this property");
		}
		else {
	        uiDefBut(block, TEX, 1, "Value: ",		xco+30,yco-64,width-60, 19, pa->value, 0, 31, 0, 0, "change with this value");
		}
		yco-= ysize;
        
		break;
    case ACT_SOUND:
		ysize= 34;
		
		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);

		sa= act->data;
		sa->sndnr= 0;

		IDnames_to_pupstring_title("Sound files", &str, &(G.main->sound), (ID *)sa->sound, &(sa->sndnr));
		if(str[0]) {
			/* reset this value, it is for handling the event */
			sa->sndnr= 0;
			uiDefBut(block, MENU|SHO, B_SOUNDACT_BROWSE, str, xco+10,yco-24,20,19, &(sa->sndnr), 0, 0, 0, 0, "");
			
			if(sa->sound) 
				uiDefBut(block, TEX, B_IDNAME, "SO:",	xco+30,yco-24,width-40,19, sa->sound->id.name+2, 0.0, 18.0, 0, 0, "");

		}
		else {
			uiDefBut(block, LABEL, 0, "Use Sound window to load files", xco, yco-24, width, 19, NULL, 0, 0, 0, 0, "");
		}

		freeN(str);

		yco-= ysize;
		
		break;
		
	case ACT_CAMERA:
	
		ysize= 48;
        
		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		ca= act->data;
        
		but= uiDefBut(block, IDPOIN, 0, "OB:",		xco+10, yco-24, (width-20)/2, 19, &(ca->ob), 0, 0, 0, 0, "Look at this Object");
		but->func= (test_obpoin_but);
		uiDefBut(block, NUM|FLO, 0, "Height:",	xco+10+(width-20)/2, yco-24, (width-20)/2, 19, &ca->height, 0.0, 20.0, 0, 0, "");
		
		uiDefBut(block, NUM|FLO, 0, "Min:",	xco+10, yco-44, (width-60)/2, 19, &ca->min, 0.0, 20.0, 0, 0, "");
		
		if(ca->axis==0) ca->axis= 'x';
		uiDefBut(block, ROW|SHO, 0, "X",	xco+10+(width-60)/2, yco-44, 20, 19, &ca->axis, 4.0, (float)'x', 0, 0, "Camera tries to get behind the X axis");
		uiDefBut(block, ROW|SHO, 0, "Y",	xco+30+(width-60)/2, yco-44, 20, 19, &ca->axis, 4.0, (float)'y', 0, 0, "Camera tries to get behind the Y axis");
		
		uiDefBut(block, NUM|FLO, 0, "Max:",	xco+20+(width)/2, yco-44, (width-60)/2, 19, &ca->max, 0.0, 20.0, 0, 0, "");

		yco-= ysize;
        
        break;
 		
	case ACT_EDIT_OBJECT:
		
		eoa= act->data;

		if(eoa->type==ACT_EDOB_ADD_OBJECT) {
			ysize= 48;
			glRects(xco, yco-ysize, xco+width, yco);
			uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
	 
			but= uiDefBut(block, IDPOIN, 0, "OB:",		xco+10, yco-44, (width-20)/2, 19, &(eoa->ob), 0, 0, 0, 0, "Add this Object");
			but->func= (test_obpoin_but);
			uiDefBut(block, NUM|INT, 0, "Time:",	xco+10+(width-20)/2, yco-44, (width-20)/2, 19, &eoa->time, 0.0, 2000.0, 0, 0, "Duration the new Object lives");
		}
		else if(eoa->type==ACT_EDOB_END_OBJECT) {
			ysize= 28;
			glRects(xco, yco-ysize, xco+width, yco);
			uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		}
		else if(eoa->type==ACT_EDOB_REPLACE_MESH) {
			ysize= 48;
			glRects(xco, yco-ysize, xco+width, yco);
			uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
	 
			but= uiDefBut(block, IDPOIN, 0, "ME:",		xco+40, yco-44, (width-80), 19, &(eoa->me), 0, 0, 0, 0, "Add this Object");
			but->func= (test_meshpoin_but);
		}
		else if(eoa->type==ACT_EDOB_TRACK_TO) {
			ysize= 48;
			glRects(xco, yco-ysize, xco+width, yco);
			uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
	 
			but= uiDefBut(block, IDPOIN, 0, "OB:",		xco+10, yco-44, (width-20)/2, 19, &(eoa->ob), 0, 0, 0, 0, "Track to this Object");
			but->func= (test_obpoin_but);
			uiDefBut(block, NUM|INT, 0, "Time:",	xco+10+(width-20)/2, yco-44, (width-20)/2-40, 19, &eoa->time, 0.0, 2000.0, 0, 0, "Duration the tracking takes");
			block->col= BUTGREEN;
			uiDefBut(block, TOG|SHO, 0, "3D",	xco+width-50, yco-44, 40, 19, &eoa->flag, 0.0, 0.0, 0, 0, "Enable 3D tracking");
			block->col= BUTGREY;
		}
		
		str= "Edit Object %t|Add Object %x0|End Object %x1|Replace Mesh %x2|Track to %x3";
		uiDefBut(block, MENU|SHO, 1, str,		xco+40, yco-24, (width-80), 19, &eoa->type, 0.0, 0.0, 0, 0, "");

 		yco-= ysize;
        
        break;
 
 	case ACT_CONSTRAINT:
	
		ysize= 44;
        
		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		coa= act->data;
		
		str= "Limit %t|None %x0|Loc X %x1|Loc Y %x2|Loc Z %x4|Rot X %x8|Rot Y %x16|Rot Z %x32";
		but= uiDefBut(block, MENU|SHO, 1, str,		xco+10, yco-40, 70, 19, &coa->flag, 0.0, 0.0, 0, 0, "");
	
		uiDefBut(block, NUM|SHO,		0, "Damp:",	xco+10, yco-20, 70, 19, &coa->damp, 0.0, 100.0, 0, 0, "");
		uiDefBut(block, LABEL,			0, "Min",	xco+80, yco-20, (width-90)/2, 19, NULL, 0.0, 0.0, 0, 0, "");
		uiDefBut(block, LABEL,			0, "Max",	xco+80+(width-90)/2, yco-20, (width-90)/2, 19, NULL, 0.0, 0.0, 0, 0, "");

		if(coa->flag & ACT_CONST_LOCX) fp= coa->minloc;
		else if(coa->flag & ACT_CONST_LOCY) fp= coa->minloc+1;
		else if(coa->flag & ACT_CONST_LOCZ) fp= coa->minloc+2;
		else if(coa->flag & ACT_CONST_ROTX) fp= coa->minrot;
		else if(coa->flag & ACT_CONST_ROTY) fp= coa->minrot+1;
		else fp= coa->minrot+2;
		
		but= uiDefBut(block, NUM|FLO, 0, "",		xco+80, yco-40, (width-90)/2, 19, fp, -2000.0, 2000.0, 10, 0, "");
		but= uiDefBut(block, NUM|FLO, 0, "",		xco+80+(width-90)/2, yco-40, (width-90)/2, 19, fp+3, -2000.0, 2000.0, 10, 0, "");

 		yco-= ysize;
        
        break;
 
	case ACT_SCENE:
		
		sca= act->data;
		
		if(sca->type==ACT_SCENE_RESTART) {
			ysize= 28;
			glRects(xco, yco-ysize, xco+width, yco);
			uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		}
		else if(sca->type==ACT_SCENE_CAMERA) {
			
			ysize= 48;
			glRects(xco, yco-ysize, xco+width, yco);
			uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
	 
			but= uiDefBut(block, IDPOIN, 0, "OB:",		xco+40, yco-44, (width-80), 19, &(sca->camera), 0, 0, 0, 0, "Set this Camera");
			but->func= (test_obpoin_but);
		}
		else if(sca->type==ACT_SCENE_SET) {
			
			ysize= 48;
			glRects(xco, yco-ysize, xco+width, yco);
			uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
	 
			but= uiDefBut(block, IDPOIN, 0, "SCE:",		xco+40, yco-44, (width-80), 19, &(sca->scene), 0, 0, 0, 0, "Set this Scene");
			but->func= (test_scenepoin_but);
		}
	
		str= "Scene %t|Restart %x0|Set Scene %x1|Set Camera %x2";
		uiDefBut(block, MENU|SHO, 1, str,		xco+40, yco-24, (width-80), 19, &sca->type, 0.0, 0.0, 0, 0, "");

		yco-= ysize;
		break;
		
	case ACT_GROUP:
		ga= act->data;

		ysize= 52;

		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		str= "GroupKey types   %t|Set key %x6|Play %x0|Ping Pong %x1|Flipper %x2|Loop stop %x3|Loop End %x4|Property %x5";

		uiDefBut(block, MENU|SHO, 1, str,			xco+20, yco-24, width-40, 19, &ga->type, 0, 0, 0, 0, "");
		if(ga->type==ACT_GROUP_SET) {
			uiDefBut(block, TEX, 0, "Key: ",		xco+20, yco-44, (width-10)/2, 19, ga->name, 0.0, 31.0, 0, 0, "This name defines groupkey to be set");
			uiDefBut(block, NUM|SHO, 0, "Frame:",	xco+20+(width-10)/2, yco-44, (width-70)/2, 19, &ga->sta, 0.0, 2500.0, 0, 0, "Set this frame");
		}
		else if(ga->type==ACT_GROUP_FROM_PROP) {
			uiDefBut(block, TEX, 0, "Prop: ",		xco+20, yco-44, width-40, 19, ga->name, 0.0, 31.0, 0, 0, "Use this property to define the Group position");
		}
		else {
			uiDefBut(block, NUM|SHO, 0, "Sta",		xco+20, yco-44, (width-40)/2, 19, &ga->sta, 0.0, 2500.0, 0, 0, "Start frame");
			uiDefBut(block, NUM|SHO, 0, "End",		xco+20+(width-40)/2, yco-44, (width-40)/2, 19, &ga->end, 0.0, 2500.0, 0, 0, "End frame");
		}
		yco-= ysize;
		break;
		
 	default:
		ysize= 4;

		glRects(xco, yco-ysize, xco+width, yco);
		uiEmbossW(&UIcol[MIDGREY], block->aspect, (float)xco, (float)yco-ysize, (float)xco+width, (float)yco, 1);
		
		yco-= ysize;
		break;
	}

	block->dt= UI_EMBOSSM;
	block->col= BUTGREY;

	return yco-4;
}

void do_sensor_menu(int event)
{	
	ID **idar;
	Object *ob;
	bSensor *sens;
	short count, a;
	
	idar= get_selected_and_linked_obs(&count, G.buts->scaflag);
	
	for(a=0; a<count; a++) {
		ob= (Object *)idar[a];
		if(event==0 || event==2) ob->scaflag |= OB_SHOWSENS;
		else if(event==1) ob->scaflag &= ~OB_SHOWSENS;
	}
		
	for(a=0; a<count; a++) {
		ob= (Object *)idar[a];
		sens= ob->sensors.first;
		while(sens) {
			if(event==2) sens->flag |= SENS_SHOW;
			else if(event==3) sens->flag &= ~SENS_SHOW;
			sens= sens->next;
		}
	}

	if(idar) freeN(idar);
	allqueue(REDRAWBUTSGAME, 0);
}

uiBlock *sensor_menu()
{
	uiBlock *block;
	int yco=0;
	
	block= uiNewBlock(&curarea->uiblocks, "filemenu", UI_EMBOSSW, UI_HELV, 0x808080, curarea->win);
	block->func= do_sensor_menu;
	
	uiDefBut(block, BUTM, 1, "Show Objects",	0, yco-=20, 160, 19, NULL, 0.0, 0.0, 1, 0, "");
	uiDefBut(block, BUTM, 1, "Hide Objects",	0, yco-=20, 160, 19, NULL, 0.0, 0.0, 1, 1, "");
	uiDefBut(block, SEPR, 0, "",					0, yco-=6, 160, 6, NULL, 0.0, 0.0, 0, 0, "");
	uiDefBut(block, BUTM, 1, "Show Sensors",	0, yco-=20, 160, 19, NULL, 0.0, 0.0, 1, 2, "");
	uiDefBut(block, BUTM, 1, "Hide Sensors",	0, yco-=20, 160, 19, NULL, 0.0, 0.0, 1, 3, "");

	block->direction= UI_TOP;
	
	return block;
}

void do_controller_menu(int event)
{	
	ID **idar;
	Object *ob;
	bController *cont;
	short count, a;
	
	idar= get_selected_and_linked_obs(&count, G.buts->scaflag);
	
	for(a=0; a<count; a++) {
		ob= (Object *)idar[a];
		if(event==0 || event==2) ob->scaflag |= OB_SHOWCONT;
		else if(event==1) ob->scaflag &= ~OB_SHOWCONT;
	}

	for(a=0; a<count; a++) {
		ob= (Object *)idar[a];
		cont= ob->controllers.first;
		while(cont) {
			if(event==2) cont->flag |= CONT_SHOW;
			else if(event==3) cont->flag &= ~CONT_SHOW;
			cont= cont->next;
		}
	}

	if(idar) freeN(idar);
	allqueue(REDRAWBUTSGAME, 0);
}

uiBlock *controller_menu()
{
	uiBlock *block;
	int yco=0;
	
	block= uiNewBlock(&curarea->uiblocks, "filemenu", UI_EMBOSSW, UI_HELV, 0x808080, curarea->win);
	block->func= do_controller_menu;
	
	uiDefBut(block, BUTM, 1, "Show Objects",	0, yco-=20, 160, 19, NULL, 0.0, 0.0, 1, 0, "");
	uiDefBut(block, BUTM, 1, "Hide Objects",	0, yco-=20, 160, 19, NULL, 0.0, 0.0, 1, 1, "");
	uiDefBut(block, SEPR, 0, "",					0, yco-=6, 160, 6, NULL, 0.0, 0.0, 0, 0, "");
	uiDefBut(block, BUTM, 1, "Show Controllers",	0, yco-=20, 160, 19, NULL, 0.0, 0.0, 2, 2, "");
	uiDefBut(block, BUTM, 1, "Hide Controllers",	0, yco-=20, 160, 19, NULL, 0.0, 0.0, 3, 3, "");

	block->direction= UI_TOP;
	
	return block;
}

void do_actuator_menu(int event)
{	
	ID **idar;
	Object *ob;
	bActuator *act;
	short count, a;
	
	idar= get_selected_and_linked_obs(&count, G.buts->scaflag);
	
	for(a=0; a<count; a++) {
		ob= (Object *)idar[a];
		if(event==0 || event==2) ob->scaflag |= OB_SHOWACT;
		else if(event==1) ob->scaflag &= ~OB_SHOWACT;
	}

	for(a=0; a<count; a++) {
		ob= (Object *)idar[a];
		act= ob->actuators.first;
		while(act) {
			if(event==2) act->flag |= ACT_SHOW;
			else if(event==3) act->flag &= ~ACT_SHOW;
			act= act->next;
		}
	}

	if(idar) freeN(idar);
	allqueue(REDRAWBUTSGAME, 0);
}

uiBlock *actuator_menu()
{
	uiBlock *block;
	int xco=0;
	
	block= uiNewBlock(&curarea->uiblocks, "filemenu", UI_EMBOSSW, UI_HELV, 0x808080, curarea->win);
	block->func= do_actuator_menu;
	
	uiDefBut(block, BUTM, 1, "Show Objects",	0, xco-=20, 160, 19, NULL, 0.0, 0.0, 1, 0, "");
	uiDefBut(block, BUTM, 1, "Hide Objects",	0, xco-=20, 160, 19, NULL, 0.0, 0.0, 1, 1, "");
	uiDefBut(block, SEPR, 0, "",					0, xco-=6, 160, 6, NULL, 0.0, 0.0, 0, 0, "");
	uiDefBut(block, BUTM, 1, "Show Actuators",	0, xco-=20, 160, 19, NULL, 0.0, 0.0, 1, 2, "");
	uiDefBut(block, BUTM, 1, "Hide Actuators",	0, xco-=20, 160, 19, NULL, 0.0, 0.0, 1, 3, "");

	block->direction= UI_TOP;
	
	return block;
}

FreeCamera *new_freecamera()
{
	FreeCamera *fcam;
	
	fcam= callocN(sizeof(FreeCamera), "freecamera");
	fcam->mass= 1.0;
	fcam->accelleration= 0.3;
	fcam->maxspeed= 1.0;
	fcam->maxrotspeed= 1.0;
	fcam->maxtiltspeed= 1.0;
	fcam->speeddamp= 0.8;
	fcam->rotdamp= 0.8;
	fcam->tiltdamp= 0.8;
	
	return fcam;
}

uiBlock *freecamera_menu()
{
	uiBlock *block;
	FreeCamera *fcam;
	int yco=0;
	
	block= uiNewBlock(&curarea->uiblocks, "filemenu", UI_EMBOSSX, UI_HELV, 0x808080, curarea->win);
	
	if(G.scene->fcam==NULL) G.scene->fcam= new_freecamera();
	fcam= G.scene->fcam;
	
	/* use this for a boundbox */
	uiDefBut(block, LABEL, 0, "",	-5, -185, 260, 200, NULL, 0, 0, 0, 0, "");
	
	uiDefBut(block, NUMSLI|FLO, 0, "Mass:",	0, yco-=20, 250, 19, &fcam->mass, 0.0, 10.0, 0, 0, "");
	uiDefBut(block, NUMSLI|FLO, 0, "Accel:",	0, yco-=20, 250, 19, &fcam->accelleration, 0.0, 10.0, 0, 0, "");
	yco-= 10;
	uiDefBut(block, NUMSLI|FLO, 0, "Max Speed:",	0, yco-=20, 250, 19, &fcam->maxspeed, 0.0, 10.0, 0, 0, "");
	uiDefBut(block, NUMSLI|FLO, 0, "Max Rot Sp:",	0, yco-=20, 250, 19, &fcam->maxrotspeed, 0.0, 2.0, 0, 0, "");
	uiDefBut(block, NUMSLI|FLO, 0, "Max Tilt Sp:",	0, yco-=20, 250, 19, &fcam->maxtiltspeed, 0.0, 2.0, 0, 0, "");
	yco-= 10;
	uiDefBut(block, NUMSLI|FLO, 0, "Speed Damp:",	0, yco-=20, 250, 19, &fcam->speeddamp, 0.0, 1.0, 0, 0, "");
	uiDefBut(block, NUMSLI|FLO, 0, "Rot Damp:",	0, yco-=20, 250, 19, &fcam->rotdamp, 0.0, 1.0, 0, 0, "");
	uiDefBut(block, NUMSLI|FLO, 0, "Tilt Damp:",	0, yco-=20, 250, 19, &fcam->tiltdamp, 0.0, 1.0, 0, 0, "");
	
	block->direction= UI_TOP;
	
	return block;
}

void gamebuts()
{
	ID *id, **idar;
	Object *ob;
	bProperty *prop;
	bSensor *sens;
	bController *cont;
	bActuator *act;
	uiBlock *block;
	uiBut *but;
	uint col;
	int a;
	short *sp, xco, yco, count, width, ycoo;
	char *pupstr, *str, name[32];
	
	ob= OBACT;

	if(ob==0) return;
	
	sprintf(name, "buttonswin %d", curarea->win);
	block= uiNewBlock(&curarea->uiblocks, name, UI_EMBOSSX, UI_HELV, 0x8f8f8f, curarea->win);
	
	block->col= BUTGREEN;
#ifndef NAN_GAME
	uiDefBut(block, TOG|INT|BIT|13, B_SETSECTOR, "Sector",		10,205,65,19, &ob->gameflag, 0, 0, 0, 0, "All game elements should be in the Sector boundbox");
	uiDefBut(block, TOG|INT|BIT|14, B_SETPROP, "Prop",			75,205,65,19, &ob->gameflag, 0, 0, 0, 0, "An Object fixed within a sector");
	block->col= BUTPURPLE;
	uiDefBut(block, TOG|INT|BIT|2, B_SETACTOR, "Actor",			140,205,65,19, &ob->gameflag, 0, 0, 0, 0, "Objects that are evaluated by the engine ");
	if(ob->gameflag & OB_ACTOR) {	
		uiDefBut(block, TOG|INT|BIT|0, B_SETDYNA, "Dynamic",	205,205,75,19, &ob->gameflag, 0, 0, 0, 0, "Motion defined by laws of physics");
		uiDefBut(block, TOG|INT|BIT|15, B_SETMAINACTOR, "MainActor",	280,205,70,19, &ob->gameflag, 0, 0, 0, 0, "");
	
		if(ob->gameflag & OB_DYNAMIC) {
			
			uiDefBut(block, TOG|INT|BIT|6, B_DIFF, "Do Fh",		10,185,50,19, &ob->gameflag, 0, 0, 0, 0, "Use Fh settings in Materials");
			uiDefBut(block, TOG|INT|BIT|7, B_DIFF, "Rot Fh",	60,185,50,19, &ob->gameflag, 0, 0, 0, 0, "Use face normal to rotate Object");
	
			block->col= BUTGREY;
			uiDefBut(block, NUM|FLO, B_DIFF, "Mass:",			110, 185, 120, 19, &ob->mass, 0.01, 100.0, 10, 0, "The mass of the Object");
			uiDefBut(block, NUM|FLO, REDRAWVIEW3D, "Size:",		230, 185, 120, 19, &ob->inertia, 0.01, 10.0, 10, 0, "Bounding sphere size");
			uiDefBut(block, NUM|FLO, B_DIFF, "Damp:",			10, 165, 100, 19, &ob->damping, 0.0, 1.0, 10, 0, "General movement damping");
			uiDefBut(block, NUM|FLO, B_DIFF, "RotDamp:",		110, 165, 120, 19, &ob->rdamping, 0.0, 1.0, 10, 0, "General rotation damping");
		}
	}


#else
	block->col= BUTGREY;
	but= uiDefBut(block, BLOCK, 1, "FreeCamera",		10, 205, 80, 19, NULL, 0, 0, 0, 0, "");
	but->blockfunc= freecamera_menu;
	uiDefBut(block, TOG|INT|BIT|0, 1, "Dynamic",90,205, 80,19, &ob->gameflag, 0, 0, 0, 0, "Motion defined by laws of physics");
	uiDefBut(block, NUM|FLO, B_DIFF, "Mass:",			170, 205, 80, 19, &ob->mass, 0.01, 1000.0, 100, 0, "The mass of the Object");
	uiDefBut(block, NUM|FLO, B_DIFF, "Damp:",			250, 205, 90, 19, &ob->damping, 0.0, 1.0, 100, 0, "General movement damping");

	if((ob->gameflag & OB_DYNAMIC) == 0)
		uiDefBut(block, TOG|INT|BIT|12, 0, "Collision",	10,170,80,19, &ob->gameflag, 0, 0, 0, 0, "For non-dyna's: collision detection and response");

	uiDefBut(block, NUM|FLO, B_DIFF, "Inertia:",		90, 170, 90, 19, &ob->inertia, 0.01, 1000.0, 100, 0, "Defines how easy an Object can be rotated by dynamics");
	block->col= BUTGREEN;
	uiDefBut(block, TOG|INT|BIT|3, B_DIFF, "X",			180, 170, 25, 19, &ob->gameflag, 0.0, 0.0, 0, 0, "Lock X-axis for rotation");
	uiDefBut(block, TOG|INT|BIT|4, B_DIFF, "Y",			205, 170, 25, 19, &ob->gameflag, 0.0, 0.0, 0, 0, "Lock Y-axis for rotation");
	uiDefBut(block, TOG|INT|BIT|5, B_DIFF, "Z",			230, 170, 25, 19, &ob->gameflag, 0.0, 0.0, 0, 0, "Lock Z-axis for rotation");
	uiDefBut(block, TOG|INT|BIT|2, 1,	"MainActor",	255, 170, 80,19, &ob->gameflag, 0, 0, 0, 0, "Objects that are evaluated by the engine ");
#endif

	block->col= BUTSALMON;
	uiDefBut(block, BUT, B_ADD_PROP, "ADD property",		10, 135, 340, 24, NULL, 0.0, 100.0, 100, 0, "");
	
	pupstr= "Types %t|Bool %x0|Int %x1|Float %x2|String %x3|Timer %x5";
	
	a= 0;
	prop= ob->prop.first;
	while(prop) {
		
		block->col= BUTSALMON;
		but= uiDefBut(block, BUT, 1, "Del",		10, 115-20*a, 40, 19, NULL, 0.0, 0.0, 1, (float)a, "");
		but->func= del_property;
		block->col= BUTGREY;
		uiDefBut(block, MENU|SHO, B_CHANGE_PROP, pupstr,		50, 115-20*a, 60, 19, &prop->type, 0, 0, 0, 0, "");
		but= uiDefBut(block, TEX, 1, "Name:",					110, 115-20*a, 105, 19, prop->name, 0, 31, 0, 0, "");
		but->func= make_unique_prop_names;
		
		if(prop->type==PROP_BOOL) {
			block->col= BUTGREEN;
			uiDefBut(block, TOG|INT|BIT|0, B_REDR, "True",		215, 115-20*a, 55, 19, &prop->data, 0, 0, 0, 0, "");
			uiDefBut(block, TOGN|INT|BIT|0, B_REDR, "False",	270, 115-20*a, 55, 19, &prop->data, 0, 0, 0, 0, "");
			block->col= BUTGREY;
		}
		else if(prop->type==PROP_INT) 
			uiDefBut(block, NUM|INT, 0, "",			215, 115-20*a, 110, 19, &prop->data, -10000, 10000, 0, 0, "");
		else if(prop->type==PROP_FLOAT) 
			uiDefBut(block, NUM|FLO, 0, "",			215, 115-20*a, 110, 19, &prop->data, -10000, 10000, 100, 0, "");
		else if(prop->type==PROP_STRING) 
			uiDefBut(block, TEX, 0, "",				215, 115-20*a, 110, 19, prop->poin, 0, 127, 0, 0, "");
		else if(prop->type==PROP_TIME) 
			uiDefBut(block, NUM|INT, 0, "",			215, 115-20*a, 110, 19, &prop->data, -10000, 10000, 0, 0, "");
		
		uiDefBut(block, TOG|SHO|BIT|0, 0, "D",		325, 115-20*a, 20, 19, &prop->flag, 0, 0, 0, 0, "Print Debug info");
		
		a++;
		prop= prop->next;
	}
	
	idar= get_selected_and_linked_obs(&count, G.buts->scaflag);
	
	/* ******************************* */
	xco= 375; yco= 170; width= 230;

	block->col= BUTGREY;
	but= uiDefBut(block, BLOCK, 1, "Sensors",		xco-10, yco+35, 80, 19, NULL, 0, 0, 0, 0, "");
	but->blockfunc= sensor_menu;
	block->col= BUTGREEN;
	block->dt= UI_EMBOSSX;
	uiDefBut(block, TOG|SHO|BIT|0, B_REDR, "Sel", xco+110, yco+35, (width-100)/3, 19, &G.buts->scaflag, 0, 0, 0, 0, "Show all selected Objects");
	uiDefBut(block, TOG|SHO|BIT|1, B_REDR, "Act", xco+110+(width-100)/3, yco+35, (width-100)/3, 19, &G.buts->scaflag, 0, 0, 0, 0, "Show active Object");
	uiDefBut(block, TOG|SHO|BIT|2, B_REDR, "Link", xco+110+2*(width-100)/3, yco+35, (width-100)/3, 19, &G.buts->scaflag, 0, 0, 0, 0, "Show linked Objects to Controller");
	
	for(a=0; a<count; a++) {
		ob= (Object *)idar[a];
		if( (ob->scavisflag & OB_VIS_SENS) == 0) continue;
		
		/* presume it is only objects for now */
		block->dt= UI_EMBOSSX;
		block->col= BUTGREY;
		if(ob->sensors.first) uiSetCurFont(block, block->font+1);
		block->col= MIDGREY;
		uiDefBut(block, TOG|SHO|BIT|6, B_REDR, ob->id.name+2,	xco-10, yco, width-30, 19, &ob->scaflag, 0, 31, 0, 0, "Object name, click to show/hide sensors");
		if(ob->sensors.first) uiSetCurFont(block, block->font);
		block->col= BUTSALMON;
		uiDefBut(block, TOG|SHO|BIT|8, B_ADD_SENS, "Add",		xco+width-40, yco, 50, 19, &ob->scaflag, 0, 0, 0, 0, "Add a new Sensor");
		yco-=20;
		
		if(ob->scaflag & OB_SHOWSENS) {
			block->dt= UI_EMBOSSM;
			sens= ob->sensors.first;
			while(sens) {
				block->col= BUTSALMON;
				uiDefBut(block, TOG|SHO|BIT|1, B_DEL_SENS, "ICON 0 0 4",	xco, yco, 22, 19, &sens->flag, 0, 0, 0, 0, "Delete Sensor");
				block->col= BUTGREY;
				uiDefBut(block, ICONTOG|SHO|BIT|0, B_REDR, "ICON 0 0 6:", xco+width-22, yco, 22, 19, &sens->flag, 0, 0, 0, 0, "Sensor settings");

				ycoo= yco;
				if(sens->flag & SENS_SHOW) {
					block->col= BUTYELLOW;

					uiDefBut(block, MENU|SHO, B_CHANGE_SENS, sensor_pup(),	xco+22, yco, 100, 19, &sens->type, 0, 0, 0, 0, "Sensor type");
					but= uiDefBut(block, TEX, 1, "", xco+122, yco, width-144, 19, sens->name, 0, 31, 0, 0, "Sensor name");
					but->func= make_unique_prop_names;

					sens->otype= sens->type;
					block->col= BUTGREY;
					yco= draw_sensorbuttons(sens, block, xco, yco, width);
					if(yco-6 < ycoo) ycoo= (yco+ycoo-20)/2;
				}
				else {
					col= get_col_sensor(sens->type, 1);
					cpack(col);
					glRecti(xco+22, yco, xco+width-22,yco+19);
					but= uiDefBut(block, LABEL, 0, sensor_name(sens->type),	xco+22, yco, 100, 19, sens, 0, 0, 0, 0, "");
					but->butfunc= sca_move_sensor;
					but= uiDefBut(block, LABEL, 0, sens->name, xco+122, yco, width-144, 19, sens, 0, 31, 0, 0, "");
					but->butfunc= sca_move_sensor;
				}

				but= uiDefBut(block, LINK, 0, "ICON 0 9 6",			xco+width-3, ycoo, 19, 19, NULL, 0, 0, 0, 0, "Link button, drag a line to a Controller");
				uiSetButLink(but, NULL, (void ***)&(sens->links), &sens->totlinks, LINK_SENSOR, LINK_CONTROLLER);

				yco-=20;

				sens= sens->next;
			}
			yco-= 6;
		}
	}

	/* ******************************* */
	xco= 675; yco= 170; width= 290;

	block->col= BUTGREY;
	but= uiDefBut(block, BLOCK, 1, "Controllers",		xco-10, yco+35, 100, 19, NULL, 0, 0, 0, 0, "");
	but->blockfunc= controller_menu;
	block->col= BUTGREEN;
	block->dt= UI_EMBOSSX;
	uiDefBut(block, TOG|SHO|BIT|3, B_REDR, "Sel", xco+110, yco+35, (width-100)/3, 19, &G.buts->scaflag, 0, 0, 0, 0, "Show all selected Objects");
	uiDefBut(block, TOG|SHO|BIT|4, B_REDR, "Act", xco+110+(width-100)/3, yco+35, (width-100)/3, 19, &G.buts->scaflag, 0, 0, 0, 0, "Show active Object");
	uiDefBut(block, TOG|SHO|BIT|5, B_REDR, "Link", xco+110+2*(width-100)/3, yco+35, (width-100)/3, 19, &G.buts->scaflag, 0, 0, 0, 0, "Show linked Objects to Sensor/Actuator");
	
	ob= OBACT;
	
	for(a=0; a<count; a++) {
		ob= (Object *)idar[a];
		if( (ob->scavisflag & OB_VIS_CONT) == 0) continue;

		/* presume it is only objects for now */
		block->dt= UI_EMBOSSX;
		block->col= BUTSALMON;
		uiDefBut(block, TOG|SHO|BIT|9, B_ADD_CONT, "Add",		xco+width-40, yco, 50, 19, &ob->scaflag, 0, 0, 0, 0, "Add a new Controller");
		block->col= MIDGREY;
		if(ob->controllers.first) uiSetCurFont(block, block->font+1);
		uiDefBut(block, TOG|SHO|BIT|11, B_REDR, ob->id.name+2,	xco-10, yco, width-30, 19, &ob->scaflag, 0, 0, 0, 0, "Active Object name");
		if(ob->controllers.first) uiSetCurFont(block, block->font);
		yco-=20;
		
		if(ob->scaflag & OB_SHOWCONT) {
			block->dt= UI_EMBOSSM;
			cont= ob->controllers.first;
			while(cont) {
				block->col= BUTSALMON;
				uiDefBut(block, TOG|SHO|BIT|1, B_DEL_CONT, "ICON 0 0 4",	xco, yco, 22, 19, &cont->flag, 0, 0, 0, 0, "Delete Controller");
				block->col= BUTGREY;
				uiDefBut(block, ICONTOG|SHO|BIT|0, B_REDR, "ICON 0 0 6:", xco+width-22, yco, 22, 19, &cont->flag, 0, 0, 0, 0, "Controller settings");
		
				if(cont->flag & CONT_SHOW) {
					block->col= BUTYELLOW;
					cont->otype= cont->type;
					uiDefBut(block, MENU|SHO, B_CHANGE_CONT, controller_pup(),	xco+22, yco, 100, 19, &cont->type, 0, 0, 0, 0, "Controller type");
					but= uiDefBut(block, TEX, 1, "", xco+122, yco, width-144, 19, cont->name, 0, 31, 0, 0, "Controller name");
					but->func= make_unique_prop_names;
					block->col= BUTGREY;
		
					ycoo= yco;
					yco= draw_controllerbuttons(cont, block, xco, yco, width);
					if(yco-6 < ycoo) ycoo= (yco+ycoo-20)/2;
				}
				else {
					cpack(0x999999);
					glRecti(xco+22, yco, xco+width-22,yco+19);
					but= uiDefBut(block, LABEL, 0, controller_name(cont->type), xco+22, yco, 100, 19, cont, 0, 0, 0, 0, "Controller type");
					but->butfunc= sca_move_controller;
					but= uiDefBut(block, LABEL, 0, cont->name, xco+122, yco, width-144, 19, cont, 0, 0, 0, 0, "Conroller name");
					but->butfunc= sca_move_controller;
					ycoo= yco;
				}
		
				but= uiDefBut(block, LINK, 0, "ICON 0 9 6",			xco+width-3, ycoo, 19, 19, NULL, 0, 0, 0, 0, "Link button, drag a line to an Actuator");
				uiSetButLink(but, NULL, (void ***)&(cont->links), &cont->totlinks, LINK_CONTROLLER, LINK_ACTUATOR);
		
				uiDefBut(block, INLINK, 0, "ICON 0 10 6",		xco-19, ycoo, 19, 19, cont, LINK_CONTROLLER, 0, 0, 0, "");
		
				yco-=20;
				
				cont= cont->next;
			}
			yco-= 6;
		}
	}
	
	/* ******************************* */
	xco= 1035; yco= 170; width= 230;
	
	but= uiDefBut(block, BLOCK, 1, "Actuators",		xco-10, yco+35, 100, 19, NULL, 0, 0, 0, 0, "");
	but->blockfunc= actuator_menu;
	block->col= BUTGREEN;
	block->dt= UI_EMBOSSX;
	uiDefBut(block, TOG|SHO|BIT|6, B_REDR, "Sel", xco+110, yco+35, (width-110)/3, 19, &G.buts->scaflag, 0, 0, 0, 0, "Show all selected Objects");
	uiDefBut(block, TOG|SHO|BIT|7, B_REDR, "Act", xco+110+(width-110)/3, yco+35, (width-110)/3, 19, &G.buts->scaflag, 0, 0, 0, 0, "Show active Object");
	uiDefBut(block, TOG|SHO|BIT|8, B_REDR, "Link", xco+110+2*(width-110)/3, yco+35, (width-110)/3, 19, &G.buts->scaflag, 0, 0, 0, 0, "Show linked Objects to Controller");
	
	for(a=0; a<count; a++) {
		ob= (Object *)idar[a];
		if( (ob->scavisflag & OB_VIS_ACT) == 0) continue;

		/* presume it is only objects for now */
		block->dt= UI_EMBOSSX;
		block->col= BUTGREY;
		if(ob->actuators.first) uiSetCurFont(block, block->font+1);
		block->col= MIDGREY;
		uiDefBut(block, TOG|SHO|BIT|7, B_REDR, ob->id.name+2,	xco-10, yco, width-30, 19, &ob->scaflag, 0, 31, 0, 0, "Object name, click to show/hide actuators");
		if(ob->actuators.first) uiSetCurFont(block, block->font);
		block->col= BUTSALMON;
		uiDefBut(block, TOG|SHO|BIT|10, B_ADD_ACT, "Add",		xco+width-40, yco, 50, 19, &ob->scaflag, 0, 0, 0, 0, "Add a new Actuator");
		yco-=20;
		
		if(ob->scaflag & OB_SHOWACT) {
			block->dt= UI_EMBOSSM;
			act= ob->actuators.first;
			while(act) {
				block->col= BUTSALMON;
				uiDefBut(block, TOG|SHO|BIT|1, B_DEL_ACT, "ICON 0 0 4",	xco, yco, 22, 19, &act->flag, 0, 0, 0, 0, "Delete Actuator");
				block->col= BUTGREY;
				uiDefBut(block, ICONTOG|SHO|BIT|0, B_REDR, "ICON 0 0 6:", xco+width-22, yco, 22, 19, &act->flag, 0, 0, 0, 0, "Actuator settings");

				if(act->flag & ACT_SHOW) {
					block->col= BUTYELLOW;
					act->otype= act->type;
					uiDefBut(block, MENU|SHO, B_CHANGE_ACT, actuator_pup(),	xco+22, yco, 100, 19, &act->type, 0, 0, 0, 0, "Actuator type");
					but= uiDefBut(block, TEX, 1, "", xco+122, yco, width-144, 19, act->name, 0, 31, 0, 0, "Actuator name");
					but->func= make_unique_prop_names;
					block->col= BUTGREY;

					ycoo= yco;
					yco= draw_actuatorbuttons(act, block, xco, yco, width);
					if(yco-6 < ycoo) ycoo= (yco+ycoo-20)/2;
				}
				else {
					col= get_col_actuator(act->type, 1);
					cpack(col);
					glRecti(xco+22, yco, xco+width-22,yco+19);
					but= uiDefBut(block, LABEL, 0, actuator_name(act->type), xco+22, yco, 100, 19, act, 0, 0, 0, 0, "Actuator type");
					but->butfunc= sca_move_actuator;
					but= uiDefBut(block, LABEL, 0, act->name, xco+122, yco, width-144, 19, act, 0, 0, 0, 0, "Actuator name");
					but->butfunc= sca_move_actuator;
					ycoo= yco;
				}

				uiDefBut(block, INLINK, 0, "ICON 0 10 6",		xco-19, ycoo, 19, 19, act, LINK_ACTUATOR, 0, 0, 0, "");

				yco-=20;

				act= act->next;
			}
			yco-= 6;
		}
	}

	uiComposeLinks(block);
	uiDrawBlock(block);

	if(idar) freeN(idar);
}

