#ifndef MIDIROLLTRACK_H
#define MIDIROLLTRACK_H

#include <string>
#include <queue>
#include <memory>

#include "MidiRollEvent.h"

namespace Audio {
    namespace Midi {
        class MidiRollTrack {
            public:
                MidiRollTrack(const std::string& trackName, std::queue<std::unique_ptr<MidiRollEvent>>&& events) 
                    : trackName(trackName), events(std::move(events)) {}

                virtual ~MidiRollTrack() = default;
                
                const std::string& getTrackName() const { return trackName; }
                void setTrackName(std::string tn) { trackName = std::move(tn); }
                const std::queue<std::unique_ptr<MidiRollEvent>>& getEvents() const { return events; }
                const MidiRollEvent& takeNext() {
                    MidiRollEvent& event = *(events.front());
                    events.pop();
                    return event;
                }
            private:
                std::string trackName;
                std::queue<std::unique_ptr<MidiRollEvent>> events;
        };
    }
}
#endif //MIDIROLLTRACK_H