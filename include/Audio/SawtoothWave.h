#ifndef SAWTOOTHWAVE_H_INCLUDED
#define SAWTOOTHWAVE_H_INCLUDED

#include <cmath>

#include "IWaveform.h"
#include "Constants.h"

namespace Audio {
    class SawtoothWave : public IWaveform {
        public:
            SawtoothWave(int sampleRate) : IWaveform(sampleRate) {}

            double sample(int t, double f) const override {
                double p = 1.0 / f;
                double normalized_time = static_cast<double>(t) / getSampleRate();
                double phase_offset = (normalized_time / p) - std::floor((normalized_time / p) + 0.5);
                return 2.0 * std::abs(2.0 * phase_offset) - 1.0;
            }
    };
}

#endif // SAWTOOTHWAVE_H_INCLUDED
