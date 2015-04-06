include_directories (../include) 
include_directories (../../../include) 
include_directories (../../extern/include) 


add_library (bf_sound
pthread_cancel.c
sound.c
sound_linux.c)