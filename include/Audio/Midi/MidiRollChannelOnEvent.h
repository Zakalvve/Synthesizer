#ifndef MIDIROLLCHANNELONEVENT_H
#define MIDIROLLCHANNELONEVENT_H

#include <string>

#include "MidiRollEvent.h"
#include "MidiEventProcessor.h"

namespace Audio {
    namespace Midi {
        class MidiRollChannelOnEvent : public MidiRollEvent {
            public:
                MidiRollChannelOnEvent(int deltaTime, const std::string& type, const std::string& subType, int channel) : 
                    MidiRollEvent(deltaTime, type, subType, channel) {}
                virtual ~MidiRollChannelOnEvent() {}

                virtual void process(MidiEventProcessor& processor) { processor.processMidiEvent(*this); }
            private:
        };
    }
}

#endif //MIDIROLLCHANNELONEVENT_H