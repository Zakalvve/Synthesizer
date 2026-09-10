#ifndef AUDIO_PROCESSING_NODES_INPUTNODE_H
#define AUDIO_PROCESSING_NODES_INPUTNODE_H

#include "SignalProcessor.h"

namespace Audio::Processing {
    class InputNode : public SignalProcessor {
    public:
        void process(AudioContext &ctx) const override {
            Signal *out = ctx.output(0);
            for (int f = 0; f < ctx.blockSize; ++f) out[f] = ctx.sourceValue;
        }

        int inputCount() const override { return 0; }
        int outputCount() const override { return 1; }
    };
}

#endif
