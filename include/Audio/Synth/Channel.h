#ifndef AUDIO_SYNTH_CHANNEL_H
#define AUDIO_SYNTH_CHANNEL_H

#include <string>

#include "AudioContext.h"
#include "AudioPipeline.h"
#include "AudioSample.h"
#include "Instrument.h"
#include "InstrumentConfig.h"

namespace Audio::Synth {
    class Channel {
    public:
        explicit Channel(const InstrumentConfig &config, bool postFx = false);

        void playNote(const std::string &noteName, double velocity, long transportSample);

        void releaseNote(const std::string &noteName);

        Audio::AudioSample sample(Processing::AudioContext &ctx);

        bool isActive();

    private:
        Instrument _instrument;
        bool _hasPostFx = false;
        Processing::AudioPipeline _postFx;
        double _volume = 0.5;
        double _pan = 0.0;
    };
}

#endif
