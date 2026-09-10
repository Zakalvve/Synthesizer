#include "Instrument.h"

#include <algorithm>
#include <limits>
#include <memory>

#include "SignalGraph.h"
#include "OscillatorNode.h"
#include "PhaseOscillatorNode.h"
#include "ADSRNode.h"
#include "VcaNode.h"
#include "SawtoothWave.h"

namespace Audio::Synth {
    using namespace Audio::Processing;

    Instrument::Instrument(const InstrumentConfig &config) : _config(config) {
        _attackDur = static_cast<int>(_config.attack * _config.sampleRate);
        _decayDur = static_cast<int>(_config.decay * _config.sampleRate);
        _releaseDur = static_cast<int>(_config.release * _config.sampleRate);

        SignalGraph graph;
        SignalGraph::NodeId osc;
        switch (_config.oscType) {
            case OscType::PhaseSine:
                osc = graph.add(std::make_unique<PhaseOscillatorNode>(
                    PhaseOscillatorNode::Wave::Sine, _config.glideTime));
                break;
            case OscType::PhaseSaw:
                osc = graph.add(std::make_unique<PhaseOscillatorNode>(
                    PhaseOscillatorNode::Wave::Saw, _config.glideTime));
                break;
            case OscType::LegacySaw:
            default:
                osc = graph.add(std::make_unique<OscillatorNode>(
                    std::make_shared<Audio::SawtoothWave>(_config.sampleRate)));
                break;
        }
        auto adsr = graph.add(std::make_unique<ADSRNode>(
            _config.sampleRate, _config.attack, _config.decay, _config.release, _config.sustain));
        auto vca = graph.add(std::make_unique<VcaNode>());
        graph.connect(osc, 0, vca, 0);
        graph.connect(adsr, 0, vca, 1);
        graph.setOutput(vca, 0);

        _pipeline = graph.compile(_config.maxVoices,  1);
        _voices.resize(_config.maxVoices);
    }

    int Instrument::findFreeOrOldest() {
        int oldest = 0;
        long oldestOrder = std::numeric_limits<long>::max();
        for (int i = 0; i < static_cast<int>(_voices.size()); ++i) {
            if (!_voices[i].inUse) return i;
            if (_voices[i].order < oldestOrder) {
                oldestOrder = _voices[i].order;
                oldest = i;
            }
        }
        return oldest;
    }

    void Instrument::playNote(const std::string &noteKey, double frequency, double velocity, long transportSample) {
        int i = -1;
        for (int j = 0; j < static_cast<int>(_voices.size()); ++j) {
            if (_voices[j].inUse && _voices[j].noteKey == noteKey) {
                i = j;
                break;
            }
        }
        if (i < 0) i = findFreeOrOldest();
        _pipeline.resetInstance(i);
        Voice &v = _voices[i];
        v.inUse = true;
        v.gate = true;
        v.frequency = frequency;
        v.velocity = velocity;
        v.triggerSample = transportSample;
        v.endNoteSample = -1;
        v.noteKey = noteKey;
        v.order = _orderCounter++;
    }

    void Instrument::releaseNote(const std::string &noteKey) {
        for (Voice &v: _voices) {
            if (v.inUse && v.gate && v.noteKey == noteKey) {
                v.gate = false;
                return;
            }
        }
    }

    double Instrument::sample(Processing::AudioContext &ctx) {
        double mono = 0.0;
        for (int i = 0; i < static_cast<int>(_voices.size()); ++i) {
            Voice &v = _voices[i];
            if (!v.inUse) continue;

            const long noteSample = ctx.transportSample - v.triggerSample;

            if (!v.gate && v.endNoteSample < 0) {
                v.endNoteSample = std::max<long>(noteSample, _attackDur + _decayDur) + _releaseDur;
            }
            if (v.endNoteSample >= 0 && noteSample >= v.endNoteSample) {
                v.inUse = false;
                continue;
            }

            ctx.frequency = v.frequency;
            ctx.velocity = v.velocity;
            ctx.gate = v.gate;
            ctx.noteSample = noteSample;
            mono += _pipeline.renderInstance(i, ctx)[0];
        }
        return mono * _config.masterGain;
    }

    bool Instrument::isActive() const {
        for (const Voice &v: _voices) if (v.inUse) return true;
        return false;
    }
}
