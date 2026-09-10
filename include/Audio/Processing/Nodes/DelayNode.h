#ifndef AUDIO_PROCESSING_NODES_DELAYNODE_H
#define AUDIO_PROCESSING_NODES_DELAYNODE_H

#include <cstddef>

#include "SignalProcessor.h"

namespace Audio::Processing {
    class DelayNode : public SignalProcessor {
    public:
        DelayNode(int capacitySamples, int delaySamples, double feedback, double wet, double dry)
            : _capacity(capacitySamples < 1 ? 1 : capacitySamples),
              _delay(delaySamples), _feedback(feedback), _wet(wet), _dry(dry) {
            if (_delay < 1) _delay = 1;
            if (_delay > _capacity) _delay = _capacity;
        }

        void process(AudioContext &ctx) const override {
            double *st = ctx.state();
            int w = static_cast<int>(st[_capacity]);
            const Signal *in = ctx.input(0);
            Signal *out = ctx.output(0);
            for (int f = 0; f < ctx.blockSize; ++f) {
                int r = w - _delay;
                if (r < 0) r += _capacity;
                const double delayed = st[r];
                out[f] = _dry * in[f] + _wet * delayed;
                st[w] = in[f] + _feedback * delayed;
                if (++w >= _capacity) w = 0;
            }
            st[_capacity] = static_cast<double>(w);
        }

        int inputCount() const override { return 1; }
        int outputCount() const override { return 1; }
        std::size_t stateSize() const override { return static_cast<std::size_t>(_capacity) + 1; }

    private:
        int _capacity;
        int _delay;
        double _feedback;
        double _wet;
        double _dry;
    };
}

#endif
