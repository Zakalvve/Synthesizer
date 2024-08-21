#include <algorithm>
#include <iostream>

#include "AudioNote.h"
#include "Clamp.h"
#include "Oscillator.h"

namespace Audio {

    AudioNote::AudioNote(std::shared_ptr<Oscillator> osc, std::shared_ptr<ADSRProfile> adsrProfile, double frequency, double strength) :
        osc(osc),
        adsr(adsrProfile),
        frequency(clamp(frequency, 20.0, 20000.0)),
        strength(clamp(strength, 0.0, 1.0)),
        t(0)
    {
        total_samples = adsrProfile->getTotalSamples();
    }

    double AudioNote::sample() {
        if (!isEnded()) {
            double amplitude = adsr->calculateAmplitude(t);
            t++;

            double sample = osc->sample(t, frequency);

            for (int n = 1; n < harmonics; ++n){
                double harmoicFrequency = n * frequency;
                sample += osc->sample(t, harmoicFrequency);
            }

            sample /= harmonics;

            return sample * amplitude * strength;
        }
        return 0;
    }

    bool AudioNote::isEnded() {
        return t > total_samples;
    }

    AudioNote::~AudioNote() { }
}
