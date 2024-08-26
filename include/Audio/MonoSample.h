#ifndef MONOSAMPLE_H
#define MONOSAMPLE_H

#include "Utils/Clamp.h"

namespace Audio {
    struct StereoSample;

    struct MonoSample {
        double sample;

        MonoSample(const double sample) : sample(sample) {}

        MonoSample(const MonoSample& other) = default;

        MonoSample(MonoSample&& other) noexcept = default;
        
        MonoSample& operator=(const MonoSample& other) = default;

        MonoSample& operator=(MonoSample&& other) noexcept = default;

        MonoSample operator+(const MonoSample& other) const {
            return MonoSample(this->sample + other.sample);
        }

        MonoSample operator*(double scalar) const {
            return MonoSample(sample * scalar);
        }

        StereoSample toStereo() const;

        MonoSample clamp(double min, double max) const {
            double clamped_sample = ::clamp(sample, min, max);

            return MonoSample(clamped_sample);
        }
    };
}

#endif //MONOSAMPLE_H
