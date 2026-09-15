#ifndef MIDICHANNELONMESSAGE_H
#define MIDICHANNELONMESSAGE_H

#include <string>

#include "MidiMessage.h"
#include "MidiMessageVisitor.h"

namespace Audio {
    namespace Midi {
        class MidiChannelOnMessage : public MidiMessage {
            public:
                MidiChannelOnMessage(int deltaTime, const std::string& type, const std::string& subType, int channel) :
                    MidiMessage(deltaTime, type, subType, channel) {}
                virtual ~MidiChannelOnMessage() {}
                virtual void process(MidiMessageVisitor& visitor) { visitor.visit(*this); }
        };
    }
}

#endif // MIDICHANNELONMESSAGE_H
