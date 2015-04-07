include_directories (../include)
include_directories (../render/extern/include/)

IF(WIN32)
    set(OB_ADD winstuff.c)
ENDIF(WIN32)

set(OB_EDIT edit.c editcurve.c editfont.c editipo.c 
	editmesh.c editobject.c isect.c scanfill.c editface.c
	sector.c life.c editgroup.c)

set(OB_GRAPHICS buttons.c drawimage.c 
	drawimasel.c drawipo.c displist.c 
	drawobject.c drawoops.c drawscene.c 
	drawseq.c drawtext.c drawview.c 
	editika.c editimasel.c editkey.c editlattice.c 
	editmball.c editoops.c drawmesh.c 
	editseq.c editsima.c editview.c filesel.c 
	headerbuttons.c mywindow.c os_specific.c playanim.c 
	screen.c space.c toets.c 
	toolbox.c view.c view2d.c 
	vpaint.c interface.c editsca.c packedFile.c)

set(OB_FILE genfile.c fileops.c readfile.c writefile.c)

#set(OB_STANDALONE $(DIR)/../makesdna/DNA.c keyval.c imageprocess.c noise.c psfont.c rct.c)
set(OB_STANDALONE keyval.c imageprocess.c noise.c psfont.c rct.c)

set(OB_MIXMODEL anim.c arithb.c blender.c curve.c effect.c 
	exotic.c font.c ika.c image.c imasel.c ipo.c 
	key.c library.c material.c mball.c mesh.c object.c oops.c 
	scene.c sequence.c text.c world.c property.c sca.c group.c)

set(OB_RENDER writeavi.c writemovie.c)

set(OB_RADIO radio.c radfactors.c radnode.c radpreprocess.c radpostprocess.c 
	raddisplay.c)

set(OB_PYTHON py_main.c py_vector.c py_matrix.c py_datablock.c
	py_interface.c py_draw.c py_demo.c py_nmesh.c py_nurbs.c)

	
add_library (bf_edit
	${OB_EDIT} ${OB_GRAPHICS} ${OB_FILE} ${OB_STANDALONE} ${OB_MIXMODEL} ${OB_RENDER} ${OB_RADIO} ${OB_PYTHON} ${OB_ADD})
target_link_libraries (bf_edit bf_glut)

add_executable (padding padding.c) 
target_link_libraries (padding bf_edit)