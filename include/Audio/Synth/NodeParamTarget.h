#ifndef AUDIO_SYNTH_NODEPARAMTARGET_H
#define AUDIO_SYNTH_NODEPARAMTARGET_H

#include <string>

#include "SignalProcessor.h"
#include "ParamTarget.h"

namespace Audio::Synth {
    class NodeParamTarget : public ParamTarget {
    public:
        explicit NodeParamTarget(Audio::Processing::SignalProcessor *processor) : _processor(processor) {
        }

        void setParam(const std::string &path, const ParamValue &value) override {
            _processor->setParam(path, value);
        }

        ParamValue getParam(const std::string &path) const override {
            return _processor->getParam(path);
        }

    private:
        Audio::Processing::SignalProcessor *_processor;
    };
}

#endif
