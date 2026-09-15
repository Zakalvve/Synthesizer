#ifndef AUDIO_SYNTH_TRANSPORT_H
#define AUDIO_SYNTH_TRANSPORT_H

#include <string>

#include "ParamTarget.h"

namespace Audio::Synth {
    class Transport : public ParamTarget {
    public:
        bool playing = false;
        long samplePosition = 0;
        double tempo = 120.0;
        int timeSigNum = 4;
        int timeSigDen = 4;
        int sampleRate = 44100;

        double positionSeconds() const {
            return sampleRate > 0 ? static_cast<double>(samplePosition) / sampleRate : 0.0;
        }

        double beats() const { return positionSeconds() * tempo / 60.0; }

        double bars() const { return timeSigNum > 0 ? beats() / timeSigNum : 0.0; }

        void setParam(const std::string &path, const ParamValue &v) override {
            if (path == "samplePosition") samplePosition = static_cast<long>(v.asDouble());
            else if (path == "tempo") tempo = v.asDouble();
            else if (path == "timeSigNum") timeSigNum = v.asInt();
            else if (path == "timeSigDen") timeSigDen = v.asInt();
            else if (path == "playing") playing = v.asBool();
        }

        ParamValue getParam(const std::string &path) const override {
            if (path == "samplePosition") return static_cast<double>(samplePosition);
            if (path == "tempo") return tempo;
            if (path == "timeSigNum") return timeSigNum;
            if (path == "timeSigDen") return timeSigDen;
            if (path == "playing") return playing;
            if (path == "positionSeconds") return positionSeconds();
            if (path == "beat") return beats();
            if (path == "bar") return bars();
            return ParamValue{};
        }
    };
}

#endif
