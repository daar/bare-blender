include_directories (../include) 
include_directories (../../../include) 
include_directories (../../extern/include) 
include_directories (../../../glut-win/gl) 

add_library (bf_render
edgeRender.c
envmap.c
errorHandler.c
initrender.c
matrixops.c
pixelblending.c
pixelshading.c
previewrender.c
rendercore.c
renderfg.c
shadbuf.c
texture.c
vanillaRenderPipe.c
vectorops.c
zbuf.c
zbufferdatastruct.c)

target_link_libraries (bf_render bf_edit)