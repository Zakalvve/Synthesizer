#ifndef AUDIO_PROCESSING_NODES_FILTERNODE_H
#define AUDIO_PROCESSING_NODES_FILTERNODE_H

#include <cmath>
#include <cstddef>

#include "SignalProcessor.h"

namespace Audio::Processing {
    class FilterNode : public SignalProcessor {
    public:
        enum class Mode { LowPass, HighPass };

        FilterNode(double cutoffHz, Mode mode = Mode::LowPass) : _cutoff(cutoffHz), _mode(mode) {
        }

        void process(AudioContext &ctx) const override {
            double *st = ctx.state();
            const double a = 1.0 - std::exp(-2.0 * M_PI * _cutoff / static_cast<double>(ctx.sampleRate));
            const Signal *in = ctx.input(0);
            Signal *out = ctx.output(0);
            for (int f = 0; f < ctx.blockSize; ++f) {
                const double lp = st[0] + a * (in[f] - st[0]);
                st[0] = lp;
                out[f] = (_mode == Mode::HighPass) ? (in[f] - lp) : lp;
            }
        }

        int inputCount() const override { return 1; }
        int outputCount() const override { return 1; }
        std::size_t stateSize() const override { return 1; }

    private:
        double _cutoff;
        Mode _mode;
    };
}

#endif
