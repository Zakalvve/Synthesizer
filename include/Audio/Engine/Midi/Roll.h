#ifndef AUDIO_ENGINE_MIDI_ROLL_H
#define AUDIO_ENGINE_MIDI_ROLL_H

#include <utility>
#include <vector>

#include "Clip.h"
#include "Event.h"

namespace Audio::Engine::Midi {
    // A per-engine-channel score: an ordered composite of clips. Drained as one merged stream that the
    // router feeds to a single channel.
    class Roll {
    public:
        void addClip(Clip clip) { _clips.push_back(std::move(clip)); }

        void drain(long from, long to, EventSink &sink) {
            for (Clip &c: _clips) c.drain(from, to, sink);
        }

        bool exhausted() const {
            for (const Clip &c: _clips) if (!c.exhausted()) return false;
            return true;
        }

        void reset() {
            for (Clip &c: _clips) c.reset();
        }

        bool empty() const { return _clips.empty(); }

    private:
        std::vector<Clip> _clips;
    };
}

#endif
