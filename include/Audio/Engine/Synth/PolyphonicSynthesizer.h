#ifndef AUDIO_ENGINE_POLYPHONICSYNTHESIZER_H
#define AUDIO_ENGINE_POLYPHONICSYNTHESIZER_H

#include <string>
#include <vector>

#include "AudioContext.h"
#include "AudioPipeline.h"
#include "SignalProcessor.h"
#include "ParamTarget.h"

namespace Audio::Engine {
    class PolyphonicSynthesizer : public ParamTarget {
    public:
        PolyphonicSynthesizer() = default;

        void configure(Audio::Dsp::AudioPipeline voicePipeline, int maxVoices, int sampleRate,
                       double attackSec, double decaySec, double sustainLevel, double releaseSec,
                       bool velocitySensitivity, Audio::Dsp::SignalProcessor *envelopeNode);

        void playNote(const std::string &noteKey, double frequency, double velocity, long transportSample);

        void releaseNote(const std::string &noteKey);

        double sample(Audio::Dsp::AudioContext &ctx);

        bool isActive() const;

        bool velocitySensitivity() const { return _velocitySensitivity; }

        bool configured() const { return _configured; }

        void setParam(const std::string &path, const ParamValue &value) override;

        ParamValue getParam(const std::string &path) const override;

    private:
        struct Voice {
            bool inUse = false;
            bool gate = false;
            double frequency = 0.0;
            double velocity = 1.0;
            long triggerSample = 0;
            long endNoteSample = -1;
            std::string noteKey;
            long order = 0;
        };

        int findFreeOrOldest();

        Audio::Dsp::AudioPipeline _pipeline;
        std::vector<Voice> _voices;
        long _orderCounter = 0;
        int _sampleRate = 44100;
        double _attackSec = 0.0;
        double _decaySec = 0.0;
        double _sustainLevel = 0.0;
        double _releaseSec = 0.0;
        int _attackDur = 0;
        int _decayDur = 0;
        int _releaseDur = 0;
        bool _velocitySensitivity = false;
        bool _configured = false;
        Audio::Dsp::SignalProcessor *_envelopeNode = nullptr;
    };
}

#endif
