#include "AudioInput.hpp"
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <exception>

#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (1024)
#define NUM_SECONDS     (10)
#define NUM_CHANNELS    (2)
#define DITHER_FLAG     (0)

typedef float SAMPLE;
#define SAMPLE_SIZE (4)
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"

#define CLEAR( a ) { \
    int i; \
    for( i=0; i<FRAMES_PER_BUFFER*NUM_CHANNELS; i++ ) \
        ((unsigned char *)a)[i] = (SAMPLE_SILENCE); \
}

using namespace std;

ai::AudioInput::AudioInput()
{
    PaError err = paNoError;
    err = Pa_Initialize();
    if(err != paNoError){
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        throw err;
    }
}

ai::AudioInput::~AudioInput()
{
    PaError err = paNoError;
    err = Pa_Terminate();
    if( err != paNoError )
       printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
}

typedef struct
{
    float left_phase;
    float right_phase;
}
paTestData;

int myPaStreamCallback(const void *input,
                       void *output,
                       unsigned long frameCount,
                       const PaStreamCallbackTimeInfo *timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *userData){

    /* Cast data passed through stream to our structure. */
        paTestData *data = (paTestData*)userData;
        float *out = (float*)output;
        unsigned int i;
        (void) input; /* Prevent unused variable warning. */

        for( i=0; i<frameCount; i++ )
        {
            *out++ = data->left_phase;  /* left */
            *out++ = data->right_phase;  /* right */
            /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
            data->left_phase += 0.01f;
            /* When signal reaches top, drop back down. */
            if( data->left_phase >= 1.0f ) data->left_phase -= 2.0f;
            /* higher pitch so we can distinguish left and right. */
            data->right_phase += 0.03f;
            if( data->right_phase >= 1.0f ) data->right_phase -= 2.0f;
        }
        return 0;
}

static paTestData data;

void ai::AudioInput::Stream(int device)
{
    PaStreamParameters inputParameters, outputParameters;
    PaStream* stream;
    PaError err = paNoError;
    char *sampleBlock;
    int i;
    int numBytes;

    numBytes = FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE ;
    sampleBlock = (char *) malloc( numBytes );

    CLEAR( sampleBlock );

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultHighInputLatency ;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    outputParameters.channelCount = NUM_CHANNELS;
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultHighOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    /* Open an audio I/O stream. */
//    err = Pa_OpenDefaultStream( &stream,
//                                0,          /* no input channels */
//                                2,          /* stereo output */
//                                paFloat32,  /* 32 bit floating point output */
//                                SAMPLE_RATE,
//                                256,        /* frames per buffer, i.e. the number
//                                                   of sample frames that PortAudio will
//                                                   request from the callback. Many apps
//                                                   may want to use
//                                                   paFramesPerBufferUnspecified, which
//                                                   tells PortAudio to pick the best,
//                                                   possibly changing, buffer size.*/
//                                myPaStreamCallback, /* this is your callback function */
//                                &data ); /*This is a pointer that will be passed to
//                                                   your callback*/

    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              NULL, /* no callback, use blocking API */
              NULL ); /* no callback, so no callback userData */

    if( err != paNoError )
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );

    err = Pa_StartStream( stream );
    if( err != paNoError )
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );


    for( i=0; i<(60*SAMPLE_RATE)/FRAMES_PER_BUFFER; ++i )
    {
        err = Pa_WriteStream( stream, sampleBlock, FRAMES_PER_BUFFER );
        //if( err ) goto xrun;
        err = Pa_ReadStream( stream, sampleBlock, FRAMES_PER_BUFFER );
        //if( err ) goto xrun;
    }
}

void ai::AudioInput::Stop()
{
//    PaError err = paNoError;
//    err = Pa_StopStream( stream );
//    if( err != paNoError )
//        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
}
