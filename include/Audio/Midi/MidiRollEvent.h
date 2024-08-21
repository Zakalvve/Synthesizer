#ifndef MIDIROLLEVENT_H
#define MIDIROLLEVENT_H

#include <string>

namespace Audio {
    namespace Midi {
        class MidiRollEvent {
            public:
                MidiRollEvent(int deltaTime, const std::string& type, const std::string& subType, int channel) : 
                    deltaTime(deltaTime), type(type), subType(subType), channel(channel) {}
                virtual ~MidiRollEvent() {}
                int getDeltaTime() const { return deltaTime; }
                const std::string& getType() const { return type; }
                const std::string& getSubType() const { return subType; }
                int getChannel() const { return channel; }
            private:
                int deltaTime;
                std::string type;
                std::string subType;
                int channel;
        };
    }
}

#endif //MIDIROLLEVENT_H