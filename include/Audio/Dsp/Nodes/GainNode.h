#ifndef AUDIO_DSP_NODES_GAINNODE_H
#define AUDIO_DSP_NODES_GAINNODE_H

#include "SignalProcessor.h"

namespace Audio::Dsp {
    class GainNode : public SignalProcessor {
    public:
        explicit GainNode(double gain) : _gain(gain) {
        }

        void process(AudioContext &ctx) const override {
            const Signal *in = ctx.input(0);
            Signal *out = ctx.output(0);
            for (int f = 0; f < ctx.blockSize; ++f) out[f] = in[f] * _gain;
        }

        int inputCount() const override { return 1; }
        int outputCount() const override { return 1; }

        void setParam(const std::string &name, const ParamValue &v) override {
            if (name == "gain") _gain = v.asDouble();
        }

        ParamValue getParam(const std::string &name) const override {
            if (name == "gain") return _gain;
            return ParamValue{};
        }

    private:
        double _gain;
    };
}

#endif
