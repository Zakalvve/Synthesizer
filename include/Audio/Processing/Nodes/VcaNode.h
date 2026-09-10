#ifndef AUDIO_PROCESSING_NODES_VCANODE_H
#define AUDIO_PROCESSING_NODES_VCANODE_H

#include "SignalProcessor.h"

namespace Audio::Processing {
    class VcaNode : public SignalProcessor {
    public:
        void process(AudioContext &ctx) const override {
            const Signal *audio = ctx.input(0);
            const Signal *gain = ctx.input(1);
            Signal *out = ctx.output(0);
            for (int f = 0; f < ctx.blockSize; ++f) out[f] = audio[f] * gain[f] * ctx.velocity;
        }

        int inputCount() const override { return 2; }
        int outputCount() const override { return 1; }
    };
}

#endif
