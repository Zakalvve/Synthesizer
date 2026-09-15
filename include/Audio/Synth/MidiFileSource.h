#ifndef AUDIO_SYNTH_MIDIFILESOURCE_H
#define AUDIO_SYNTH_MIDIFILESOURCE_H

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "MidiEvent.h"
#include "MidiSource.h"

namespace Audio::Midi {
    class MidiRoll;
}

namespace Audio::Synth {
    // Immutable, shared file source. Flattens a parsed MIDI file's tracks into time-sorted per-channel
    // event lists. Event sample times reproduce the old player's tick-quantised timing exactly.
    class MidiFileSource : public MidiSource {
    public:
        // Consumes `roll` (drains its track queues) to build the per-channel event lists.
        MidiFileSource(Audio::Midi::MidiRoll &roll, int sampleRate);

        // Loads `path` via MidiJsonFile and builds the source. Returns nullptr on load failure.
        static std::unique_ptr<MidiFileSource> load(const std::string &path, int sampleRate);

        void drain(int channel, long offset, long from, long to,
                   std::size_t &cursor, MidiEventSink &sink) override;

        bool exhausted(int channel, std::size_t cursor) const override;

    private:
        const std::vector<MidiEvent> &channelEvents(int channel) const;

        std::unordered_map<int, std::vector<MidiEvent> > _byChannel;
        static const std::vector<MidiEvent> _empty;
    };
}

#endif
