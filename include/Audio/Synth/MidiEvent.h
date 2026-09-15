#ifndef AUDIO_SYNTH_MIDIEVENT_H
#define AUDIO_SYNTH_MIDIEVENT_H

#include <string>
#include <vector>

namespace Audio::Synth {
    struct MidiEvent {
        enum class Kind { NoteOn, NoteOff };

        long sampleTime = 0;
        int channel = 0;
        Kind kind = Kind::NoteOn;
        std::string note;
        int velocity = 0;
    };

    using MidiEventSink = std::vector<MidiEvent>;
}

#endif
