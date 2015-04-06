include_directories (../include) 

add_executable (makesdna makesdna.c) 
target_link_libraries (makesdna bf_misc)