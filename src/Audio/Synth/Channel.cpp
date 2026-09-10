#include "Channel.h"

#include <memory>
#include <utility>

#include "Pitch.h"
#include "SignalGraph.h"
#include "InputNode.h"
#include "GainNode.h"
#include "PanNode.h"

namespace Audio::Synth {
    using namespace Audio::Processing;

    Channel::Channel(Instrument instrument, double pan, double gain)
        : _instrument(std::move(instrument)) {
        SignalGraph graph;
        auto in = graph.add(std::make_unique<InputNode>());
        auto gainNode = graph.add(std::make_unique<GainNode>(gain));
        auto panNode = graph.add(std::make_unique<PanNode>(pan));
        graph.connect(in, 0, gainNode, 0);
        graph.connect(gainNode, 0, panNode, 0);
        graph.setOutput(panNode, 0);
        graph.setOutput(panNode, 1);
        _channelGraph = graph.compile(1, 1);
    }

    void Channel::setPostFx(SignalGraph postFx) {
        _postFx = postFx.compile(1, 1);
        _hasPostFx = true;
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
        ctx.sourceValue = mono;
        _channelGraph.renderInstance(0, ctx);
        return Audio::AudioSample(_channelGraph.terminal(0), _channelGraph.terminal(1));
    }

    bool Channel::isActive() { return _instrument.isActive(); }
}
