#ifndef AUDIO_SYNTH_PIPELINESYNTHESIZER_H
#define AUDIO_SYNTH_PIPELINESYNTHESIZER_H

#include <unordered_map>

#include "MidiEventProcessor.h"
#include "MidiRollEvent.h"
#include "MidiRollKeyEvent.h"
#include "MidiRollChannelOnEvent.h"

#include "AudioSample.h"
#include "Channel.h"
#include "InstrumentConfig.h"

namespace Audio::Synth {
    class PipelineSynthesizer : public Midi::MidiEventProcessor {
    public:
        PipelineSynthesizer(int sampleRate, double volume,
                            InstrumentConfig config = {}, bool postFx = false, bool useVelocity = false);

        Audio::AudioSample sample();

        bool isPlaying();

        void processMidiEvent(Midi::MidiRollEvent &event) override;

        void processMidiEvent(Midi::MidiRollChannelOnEvent &event) override;

        void processMidiEvent(Midi::MidiRollKeyEvent &event) override;

    private:
        int _sampleRate;
        double _volume;
        InstrumentConfig _config;
        bool _postFx;
        bool _useVelocity;
        long _transportSample = 0;
        std::unordered_map<int, Channel> _channels;
    };
}

#endif
