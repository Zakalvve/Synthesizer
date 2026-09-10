#ifndef AUDIO_SYNTH_INSTRUMENT_H
#define AUDIO_SYNTH_INSTRUMENT_H

#include <string>
#include <vector>

#include "AudioContext.h"
#include "AudioPipeline.h"
#include "InstrumentConfig.h"

namespace Audio::Synth {
    class Instrument {
    public:
        explicit Instrument(const InstrumentConfig &config);

        void playNote(const std::string &noteKey, double frequency, double velocity, long transportSample);

        void releaseNote(const std::string &noteKey);

        double sample(Processing::AudioContext &ctx);

        bool isActive() const;

    private:
        struct Voice {
            bool inUse = false;
            bool gate = false;
            double frequency = 0.0;
            double velocity = 1.0;
            long triggerSample = 0;
            long endNoteSample = -1;
            std::string noteKey;
            long order = 0;
        };

        int findFreeOrOldest();

        InstrumentConfig _config;
        Processing::AudioPipeline _pipeline;
        std::vector<Voice> _voices;
        long _orderCounter = 0;
        int _attackDur = 0;
        int _decayDur = 0;
        int _releaseDur = 0;
    };
}

#endif
