#ifndef AUDIO_SYNTH_BUS_H
#define AUDIO_SYNTH_BUS_H

#include <string>
#include <utility>

#include "AudioContext.h"
#include "AudioPipeline.h"
#include "AudioSample.h"
#include "ParamTarget.h"

namespace Audio::Synth {
    inline Audio::AudioSample renderStereoPostFx(Audio::Processing::AudioPipeline &pipe,
                                                 const Audio::AudioSample &in,
                                                 Audio::Processing::AudioContext &ctx) {
        ctx.sourceValue = in.c1;
        const double l = pipe.renderInstance(0, ctx)[0];
        ctx.sourceValue = in.c2;
        const double r = pipe.renderInstance(1, ctx)[0];
        return Audio::AudioSample(l, r);
    }

    class Bus : public ParamTarget {
    public:
        enum class Type { Sum, Average };

        Bus() = default;

        void configure(Type type, double gain, int outputTargetIndex) {
            _type = type;
            _gain = gain;
            _outputTarget = outputTargetIndex;
            _active = true;
        }

        void setPostFx(Audio::Processing::AudioPipeline postFx) {
            _postFx = std::move(postFx);
            _hasPostFx = true;
        }

        bool active() const { return _active; }

        int outputTarget() const { return _outputTarget; }

        Audio::AudioSample process(const Audio::AudioSample &accum, int count,
                                   Audio::Processing::AudioContext &ctx) {
            Audio::AudioSample v = accum;
            if (_type == Type::Average && count > 0) v = v * (1.0 / static_cast<double>(count));
            if (_hasPostFx) v = renderStereoPostFx(_postFx, v, ctx);
            v = v * _gain;
            return v;
        }

        void setParam(const std::string &path, const ParamValue &value) override {
            if (path == "gain") _gain = value.asDouble();
        }

        ParamValue getParam(const std::string &path) const override {
            if (path == "gain") return _gain;
            return ParamValue{};
        }

        static Type parseType(const std::string &s) {
            if (s == "average") return Type::Average;
            return Type::Sum;
        }

    private:
        Type _type = Type::Sum;
        double _gain = 1.0;
        int _outputTarget = -1;
        bool _active = false;
        bool _hasPostFx = false;
        Audio::Processing::AudioPipeline _postFx;
    };
}

#endif
