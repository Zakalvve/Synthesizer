#include <vector>
#include <memory>
#include <cmath>

#include "MidiRoll.h"
#include "MidiRollTrack.h"
#include "MidiRollEvent.h"

namespace Audio {
    namespace Midi {
        MidiRoll::MidiRoll(int sampleRate, int ticksPerQuarterNote, int microsecondsPerQuarterNote, int tracksCount, std::vector<std::unique_ptr<MidiRollTrack>>&& ts) : 
            sampleRate(sampleRate), ticksPerQuarterNote(ticksPerQuarterNote), microsecondsPerQuarterNote(microsecondsPerQuarterNote), tracksCount(tracksCount), ticksSinceLastEvent(0), tracks(std::move(ts)), activeTrack(nullptr), activeEvent(nullptr)
        {
            samplesPerTick = static_cast<int>(std::round((static_cast<double>(microsecondsPerQuarterNote) * sampleRate) / (static_cast<double>(ticksPerQuarterNote) * MICROSECONDS_PER_SECOND)));

            if (tracks.size() > 0){
                setTrack(0);
            }
        }

        std::vector<std::unique_ptr<Events::MidiRollEvent>> MidiRoll::tickRoll(int t) {
            std::vector<std::unique_ptr<Events::MidiRollEvent>> eventsThisTick;

            if (t % samplesPerTick == 0 && isRolling()) {
                while (activeEvent->getDeltaTime() == ticksSinceLastEvent)
                {
                    ticksSinceLastEvent = 0;

                    eventsThisTick.push_back(std::move(activeEvent));

                    setEvent(*activeTrack);

                    if (activeEvent == nullptr) break;
                }
                
                ticksSinceLastEvent++;
            }

            return eventsThisTick;
        }

        void MidiRoll::setTrack(int i){
            if (i >= 0 && i < tracks.size()){
                activeTrack = tracks[i].get();

                if (activeTrack != nullptr){
                    setEvent(*activeTrack);
                }
            }
        }

        void MidiRoll::setEvent(MidiRollTrack& track){
            activeEvent = track.takeNext();
        }

        bool MidiRoll::isRolling() {
            return activeTrack != nullptr && activeEvent != nullptr;
        }
    }
}