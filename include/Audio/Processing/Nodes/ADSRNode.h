#ifndef AUDIO_PROCESSING_NODES_ADSRNODE_H
#define AUDIO_PROCESSING_NODES_ADSRNODE_H

#include <cmath>
#include <cstddef>

#include "SignalProcessor.h"
#include "Clamp.h"

namespace Audio::Processing {
    class ADSRNode : public SignalProcessor {
    public:
        ADSRNode(int sampleRate, double attackSec, double decaySec, double releaseSec, double sustainLevel) {
            _attack_duration = static_cast<int>(attackSec * sampleRate);
            _decay_duration = static_cast<int>(decaySec * sampleRate);
            _release_duration = static_cast<int>(releaseSec * sampleRate);
            _sustain_threshold = clamp(sustainLevel, 0.0, 1.0);
        }

        void process(AudioContext &ctx) const override {
            double *st = ctx.state();
            Signal *out = ctx.output(0);
            for (int f = 0; f < ctx.blockSize; ++f) {
                const int t = static_cast<int>(ctx.noteSample + f);
                out[f] = amplitude(t, ctx.gate, st[0]);
            }
        }

        int inputCount() const override { return 0; }
        int outputCount() const override { return 1; }
        std::size_t stateSize() const override { return 1; }
        void resetState(double *st) const override { st[0] = 0.0; }

    private:
        int _attack_duration = 0;
        int _decay_duration = 0;
        int _release_duration = 0;
        double _sustain_threshold = 0.0;
        double _max_amplitude = 1.0;

        static double cubeRoot(double x) { return std::cbrt(x); }
        static double cubed(double x) { return x * x * x; }

        static double applyNormalizedInverseCubicScaling(double x, double lower, double upper) {
            const double n = (x - lower) / (upper - lower);
            return cubeRoot(n) * (upper - lower) + lower;
        }

        static double applyNormalizedCubicScaling(double x, double lower, double upper) {
            const double n = (x - lower) / (upper - lower);
            return cubed(n) * (upper - lower) + lower;
        }

        double amplitude(int t, bool sustaining, double &sustainDuration) const {
            if (t < _attack_duration) {
                const double nt = static_cast<double>(t) / static_cast<double>(_attack_duration);
                return _max_amplitude * cubeRoot(nt);
            }
            if (t < _attack_duration + _decay_duration) {
                const double x = (_sustain_threshold - _max_amplitude) / static_cast<double>(_decay_duration)
                                 * (t - _attack_duration) + _max_amplitude;
                return applyNormalizedCubicScaling(x, _sustain_threshold, _max_amplitude);
            }
            if (sustaining) {
                sustainDuration += 1.0;
                return _sustain_threshold;
            }
            const int sd = static_cast<int>(sustainDuration);
            if (t < _attack_duration + _decay_duration + sd + _release_duration) {
                const double x = (-_sustain_threshold / static_cast<double>(_release_duration))
                                 * (t - (_attack_duration + _decay_duration + sd)) + _sustain_threshold;
                return applyNormalizedInverseCubicScaling(x, 0.0, _sustain_threshold);
            }
            return 0.0;
        }
    };
}

#endif
