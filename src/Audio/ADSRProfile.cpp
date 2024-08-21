#include <iostream>

#include "ADSRProfile.h"
#include "Clamp.h"

    namespace Audio {
        ADSRProfile::ADSRProfile(int sampleRate, double attackDuration, double decayDuration, double holdDuration, double releaseDuration, double sustainThreshold) :
            sample_rate(sampleRate)
        {
            attack_duration = static_cast<int>(attackDuration * sampleRate);
            decay_duration = static_cast<int>(decayDuration * sampleRate);

            double minHoldDuration = attackDuration + decayDuration;
            if (holdDuration < minHoldDuration) {
                holdDuration = minHoldDuration;
            }

            sustain_duration = static_cast<int>((holdDuration * sampleRate) - attack_duration - decay_duration);
            release_duration = static_cast<int>(releaseDuration * sampleRate);
            sustain_threshold = clamp(sustainThreshold, 0.0, 1.0);
        }

        ADSRProfile::~ADSRProfile() {}

        double ADSRProfile::calculateAmplitude(int t) {
            if (t < attack_duration) {
                double normalized_time = static_cast<double>(t) / static_cast<double>(attack_duration);
                return max_amplitude * cubeRoot(normalized_time);
            }
            else if (t < attack_duration + decay_duration) {
                double x = (sustain_threshold - max_amplitude) / static_cast<double>(decay_duration) * (t - attack_duration) + max_amplitude;
                return applyNormalizedCubicScaling(x, sustain_threshold, max_amplitude);
            }
            else if (t < attack_duration + decay_duration + sustain_duration) {
                return sustain_threshold;
            }
            else if (t < attack_duration + decay_duration + sustain_duration + release_duration) {
                double x = (-static_cast<double>(sustain_threshold) / static_cast<double>(release_duration)) * (t - (attack_duration + decay_duration + sustain_duration)) + static_cast<double>(sustain_threshold);
                return applyNormalizedInverseCubicScaling(x, 0.0, sustain_threshold);
            }
            else {
                return 0.0;
            }
        }

        int ADSRProfile::getTotalSamples() {
            return attack_duration + decay_duration + sustain_duration + release_duration;
        }
    }
