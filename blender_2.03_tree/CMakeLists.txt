cmake_minimum_required (VERSION 2.8)
project (bf_blender)

add_definitions(-DWITHOUT_PYTHON)
add_definitions(-DDEBUG)

find_package(OpenGL)
IF(APPLE)
    SET(CMAKE_CXX_LINK_FLAGS "-framework OpenGL -framework GLUT")
	
    add_subdirectory(glut-x11)
ELSEIF(WIN32)
    add_subdirectory(glut-win)
	include_directories (./glut-win/gl) 
ELSE(UNIX)
    add_subdirectory(glut-x11)
ENDIF(APPLE)
 
#BeOS is missing, no worries though

add_subdirectory(avi)
add_subdirectory(datafiles)
add_subdirectory(imbuf)
add_subdirectory(jpeg)
add_subdirectory(misc)
add_subdirectory(render) 
add_subdirectory(sound)
add_subdirectory(makesdna)
add_subdirectory(src)

SET(PYTHON_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/Python/libs/python15.lib")
include_directories(lib/Python/include)

# output .Bfont.c
add_custom_command( 
    OUTPUT ${CMAKE_SOURCE_DIR}/datafiles/Bfont.c 
    COMMAND datatoc .Bfont
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/datafiles
    DEPENDS datatoc
    VERBATIM) 
	
# output splash.jpg.c
add_custom_command( 
    OUTPUT ${CMAKE_SOURCE_DIR}/datafiles/splash.jpg.c 
    COMMAND datatoc splash.jpg
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/datafiles
    DEPENDS datatoc
    VERBATIM) 

# output blenderbuttons.jpg.c
add_custom_command( 
    OUTPUT ${CMAKE_SOURCE_DIR}/datafiles/blenderbuttons.c 
    COMMAND datatoc blenderbuttons
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/datafiles
    DEPENDS datatoc
    VERBATIM) 

# output B.blend.c
add_custom_command( 
    OUTPUT ${CMAKE_SOURCE_DIR}/datafiles/B.blend.c 
    COMMAND datatoc .B.blend
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/datafiles
    DEPENDS datatoc
    VERBATIM) 
	
# output Bfs.c
add_custom_command( 
    OUTPUT ${CMAKE_SOURCE_DIR}/datafiles/Bfs.c 
    COMMAND datatoc .Bfs
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/datafiles
    DEPENDS datatoc
    VERBATIM) 

set(OB_DATA ${CMAKE_SOURCE_DIR}/datafiles/Bfont.c ${CMAKE_SOURCE_DIR}/datafiles/splash.jpg.c ${CMAKE_SOURCE_DIR}/datafiles/blenderbuttons.c ${CMAKE_SOURCE_DIR}/src/cmap.tga.c ${CMAKE_SOURCE_DIR}/src/cmovie.tga.c ${CMAKE_SOURCE_DIR}/datafiles/B.blend.c ${CMAKE_SOURCE_DIR}/datafiles/Bfs.c )

# output dna.c
add_custom_command( 
    OUTPUT ${CMAKE_BINARY_DIR}/dna.c 
    COMMAND makesdna ${CMAKE_BINARY_DIR}/dna.c
    DEPENDS makesdna
    VERBATIM) 

include_directories (./include)
include_directories (./render/extern/include/)
add_executable (blender ${CMAKE_BINARY_DIR}/dna.c  ${OB_DATA} ./src/usiblender.c) 
target_link_libraries (blender bf_glut bf_edit bf_imbuf bf_avi bf_jpeg bf_misc bf_render bf_sound ${OPENGL_LIBRARIES} ${PYTHON_LIBRARY})
