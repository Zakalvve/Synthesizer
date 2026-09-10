#include "PipelineSynthesizer.h"

#include "AudioContext.h"
#include "InstrumentConfig.h"

namespace Audio::Synth {
    PipelineSynthesizer::PipelineSynthesizer(int sampleRate, double volume,
                                             InstrumentConfig config, bool postFx, bool useVelocity)
        : _sampleRate(sampleRate), _volume(volume), _config(config),
          _postFx(postFx), _useVelocity(useVelocity) {
        _config.sampleRate = sampleRate;
    }

    Audio::AudioSample PipelineSynthesizer::sample() {
        Processing::AudioContext ctx;
        ctx.sampleRate = _sampleRate;
        ctx.transportSample = _transportSample;

        Audio::AudioSample total(0.0, 0.0);
        for (auto &[idx, channel]: _channels) total = total + channel.sample(ctx);
        total = total * _volume;

        ++_transportSample;
        return total;
    }

    bool PipelineSynthesizer::isPlaying() {
        for (auto &[idx, channel]: _channels) if (channel.isActive()) return true;
        return false;
    }

    void PipelineSynthesizer::processMidiEvent(Midi::MidiRollEvent &) {
    }

    void PipelineSynthesizer::processMidiEvent(Midi::MidiRollChannelOnEvent &event) {
        const int idx = event.getChannel();
        if (_channels.find(idx) == _channels.end()) {
            _channels.emplace(idx, Channel(_config, _postFx));
        }
    }

    void PipelineSynthesizer::processMidiEvent(Midi::MidiRollKeyEvent &event) {
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
