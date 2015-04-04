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



/*  sca.c   june 2000
 *  
 *  sensor/controller/actuator code
 * 
 *  these all are linked to objects (listbase)
 * 
 *  all data is 'direct data', not Blender lib data.
 * 
 *  ton roosendaal
 * Version: $Id: sca.c,v 1.19 2000/09/25 22:02:54 ton Exp $
 */


#include "blender.h"
#include "graphics.h"	/* for keyvals */
#include "sound.h"
#include "game.h"

/* ******************* SENSORS ************************ */

void free_sensor(bSensor *sens)
{
	if(sens->links) freeN(sens->links);
	if(sens->data) freeN(sens->data);
	freeN(sens);
	
}

void free_sensors(ListBase *lb)
{
	bSensor *sens;
	
	while((sens= lb->first)) {
		remlink(lb, sens);
		free_sensor(sens);
	}
}

bSensor *copy_sensor(bSensor *sens)
{
	bSensor *sensn;
	
	sensn= dupallocN(sens);
	sensn->flag |= SENS_NEW;
	if(sens->data) {
		sensn->data= dupallocN(sens->data);
	}

	if(sens->links) sensn->links= dupallocN(sens->links);
	
	return sensn;
}

void copy_sensors(ListBase *lbn, ListBase *lbo)
{
	bSensor *sens, *sensn;
	
	lbn->first= lbn->last= 0;
	sens= lbo->first;
	while(sens) {
		sensn= copy_sensor(sens);
		addtail(lbn, sensn);
		sens= sens->next;
	}
}

void init_sensor(bSensor *sens)
{
	/* also use when sensor changes type */
	bNearSensor *ns;
	bTouchSensor *ts;
	bKeyboardSensor *ks;
	bPropertySensor *ps;
	bMouseSensor *ms;
	
	if(sens->data) freeN(sens->data);
	sens->data= NULL;
	
	switch(sens->type) {
	case SENS_TOUCH:
		ts=sens->data= callocN(sizeof(bTouchSensor), "touchsens");
		break;
	case SENS_NEAR:
		ns=sens->data= callocN(sizeof(bNearSensor), "nearsens");
		ns->dist= 1.0;
		ns->resetdist= 2.0;
		break;
	case SENS_KEYBOARD:
		ks=sens->data= callocN(sizeof(bKeyboardSensor), "keysens");
		break;
	case SENS_PROPERTY:
		ps=sens->data= callocN(sizeof(bPropertySensor), "propsens");
		break;
	case SENS_MOUSE:
		ms=sens->data= callocN(sizeof(bMouseSensor), "mousesens");
		ms->type= LEFTMOUSE;
		break;
	case SENS_COLLISION:
		sens->data= callocN(sizeof(bCollisionSensor), "colsens");
		break;
	case SENS_RADAR:
		sens->data= callocN(sizeof(bRadarSensor), "radarsens");
		break;
	}
}

bSensor *new_sensor(int type)
{
	bSensor *sens;

	sens= callocN(sizeof(bSensor), "Sensor");
	sens->type= type;
	sens->flag= SENS_SHOW;
	
	init_sensor(sens);
	
	strcpy(sens->name, "sensor");
	make_unique_prop_names(sens->name);
	
	return sens;
}

/* ******************* CONTROLLERS ************************ */

void free_controller(bController *cont)
{
	bSensor *sens;
	Object *ob;
	int a, removed;
	
	/* check for controller pointers in sensors */
	ob= G.main->object.first;
	while(ob) {
		sens= ob->sensors.first;
		while(sens) {
			removed= 0;
			for(a=0; a<sens->totlinks; a++) {
				if(removed) (sens->links)[a-1] = (sens->links)[a];
				else if((sens->links)[a] == cont) removed= 1;
			}
			if(removed) {
				sens->totlinks--;
				
				if(sens->totlinks==0) {
					freeN(sens->links);
					sens->links= NULL;
				}
			}
			sens= sens->next;
		}
		ob= ob->id.next;
	}

	if(cont->links) freeN(cont->links);

	/* the controller itself */
	if(cont->data) freeN(cont->data);
	freeN(cont);
	
}

void free_controllers(ListBase *lb)
{
	bController *cont;
	
	while((cont= lb->first)) {
		remlink(lb, cont);
		if(cont->slinks) freeN(cont->slinks);
		free_controller(cont);
	}
}

bController *copy_controller(bController *cont)
{
	bController *contn;
	
	cont->mynew=contn= dupallocN(cont);
	contn->flag |= CONT_NEW;
	if(cont->data) {
		contn->data= dupallocN(cont->data);
	}

	if(cont->links) contn->links= dupallocN(cont->links);
	contn->slinks= NULL;
	contn->totslinks= 0;
	
	return contn;
}

void copy_controllers(ListBase *lbn, ListBase *lbo)
{
	bController *cont, *contn;
	
	lbn->first= lbn->last= 0;
	cont= lbo->first;
	while(cont) {
		contn= copy_controller(cont);
		addtail(lbn, contn);
		cont= cont->next;
	}
}

void init_controller(bController *cont)
{
	bExpressionCont *ec;
	/* also use when controller changes type, leave actuators... */
	
	if(cont->data) freeN(cont->data);
	cont->data= 0;
	
	switch(cont->type) {
	case CONT_EXPRESSION:
		ec= cont->data= callocN(sizeof(bExpressionCont), "expcont");
		break;
	case CONT_PYTHON:
		cont->data= callocN(sizeof(bPythonCont), "pycont");
		break;
	}
}

bController *new_controller(int type)
{
	bController *cont;

	cont= callocN(sizeof(bController), "Controller");
	cont->type= type;
	cont->flag= CONT_SHOW;

	init_controller(cont);
	
	strcpy(cont->name, "cont");
	make_unique_prop_names(cont->name);
	
	return cont;
}

/* ******************* ACTUATORS ************************ */

void free_actuator(bActuator *act)
{
	bController *cont;
	Object *ob;
	int a, removed;
	
	/* check for actuator pointers in controllers */
	ob= G.main->object.first;
	while(ob) {
		cont= ob->controllers.first;
		while(cont) {
			removed= 0;
			for(a=0; a<cont->totlinks; a++) {
				if(removed) (cont->links)[a-1] = (cont->links)[a];
				else if((cont->links)[a] == act) removed= 1;
			}
			if(removed) {
				cont->totlinks--;
				
				if(cont->totlinks==0) {
					freeN(cont->links);
					cont->links= NULL;
				}
			}
			cont= cont->next;
		}
		ob= ob->id.next;
	}


	if(act->data) freeN(act->data);
	freeN(act);
	
}

void free_actuators(ListBase *lb)
{
	bActuator *act;
	
	while((act= lb->first)) {
		remlink(lb, act);
		free_actuator(act);
	}
}

bActuator *copy_actuator(bActuator *act)
{
	bActuator *actn;
	
	act->mynew=actn= dupallocN(act);
	actn->flag |= ACT_NEW;
	if(act->data) {
		actn->data= dupallocN(act->data);
	}
	
	return actn;
}

void copy_actuators(ListBase *lbn, ListBase *lbo)
{
	bActuator *act, *actn;
	
	lbn->first= lbn->last= 0;
	act= lbo->first;
	while(act) {
		actn= copy_actuator(act);
		addtail(lbn, actn);
		act= act->next;
	}
}

void init_actuator(bActuator *act)
{
	/* also use when actuator changes type */
	bObjectActuator *oa;
	
	if(act->data) freeN(act->data);
	act->data= 0;
	
	switch(act->type) {
	case ACT_SOUND:
		act->data= callocN(sizeof(bSoundActuator), "soundact");
		break;
	case ACT_OBJECT:
		act->data= callocN(sizeof(bObjectActuator), "objectact");
		oa= act->data;
		oa->flag= 15;
		break;
	case ACT_IPO:
		act->data= callocN(sizeof(bIpoActuator), "ipoact");
		break;
	case ACT_PROPERTY:
		act->data= callocN(sizeof(bPropertyActuator), "propact");
		break;
	case ACT_CAMERA:
		act->data= callocN(sizeof(bCameraActuator), "camact");
		break;
	case ACT_EDIT_OBJECT:
		act->data= callocN(sizeof(bEditObjectActuator), "editobact");
		break;
	case ACT_CONSTRAINT:
		act->data= callocN(sizeof(bConstraintActuator), "cons act");
		break;
	case ACT_SCENE:
		act->data= callocN(sizeof(bSceneActuator), "scene act");
		break;
	case ACT_GROUP:
		act->data= callocN(sizeof(bGroupActuator), "group act");
		break;
	}
}

bActuator *new_actuator(int type)
{
	bActuator *act;

	act= callocN(sizeof(bActuator), "Actuator");
	act->type= type;
	act->flag= ACT_SHOW;
	
	init_actuator(act);
	
	strcpy(act->name, "act");
	make_unique_prop_names(act->name);
	
	return act;
}

/* ******************** GENERAL ******************* */

void clear_sca_new_poins_ob(Object *ob)
{
	bSensor *sens;
	bController *cont;
	bActuator *act;
	
	sens= ob->sensors.first;
	while(sens) {
		sens->flag &= ~SENS_NEW;
		sens= sens->next;
	}
	cont= ob->controllers.first;
	while(cont) {
		cont->mynew= NULL;
		cont->flag &= ~CONT_NEW;
		cont= cont->next;
	}
	act= ob->actuators.first;
	while(act) {
		act->mynew= NULL;
		act->flag &= ~ACT_NEW;
		act= act->next;
	}
}

void clear_sca_new_poins()
{
	Object *ob;
	
	ob= G.main->object.first;
	while(ob) {
		clear_sca_new_poins_ob(ob);
		ob= ob->id.next;	
	}
}

void set_sca_new_poins_ob(Object *ob)
{
	bSensor *sens;
	bController *cont;
	bActuator *act;
	int a;
	
	sens= ob->sensors.first;
	while(sens) {
		if(sens->flag & SENS_NEW) {
			for(a=0; a<sens->totlinks; a++) {
				if(sens->links[a] && sens->links[a]->mynew)
					sens->links[a]= sens->links[a]->mynew;
			}
		}
		sens= sens->next;
	}

	cont= ob->controllers.first;
	while(cont) {
		if(cont->flag & CONT_NEW) {
			for(a=0; a<cont->totlinks; a++) {
				if( cont->links[a] && cont->links[a]->mynew)
					cont->links[a]= cont->links[a]->mynew;
			}
		}
		cont= cont->next;
	}
	
	
	act= ob->actuators.first;
	while(act) {
		if(act->flag & ACT_NEW) {
			if(act->type==ACT_EDIT_OBJECT) {
				bEditObjectActuator *eoa= act->data;
				ID_NEW(eoa->ob);
			}
			else if(act->type==ACT_SCENE) {
				bSceneActuator *sca= act->data;
				ID_NEW(sca->camera);
			}
			else if(act->type==ACT_CAMERA) {
				bCameraActuator *ca= act->data;
				ID_NEW(ca->ob);
			}
			else if(act->type==ACT_SCENE) {
				bSceneActuator *sca= act->data;
				ID_NEW(sca->camera);
			}
		}
		act= act->next;
	}
}


void set_sca_new_poins()
{
	Object *ob;
	int a;
	
	ob= G.main->object.first;
	while(ob) {
		set_sca_new_poins_ob(ob);
		ob= ob->id.next;	
	}
}

void sca_remove_ob_poin(Object *obt, Object *ob)
{
	bSensor *sens;
	bActuator *act;
	bCameraActuator *ca;
	bSceneActuator *sa;
	bEditObjectActuator *eoa;
	bPropertyActuator *pa;

	act= obt->actuators.first;
	while(act) {
		switch(act->type) {
		case ACT_CAMERA:
			ca= act->data;
			if(ca->ob==ob) ca->ob= NULL;
			break;
		case ACT_PROPERTY:
			pa= act->data;
			if(pa->ob==ob) pa->ob= NULL;
			break;
		case ACT_SCENE:
			sa= act->data;
			if(sa->camera==ob) sa->camera= NULL;
			break;
		case ACT_EDIT_OBJECT:
			eoa= act->data;
			if(eoa->ob==ob) eoa->ob= NULL;
			break;
		}
		act= act->next;
	}	
}

