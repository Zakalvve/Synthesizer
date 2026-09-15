#ifndef AUDIO_ENGINE_PARAMTARGET_H
#define AUDIO_ENGINE_PARAMTARGET_H

#include <string>

#include "ParamValue.h"

namespace Audio::Engine {
    using Audio::Dsp::ParamValue;

    class ParamTarget {
    public:
        virtual ~ParamTarget() = default;

        virtual void setParam(const std::string &path, const ParamValue &value) = 0;

        virtual ParamValue getParam(const std::string &path) const = 0;
    };
}

#endif
