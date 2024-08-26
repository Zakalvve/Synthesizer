#include <memory>

#include "AudioNote.h"
#include "Clamp.h"
#include "Oscillator.h"

namespace Audio {

    AudioNote::AudioNote(std::shared_ptr<Oscillator> osc, std::unique_ptr<AdsrProfile> adsrProfile, double frequency, double strength) :
        osc(osc),
        adsr(std::move(adsrProfile)),
        frequency(clamp(frequency, 20.0, 20000.0)),
        strength(clamp(strength, 0.0, 1.0)),
        t(0),
        isPlaying(false) { }

    double AudioNote::sample() {
        if (!isPlaying) return 0.0;

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

    void AudioNote::play(){
        t = 0;
        isPlaying = true;
        adsr->play();
    }

    void AudioNote::release(){
        adsr->release();
    }

    bool AudioNote::isEnded() {
        // The note is over when the ADSR profile causes its amplitude to be 0
        return adsr->isEnded();
    }
}
