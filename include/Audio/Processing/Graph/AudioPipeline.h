#ifndef AUDIO_PROCESSING_AUDIOPIPELINE_H
#define AUDIO_PROCESSING_AUDIOPIPELINE_H

#include <cstddef>
#include <memory>
#include <vector>

#include "Signal.h"
#include "AudioContext.h"
#include "SignalProcessor.h"

namespace Audio::Processing {
    class SignalGraph;

    class AudioPipeline {
    public:
        AudioPipeline() = default;

        const Signal *renderInstance(int instanceIndex, AudioContext &ctx);

        void resetInstance(int instanceIndex);

        int instanceCount() const { return _maxInstances; }
        int blockSize() const { return _blockSize; }

    private:
        friend class SignalGraph;

        struct NodeRec {
            const SignalProcessor *processor = nullptr;
            std::vector<int> inputSlots;
            std::vector<int> outputSlots;
            std::size_t stateOffset = 0;
        };

        std::vector<std::unique_ptr<SignalProcessor> > _owned;
        std::vector<NodeRec> _nodes;
        std::vector<Signal> _scratch;
        std::vector<double> _stateArena;
        std::size_t _instanceStateSize = 0;
        int _maxInstances = 0;
        int _blockSize = 1;
        int _terminalSlot = -1;
    };
}

#endif
