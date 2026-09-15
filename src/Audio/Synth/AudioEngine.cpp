#include "AudioEngine.h"

#include <stdexcept>
#include <vector>

#include "AudioContext.h"
#include "EngineCompiler.h"
#include "MidiFileSource.h"
#include "WAVFile.h"

namespace Audio::Synth {
    using namespace Audio::Processing;

    AudioEngine::AudioEngine(const EngineDef &def) {
        compile(def);
    }

    void AudioEngine::compile(const EngineDef &def) {
        if (_transport.playing)
            throw std::runtime_error("AudioEngine::compile: cannot compile while playing");
        EngineCompiler::build(*this, def);
    }

    Handle AudioEngine::handle(const std::string &name) const {
        auto it = _names.find(name);
        return it == _names.end() ? InvalidHandle : it->second;
    }

    void AudioEngine::set(Handle target, const std::string &path, const ParamValue &value) {
        auto it = _targets.find(target);
        if (it == _targets.end()) return;
        it->second->setParam(path, value);

        auto nameIt = _handleToName.find(target);
        if (nameIt != _handleToName.end()) _overrides[nameIt->second][path] = value;
    }

    void AudioEngine::set(const std::string &name, const std::string &path, const ParamValue &value) {
        set(handle(name), path, value);
    }

    ParamValue AudioEngine::get(Handle target, const std::string &path) const {
        auto it = _targets.find(target);
        if (it == _targets.end()) return ParamValue{};
        return it->second->getParam(path);
    }

    ParamValue AudioEngine::get(const std::string &name, const std::string &path) const {
        return get(handle(name), path);
    }

    void AudioEngine::play() { _transport.playing = true; }

    void AudioEngine::stop() { _transport.playing = false; }

    void AudioEngine::rewind() {
        _transport.samplePosition = 0;
        _router.reset();
    }

    bool AudioEngine::isPlaying() const { return _transport.playing; }

    bool AudioEngine::isSounding() const {
        for (const auto &ch: _channels)
            if (ch && ch->active() && ch->isActive()) return true;
        return false;
    }

    bool AudioEngine::isFinished() const {
        return _router.exhausted() && !isSounding();
    }

    SourceId AudioEngine::addMidiFile(const std::string &path) {
        auto source = MidiFileSource::load(path, _sampleRate);
        if (!source) return InvalidSource;
        return _router.addSource(std::move(source));
    }

    void AudioEngine::assignClip(SourceId sourceId, int sourceChannel, Handle channelHandle, long offset) {
        _router.addClip(sourceId, sourceChannel, channelHandle, offset);
    }

    bool AudioEngine::anySoloed() const {
        for (const auto &ch: _channels)
            if (ch && ch->active() && ch->solo()) return true;
        return false;
    }

    Channel *AudioEngine::channelForMidi(int midiChannel) {
        auto it = _midiToChannel.find(midiChannel);
        if (it == _midiToChannel.end()) return nullptr;
        return _channels[it->second].get();
    }

    void AudioEngine::routeAdd(int target, const Audio::AudioSample &s) {
        if (target < 0) {
            _masterAccum = _masterAccum + s;
        } else {
            _busAccum[target] = _busAccum[target] + s;
            ++_busCount[target];
        }
    }

    Audio::AudioSample AudioEngine::sample() {
        AudioContext ctx;
        ctx.sampleRate = _sampleRate;
        const long pos = _transport.samplePosition;
        ctx.transportSample = pos;

        if (_transport.playing) {
            _router.pump(pos, pos + 1, [this, pos](Handle h, const MidiEvent &ev) {
                auto it = _channelByHandle.find(h);
                if (it == _channelByHandle.end()) return;
                Channel *ch = it->second;
                if (ev.kind == MidiEvent::Kind::NoteOn) {
                    const double velocity = ch->synth().velocitySensitivity() ? (ev.velocity / 127.0) : 1.0;
                    ch->playNote(ev.note, velocity, pos);
                } else {
                    ch->releaseNote(ev.note);
                }
            });
        }

        _masterAccum = Audio::AudioSample(0.0, 0.0);
        for (std::size_t i = 0; i < _busAccum.size(); ++i) {
            _busAccum[i] = Audio::AudioSample(0.0, 0.0);
            _busCount[i] = 0;
        }

        const bool solo = anySoloed();

        for (auto &ch: _channels) {
            if (!ch || !ch->active()) continue;

            const double mono = ch->renderMono(ctx);
            const Audio::AudioSample out = ch->applyGainPan(mono);
            if (ch->audible(solo)) routeAdd(ch->routeTarget(), out);

            for (auto &sp: ch->sends()) {
                Send &s = *sp;
                const double tap = s.preFader() ? mono : (mono * ch->gain());
                const double sent = s.render(tap);
                const int b = s.targetBus();
                if (b >= 0 && b < static_cast<int>(_buses.size())) {
                    _busAccum[b] = _busAccum[b] + Audio::AudioSample(sent, sent);
                    ++_busCount[b];
                }
            }
        }

        for (const int bi: _busOrder) {
            Bus *bus = _buses[bi].get();
            if (!bus || !bus->active()) continue;
            const Audio::AudioSample v = bus->process(_busAccum[bi], _busCount[bi], ctx);
            routeAdd(bus->outputTarget(), v);
        }

        Audio::AudioSample total = _masterAccum;
        if (_master.hasPostFx) total = renderStereoPostFx(_master.postFx, total, ctx);
        total = total * _master.gain;

        if (_transport.playing) ++_transport.samplePosition;
        return total;
    }

    int AudioEngine::render(Audio::AudioSample *out, int frames) {
        int i = 0;
        for (; i < frames; ++i) {
            if (isFinished()) break;
            out[i] = sample();
        }
        return i;
    }

    bool AudioEngine::save(const std::string &path) {
        if (path.size() < 4 || path.compare(path.size() - 4, 4, ".wav") != 0) return false;

        const int bits = 32;
        const int maxAmplitude = (1u << (bits - 1)) - 1;

        rewind();
        play();

        Audio::WAVFile wav(true, _sampleRate, bits, maxAmplitude);
        wav.open(path);

        std::vector<Audio::AudioSample> buffer(4096, Audio::AudioSample(0.0, 0.0));
        while (!isFinished()) {
            const int n = render(buffer.data(), static_cast<int>(buffer.size()));
            if (n == 0) break;
            for (int i = 0; i < n; ++i) {
                Audio::AudioSample normalized = buffer[i] * static_cast<double>(maxAmplitude);
                Audio::AudioSample clamped = normalized.clamp(-maxAmplitude, maxAmplitude);
                wav.sample(clamped);
            }
        }

        wav.close();
        stop();
        return true;
    }

    void AudioEngine::processMidiEvent(Midi::MidiRollEvent &) { }

    void AudioEngine::processMidiEvent(Midi::MidiRollChannelOnEvent &event) {
        Channel *ch = channelForMidi(event.getChannel());
        if (ch) ch->activate();
    }

    void AudioEngine::processMidiEvent(Midi::MidiRollKeyEvent &event) {
        Channel *ch = channelForMidi(event.getChannel());
        if (!ch) return;

        const std::string &subType = event.getSubType();
        if (subType == "keyOn") {
            const double velocity = ch->synth().velocitySensitivity() ? (event.getVelocity() / 127.0) : 1.0;
            ch->playNote(event.getNote(), velocity, _transport.samplePosition);
        } else if (subType == "keyOff") {
            ch->releaseNote(event.getNote());
        }
    }
}
