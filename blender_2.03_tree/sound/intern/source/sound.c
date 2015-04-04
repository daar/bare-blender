
/*  sound.c   june 2000
 *  
 *  support for wav files
 * 
 * 
 *  ton roosendaal
 * Version: $Id: sound.c,v 1.20 2000/09/13 15:03:07 hans Exp $
 */

#include "blender.h"
#include "sound.h"
#include "file.h"
#include "packedFile.h"


/* presume that FALSE == 0 and TRUE != 0 */
#if defined(WIN32) || defined (__FreeBSD__)
int noaudio = FALSE;
#else
int noaudio = TRUE;
#endif

#ifdef USE_OPENAL
#ifndef _WIN32
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alkludge.h>
#endif
#include <AL/alut.h>

static ALfloat listenerPos[]={0.0,0.0,0.0};
static ALfloat listenerOri[]={0.0,0.0,1.0, 0.0,1.0,0.0};

static ALfloat source0Pos[]={ 0.0, 0.0, 1.0};
static ALfloat source0Vel[]={ 0.0, 0.0, 1.0};

static ALfloat source1Pos[]={ 2.0, 0.0,-1.0};
static ALfloat source1Vel[]={ 0.0, 0.0, 1.0};
static ALfloat zeroVel[]={ 0.0, 0.0, 0.0};

#define NUM_BUFFERS 256
#define NUM_SOURCES 256
#define NUM_ENVIRONMENTS 1

static char	in_use[NUM_BUFFERS];
static ALuint	buffer[NUM_BUFFERS];
static ALuint	source[NUM_SOURCES];
static ALuint  environment[NUM_ENVIRONMENTS];

static int sound_init_ok = FALSE;

void sound_exit_trap()
{
	if (sound_init_ok == FALSE) {
		printf("exit() called in OpenAL library. Please restart blender with -noaudio option.\n");
	}
}

void play_sound(Object *object, int alindex);

void initialize_sound(int * argc, char ** argv)
{
	ALsizei size,freq;
	ALvoid *data;
	int ch;

	/* testing code
	void *wave = NULL;
	ALsizei bits;
	ALsizei format;
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	*/

	if (!noaudio) {
		atexit(sound_exit_trap);

#if defined (__FreeBSD__) || defined(__linux__)
		/* The new linux code do not return anything ... checks removed */
		if (alutInit(argc, argv)) {
			alGenBuffers(NUM_BUFFERS,buffer);
			alGenEnvironmentIASIG(NUM_ENVIRONMENTS,environment);
			alGenSources(NUM_SOURCES,source);
		} else {
			noaudio = 1;
			printf("\nUnable to open audio context, "
				   "Blender continues with -noaudio\n");
		}
#else /* That would be win32 methinks. Others don't do audio. */
		alutInit(argc, argv);
		alGenBuffers(NUM_BUFFERS,buffer);
		alGenEnvironmentIASIG(NUM_ENVIRONMENTS,environment);
		alGenSources(NUM_SOURCES,source);
#endif

		/* some testing code that f*cks up in_use[0]
		alutLoadWAV("/tmp/test0.wav", &wave, &format, &size, &bits, &freq);
		alBufferData(buffer[0], format, wave, size, freq );
		free(wave);
		alSource3f( source[0], AL_POSITION, 0.0, 0.0, 4.0 );
		alSourcefv( source[0], AL_VELOCITY, zeroes );
		alSourcefv( source[0], AL_ORIENTATION, back );
		alSourcei(  source[0], AL_BUFFER, buffer[0] );
		// alSourcei(  source[0], AL_SOURCE_LOOPING, AL_FALSE);
		alSourcePlay(source[0]);
		sleep(3);
		//in_use[0] = TRUE;
		//play_sound(NULL, 0);
		*/

		// we've survived the initialization, don't print 
		// message at exit time
		sound_init_ok = TRUE;
	}
}

void play_sound(Object *object, int alindex)
{
	int i, j;
	ALfloat listenOri[6];
	float mat[4][4];
	static int srcindex = 0;

	if (!noaudio) {
		if (alindex >= 0 && alindex < NUM_BUFFERS && in_use[alindex]) {
			// we'll have to do our own transformations. OpenAL (windows)
			// doesn't support AL_ORIENTATION

			if (object != NULL) {
				Mat4MulMat4(mat, object->obmat, G.vd->viewmat);
			} else {
				// only useful for testing.
				mat[3][0] = 0.0;
				mat[3][1] = 1.0;
				mat[3][2] = 0.0;
				mat[3][3] = 0.0;
			}

			alListenerfv(AL_ORIENTATION, listenerOri);
			alListenerfv(AL_POSITION, listenerPos);
			alListenerfv(AL_VELOCITY, zeroVel);
			alSourcefv(source[srcindex], AL_POSITION, mat[3]);
			alSourcef(source[srcindex], AL_PITCH, 1.0f);
			alSourcef(source[srcindex], AL_GAIN, 1.0f);
			alSourcefv(source[srcindex], AL_VELOCITY, zeroVel);
			alSourcei(source[srcindex], AL_BUFFER, buffer[alindex]);
			// alSourcei(source[srcindex], AL_SOURCE_LOOPING, AL_FALSE);
			//printf("P(%d-%d) ", alindex, srcindex);
			//fflush(0);
			alSourcePlay(source[srcindex]);

			srcindex++;

			if (srcindex > NUM_SOURCES) {
				srcindex = 0;
			}
		}
	}
}

int register_sound(PackedFile * pf)
{
	int i = 0;
	ALsizei size, freq;
	ALenum format;
	ALvoid *data;

	if (!noaudio) {
		for (i = 0; i < NUM_BUFFERS; i++) {
			if (in_use[i] == FALSE) {
				break;
			}
		}
		if (i >= NUM_BUFFERS) {
			i = -1;
			printf("No more audio buffers available (max %d)\n",
			       NUM_BUFFERS);
		} else {
			alutLoadWAVMemory(pf->data, &format, &data, &size, &freq);
			alBufferData(buffer[i], format, data, size, freq);
			alutUnloadWAV(format, data, size, freq);
			in_use[i] = TRUE;
		}
	}

	return(i);
}

void unregister_sound(int alindex)
{
	if (!noaudio) {
		if (alindex > -1 && alindex < NUM_BUFFERS && in_use[alindex]) {
			// sound can still be playing
			// we can't test it at this moment

			// memory isn't freed by OpenAL (windows) anyhow
			// but it's recycled at next use...
			in_use[alindex] = FALSE;
		}
	}
}

#else /* USE_OPENAL */

// create stubs...

void initialize_sound(int * argc, char ** argv){}
void play_sound(Object *object, int alindex){}
int register_sound(PackedFile * pf){ return(-1);}
void unregister_sound(int alindex){}

#endif /* USE_OPENAL */

void free_sound(bSound *sound)
{
	if(sound->data) freeN(sound->data);
	sound->data= NULL;
	if (sound->packedfile) {
		freePackedFile(sound->packedfile);
		sound->packedfile = NULL;
	}

	unregister_sound(sound->alindex);
}

bSound *add_sound(char *name)
{
	bSound *sound;
	int file, len;
	char *libname, str[256], strtest[256];
	
	strcpy(str, name);
	convertstringcode(str);
	
	file= open(str, O_BINARY|O_RDONLY);
	if(file== -1) return 0;
	close(file);
	
	// eerst zoeken naar eenzelfde sound
	sound= G.main->sound.first;
	while(sound) {
		strcpy(strtest, sound->name);
		convertstringcode(strtest);
		if( strcmp(strtest, str)==0 ) {
			strcpy(sound->name, name);	// for stringcode
			sound->id.us++;
			return sound;
		}
		sound= sound->id.next;
	}

	len= strlen(name);
	
	while (len > 0 && name[len - 1] != '/' && name[len - 1] != '\\') len--;
	libname= name+len;
	
	sound = alloc_libblock(&G.main->sound, ID_SO, libname);
	strcpy(sound->name, name);

	return sound;
}


void * read_wav_data(bSound * sound, PackedFile * pf)
{
	int i, temp, reallen;
	short shortbuf, *temps;
	int longbuf;
	char buffer[25];
	char *data = NULL;
	char *tempc;
		
	rewindPackedFile(pf);

	// is it a file in "RIFF WAVE fmt" format ?
	//      there's 4 bytes of rLen in between, not used here

	if (readPackedFile(pf, buffer, 16) != 16) {
		printf("File to short\n");
		return NULL;
	}

	if(!(memcmp(buffer, "RIFF", 4) &&
	     memcmp(&(buffer[8]), "WAVEfmt ", 8))) {


		readPackedFile(pf, &i, 4);// start of data
		if(G.order==B_ENDIAN) SWITCH_INT(i);

		readPackedFile(pf, &shortbuf, 2);
		if(G.order==B_ENDIAN) SWITCH_SHORT(shortbuf);

		if(shortbuf != 1 ) {
			printf("Unsupported packtype\n");
			return NULL;
		}

		readPackedFile(pf, &shortbuf, 2);
		if(G.order==B_ENDIAN) SWITCH_SHORT(shortbuf);

		if(shortbuf != 1 && shortbuf != 2) {
			printf("Unsupported number of channels\n");
			return NULL;		// no mono or stereo
		}

		sound->channels = shortbuf;

		readPackedFile(pf, &longbuf, 4);
		if(G.order==B_ENDIAN) SWITCH_INT(longbuf);

		sound->rate = longbuf;

		// Ton's code to determine the number of bits
		readPackedFile(pf, &temp, 4);
		if(G.order==B_ENDIAN) SWITCH_INT(temp);
		if(sound->channels && sound->rate)
		    sound->bits= 8*temp/(sound->rate * sound->channels);
		// printf("%08x %d\n", temp, sound->bits);

		// Frank's code to determine the number of bits
		readPackedFile(pf, &shortbuf, 2);
		readPackedFile(pf, &shortbuf, 2);
		if(G.order==B_ENDIAN) SWITCH_SHORT(shortbuf);
		// printf("bits = %d\n", shortbuf);
		sound->bits = shortbuf;

		seekPackedFile(pf, i-16, SEEK_CUR);
		readPackedFile(pf, buffer, 4);

		while(memcmp(buffer, "data", 4)!=0) {
			// PRINT4(c, c, c, c, buffer[0], buffer[1],
			// buffer[2], buffer[3]);
			if (readPackedFile(pf, &i, 4) != 4) {
				break;
			}
			if(G.order==B_ENDIAN) SWITCH_INT(i);
			seekPackedFile(pf, i, SEEK_CUR);

			if (readPackedFile(pf, buffer, 4) != 4) {
				break;
			}
		}
		
		if (memcmp(buffer, "data", 4) !=0) {
			printf("No data found\n");
			return NULL;		// no data
		}

		readPackedFile(pf, &longbuf, 4); 
		if(G.order==B_ENDIAN) SWITCH_INT(longbuf);

		if (sound->bits == 8) {
			data = (char *)mallocN(2 * longbuf, "sound data");
		} else {
			data = (char *)mallocN(longbuf, "sound data");
		}
		sound->len = longbuf;

		if(data) {
			readPackedFile(pf, data, sound->len);
			
			// data is only used to draw!
			if (sound->bits == 8) {
				temps = (short *) data;
				tempc = (char *) data;
				for (i = sound->len - 1; i >= 0; i--) {
					temps[i] = tempc[i] << 8;
				}
			} else {
				if(G.order==B_ENDIAN) {
					temps= (short *)data;
					for(i=0; i< sound->len / 2; i++, temps++) {
						SWITCH_SHORT(*temps);
					}
				}
			}
		}
	}
	else {
		printf("No Wav file: %s\n", sound->name);
	}

	return(data);
}


void init_sound(bSound *sound)
{
	PackedFile * pf;

	if(sound) {
		if(sound->data == NULL) {
			if (sound->packedfile) {
				pf = sound->packedfile;
			} else {
				pf = newPackedFile(sound->name);
				if (G.fileflags & G_AUTOPACK) {
					sound->packedfile = pf;
				}
			}
			if (pf != NULL) {
				sound->data = read_wav_data(sound, pf);
				// printf("Setting sound->data to %08x\n", sound->data);

				if (sound->data) {
					sound->alindex = register_sound(pf);
				}

				if (pf != sound->packedfile) {
					freePackedFile(pf);
				}
			}
		}
	}
}
