#ifndef AUDIO_ENGINE_MASTER_H
#define AUDIO_ENGINE_MASTER_H

#include <string>

#include "AudioPipeline.h"
#include "ParamTarget.h"

namespace Audio::Engine {
    class Master : public ParamTarget {
    public:
        double gain = 1.0;
        bool hasPostFx = false;
        Audio::Dsp::AudioPipeline postFx;

        void setParam(const std::string &path, const ParamValue &value) override {
            if (path == "gain") gain = value.asDouble();
        }

        ParamValue getParam(const std::string &path) const override {
            if (path == "gain") return gain;
            return ParamValue{};
        }
    };
}

#endif
