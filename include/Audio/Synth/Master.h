#ifndef AUDIO_SYNTH_MASTER_H
#define AUDIO_SYNTH_MASTER_H

#include <string>

#include "AudioPipeline.h"
#include "ParamTarget.h"

namespace Audio::Synth {
    class Master : public ParamTarget {
    public:
        double gain = 1.0;
        bool hasPostFx = false;
        Audio::Processing::AudioPipeline postFx;

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
