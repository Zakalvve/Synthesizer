#include <iostream>

#include "ADSRProfile.h"
#include "Clamp.h"

    namespace Audio {
        ADSRProfile::ADSRProfile(int sampleRate, double attackDuration, double decayDuration, double releaseDuration, double sustainThreshold) :
            sample_rate(sampleRate),
            is_sustaining(false),
            is_active(false)
        {
            attack_duration = static_cast<int>(attackDuration * sampleRate);
            decay_duration = static_cast<int>(decayDuration * sampleRate);
            sustain_duration = 0;
            sustain_threshold = clamp(sustainThreshold, 0.0, 1.0);
            release_duration = static_cast<int>(releaseDuration * sampleRate);
        }

        double ADSRProfile::calculateAmplitude(int t) {
            if (t < attack_duration) {
                double normalized_time = static_cast<double>(t) / static_cast<double>(attack_duration);
                return max_amplitude * cubeRoot(normalized_time);
            }
            else if (t < attack_duration + decay_duration) {
                double x = (sustain_threshold - max_amplitude) / static_cast<double>(decay_duration) * (t - attack_duration) + max_amplitude;
                return applyNormalizedCubicScaling(x, sustain_threshold, max_amplitude);
            }
            else if (is_sustaining) {
                sustain_duration++;
                return sustain_threshold;
            }
            else if (t < attack_duration + decay_duration + sustain_duration + release_duration) {
                double x = (-static_cast<double>(sustain_threshold) / static_cast<double>(release_duration)) * (t - (attack_duration + decay_duration + sustain_duration)) + static_cast<double>(sustain_threshold);
                return applyNormalizedInverseCubicScaling(x, 0.0, sustain_threshold);
            }
            else {
                sustain_duration = 0;
                is_active = false;
                return 0.0;
            }
        }

        void ADSRProfile::play() {
            is_active = true;
            is_sustaining = true;
        }

        void ADSRProfile::release() {
            is_sustaining = false;
        }

        bool ADSRProfile::isEnded(){
            return !is_active;
        }
    }
