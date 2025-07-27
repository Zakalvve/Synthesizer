#ifndef MIDIEVENTPROCESSOR_H
#define MIDIEVENTPROCESSOR_H

namespace Audio {
    namespace Midi {
        class MidiRollEvent;
        class MidiRollKeyEvent;
        class MidiRollChannelOnEvent;
        
        class MidiEventProcessor {
        public:
            virtual void processMidiEvent(MidiRollEvent& event) = 0;
            virtual void processMidiEvent(MidiRollKeyEvent& event) = 0;
            virtual void processMidiEvent(MidiRollChannelOnEvent& event) = 0;
        };
    }
}

#endif // MIDIEVENTPROCESSOR_H