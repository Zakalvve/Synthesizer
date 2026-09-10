#ifndef AUDIO_PROCESSING_NODES_PANNODE_H
#define AUDIO_PROCESSING_NODES_PANNODE_H

#include <cmath>

#include "SignalProcessor.h"

namespace Audio::Processing {
    class PanNode : public SignalProcessor {
    public:
        explicit PanNode(double pan) : _pan(pan) {
        }

        void process(AudioContext &ctx) const override {
            const double panMapped = ((_pan + 1.0) / 2.0) * (M_PI / 2.0);
            const double leftGain = std::sin(panMapped);
            const double rightGain = std::cos(panMapped);
            const Signal *in = ctx.input(0);
            Signal *left = ctx.output(0);
            Signal *right = ctx.output(1);
            for (int f = 0; f < ctx.blockSize; ++f) {
                left[f] = in[f] * leftGain;
                right[f] = in[f] * rightGain;
            }
        }

        int inputCount() const override { return 1; }
        int outputCount() const override { return 2; }

    private:
        double _pan;
    };
}

#endif
