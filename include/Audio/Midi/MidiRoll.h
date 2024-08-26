#ifndef MIDIROLL_H
#define MIDIROLL_H

#include <vector>
#include <memory>

#include "MidiRollTrack.h"
#include "MidiRollEvent.h"

namespace Audio {
    namespace Midi {
        class MidiRoll {
            public:
                MidiRoll(int sampleRate, int ticksPerQuarterNote, int microsecondsPerQuarterNote, int tracksCount, std::vector<std::unique_ptr<MidiRollTrack>>&& ts);
                ~MidiRoll() = default;
                
                int getTicksPerQuarterNote() const { return ticksPerQuarterNote; }
                int getMicrosecondsPerQuarterNote() const { return microsecondsPerQuarterNote; }
                int getTracksCount() const { return tracksCount; }
                std::vector<std::unique_ptr<Events::MidiRollEvent>> tickRoll(int t);
                void setTrack(int i);
                void setEvent(MidiRollTrack& track);
                bool isRolling();
                const std::vector<std::unique_ptr<MidiRollTrack>>& getTracks() const { return tracks; }
            private:
                const int MICROSECONDS_PER_SECOND = 1000000;
                int sampleRate;
                int ticksPerQuarterNote;
                int microsecondsPerQuarterNote;
                int tracksCount;
                int ticksSinceLastEvent;
                int samplesPerTick;

                std::vector<std::unique_ptr<MidiRollTrack>> tracks;
                MidiRollTrack* activeTrack;
                std::unique_ptr<Events::MidiRollEvent> activeEvent;
        };
    }
}

#endif //MIDIROLL_H