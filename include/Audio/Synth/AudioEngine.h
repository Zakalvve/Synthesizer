#ifndef AUDIO_SYNTH_AUDIOENGINE_H
#define AUDIO_SYNTH_AUDIOENGINE_H

#include <unordered_map>

#include "MidiEventProcessor.h"
#include "MidiRollEvent.h"
#include "MidiRollKeyEvent.h"
#include "MidiRollChannelOnEvent.h"

#include "AudioSample.h"
#include "Channel.h"

namespace Audio::Synth {
    class AudioEngine : public Midi::MidiEventProcessor {
    public:
        AudioEngine(int sampleRate, double volume, int synthIndex,
                    bool useVelocity = false, bool channelPostFx = false);

        Audio::AudioSample sample();

        bool isPlaying();

        void processMidiEvent(Midi::MidiRollEvent &event) override;

        void processMidiEvent(Midi::MidiRollChannelOnEvent &event) override;

        void processMidiEvent(Midi::MidiRollKeyEvent &event) override;

    private:
        int _sampleRate;
        double _volume;
        int _synthIndex;
        bool _useVelocity;
        bool _channelPostFx;
        long _transportSample = 0;
        std::unordered_map<int, Channel> _channels;
    };
}

#endif
