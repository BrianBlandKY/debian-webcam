#ifndef PORTAUDIOSTREAM_HPP
#define PORTAUDIOSTREAM_HPP

#include "Stream.hpp"

#include <string>
#include <portaudio.h>
#include <boost/thread.hpp>
#include <boost/signals2.hpp>
#include <boost/date_time.hpp>

typedef boost::signals2::signal<void (float)> portAudio_sig;

typedef struct
{
    portAudio_sig sig;
}
paUserData;

class PortAudioStream : public Stream
{
public:
    boost::signals2::connection connect(const portAudio_sig::slot_type &subscriber);
    PortAudioStream();
    ~PortAudioStream();
    void Open();
    void Stop();

private:
    boost::thread* intThread = nullptr;
    portAudio_sig sig;
    static int defaultCallback( const void *inputBuffer,
                         void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData );
    float SAMPLE;
    const PaSampleFormat PA_SAMPLE_TYPE = paFloat32;
    const int SAMPLE_RATE = 44100;
    const int FRAMES_PER_BUFFER = 1024;
    const int NUM_CHANNELS = 2;
    const int DITHER_FLAG = 0;
    const int SAMPLE_SIZE = 4;
    const float SAMPLE_SILENCE = 0.0;
    void clear(char * a);
};

#endif // PORTAUDIOSTREAM_HPP
