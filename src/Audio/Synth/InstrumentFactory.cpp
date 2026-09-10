#include "InstrumentFactory.h"

#include <memory>
#include <utility>

#include "OscillatorNode.h"
#include "ADSRNode.h"
#include "VcaNode.h"
#include "InputNode.h"
#include "DelayNode.h"
#include "FilterNode.h"
#include "SawtoothWave.h"
#include "SineWave.h"

namespace Audio::Synth {
    using namespace Audio::Processing;

    InstrumentSpec buildInstrument(int index, int sampleRate) {
        double attack;
        double decay;
        double sustain;
        double release;

        SignalGraph graph;
        SignalGraph::NodeId osc;

        if (index == 1) {
            attack = 0.02;
            decay = 0.06;
            sustain = 0.5;
            release = 0.25;
            osc = graph.add(std::make_unique<OscillatorNode>(std::make_shared<SineWave>(sampleRate)));
        } else {
            attack = 0.01;
            decay = 0.03;
            sustain = 0.4;
            release = 0.1;
            osc = graph.add(std::make_unique<OscillatorNode>(std::make_shared<SawtoothWave>(sampleRate)));
        }

        auto adsr = graph.add(std::make_unique<ADSRNode>(sampleRate, attack, decay, release, sustain));
        auto vca = graph.add(std::make_unique<VcaNode>());
        graph.connect(osc, 0, vca, 0);
        graph.connect(adsr, 0, vca, 1);
        graph.setOutput(vca, 0);

        InstrumentSpec spec;
        spec.maxVoices = 16;
        spec.voiceGraph = std::move(graph);
        spec.attackDur = static_cast<int>(attack * sampleRate);
        spec.decayDur = static_cast<int>(decay * sampleRate);
        spec.releaseDur = static_cast<int>(release * sampleRate);
        return spec;
    }

    SignalGraph buildExamplePostFx(int sampleRate) {
        SignalGraph graph;
        auto in = graph.add(std::make_unique<InputNode>());
        auto dly = graph.add(std::make_unique<DelayNode>(sampleRate, sampleRate * 3 / 8, 0.55, 0.6, 1.0));
        auto flt = graph.add(std::make_unique<FilterNode>(1200.0, FilterNode::Mode::LowPass));
        graph.connect(in, 0, dly, 0);
        graph.connect(dly, 0, flt, 0);
        graph.setOutput(flt, 0);
        return graph;
    }
}
