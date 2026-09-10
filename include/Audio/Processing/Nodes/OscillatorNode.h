#ifndef AUDIO_PROCESSING_NODES_OSCILLATORNODE_H
#define AUDIO_PROCESSING_NODES_OSCILLATORNODE_H

#include <memory>
#include <utility>

#include "SignalProcessor.h"
#include "IWaveform.h"

namespace Audio::Processing {
    class OscillatorNode : public SignalProcessor {
    public:
        explicit OscillatorNode(std::shared_ptr<Audio::IWaveform> waveform)
            : _waveform(std::move(waveform)) {
        }

        void process(AudioContext &ctx) const override {
            Signal *out = ctx.output(0);
            for (int f = 0; f < ctx.blockSize; ++f) {
                const int t = static_cast<int>(ctx.noteSample + f + 1);
                out[f] = _waveform->sample(t, ctx.frequency);
            }
        }

        int inputCount() const override { return 0; }
        int outputCount() const override { return 1; }

    private:
        std::shared_ptr<Audio::IWaveform> _waveform;
    };
}

#endif
