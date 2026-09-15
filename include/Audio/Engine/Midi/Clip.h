#ifndef AUDIO_ENGINE_MIDI_CLIP_H
#define AUDIO_ENGINE_MIDI_CLIP_H

#include <cstddef>

#include "Event.h"
#include "Source.h"

namespace Audio::Engine::Midi {
    // Maps one channel of one source onto a roll, shifted by `offset` samples. Owns its cursor, so the
    // same shared source can appear in many clips independently.
    class Clip {
    public:
        Clip(Source *source, int sourceChannel, long offset)
            : _source(source), _sourceChannel(sourceChannel), _offset(offset) {
        }

        void drain(long from, long to, EventSink &sink) {
            _source->drain(_sourceChannel, _offset, from, to, _cursor, sink);
        }

        bool exhausted() const { return _source->exhausted(_sourceChannel, _cursor); }

        void reset() { _cursor = 0; }

    private:
        Source *_source;
        int _sourceChannel;
        long _offset;
        std::size_t _cursor = 0;
    };
}

#endif
