#ifndef AUDIO_SYNTH_PARAMTARGET_H
#define AUDIO_SYNTH_PARAMTARGET_H

#include <string>

#include "ParamValue.h"

namespace Audio::Synth {
    using Audio::Processing::ParamValue;

    class ParamTarget {
    public:
        virtual ~ParamTarget() = default;

        virtual void setParam(const std::string &path, const ParamValue &value) = 0;

        virtual ParamValue getParam(const std::string &path) const = 0;
    };
}

#endif
