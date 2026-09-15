#ifndef AUDIO_PROCESSING_SIGNALPROCESSOR_H
#define AUDIO_PROCESSING_SIGNALPROCESSOR_H

#include <cstddef>
#include <string>

#include "AudioContext.h"
#include "ParamValue.h"

namespace Audio::Processing {
    class SignalProcessor {
    public:
        virtual ~SignalProcessor() = default;

        virtual void process(AudioContext &ctx) const = 0;

        virtual int inputCount() const = 0;

        virtual int outputCount() const = 0;

        virtual std::size_t stateSize() const { return 0; }

        virtual void resetState(double * ) const {
        }

        virtual void setParam(const std::string &, const ParamValue &) {
        }

        virtual ParamValue getParam(const std::string &) const { return ParamValue{}; }
    };
}

#endif
