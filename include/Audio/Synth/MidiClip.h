#ifndef AUDIO_SYNTH_MIDICLIP_H
#define AUDIO_SYNTH_MIDICLIP_H

#include <cstddef>

#include "MidiEvent.h"
#include "MidiSource.h"

namespace Audio::Synth {
    // Maps one channel of one source onto a target roll, shifted by `offset` samples. Owns its cursor,
    // so the same shared source can appear in many clips independently.
    class MidiClip {
    public:
        MidiClip(MidiSource *source, int sourceChannel, long offset)
            : _source(source), _sourceChannel(sourceChannel), _offset(offset) {
        }

        void drain(long from, long to, MidiEventSink &sink) {
            _source->drain(_sourceChannel, _offset, from, to, _cursor, sink);
        }

        bool exhausted() const { return _source->exhausted(_sourceChannel, _cursor); }

        void reset() { _cursor = 0; }

    private:
        MidiSource *_source;
        int _sourceChannel;
        long _offset;
        std::size_t _cursor = 0;
    };
}

#endif
