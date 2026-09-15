#ifndef AUDIO_ENGINE_MIDI_EVENT_H
#define AUDIO_ENGINE_MIDI_EVENT_H

#include <string>
#include <vector>

namespace Audio::Engine::Midi {
    struct Event {
        enum class Kind { NoteOn, NoteOff };

        long sampleTime = 0;
        int channel = 0;
        Kind kind = Kind::NoteOn;
        std::string note;
        int velocity = 0;
    };

    using EventSink = std::vector<Event>;
}

#endif
