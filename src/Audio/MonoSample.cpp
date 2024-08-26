#include "MonoSample.h"
#include "StereoSample.h"

namespace Audio {
    StereoSample MonoSample::toStereo() const {
        return StereoSample(sample, sample);
    }
}