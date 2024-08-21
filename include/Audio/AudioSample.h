#ifndef AUDIOSAMPLE_H
#define AUDIOSAMPLE_H

#include "Utils/Clamp.h"

namespace Audio {

    // An immutable stereo audio sample
    struct AudioSample {
        const double c1;
        const double c2;

        AudioSample(const double c1, const double c2) : c1(c1), c2(c2) {}

        AudioSample(const AudioSample& other) = default;

        AudioSample(AudioSample&& other) noexcept = default;

        AudioSample& operator=(const AudioSample& other) = delete;

        AudioSample& operator=(AudioSample&& other) noexcept = delete;

        AudioSample toMono() const {
            double normalized = (c1 + c2) / 2;
            return AudioSample(normalized, normalized);
        }

        AudioSample operator*(double scalar) const {
            return AudioSample(c1 * scalar, c2 * scalar);
        }

        AudioSample clamp(double min, double max) const {
            double clamped_c1 = ::clamp(c1, min, max);
            double clamped_c2 = ::clamp(c2, min, max);

            return AudioSample(clamped_c1, clamped_c2);
        }
    };
}

#endif // AUDIOSAMPLE_H
