#ifndef MIDINOTEMESSAGE_H
#define MIDINOTEMESSAGE_H

#include <string>

#include "MidiMessage.h"
#include "MidiMessageVisitor.h"

namespace Audio {
    namespace Midi {
        class MidiNoteMessage final : public MidiMessage {
            public:
                MidiNoteMessage(int deltaTime, const std::string& type, const std::string& subType, int channel, const std::string& note, int velocity) :
                    MidiMessage(deltaTime, type, subType, channel), note(note), velocity(velocity) {}
                virtual ~MidiNoteMessage() {}
                const std::string& getNote() const { return note; }
                int getVelocity() const { return velocity; }
                virtual void process(MidiMessageVisitor& visitor) { visitor.visit(*this); }
            private:
                std::string note;
                int velocity;
        };
    }
}

#endif // MIDINOTEMESSAGE_H
