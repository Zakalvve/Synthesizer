#ifndef AUDIO_PROCESSING_NODES_PHASEOSCILLATORNODE_H
#define AUDIO_PROCESSING_NODES_PHASEOSCILLATORNODE_H

#include <cmath>
#include <cstddef>

#include "SignalProcessor.h"

namespace Audio::Processing {
    class PhaseOscillatorNode : public SignalProcessor {
    public:
        enum class Wave { Sine, Saw };

        explicit PhaseOscillatorNode(Wave wave = Wave::Saw, double glideSeconds = 0.0)
            : _wave(wave), _glide(glideSeconds) {
        }

        void process(AudioContext &ctx) const override {
            double *st = ctx.state();
            const double target = ctx.frequency;
            if (st[1] == 0.0) st[1] = target;
            const double glideCoeff = (_glide > 0.0)
                                          ? (1.0 - std::exp(-1.0 / (_glide * static_cast<double>(ctx.sampleRate))))
                                          : 1.0;

            Signal *out = ctx.output(0);
            for (int f = 0; f < ctx.blockSize; ++f) {
                st[1] += (target - st[1]) * glideCoeff;
                const double phase = st[0];
                out[f] = (_wave == Wave::Sine) ? std::sin(2.0 * M_PI * phase) : (2.0 * phase - 1.0);
                double next = phase + st[1] / static_cast<double>(ctx.sampleRate);
                next -= std::floor(next);
                st[0] = next;
            }
        }

        int inputCount() const override { return 0; }
        int outputCount() const override { return 1; }
        std::size_t stateSize() const override { return 2; }

    private:
        Wave _wave;
        double _glide;
    };
}

#endif
