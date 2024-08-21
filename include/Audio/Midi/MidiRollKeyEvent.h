#ifndef MIDIROLLKEYEVENT_H
#define MIDIROLLKEYEVENT_H

#include <string>

#include "MidiRollEvent.h"

namespace Audio {
    namespace Midi {
        class MidiRollKeyEvent : public MidiRollEvent {
            public:
                MidiRollKeyEvent(int deltaTime, const std::string& type, const std::string& subType, int channel, const std::string& note, int velocity) : 
                    MidiRollEvent(deltaTime, type, subType, channel), note(note), velocity(velocity) {}
                virtual ~MidiRollKeyEvent() {}
                const std::string& getNote() const { return note; }
                int getVelocity() const { return velocity; }
            private:
            std::string note;
            int velocity;
        };
    }
}

#endif //MIDIROLLKEYEVENT_H