#ifndef AUDIO_SYNTH_MIDISOURCE_H
#define AUDIO_SYNTH_MIDISOURCE_H

#include <cstddef>

#include "MidiEvent.h"

namespace Audio::Synth {
    // A stream of channel-tagged MIDI events. A file source (immutable, cursored) now; a real-time
    // port binding later. Cursors are owned by the caller (a MidiClip) so one shared source can feed
    // many clips at different channels/offsets.
    class MidiSource {
    public:
        virtual ~MidiSource() = default;

        // Append events on `channel` whose time (shifted by `offset`) falls in [from, to) to `sink`,
        // advancing the caller-owned `cursor`.
        virtual void drain(int channel, long offset, long from, long to,
                           std::size_t &cursor, MidiEventSink &sink) = 0;

        // True once `cursor` has consumed every event this source has for `channel`.
        virtual bool exhausted(int channel, std::size_t cursor) const = 0;
    };
}

#endif
