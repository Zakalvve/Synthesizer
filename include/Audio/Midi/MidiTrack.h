#ifndef MIDITRACK_H
#define MIDITRACK_H

#include <string>
#include <queue>
#include <memory>

#include "MidiMessage.h"

namespace Audio {
    namespace Midi {
        class MidiTrack {
            public:
                MidiTrack(const std::string& trackName, std::queue<std::unique_ptr<MidiMessage>>&& messages)
                    : trackName(trackName), messages(std::move(messages)) {}

                virtual ~MidiTrack() = default;

                const std::string& getTrackName() const { return trackName; }
                void setTrackName(std::string tn) { trackName = std::move(tn); }
                std::unique_ptr<MidiMessage> takeNext() {
                    if (messages.empty()) {
                        return nullptr;
                    }

                    std::unique_ptr<MidiMessage> message = std::move(messages.front());
                    messages.pop();
                    return message;
                }
            private:
                std::string trackName;
                std::queue<std::unique_ptr<MidiMessage>> messages;
        };
    }
}
#endif // MIDITRACK_H
