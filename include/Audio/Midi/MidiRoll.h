#ifndef MIDIROLL_H
#define MIDIROLL_H

#include <vector>

#include "MidiRollTrack.h"

namespace Audio {
    namespace Midi {
        class MidiRoll {
            public:
                MidiRoll(int ticksPerQuarterNote, int microsecondsPerQuarterNote, int tracksCount, std::vector<std::unique_ptr<MidiRollTrack>>&& tracks) : 
                    ticksPerQuarterNote(ticksPerQuarterNote), microsecondsPerQuarterNote(microsecondsPerQuarterNote), tracksCount(tracksCount), tracks(std::move(tracks)) {}
                ~MidiRoll() = default;
                
                int getTicksPerQuarterNote() const { return ticksPerQuarterNote; }
                int getMicrosecondsPerQuarterNote() const { return microsecondsPerQuarterNote; }
                int getTracksCount() const { return tracksCount; }

                const std::vector<std::unique_ptr<MidiRollTrack>>& getTracks() const { return tracks; }
            private:
                int ticksPerQuarterNote;
                int microsecondsPerQuarterNote;
                int tracksCount;

                std::vector<std::unique_ptr<MidiRollTrack>> tracks;
        };
    }
}

#endif //MIDIROLL_H