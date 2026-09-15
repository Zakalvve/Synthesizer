#include "AudioEngine.h"

#include <stdexcept>

#include "AudioContext.h"
#include "EngineCompiler.h"

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

    bool AudioEngine::isPlaying() const { return _transport.playing; }

    bool AudioEngine::isSounding() const {
        for (const auto &ch: _channels)
            if (ch && ch->active() && ch->isActive()) return true;
        return false;
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
        ctx.transportSample = _transport.samplePosition;

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
