#include "MidiFileSource.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "MidiJsonFile.h"
#include "MidiRoll.h"
#include "MidiRollTrack.h"
#include "MidiRollEvent.h"
#include "MidiRollKeyEvent.h"

namespace Audio::Synth {
    const std::vector<MidiEvent> MidiFileSource::_empty;

    MidiFileSource::MidiFileSource(Audio::Midi::MidiRoll &roll, int sampleRate) {
        using namespace Audio::Midi;

        const int ticksPerQuarter = roll.getTicksPerQuarterNote();
        const int usPerQuarter = roll.getMicrosecondsPerQuarterNote();
        if (ticksPerQuarter <= 0) return;

        const int samplesPerTick = static_cast<int>(std::round(
            (static_cast<double>(usPerQuarter) * sampleRate) /
            (static_cast<double>(ticksPerQuarter) * 1000000.0)));

        for (auto &trackPtr: roll.getTracks()) {
            if (!trackPtr) continue;
            long cumTick = 0;
            std::unique_ptr<MidiRollEvent> ev;
            while ((ev = trackPtr->takeNext()) != nullptr) {
                cumTick += ev->getDeltaTime();
                const long sampleTime = cumTick * samplesPerTick;

                if (auto *key = dynamic_cast<MidiRollKeyEvent *>(ev.get())) {
                    MidiEvent me;
                    me.sampleTime = sampleTime;
                    me.channel = key->getChannel();
                    me.kind = (key->getSubType() == "keyOn") ? MidiEvent::Kind::NoteOn : MidiEvent::Kind::NoteOff;
                    me.note = key->getNote();
                    me.velocity = key->getVelocity();
                    _byChannel[me.channel].push_back(std::move(me));
                }
            }
        }

        for (auto &kv: _byChannel) {
            std::stable_sort(kv.second.begin(), kv.second.end(),
                             [](const MidiEvent &a, const MidiEvent &b) { return a.sampleTime < b.sampleTime; });
        }
    }

    std::unique_ptr<MidiFileSource> MidiFileSource::load(const std::string &path, int sampleRate) {
        Audio::Midi::MidiJsonFile mj;
        if (!mj.load(path.c_str(), sampleRate)) return nullptr;
        auto &roll = mj.getRoll();
        if (!roll) return nullptr;
        return std::make_unique<MidiFileSource>(*roll, sampleRate);
    }

    const std::vector<MidiEvent> &MidiFileSource::channelEvents(int channel) const {
        auto it = _byChannel.find(channel);
        return it == _byChannel.end() ? _empty : it->second;
    }

    void MidiFileSource::drain(int channel, long offset, long from, long to,
                               std::size_t &cursor, MidiEventSink &sink) {
        const std::vector<MidiEvent> &events = channelEvents(channel);
        while (cursor < events.size()) {
            const long t = events[cursor].sampleTime + offset;
            if (t >= to) break;
            if (t >= from) {
                MidiEvent e = events[cursor];
                e.sampleTime = t;
                sink.push_back(std::move(e));
            }
            ++cursor;
        }
    }

    bool MidiFileSource::exhausted(int channel, std::size_t cursor) const {
        return cursor >= channelEvents(channel).size();
    }
}
