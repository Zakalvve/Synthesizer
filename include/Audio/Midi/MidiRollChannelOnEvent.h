#ifndef MIDIROLLCHANNELONEVENT_H
#define MIDIROLLCHANNELONEVENT_H

#include <string>

#include "MidiRollEvent.h"

namespace Audio {
    namespace Midi {
        class MidiRollChannelOnEvent : public MidiRollEvent {
            public:
                MidiRollChannelOnEvent(int deltaTime, const std::string& type, const std::string& subType, int channel, const std::string& note, int velocity) : 
                    MidiRollEvent(deltaTime, type, subType, channel) {}
                virtual ~MidiRollChannelOnEvent() {}
            private:
        };
    }
}

#endif //MIDIROLLCHANNELONEVENT_H