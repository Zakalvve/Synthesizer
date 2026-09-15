#ifndef AUDIO_ENGINE_MIDI_SOURCE_H
#define AUDIO_ENGINE_MIDI_SOURCE_H

#include <cstddef>

#include "Event.h"

namespace Audio::Engine::Midi {
    // A stream of channel-tagged MIDI events: a file source (immutable, cursored) now; a real-time
    // port binding later. Cursors are owned by the caller (a Clip) so one shared source can feed many
    // clips at different channels/offsets.
    class Source {
    public:
        virtual ~Source() = default;

        // Append events on `channel` whose time (shifted by `offset`) falls in [from, to) to `sink`,
        // advancing the caller-owned `cursor`.
        virtual void drain(int channel, long offset, long from, long to,
                           std::size_t &cursor, EventSink &sink) = 0;

        // True once `cursor` has consumed every event this source has for `channel`.
        virtual bool exhausted(int channel, std::size_t cursor) const = 0;
    };
}

#endif
