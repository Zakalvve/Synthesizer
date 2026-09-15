#include "Channel.h"

#include <cmath>
#include <utility>

#include "Pitch.h"

namespace Audio::Engine {
    using namespace Audio::Dsp;

    void Channel::configure(int midiChannel, double gain, double pan, bool mute, bool solo) {
        _midiChannel = midiChannel;
        _gain = gain;
        _pan = pan;
        _mute = mute;
        _solo = solo;
    }

    void Channel::setPostFx(AudioPipeline postFx) {
        _postFx = std::move(postFx);
        _hasPostFx = true;
    }

    void Channel::playNote(const std::string &noteName, double velocity, long transportSample) {
        _synth.playNote(noteName, Audio::noteToFrequency(noteName), velocity, transportSample);
    }

    void Channel::releaseNote(const std::string &noteName) {
        _synth.releaseNote(noteName);
    }

    double Channel::renderMono(AudioContext &ctx) {
        double mono = _synth.sample(ctx);

        if (_hasPostFx) {
            ctx.sourceValue = mono;
            mono = _postFx.renderInstance(0, ctx)[0];
        }

        return mono;
    }

    Audio::AudioSample Channel::applyGainPan(double mono) const {
        const double g = mono * _gain;
        const double panMapped = ((_pan + 1.0) / 2.0) * (M_PI / 2.0);
        return Audio::AudioSample(g * std::sin(panMapped), g * std::cos(panMapped));
    }

    bool Channel::isActive() {
        return _synth.isActive();
    }

    void Channel::setParam(const std::string &path, const ParamValue &value) {
        if (path == "gain") _gain = value.asDouble();
        else if (path == "pan") _pan = value.asDouble();
        else if (path == "mute") _mute = value.asBool();
        else if (path == "solo") _solo = value.asBool();
    }

    ParamValue Channel::getParam(const std::string &path) const {
        if (path == "gain") return _gain;
        if (path == "pan") return _pan;
        if (path == "mute") return _mute;
        if (path == "solo") return _solo;
        return ParamValue{};
    }
}
