/*
 * OpenAL basic functionality testprogram
 *
 * $Id: testopenal.c,v 1.1 2000/09/01 12:49:18 hans Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alkludge.h>
#include <AL/alut.h>

#define DATABUFFERSIZE (10 * (512 * 3) * 1024)

static void init(void);

static ALuint moving_source = 0;

static void *data = (void *) 0xDEADBEEF;

static void *context_id;

static void init( void ) {
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat back[]   = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	ALuint stereo;
	void *wave = NULL;
	ALsizei bits;
	ALsizei format;
	ALsizei size,freq;

	data = malloc(DATABUFFERSIZE);

	alListenerfv(AL_POSITION, zeroes );
	alListenerfv(AL_VELOCITY, zeroes );
	alListenerfv(AL_ORIENTATION, front );

	alGenBuffers( 1, &stereo);
	alGenSources( 1, &moving_source);

	alutLoadWAV("/tmp/test0.wav", &wave, &format, &size, &bits, &freq);
	alBufferData(stereo, format, wave, size, freq );
	free(wave);

	alSource3f( moving_source, AL_POSITION, 0.0, 0.0, 4.0 );
	alSourcefv( moving_source, AL_VELOCITY, zeroes );
	alSourcefv( moving_source, AL_ORIENTATION, back );
	alSourcei(  moving_source, AL_BUFFER, stereo );
	alSourcei(  moving_source, AL_SOURCE_LOOPING, AL_FALSE);
}

int main(int argc, char* argv[] ) {
	int attrlist[] = { ALC_FREQUENCY, 22050,
			   ALC_INVALID };
	/* Initialize ALUT. */
	context_id = alcCreateContext(attrlist);
	if(context_id == NULL) {
		return 1;
	}

	init( );

	alSourcePlay( moving_source );
	sleep(5);

	free(data);
	alcDestroyContext(context_id);

	return 0;
}
