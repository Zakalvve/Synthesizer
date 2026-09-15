#ifndef MIDIMESSAGE_H
#define MIDIMESSAGE_H

#include <string>

#include "MidiMessageVisitor.h"

namespace Audio {
    namespace Midi {
        class MidiMessage {
            public:
                MidiMessage(int deltaTime, const std::string& type, const std::string& subType, int channel) :
                    deltaTime(deltaTime), type(type), subType(subType), channel(channel) {}
                virtual ~MidiMessage() = default;
                int getDeltaTime() const { return deltaTime; }
                const std::string& getType() const { return type; }
                const std::string& getSubType() const { return subType; }
                int getChannel() const { return channel; }
                virtual void process(MidiMessageVisitor& visitor) { visitor.visit(*this); }
            private:
                int deltaTime;
                std::string type;
                std::string subType;
                int channel;
        };
    }
}

#endif // MIDIMESSAGE_H
