#ifndef SINEWAVE_H_INCLUDED
#define SINEWAVE_H_INCLUDED

#include "IWaveform.h"
#include "Constants.h"

#include <cmath>

namespace Audio {
    class SineWave : public IWaveform {
        public:
            SineWave(int sampleRate) : IWaveform(sampleRate) {}
            double sample(int t, double f) const override {
                return std::sin((MathConstants::TWO_PI * t * f) / getSampleRate());
            }
    };
}

#endif // SINEWAVE_H_INCLUDED
