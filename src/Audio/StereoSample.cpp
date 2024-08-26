#include "StereoSample.h"
#include "MonoSample.h"

namespace Audio {
    MonoSample StereoSample::toMono() const {
        double normalized = (c1 + c2) / 2;
        return MonoSample(normalized);
    }
}