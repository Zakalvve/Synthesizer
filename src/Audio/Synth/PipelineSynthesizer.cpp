#include "PipelineSynthesizer.h"

#include <utility>

#include "AudioContext.h"
#include "InstrumentFactory.h"
#include "Instrument.h"

namespace Audio::Synth {
    PipelineSynthesizer::PipelineSynthesizer(int sampleRate, double volume, int instrumentIndex,
                                             bool useVelocity, bool channelPostFx)
        : _sampleRate(sampleRate), _volume(volume), _instrumentIndex(instrumentIndex),
          _useVelocity(useVelocity), _channelPostFx(channelPostFx) {
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
            Channel channel(Instrument(buildInstrument(_instrumentIndex, _sampleRate)), 0.0, 0.5);
            if (_channelPostFx) channel.setPostFx(buildExamplePostFx(_sampleRate));
            _channels.emplace(idx, std::move(channel));
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
