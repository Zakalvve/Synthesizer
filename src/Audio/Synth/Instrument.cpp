#include "Instrument.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace Audio::Synth {
    using namespace Audio::Processing;

    Instrument::Instrument(InstrumentSpec spec)
        : _attackDur(spec.attackDur), _decayDur(spec.decayDur), _releaseDur(spec.releaseDur) {
        _pipeline = spec.voiceGraph.compile(spec.maxVoices, 1);
        _voices.resize(spec.maxVoices);
    }

    void Instrument::setPostFx(SignalGraph postFx) {
        _postFx = postFx.compile(1, 1);
        _hasPostFx = true;
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

    double Instrument::sample(AudioContext &ctx) {
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

        if (_hasPostFx) {
            ctx.sourceValue = mono;
            mono = _postFx.renderInstance(0, ctx)[0];
        }
        return mono;
    }

    bool Instrument::isActive() const {
        for (const Voice &v: _voices) if (v.inUse) return true;
        return false;
    }
}
