#ifndef AUDIO_SYNTH_MIDIROUTER_H
#define AUDIO_SYNTH_MIDIROUTER_H

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "EngineDef.h"
#include "MidiClip.h"
#include "MidiEvent.h"
#include "MidiRoll.h"
#include "MidiSource.h"

namespace Audio::Synth {
    using SourceId = int;
    inline constexpr SourceId InvalidSource = -1;

    // Engine-owned. Holds the shared source registry and the per-channel rolls, and pumps events into
    // channels during playback. Independent of EngineDef/compile() — it survives recompiles.
    class MidiRouter {
    public:
        SourceId addSource(std::unique_ptr<MidiSource> source) {
            _sources.push_back(std::move(source));
            return static_cast<SourceId>(_sources.size() - 1);
        }

        void addClip(SourceId sourceId, int sourceChannel, Handle channelHandle, long offset) {
            if (sourceId < 0 || sourceId >= static_cast<SourceId>(_sources.size())) return;
            _rolls[channelHandle].addClip(MidiClip(_sources[sourceId].get(), sourceChannel, offset));
        }

        // Drain each channel's roll over [from, to) and apply(handle, event) for each due event.
        template<class Apply>
        void pump(long from, long to, Apply &&apply) {
            for (auto &entry: _rolls) {
                _scratch.clear();
                entry.second.drain(from, to, _scratch);
                for (const MidiEvent &ev: _scratch) apply(entry.first, ev);
            }
        }

        bool exhausted() const {
            for (const auto &entry: _rolls)
                if (!entry.second.exhausted()) return false;
            return true;
        }

        void reset() {
            for (auto &entry: _rolls) entry.second.reset();
        }

    private:
        std::vector<std::unique_ptr<MidiSource> > _sources;
        std::unordered_map<Handle, MidiRoll> _rolls;
        MidiEventSink _scratch;
    };
}

#endif
