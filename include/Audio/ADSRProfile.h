#ifndef ADSRPROFILE_H
#define ADSRPROFILE_H

#include <cmath>

namespace Audio {
    class ADSRProfile {
        public:
            ADSRProfile(int sampleRate, double attackDuration, double decayDuration, double holdDuration, double releaseDuration, double sustainThreshold);
            virtual ~ADSRProfile();

            double calculateAmplitude(int t);
            int getTotalSamples();

        private:
            int sample_rate;
            int attack_duration;
            int decay_duration;
            int sustain_duration;
            int release_duration;
            double sustain_threshold;
            double max_amplitude = 1.0;

            double cubeRoot(double x) const {
                return std::cbrt(x);
            }
            double cubed(double x) const {
                return x * x * x;
            }
            double applyNormalizedInverseCubicScaling(double x, double lower, double upper) const {
                double normalized = (x - lower) / (upper - lower);
                return cubeRoot(normalized) * (upper - lower) + lower;
            }
            double applyNormalizedCubicScaling(double x, double lower, double upper) const {
                double normalized = (x - lower) / (upper - lower);
                return cubed(normalized) * (upper - lower) + lower;
            }
    };
}

#endif // ADSRPROFILE_H
