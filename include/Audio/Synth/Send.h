#ifndef AUDIO_SYNTH_SEND_H
#define AUDIO_SYNTH_SEND_H

#include <string>

#include "ParamTarget.h"

namespace Audio::Synth {
    class Send : public ParamTarget {
    public:
        Send() = default;

        void configure(int targetBusIndex, bool preFader, double gain) {
            _targetBus = targetBusIndex;
            _preFader = preFader;
            _gain = gain;
        }

        double render(double tap) const { return tap * _gain; }

        int targetBus() const { return _targetBus; }

        bool preFader() const { return _preFader; }

        void setParam(const std::string &path, const ParamValue &value) override {
            if (path == "gain") _gain = value.asDouble();
        }

        ParamValue getParam(const std::string &path) const override {
            if (path == "gain") return _gain;
            return ParamValue{};
        }

    private:
        int _targetBus = -1;
        bool _preFader = false;
        double _gain = 1.0;
    };
}

#endif
