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

/* Version: $Id: padding.c,v 1.4 2000/07/22 00:56:14 ton Exp $ */


#include "blender.h"
#include "util.h"
#include "screen.h"
#include "file.h"
#include "sequence.h"
#include "effect.h"
#include "ika.h"
#include "oops.h"
#include "imasel.h"
#include "sector.h"
#include "game.h"
#include "sound.h"
#include "iff.h"


int main(argc,argv)
int argc;
char **argv;
{
printf("  Link 16 %d \n",  sizeof(struct Link) );
printf("  ListBase 16 %d \n",  sizeof(struct ListBase) );
printf("  MemHead 48 %d \n",  sizeof(struct MemHead) );
printf("  MemTail 8 %d \n",  sizeof(struct MemTail) );
printf("  vec2s 4 %d \n",  sizeof(struct vec2s) );
printf("  vec2i 8 %d \n",  sizeof(struct vec2i) );
printf("  vec2f 8 %d \n",  sizeof(struct vec2f) );
printf("  vec2d 16 %d \n",  sizeof(struct vec2d) );
printf("  vec3i 12 %d \n",  sizeof(struct vec3i) );
printf("  vec3f 12 %d \n",  sizeof(struct vec3f) );
printf("  vec3d 24 %d \n",  sizeof(struct vec3d) );
printf("  vec4i 16 %d \n",  sizeof(struct vec4i) );
printf("  vec4f 16 %d \n",  sizeof(struct vec4f) );
printf("  vec4d 32 %d \n",  sizeof(struct vec4d) );
printf("  rcti 16 %d \n",  sizeof(struct rcti) );
printf("  rctf 16 %d \n",  sizeof(struct rctf) );
printf("  ID 64 %d \n",  sizeof(struct ID) );
printf("  Library 248 %d \n",  sizeof(struct Library) );
printf("  Ipo 104 %d \n",  sizeof(struct Ipo) );
printf("  KeyBlock 40 %d \n",  sizeof(struct KeyBlock) );
printf("  Key 152 %d \n",  sizeof(struct Key) );
printf("  ScriptLink 20 %d \n",  sizeof(struct ScriptLink) );
printf("  TextLine 32 %d \n",  sizeof(struct TextLine) );
printf("  Text 144 %d \n",  sizeof(struct Text) );
printf("  PackedFile 24 %d \n",  sizeof(struct PackedFile) );
printf("  Camera 132 %d \n",  sizeof(struct Camera) );
printf("  Image 360 %d \n",  sizeof(struct Image) );
printf("  anim 0 %d \n",  sizeof(struct anim) );
printf("  ImBuf 0 %d \n",  sizeof(struct ImBuf) );
printf("  MTex 88 %d \n",  sizeof(struct MTex) );
printf("  Object 708 %d \n",  sizeof(struct Object) );
printf("  Tex 208 %d \n",  sizeof(struct Tex) );
printf("  PluginTex 368 %d \n",  sizeof(struct PluginTex) );
printf("  CBData 24 %d \n",  sizeof(struct CBData) );
printf("  ColorBand 392 %d \n",  sizeof(struct ColorBand) );
printf("  EnvMap 152 %d \n",  sizeof(struct EnvMap) );
printf("  Lamp 236 %d \n",  sizeof(struct Lamp) );
printf("  Wave 72 %d \n",  sizeof(struct Wave) );
printf("  Material 324 %d \n",  sizeof(struct Material) );
printf("  VFont 344 %d \n",  sizeof(struct VFont) );
printf("  VFontData 0 %d \n",  sizeof(struct VFontData) );
printf("  MetaElem 88 %d \n",  sizeof(struct MetaElem) );
printf("  MetaBall 176 %d \n",  sizeof(struct MetaBall) );
printf("  BoundBox 0 %d \n",  sizeof(struct BoundBox) );
printf("  BezTriple 60 %d \n",  sizeof(struct BezTriple) );
printf("  BPoint 28 %d \n",  sizeof(struct BPoint) );
printf("  Nurb 72 %d \n",  sizeof(struct Nurb) );
printf("  Curve 312 %d \n",  sizeof(struct Curve) );
printf("  Path 0 %d \n",  sizeof(struct Path) );
printf("  IpoCurve 96 %d \n",  sizeof(struct IpoCurve) );
printf("  MFace 12 %d \n",  sizeof(struct MFace) );
printf("  MFaceInt 20 %d \n",  sizeof(struct MFaceInt) );
printf("  TFace 80 %d \n",  sizeof(struct TFace) );
printf("  MVert 20 %d \n",  sizeof(struct MVert) );
printf("  MCol 4 %d \n",  sizeof(struct MCol) );
printf("  MSticky 8 %d \n",  sizeof(struct MSticky) );
printf("  Mesh 264 %d \n",  sizeof(struct Mesh) );
printf("  OcInfo 0 %d \n",  sizeof(struct OcInfo) );
printf("  Lattice 104 %d \n",  sizeof(struct Lattice) );
printf("  LBuf 16 %d \n",  sizeof(struct LBuf) );
printf("  Life 440 %d \n",  sizeof(struct Life) );
printf("  World 284 %d \n",  sizeof(struct World) );
printf("  Radio 40 %d \n",  sizeof(struct Radio) );
printf("  RenderData 736 %d \n",  sizeof(struct RenderData) );
printf("  Base 40 %d \n",  sizeof(struct Base) );
printf("  Scene 916 %d \n",  sizeof(struct Scene) );
printf("  FreeCamera 40 %d \n",  sizeof(struct FreeCamera) );
printf("  BGpic 48 %d \n",  sizeof(struct BGpic) );
printf("  View3D 416 %d \n",  sizeof(struct View3D) );
printf("  View2D 112 %d \n",  sizeof(struct View2D) );
printf("  SpaceIpo 208 %d \n",  sizeof(struct SpaceIpo) );
printf("  SpaceButs 176 %d \n",  sizeof(struct SpaceButs) );
printf("  SpaceSeq 144 %d \n",  sizeof(struct SpaceSeq) );
printf("  SpaceFile 336 %d \n",  sizeof(struct SpaceFile) );
printf("  direntry 0 %d \n",  sizeof(struct direntry) );
printf("  SpaceOops 168 %d \n",  sizeof(struct SpaceOops) );
printf("  SpaceImage 160 %d \n",  sizeof(struct SpaceImage) );
printf("  SpaceText 112 %d \n",  sizeof(struct SpaceText) );
printf("  UserDef 460 %d \n",  sizeof(struct UserDef) );
printf("  bScreen 144 %d \n",  sizeof(struct bScreen) );
printf("  ScrVert 32 %d \n",  sizeof(struct ScrVert) );
printf("  ScrEdge 40 %d \n",  sizeof(struct ScrEdge) );
printf("  ScrArea 304 %d \n",  sizeof(struct ScrArea) );
printf("  FileGlobal 16 %d \n",  sizeof(struct FileGlobal) );
printf("  StripElem 80 %d \n",  sizeof(struct StripElem) );
printf("  Strip 120 %d \n",  sizeof(struct Strip) );
printf("  PluginSeq 264 %d \n",  sizeof(struct PluginSeq) );
printf("  Sequence 208 %d \n",  sizeof(struct Sequence) );
printf("  Editing 48 %d \n",  sizeof(struct Editing) );
printf("  Effect 24 %d \n",  sizeof(struct Effect) );
printf("  BuildEff 32 %d \n",  sizeof(struct BuildEff) );
printf("  PartEff 168 %d \n",  sizeof(struct PartEff) );
printf("  Particle 0 %d \n",  sizeof(struct Particle) );
printf("  WaveEff 64 %d \n",  sizeof(struct WaveEff) );
printf("  Deform 240 %d \n",  sizeof(struct Deform) );
printf("  Limb 48 %d \n",  sizeof(struct Limb) );
printf("  Ika 192 %d \n",  sizeof(struct Ika) );
printf("  Oops 64 %d \n",  sizeof(struct Oops) );
printf("  SpaceImaSel 752 %d \n",  sizeof(struct SpaceImaSel) );
printf("  ImaDir 0 %d \n",  sizeof(struct ImaDir) );
printf("  OneSelectableIma 0 %d \n",  sizeof(struct OneSelectableIma) );
printf("  bProperty 80 %d \n",  sizeof(struct bProperty) );
printf("  bNearSensor 48 %d \n",  sizeof(struct bNearSensor) );
printf("  bMouseSensor 8 %d \n",  sizeof(struct bMouseSensor) );
printf("  bTouchSensor 48 %d \n",  sizeof(struct bTouchSensor) );
printf("  bKeyboardSensor 8 %d \n",  sizeof(struct bKeyboardSensor) );
printf("  bPropertySensor 112 %d \n",  sizeof(struct bPropertySensor) );
printf("  bCollisionSensor 40 %d \n",  sizeof(struct bCollisionSensor) );
printf("  bRadarSensor 40 %d \n",  sizeof(struct bRadarSensor) );
printf("  bSensor 88 %d \n",  sizeof(struct bSensor) );
printf("  bController 104 %d \n",  sizeof(struct bController) );
printf("  bExpressionCont 128 %d \n",  sizeof(struct bExpressionCont) );
printf("  bActuator 80 %d \n",  sizeof(struct bActuator) );
printf("  bAddObjectActuator 16 %d \n",  sizeof(struct bAddObjectActuator) );
printf("  bSoundActuator 16 %d \n",  sizeof(struct bSoundActuator) );
printf("  bSound 264 %d \n",  sizeof(struct bSound) );
printf("  bPropertyActuator 72 %d \n",  sizeof(struct bPropertyActuator) );
printf("  bObjectActuator 80 %d \n",  sizeof(struct bObjectActuator) );
printf("  bIpoActuator 48 %d \n",  sizeof(struct bIpoActuator) );
printf("  bCameraActuator 32 %d \n",  sizeof(struct bCameraActuator) );
printf("  bConstraintActuator 56 %d \n",  sizeof(struct bConstraintActuator) );
printf("  SpaceSound 156 %d \n",  sizeof(struct SpaceSound) );
}

