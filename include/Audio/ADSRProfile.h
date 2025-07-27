#ifndef ADSRPROFILE_H
#define ADSRPROFILE_H

#include <cmath>

namespace Audio {
    class ADSRProfile {
        public:
            ADSRProfile(int sampleRate, double attackDuration, double decayDuration, double releaseDuration, double sustainThreshold);
            virtual ~ADSRProfile() = default;

            double calculateAmplitude(int t);
            void play();
            void release();
            bool isEnded();

        private:
            double max_amplitude = 1.0;
            int sample_rate;
            bool is_sustaining;
            bool is_active;
            int attack_duration;
            int decay_duration;
            int sustain_duration;
            double sustain_threshold;
            int release_duration;

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
