#ifndef AUDIO_ENGINE_CHANNEL_H
#define AUDIO_ENGINE_CHANNEL_H

#include <memory>
#include <string>
#include <vector>

#include "AudioContext.h"
#include "AudioPipeline.h"
#include "AudioSample.h"
#include "ParamTarget.h"
#include "PolyphonicSynthesizer.h"
#include "Send.h"

namespace Audio::Engine {
    class Channel : public ParamTarget {
    public:
        Channel() = default;

        PolyphonicSynthesizer &synth() { return _synth; }

        void configure(int midiChannel, double gain, double pan, bool mute, bool solo);

        void setPostFx(Audio::Dsp::AudioPipeline postFx);

        void setRouteTarget(int target) { _routeTarget = target; }

        void addSend(std::unique_ptr<Send> send) { _sends.push_back(std::move(send)); }

        std::vector<std::unique_ptr<Send> > &sends() { return _sends; }

        void activate() { _active = true; }

        bool active() const { return _active; }

        bool solo() const { return _solo; }

        bool audible(bool soloActive) const { return !_mute && (!soloActive || _solo); }

        int midiChannel() const { return _midiChannel; }

        int routeTarget() const { return _routeTarget; }

        double gain() const { return _gain; }

        void playNote(const std::string &noteName, double velocity, long transportSample);

        void releaseNote(const std::string &noteName);

        double renderMono(Audio::Dsp::AudioContext &ctx);

        Audio::AudioSample applyGainPan(double mono) const;

        bool isActive();

        void setParam(const std::string &path, const ParamValue &value) override;

        ParamValue getParam(const std::string &path) const override;

    private:
        PolyphonicSynthesizer _synth;
        int _midiChannel = 0;
        double _gain = 1.0;
        double _pan = 0.0;
        bool _mute = false;
        bool _solo = false;
        bool _active = false;
        int _routeTarget = -1;
        bool _hasPostFx = false;
        Audio::Dsp::AudioPipeline _postFx;
        std::vector<std::unique_ptr<Send> > _sends;
    };
}

#endif
