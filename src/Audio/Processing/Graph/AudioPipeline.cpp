#include "AudioPipeline.h"

#include <algorithm>

namespace Audio::Processing {
    const Signal *AudioPipeline::renderInstance(int instanceIndex, AudioContext &ctx) {
        double *instanceState =
                _stateArena.data() + static_cast<std::size_t>(instanceIndex) * _instanceStateSize;

        ctx._scratch = _scratch.data();
        ctx._instanceState = instanceState;
        ctx.blockSize = _blockSize;

        for (const NodeRec &rec: _nodes) {
            ctx._inputSlots = rec.inputSlots.data();
            ctx._outputSlots = rec.outputSlots.data();
            ctx._inputCount = static_cast<int>(rec.inputSlots.size());
            ctx._outputCount = static_cast<int>(rec.outputSlots.size());
            ctx._stateOffset = rec.stateOffset;
            rec.processor->process(ctx);
        }

        return _scratch.data() + static_cast<std::size_t>(_terminalSlot) * _blockSize;
    }

    void AudioPipeline::resetInstance(int instanceIndex) {
        double *instanceState =
                _stateArena.data() + static_cast<std::size_t>(instanceIndex) * _instanceStateSize;

        std::fill(instanceState, instanceState + _instanceStateSize, 0.0);
        for (const NodeRec &rec: _nodes) {
            rec.processor->resetState(instanceState + rec.stateOffset);
        }
    }
}
