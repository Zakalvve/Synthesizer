#ifndef MIDIEVENTPROCESSOR_H
#define MIDIEVENTPROCESSOR_H

#include "MidiRollEvent.h"
#include "MidiRollChannelOnEvent.h"
#include "MidiRollKeyEvent.h"

namespace Audio {
    namespace Midi {
        namespace Events {            
            class MidiEventProcessor {
                public:
                    virtual void processMidiEvent(MidiRollEvent& event) = 0;
                    virtual void processMidiEvent(MidiRollChannelOnEvent& event) = 0;
                    virtual void processMidiEvent(MidiRollKeyEvent& event) = 0;
            };
        }
    }
}

#endif // MIDIEVENTPROCESSOR_H