#ifndef STEREOSAMPLE_H
#define STEREOSAMPLE_H

#include "Utils/Clamp.h"

namespace Audio {
    struct MonoSample;

    struct StereoSample {
        double c1;
        double c2;

        StereoSample(const double c1, const double c2) : c1(c1), c2(c2) {}

        StereoSample(const StereoSample& other) = default;

        StereoSample(StereoSample&& other) noexcept = default;
        
        StereoSample& operator=(const StereoSample& other) = default;

        StereoSample& operator=(StereoSample&& other) noexcept = default;

        StereoSample operator+(const StereoSample& other) const {
            return StereoSample(this->c1 + other.c1, this->c2+other.c2);
        }

        StereoSample operator*(double scalar) const {
            return StereoSample(c1 * scalar, c2 * scalar);
        }

        MonoSample toMono() const;

        StereoSample clamp(double min, double max) const {
            double clamped_c1 = ::clamp(c1, min, max);
            double clamped_c2 = ::clamp(c2, min, max);

            return StereoSample(clamped_c1, clamped_c2);
        }
    };
}

#endif //STEREOSAMPLE_H
