#include "Channel.h"

#include <cmath>
#include <memory>

#include "Pitch.h"
#include "SignalGraph.h"
#include "InputNode.h"
#include "DelayNode.h"
#include "FilterNode.h"

namespace Audio::Synth {
    using namespace Audio::Processing;

    Channel::Channel(const InstrumentConfig &config, bool postFx) : _instrument(config) {
        if (postFx) {
            const int sr = config.sampleRate;
            SignalGraph graph;
            auto in = graph.add(std::make_unique<InputNode>());
            auto dly = graph.add(std::make_unique<DelayNode>(
                 sr / 2,  sr / 4,  0.35,  0.4,  1.0));
            auto flt = graph.add(std::make_unique<FilterNode>( 1200.0, FilterNode::Mode::LowPass));
            graph.connect(in, 0, dly, 0);
            graph.connect(dly, 0, flt, 0);
            graph.setOutput(flt, 0);
            _postFx = graph.compile( 1,  1);
            _hasPostFx = true;
        }
    }

    void Channel::playNote(const std::string &noteName, double velocity, long transportSample) {
        _instrument.playNote(noteName, Audio::noteToFrequency(noteName), velocity, transportSample);
    }

    void Channel::releaseNote(const std::string &noteName) {
        _instrument.releaseNote(noteName);
    }

    Audio::AudioSample Channel::sample(Processing::AudioContext &ctx) {
        double mono = _instrument.sample(ctx);
        if (_hasPostFx) {
            ctx.sourceValue = mono;
            mono = _postFx.renderInstance(0, ctx)[0];
        }
        const double panMapped = ((_pan + 1.0) / 2.0) * (M_PI / 2.0);
        const double left = mono * std::sin(panMapped);
        const double right = mono * std::cos(panMapped);
        return Audio::AudioSample(left, right) * _volume;
    }

    bool Channel::isActive() { return _instrument.isActive(); }
}
