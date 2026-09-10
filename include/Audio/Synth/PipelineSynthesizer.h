#ifndef AUDIO_SYNTH_PIPELINESYNTHESIZER_H
#define AUDIO_SYNTH_PIPELINESYNTHESIZER_H

#include <unordered_map>

#include "MidiEventProcessor.h"
#include "MidiRollEvent.h"
#include "MidiRollKeyEvent.h"
#include "MidiRollChannelOnEvent.h"

#include "AudioSample.h"
#include "Channel.h"

namespace Audio::Synth {
    class PipelineSynthesizer : public Midi::MidiEventProcessor {
    public:
        PipelineSynthesizer(int sampleRate, double volume, int instrumentIndex,
                            bool useVelocity = false, bool channelPostFx = false);

        Audio::AudioSample sample();

        bool isPlaying();

        void processMidiEvent(Midi::MidiRollEvent &event) override;

        void processMidiEvent(Midi::MidiRollChannelOnEvent &event) override;

        void processMidiEvent(Midi::MidiRollKeyEvent &event) override;

    private:
        int _sampleRate;
        double _volume;
        int _instrumentIndex;
        bool _useVelocity;
        bool _channelPostFx;
        long _transportSample = 0;
        std::unordered_map<int, Channel> _channels;
    };
}

#endif
