#include "NodeRegistry.h"

#include <stdexcept>

#include "OscillatorNode.h"
#include "PhaseOscillatorNode.h"
#include "ADSRNode.h"
#include "VcaNode.h"
#include "DelayNode.h"
#include "FilterNode.h"
#include "GainNode.h"
#include "PanNode.h"
#include "InputNode.h"

namespace Audio::Engine {
    using namespace Audio::Dsp;

    NodeRegistry::NodeRegistry() {
        _factories["oscillator"] = [](const NodeDef &d, int sr) {
            return std::make_unique<OscillatorNode>(sr, OscillatorNode::parseWaveform(paramOr(d, "waveform", std::string("saw"))));
        };

        _factories["phaseOscillator"] = [](const NodeDef &d, int) {
            auto node = std::make_unique<PhaseOscillatorNode>(
                paramOr(d, "waveform", std::string("saw")) == "sine" ? PhaseOscillatorNode::Wave::Sine : PhaseOscillatorNode::Wave::Saw,
                paramOr(d, "glide", 0.0));
            return node;
        };

        _factories["adsr"] = [](const NodeDef &d, int sr) {
            return std::make_unique<ADSRNode>(sr,
                                              paramOr(d, "attack", 0.0),
                                              paramOr(d, "decay", 0.0),
                                              paramOr(d, "release", 0.0),
                                              paramOr(d, "sustain", 0.0));
        };

        _factories["vca"] = [](const NodeDef &, int) {
            return std::make_unique<VcaNode>();
        };

        _factories["delay"] = [](const NodeDef &d, int sr) {
            return std::make_unique<DelayNode>(paramOr(d, "capacity", sr),
                                               paramOr(d, "delay", 1),
                                               paramOr(d, "feedback", 0.0),
                                               paramOr(d, "wet", 0.0),
                                               paramOr(d, "dry", 1.0));
        };

        _factories["filter"] = [](const NodeDef &d, int) {
            return std::make_unique<FilterNode>(paramOr(d, "cutoff", 1000.0),
                                                FilterNode::parseMode(paramOr(d, "mode", std::string("lowpass"))));
        };

        _factories["gain"] = [](const NodeDef &d, int) {
            return std::make_unique<GainNode>(paramOr(d, "gain", 1.0));
        };

        _factories["pan"] = [](const NodeDef &d, int) {
            return std::make_unique<PanNode>(paramOr(d, "pan", 0.0));
        };

        _factories["input"] = [](const NodeDef &, int) {
            return std::make_unique<InputNode>();
        };
    }

    const NodeRegistry &NodeRegistry::instance() {
        static const NodeRegistry registry;
        return registry;
    }

    bool NodeRegistry::has(const std::string &type) const {
        return _factories.find(type) != _factories.end();
    }

    std::unique_ptr<SignalProcessor> NodeRegistry::create(const NodeDef &def, int sampleRate) const {
        auto it = _factories.find(def.type);
        if (it == _factories.end())
            throw std::runtime_error("NodeRegistry: unknown node type '" + def.type + "'");
        return it->second(def, sampleRate);
    }
}
