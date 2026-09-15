#ifndef AUDIO_DSP_NODES_OSCILLATORNODE_H
#define AUDIO_DSP_NODES_OSCILLATORNODE_H

#include <memory>
#include <string>

#include "SignalProcessor.h"
#include "IWaveform.h"
#include "SineWave.h"
#include "SawtoothWave.h"

namespace Audio::Dsp {
    class OscillatorNode : public SignalProcessor {
    public:
        enum class Waveform { Sine, Saw };

        explicit OscillatorNode(int sampleRate, Waveform waveform = Waveform::Saw)
            : _sampleRate(sampleRate), _waveform(waveform) {
            rebuild();
        }

        void process(AudioContext &ctx) const override {
            Signal *out = ctx.output(0);
            for (int f = 0; f < ctx.blockSize; ++f) {
                const int t = static_cast<int>(ctx.noteSample + f + 1);
                out[f] = _wave->sample(t, ctx.frequency);
            }
        }

        int inputCount() const override { return 0; }
        int outputCount() const override { return 1; }

        void setParam(const std::string &name, const ParamValue &v) override {
            if (name == "waveform") {
                _waveform = parseWaveform(v.asString());
                rebuild();
            }
        }

        ParamValue getParam(const std::string &name) const override {
            if (name == "waveform") return waveformName(_waveform);
            return ParamValue{};
        }

        static Waveform parseWaveform(const std::string &s) {
            if (s == "sine") return Waveform::Sine;
            return Waveform::Saw;
        }

        static const char *waveformName(Waveform w) {
            return w == Waveform::Sine ? "sine" : "saw";
        }

    private:
        void rebuild() {
            if (_waveform == Waveform::Sine) _wave = std::make_shared<Audio::SineWave>(_sampleRate);
            else _wave = std::make_shared<Audio::SawtoothWave>(_sampleRate);
        }

        int _sampleRate;
        Waveform _waveform;
        std::shared_ptr<Audio::IWaveform> _wave;
    };
}

#endif
