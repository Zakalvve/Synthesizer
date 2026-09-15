#include "FileSource.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "MidiJsonFile.h"
#include "MidiSequence.h"
#include "MidiTrack.h"
#include "MidiMessage.h"
#include "MidiNoteMessage.h"

namespace Audio::Engine::Midi {
    const std::vector<Event> FileSource::_empty;

    FileSource::FileSource(Audio::Midi::MidiSequence &sequence, int sampleRate) {
        using namespace Audio::Midi;

        const int ticksPerQuarter = sequence.getTicksPerQuarterNote();
        const int usPerQuarter = sequence.getMicrosecondsPerQuarterNote();
        if (ticksPerQuarter <= 0) return;

        const int samplesPerTick = static_cast<int>(std::round(
            (static_cast<double>(usPerQuarter) * sampleRate) /
            (static_cast<double>(ticksPerQuarter) * 1000000.0)));

        for (auto &trackPtr: sequence.getTracks()) {
            if (!trackPtr) continue;
            long cumTick = 0;
            std::unique_ptr<MidiMessage> message;
            while ((message = trackPtr->takeNext()) != nullptr) {
                cumTick += message->getDeltaTime();
                const long sampleTime = cumTick * samplesPerTick;

                if (auto *note = dynamic_cast<MidiNoteMessage *>(message.get())) {
                    Event e;
                    e.sampleTime = sampleTime;
                    e.channel = note->getChannel();
                    e.kind = (note->getSubType() == "keyOn") ? Event::Kind::NoteOn : Event::Kind::NoteOff;
                    e.note = note->getNote();
                    e.velocity = note->getVelocity();
                    _byChannel[e.channel].push_back(std::move(e));
                }
            }
        }

        for (auto &kv: _byChannel) {
            std::stable_sort(kv.second.begin(), kv.second.end(),
                             [](const Event &a, const Event &b) { return a.sampleTime < b.sampleTime; });
        }
    }

    std::unique_ptr<FileSource> FileSource::load(const std::string &path, int sampleRate) {
        Audio::Midi::MidiJsonFile file;
        if (!file.load(path.c_str())) return nullptr;
        auto &sequence = file.getSequence();
        if (!sequence) return nullptr;
        return std::make_unique<FileSource>(*sequence, sampleRate);
    }

    const std::vector<Event> &FileSource::channelEvents(int channel) const {
        auto it = _byChannel.find(channel);
        return it == _byChannel.end() ? _empty : it->second;
    }

    void FileSource::drain(int channel, long offset, long from, long to,
                           std::size_t &cursor, EventSink &sink) {
        const std::vector<Event> &events = channelEvents(channel);
        while (cursor < events.size()) {
            const long t = events[cursor].sampleTime + offset;
            if (t >= to) break;
            if (t >= from) {
                Event e = events[cursor];
                e.sampleTime = t;
                sink.push_back(std::move(e));
            }
            ++cursor;
        }
    }

    bool FileSource::exhausted(int channel, std::size_t cursor) const {
        return cursor >= channelEvents(channel).size();
    }
}
