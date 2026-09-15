#ifndef AUDIO_ENGINE_MIDI_FILESOURCE_H
#define AUDIO_ENGINE_MIDI_FILESOURCE_H

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Event.h"
#include "Source.h"

namespace Audio::Midi {
    class MidiSequence;
}

namespace Audio::Engine::Midi {
    // Immutable, shared file source. Flattens a decoded MIDI file's tracks into time-sorted per-channel
    // event lists. Event sample times reproduce the old player's tick-quantised timing exactly.
    class FileSource : public Source {
    public:
        // Consumes `sequence` (drains its track queues) to build the per-channel event lists.
        FileSource(Audio::Midi::MidiSequence &sequence, int sampleRate);

        // Loads `path` via MidiJsonFile and builds the source. Returns nullptr on load failure.
        static std::unique_ptr<FileSource> load(const std::string &path, int sampleRate);

        void drain(int channel, long offset, long from, long to,
                   std::size_t &cursor, EventSink &sink) override;

        bool exhausted(int channel, std::size_t cursor) const override;

    private:
        const std::vector<Event> &channelEvents(int channel) const;

        std::unordered_map<int, std::vector<Event> > _byChannel;
        static const std::vector<Event> _empty;
    };
}

#endif
