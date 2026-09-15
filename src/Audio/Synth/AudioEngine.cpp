#include "AudioEngine.h"

#include <utility>

#include "AudioContext.h"
#include "SynthFactory.h"
#include "PolyphonicSynthesizer.h"

namespace Audio::Synth {
    AudioEngine::AudioEngine(int sampleRate,
        double volume,
        int synthIndex,
        bool useVelocity,
        bool channelPostFx
    ) : _sampleRate(sampleRate),
    _volume(volume),
    _synthIndex(synthIndex),
    _useVelocity(useVelocity),
    _channelPostFx(channelPostFx) { }

    Audio::AudioSample AudioEngine::sample() {
        Processing::AudioContext ctx;
        ctx.sampleRate = _sampleRate;
        ctx.transportSample = _transportSample;

        Audio::AudioSample total(0.0, 0.0);
        for (auto &[idx, channel]: _channels) total = total + channel.sample(ctx);
        total = total * _volume;

        ++_transportSample;
        return total;
    }

    bool AudioEngine::isPlaying() {
        for (auto &[idx, channel]: _channels) if (channel.isActive()) return true;
        return false;
    }

    void AudioEngine::processMidiEvent(Midi::MidiRollEvent &) { }

    void AudioEngine::processMidiEvent(Midi::MidiRollChannelOnEvent &event) {
        const int idx = event.getChannel();
        if (_channels.find(idx) == _channels.end()) {
            Channel channel(PolyphonicSynthesizer(buildSynth(_synthIndex, _sampleRate)), 0.0, 0.5);
            if (_channelPostFx) channel.setPostFx(buildExamplePostFx(_sampleRate));
            _channels.emplace(idx, std::move(channel));
        }
    }

    void AudioEngine::processMidiEvent(Midi::MidiRollKeyEvent &event) {
        auto it = _channels.find(event.getChannel());
        if (it == _channels.end()) return;

        const std::string &subType = event.getSubType();
        if (subType == "keyOn") {
            const double velocity = _useVelocity ? (event.getVelocity() / 127.0) : 1.0;
            it->second.playNote(event.getNote(), velocity, _transportSample);
        } else if (subType == "keyOff") {
            it->second.releaseNote(event.getNote());
        }
    }
}
