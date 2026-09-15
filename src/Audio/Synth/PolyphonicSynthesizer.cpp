#include "PolyphonicSynthesizer.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace Audio::Synth {
    using namespace Audio::Processing;

    void PolyphonicSynthesizer::configure(AudioPipeline voicePipeline, int maxVoices, int sampleRate,
                                          double attackSec, double decaySec, double sustainLevel, double releaseSec,
                                          bool velocitySensitivity, SignalProcessor *envelopeNode) {
        _pipeline = std::move(voicePipeline);
        _voices.assign(static_cast<std::size_t>(maxVoices), Voice{});
        _orderCounter = 0;
        _sampleRate = sampleRate;
        _attackSec = attackSec;
        _decaySec = decaySec;
        _sustainLevel = sustainLevel;
        _releaseSec = releaseSec;
        _attackDur = static_cast<int>(attackSec * sampleRate);
        _decayDur = static_cast<int>(decaySec * sampleRate);
        _releaseDur = static_cast<int>(releaseSec * sampleRate);
        _velocitySensitivity = velocitySensitivity;
        _envelopeNode = envelopeNode;
        _configured = true;
    }

    int PolyphonicSynthesizer::findFreeOrOldest() {
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

    void PolyphonicSynthesizer::playNote(const std::string &noteKey, double frequency, double velocity, long transportSample) {
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

    void PolyphonicSynthesizer::releaseNote(const std::string &noteKey) {
        for (Voice &v: _voices) {
            if (v.inUse && v.gate && v.noteKey == noteKey) {
                v.gate = false;
                return;
            }
        }
    }

    double PolyphonicSynthesizer::sample(AudioContext &ctx) {
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
        return mono;
    }

    bool PolyphonicSynthesizer::isActive() const {
        for (const Voice &v: _voices) if (v.inUse) return true;
        return false;
    }

    void PolyphonicSynthesizer::setParam(const std::string &path, const ParamValue &value) {
        if (path == "attack") {
            _attackSec = value.asDouble();
            _attackDur = static_cast<int>(_attackSec * _sampleRate);
            if (_envelopeNode) _envelopeNode->setParam("attack", _attackSec);
        } else if (path == "decay") {
            _decaySec = value.asDouble();
            _decayDur = static_cast<int>(_decaySec * _sampleRate);
            if (_envelopeNode) _envelopeNode->setParam("decay", _decaySec);
        } else if (path == "release") {
            _releaseSec = value.asDouble();
            _releaseDur = static_cast<int>(_releaseSec * _sampleRate);
            if (_envelopeNode) _envelopeNode->setParam("release", _releaseSec);
        } else if (path == "sustain") {
            _sustainLevel = value.asDouble();
            if (_envelopeNode) _envelopeNode->setParam("sustain", _sustainLevel);
        } else if (path == "velocitySensitivity") {
            _velocitySensitivity = value.asBool();
        }
    }

    ParamValue PolyphonicSynthesizer::getParam(const std::string &path) const {
        if (path == "attack") return _attackSec;
        if (path == "decay") return _decaySec;
        if (path == "sustain") return _sustainLevel;
        if (path == "release") return _releaseSec;
        if (path == "velocitySensitivity") return _velocitySensitivity;
        return ParamValue{};
    }
}
