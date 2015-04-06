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
add_subdirectory(imbuf)
add_subdirectory(jpeg)
add_subdirectory(misc)
add_subdirectory(render) 
add_subdirectory(sound)
add_subdirectory(makesdna)
add_subdirectory(src)

SET(PYTHON_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/Python/libs/python15.lib")
include_directories(lib/Python/include)

# output dna.c
ADD_CUSTOM_COMMAND( 
    OUTPUT ${CMAKE_BINARY_DIR}/dna.c 
    COMMAND makesdna ${CMAKE_BINARY_DIR}/dna.c
    DEPENDS makesdna
    VERBATIM) 

include_directories (./include)
include_directories (./render/extern/include/)
add_executable (blender ./src/usiblender.c ${CMAKE_BINARY_DIR}/dna.c ) 
target_link_libraries (blender bf_glut bf_edit bf_imbuf bf_avi bf_jpeg bf_misc bf_render bf_sound ${OPENGL_LIBRARIES} ${PYTHON_LIBRARY})