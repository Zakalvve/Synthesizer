#ifndef AUDIO_PROCESSING_AUDIOCONTEXT_H
#define AUDIO_PROCESSING_AUDIOCONTEXT_H

#include <cstddef>

#include "Signal.h"

namespace Audio::Processing {
    class AudioContext {
    public:
        int sampleRate = 44100;
        long transportSample = 0;
        long noteSample = 0;
        double frequency = 0.0;
        bool gate = false;
        double velocity = 0.0;
        double volume = 1.0;
        double pan = 0.0;
        double sourceValue = 0.0;
        int blockSize = 1;

        const Signal *input(int port) const { return _scratch + _inputSlots[port] * blockSize; }
        Signal *output(int port) const { return _scratch + _outputSlots[port] * blockSize; }

        int inputCount() const { return _inputCount; }
        int outputCount() const { return _outputCount; }

        double *state() const { return _instanceState + _stateOffset; }

    private:
        friend class AudioPipeline;
        Signal *_scratch = nullptr;
        double *_instanceState = nullptr;
        const int *_inputSlots = nullptr;
        const int *_outputSlots = nullptr;
        int _inputCount = 0;
        int _outputCount = 0;
        std::size_t _stateOffset = 0;
    };
}

#endif
