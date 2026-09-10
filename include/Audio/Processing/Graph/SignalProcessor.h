#ifndef AUDIO_PROCESSING_SIGNALPROCESSOR_H
#define AUDIO_PROCESSING_SIGNALPROCESSOR_H

#include <cstddef>

#include "AudioContext.h"

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
    };
}

#endif
