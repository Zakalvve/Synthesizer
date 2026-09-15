#ifndef AUDIO_SYNTH_MIDIROLL_H
#define AUDIO_SYNTH_MIDIROLL_H

#include <utility>
#include <vector>

#include "MidiClip.h"
#include "MidiEvent.h"

namespace Audio::Synth {
    // A per-engine-channel score: an ordered composite of clips. Drained as one merged stream that the
    // router feeds to a single channel. (Distinct from Audio::Midi::MidiRoll, the file-parser output.)
    class MidiRoll {
    public:
        void addClip(MidiClip clip) { _clips.push_back(std::move(clip)); }

        void drain(long from, long to, MidiEventSink &sink) {
            for (MidiClip &c: _clips) c.drain(from, to, sink);
        }

        bool exhausted() const {
            for (const MidiClip &c: _clips) if (!c.exhausted()) return false;
            return true;
        }

        void reset() {
            for (MidiClip &c: _clips) c.reset();
        }

        bool empty() const { return _clips.empty(); }

    private:
        std::vector<MidiClip> _clips;
    };
}

#endif
