#ifndef AUDIO_SYNTH_AUDIOENGINE_H
#define AUDIO_SYNTH_AUDIOENGINE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "MidiEventProcessor.h"
#include "MidiRollEvent.h"
#include "MidiRollKeyEvent.h"
#include "MidiRollChannelOnEvent.h"

#include "AudioSample.h"
#include "EngineDef.h"
#include "ParamTarget.h"
#include "Transport.h"
#include "Master.h"
#include "Channel.h"
#include "Bus.h"
#include "NodeParamTarget.h"

namespace Audio::Synth {
    class EngineCompiler;

    class AudioEngine : public Midi::MidiEventProcessor {
    public:
        AudioEngine() = default;

        explicit AudioEngine(const EngineDef &def);

        void compile(const EngineDef &def);

        Handle handle(const std::string &name) const;

        void set(Handle target, const std::string &path, const ParamValue &value);

        void set(const std::string &name, const std::string &path, const ParamValue &value);

        ParamValue get(Handle target, const std::string &path) const;

        ParamValue get(const std::string &name, const std::string &path) const;

        void play();

        void stop();

        bool isPlaying() const;

        bool isSounding() const;

        Audio::AudioSample sample();

        void processMidiEvent(Midi::MidiRollEvent &event) override;

        void processMidiEvent(Midi::MidiRollChannelOnEvent &event) override;

        void processMidiEvent(Midi::MidiRollKeyEvent &event) override;

    private:
        friend class EngineCompiler;

        Channel *channelForMidi(int midiChannel);

        bool anySoloed() const;

        void routeAdd(int target, const Audio::AudioSample &s);

        int _sampleRate = 44100;
        Transport _transport;
        Master _master;

        std::vector<std::unique_ptr<Channel> > _channels;
        std::vector<std::unique_ptr<Bus> > _buses;
        std::vector<int> _busOrder;
        std::vector<std::unique_ptr<NodeParamTarget> > _nodeTargets;

        std::unordered_map<std::string, Handle> _names;
        std::unordered_map<Handle, ParamTarget *> _targets;
        std::unordered_map<Handle, std::string> _handleToName;
        Handle _nextHandle = 0;
        std::unordered_map<std::string, std::unordered_map<std::string, ParamValue> > _overrides;
        std::unordered_map<int, int> _midiToChannel;

        Audio::AudioSample _masterAccum{0.0, 0.0};
        std::vector<Audio::AudioSample> _busAccum;
        std::vector<int> _busCount;

        EngineDef _def;
    };
}

#endif
