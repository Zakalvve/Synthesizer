#ifndef AUDIO_SYNTH_CHANNEL_H
#define AUDIO_SYNTH_CHANNEL_H

#include <string>

#include "AudioContext.h"
#include "AudioPipeline.h"
#include "AudioSample.h"
#include "SignalGraph.h"
#include "Instrument.h"

namespace Audio::Synth {
    class Channel {
    public:
        Channel(Instrument instrument, double pan, double gain);

        void setPostFx(Processing::SignalGraph postFx);

        void playNote(const std::string &noteName, double velocity, long transportSample);

        void releaseNote(const std::string &noteName);

        Audio::AudioSample sample(Processing::AudioContext &ctx);

        bool isActive();

    private:
        Instrument _instrument;
        Processing::AudioPipeline _channelGraph;
        bool _hasPostFx = false;
        Processing::AudioPipeline _postFx;
    };
}

#endif
