#include "PortAudioStream.hpp"
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <exception>

PortAudioStream::PortAudioStream()
{
    PaError err = paNoError;
    err = Pa_Initialize();
}

PortAudioStream::~PortAudioStream()
{

}

void
PortAudioStream::clear(char *a ) {
    int i;
    for( i=0; i<FRAMES_PER_BUFFER*NUM_CHANNELS; i++ )
        ((unsigned char *)a)[i] = (SAMPLE_SILENCE);
}

void
PortAudioStream::Open()
{
    PaStream* stream = nullptr;
    PaError err = paNoError;

    try{
        PaStreamParameters inputParameters;

        char *sampleBlock;
        int numBytes;
        numBytes = this->FRAMES_PER_BUFFER * this->NUM_CHANNELS * this->SAMPLE_SIZE;
        sampleBlock = (char*)malloc(numBytes);

        if(sampleBlock == NULL){
            printf("Could not allocate record array.\n");
            exit(1);
        }
        this->clear(sampleBlock);

        inputParameters.device = Pa_GetDefaultInputDevice();
        inputParameters.channelCount = this->NUM_CHANNELS;
        inputParameters.sampleFormat = this->PA_SAMPLE_TYPE;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency;
        inputParameters.hostApiSpecificStreamInfo = NULL;

        err = Pa_OpenStream(
                    &stream,
                    &inputParameters,
                    NULL,
                    this->SAMPLE_RATE,
                    this->FRAMES_PER_BUFFER,
                    paClipOff,
                    this->defaultCallback,
                    this);

        if(err != paNoError) goto exit;

        err = Pa_StartStream(stream);

        if(err != paNoError) goto exit;

        while(true){
            boost::this_thread::interruption_point();
        }

        exit:
            throw err;
    }
    catch(boost::thread_interrupted){
        std::cout << "closing microhone..." << std::endl;
        err = Pa_CloseStream(stream);
        err = Pa_Terminate();
    }
    catch(std::exception ex){
        std::cout << "unexpected err " << ex.what() << std::endl;
    }
}

int
PortAudioStream::defaultCallback(const void *inputBuffer,
                           void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    PortAudioStream *data = (PortAudioStream*)userData;
    data->sig(0);
    return 0;
}
