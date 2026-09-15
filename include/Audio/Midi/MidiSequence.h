#ifndef MIDISEQUENCE_H
#define MIDISEQUENCE_H

#include <memory>
#include <utility>
#include <vector>

#include "MidiTrack.h"

namespace Audio {
    namespace Midi {
        // A decoded MIDI file: header fields + the tracks of timed messages. Playback timing lives in
        // the engine's Midi::FileSource, so this is a pure data container.
        class MidiSequence {
            public:
                MidiSequence(int ticksPerQuarterNote, int microsecondsPerQuarterNote, int tracksCount,
                             std::vector<std::unique_ptr<MidiTrack>>&& ts)
                    : ticksPerQuarterNote(ticksPerQuarterNote),
                      microsecondsPerQuarterNote(microsecondsPerQuarterNote),
                      tracksCount(tracksCount), tracks(std::move(ts)) {}

                ~MidiSequence() = default;

                int getTicksPerQuarterNote() const { return ticksPerQuarterNote; }
                int getMicrosecondsPerQuarterNote() const { return microsecondsPerQuarterNote; }
                int getTracksCount() const { return tracksCount; }

                const std::vector<std::unique_ptr<MidiTrack>>& getTracks() const { return tracks; }
                std::vector<std::unique_ptr<MidiTrack>>& getTracks() { return tracks; }

            private:
                int ticksPerQuarterNote;
                int microsecondsPerQuarterNote;
                int tracksCount;
                std::vector<std::unique_ptr<MidiTrack>> tracks;
        };
    }
}

#endif // MIDISEQUENCE_H
