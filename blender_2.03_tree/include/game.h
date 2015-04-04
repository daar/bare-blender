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

/* game.h    june 2000

 * 
 * 
 * Version: $Id: game.h,v 1.37 2000/09/27 19:53:04 ton Exp $
 */

#ifndef GAME_H
#define GAME_H

#include "group.h"

/* 'b' means these structs are for use in Blender, the game engine converts it */

#define MAX_PROPSTRING	128

/* material->dynamode, bit one is for buttons */

#define MA_FH_NOR	2

/* ob->gameflag */
#define OB_DYNAMIC		1
#define OB_CHILD		2
#define OB_ACTOR		4
#define OB_INERTIA_LOCK_X	8
#define OB_INERTIA_LOCK_Y	16
#define OB_INERTIA_LOCK_Z	32
#define OB_DO_FH			64
#define OB_ROT_FH			128

#define OB_COLLISION_RESPONSE	4096
#define OB_SECTOR		8192
#define OB_PROP			16384
#define OB_MAINACTOR	32768

#define OB_LIFE			(OB_PROP|OB_DYNAMIC|OB_ACTOR|OB_MAINACTOR|OB_CHILD)

/* ob->scavisflag */
#define OB_VIS_SENS		1
#define OB_VIS_CONT		2
#define OB_VIS_ACT		4

/* ob->scaflag */
#define OB_SHOWSENS		64
#define OB_SHOWACT		128
#define OB_ADDSENS		256
#define OB_ADDCONT		512
#define OB_ADDACT		1024
#define OB_SHOWCONT		2048


/* property->type */
#define PROP_BOOL		0
#define PROP_INT		1
#define PROP_FLOAT		2
#define PROP_STRING		3
#define PROP_VECTOR		4
#define PROP_TIME		5

/* property->flag */
#define PROP_DEBUG		1

/* bMouseSensor->type: uses blender event defines */

/* propertysensor->type */
#define SENS_PROP_EQUAL		0
#define SENS_PROP_NEQUAL	1
#define SENS_PROP_INTERVAL	2
#define SENS_PROP_CHANGED	3
#define SENS_PROP_EXPRESSION	4


/* sensor->type */
#define SENS_ALWAYS		0
#define SENS_TOUCH		1
#define SENS_NEAR		2
#define SENS_KEYBOARD	3
#define SENS_PROPERTY	4
#define SENS_MOUSE		5
#define SENS_COLLISION	6
#define SENS_RADAR		7

/* sensor->flag */
#define SENS_SHOW		1
#define SENS_DEL		2
#define SENS_NEW		4
#define SENS_NOT		8

/* sensor->pulse */
#define SENS_PULSE_CONT 	0
#define SENS_PULSE_REPEAT	1
#define SENS_PULSE_ONCE 	2

/* controller->type */
#define CONT_LOGIC_AND	0
#define CONT_LOGIC_OR	1
#define CONT_EXPRESSION	2
#define CONT_PYTHON		3

/* controller->flag */
#define CONT_SHOW		1
#define CONT_DEL		2
#define CONT_NEW		4

/* objectactuator->flag */
#define ACT_FORCE_LOCAL		1
#define ACT_TORQUE_LOCAL	2
#define ACT_DLOC_LOCAL		4
#define ACT_DROT_LOCAL		8

/* actuator->type */
#define ACT_OBJECT		0
#define ACT_IPO			1
#define ACT_LAMP		2
#define ACT_CAMERA		3
#define ACT_MATERIAL	4
#define ACT_SOUND		5
#define ACT_PROPERTY	6
	/* these two obsolete since 2.02 */
#define ACT_ADD_OBJECT	7
#define ACT_END_OBJECT	8

#define ACT_CONSTRAINT	9
#define ACT_EDIT_OBJECT	10
#define ACT_SCENE		11
#define ACT_GROUP		12

/* actuator flag */
#define ACT_SHOW		1
#define ACT_DEL			2
#define ACT_NEW			4

/* link codes */
#define LINK_SENSOR		0
#define LINK_CONTROLLER	1
#define LINK_ACTUATOR	2

/* keyboardsensor->type */
#define SENS_ALL_KEYS	1

/* ipoactuator->type */
#define ACT_IPO_PLAY		0
#define ACT_IPO_PINGPONG	1
#define ACT_IPO_FLIPPER		2
#define ACT_IPO_LOOP_STOP	3
#define ACT_IPO_LOOP_END	4
#define ACT_IPO_KEY2KEY		5
#define ACT_IPO_FROM_PROP	6

/* groupactuator->type */
#define ACT_GROUP_PLAY		0
#define ACT_GROUP_PINGPONG	1
#define ACT_GROUP_FLIPPER	2
#define ACT_GROUP_LOOP_STOP	3
#define ACT_GROUP_LOOP_END	4
#define ACT_GROUP_FROM_PROP	5
#define ACT_GROUP_SET		6

/* ipoactuator->flag */
#define ACT_IPOFORCE		1
#define ACT_IPOEND			2

/* ipoactuator->flag for k2k */
#define ACT_K2K_PREV		1
#define ACT_K2K_CYCLIC		2
#define ACT_K2K_PINGPONG	4
#define ACT_K2K_HOLD		8

/* property actuator->type */
#define ACT_PROP_ASSIGN		0
#define ACT_PROP_ADD		1
#define ACT_PROP_COPY		2

/* constraint flag */
#define ACT_CONST_LOCX		1
#define ACT_CONST_LOCY		2
#define ACT_CONST_LOCZ		4
#define ACT_CONST_ROTX		8
#define ACT_CONST_ROTY		16
#define ACT_CONST_ROTZ		32

/* editObjectActuator->type */
#define ACT_EDOB_ADD_OBJECT		0
#define ACT_EDOB_END_OBJECT		1
#define ACT_EDOB_REPLACE_MESH	2
#define ACT_EDOB_TRACK_TO		3
#define ACT_EDOB_MAKE_CHILD		4
#define ACT_EDOB_END_CHILD		5

/* editObjectActuator->flag */
#define ACT_TRACK_3D			1

/* SceneActuator->type */
#define ACT_SCENE_RESTART		0
#define ACT_SCENE_SET			1
#define ACT_SCENE_CAMERA		2


/* ********************* PROPERTY ************************ */

typedef struct bProperty {
	struct bProperty *next, *prev;
	char name[32];
	short type, otype;		/* otype is for buttons, when a property type changes */
	int data;				/* data should be 4 bytes to store int,float stuff */
	int old;				/* old is for simul */
	short flag, pad;
	void *poin;
	void *oldpoin;			/* oldpoin is for simul */
	
} bProperty;

/* ****************** SENSORS ********************* */

typedef struct bNearSensor {
	char name[32];
	float dist, resetdist;
	int lastval, pad;
} bNearSensor;

typedef struct bMouseSensor {
	short type, flag;
	int pad;
} bMouseSensor;

typedef struct bTouchSensor {
	char name[32];
	Material *ma;
	float dist, pad;
} bTouchSensor;

typedef struct bKeyboardSensor {
	short key, qual;
	short type, qual2;
} bKeyboardSensor;

typedef struct bPropertySensor {
    int type;
    int pad;
	char name[32];
	char value[32];
    char maxvalue[32];
} bPropertySensor;

typedef struct bCollisionSensor {
	char name[32];
	short damptimer, damp;
	int pad2;
} bCollisionSensor;

typedef struct bRadarSensor {
	char name[32];
	float angle;
	short flag, axis;
} bRadarSensor;

typedef struct bSensor {
	struct bSensor *next, *prev;
	short type, otype, flag, pulse;
	short freq, totlinks, pad1, pad2;
	char name[32];
	void *data;
	
	struct bController **links;
	
	Object *ob;

	
} bSensor;

/* ****************** CONTROLLERS ********************* */

typedef struct bExpressionCont {
	char str[128];
} bExpressionCont;

typedef struct bPythonCont {
	Text *text;
} bPythonCont;

typedef struct bController {
	struct bController *next, *prev, *mynew;
	short type, flag, inputs, totlinks;
	short otype, totslinks, pad2, pad3;
	
	char name[32];
	void *data;
	
	struct bActuator **links;

	struct bSensor **slinks;
	short val, valo;
	int pad5;
	
} bController;

/* ****************** ACTUATORS ********************* */

/* unused now, moved to editobjectactuator in 2.02. Still needed for dna */
typedef struct bAddObjectActuator {
	int time, pad;
	Object *ob;
} bAddObjectActuator;

typedef struct bSoundActuator {
	short flag, sndnr;
	float volume;
	struct bSound *sound;
} bSoundActuator;

typedef struct bEditObjectActuator {
	int time;
	short type, flag;
	Object *ob;
	Mesh *me;
	char name[32];
} bEditObjectActuator;

typedef struct bSceneActuator {
	short type, flag;
	int pad;
	Scene *scene;
	Object *camera;
} bSceneActuator;

typedef struct bPropertyActuator {
	int flag, type;
	char name[32], value[32];
	Object *ob, fromname[32];
} bPropertyActuator;

typedef struct bObjectActuator {
	int flag, pad;
	float forceloc[3], forcerot[3];
	float loc[3], rot[3];
	float dloc[3], drot[3];
} bObjectActuator;

typedef struct bIpoActuator {
	short flag, type;
	short sta, end;
	char name[32];
	
	short pad1, cur, butsta, butend;
	
} bIpoActuator;

typedef struct bCameraActuator {
	Object *ob;
	float height, min, max;
	float fac;
	short flag, axis;
	float visifac;
} bCameraActuator ;

typedef struct bConstraintActuator {
	short flag, damp;
	float slow;
	float minloc[3], maxloc[3];
	float minrot[3], maxrot[3];
} bConstraintActuator;

typedef struct bGroupActuator {
	short flag, type;
	short sta, end;
	char name[32];		/* property or groupkey */
	
	short pad1, cur, butsta, butend;
	Group *group;		/* only during game */
	
} bGroupActuator;


typedef struct bActuator {
	struct bActuator *next, *prev, *mynew;
	short type, flag;
	short otype, go;
	char name[32];

	void *data;

	Object *ob;		/* for ipo's and props to find out wich ob */
	
} bActuator;

typedef struct FreeCamera {
	float mass, accelleration;
	float maxspeed, maxrotspeed,  maxtiltspeed;
	int flag;
	float rotdamp, tiltdamp, speeddamp, pad;
} FreeCamera;


/* ****************** PROTOTYPES ********************* */

/* property.c */

extern bProperty *new_property(int type);
extern void free_property(bProperty *prop);
extern void free_properties(ListBase *lb);
extern void init_property(bProperty *prop);
extern bProperty *copy_property(bProperty *prop);
extern void copy_properties(ListBase *lbn, ListBase *lbo);
extern bProperty *get_property(Object *ob, char *name);
extern int compare_property(bProperty *prop, char *str);
extern void set_property(bProperty *prop, char *str);
extern void add_property(bProperty *prop, char *str);
extern void set_property_valstr(bProperty *prop, char *str);
extern void cp_property(bProperty *prop1, bProperty *prop2);

/* sca.c */

extern void free_sensor(bSensor *sens);
extern void free_sensors(ListBase *lb);
extern bSensor *copy_sensor(bSensor *sens);
extern void copy_sensors(ListBase *lbn, ListBase *lbo);
extern void init_sensor(bSensor *sens);
extern bSensor *new_sensor(int type);

extern void free_controller(bController *cont);
extern void free_controllers(ListBase *lb);
extern bController *copy_controller(bController *cont);
extern void copy_controllers(ListBase *lbn, ListBase *lbo);
extern void init_controller(bController *cont);
extern bController *new_controller(int type);

extern void free_actuator(bActuator *act);
extern void free_actuators(ListBase *lb);
extern bActuator *copy_actuator(bActuator *act);
extern void copy_actuators(ListBase *lbn, ListBase *lbo);
extern void init_actuator(bActuator *act);
extern bActuator *new_actuator(int type);

extern void set_sca_new_poins_ob(Object *ob);
extern void clear_sca_new_poins_ob(Object *ob);
extern void sca_remove_ob_poin(Object *obt, Object *ob);


/* editsca.c */

extern char *key_event_to_string(ushort event);
extern void make_unique_prop_names(char *str);


#endif /* GAME_H */

