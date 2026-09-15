#ifndef AUDIO_DSP_PARAMVALUE_H
#define AUDIO_DSP_PARAMVALUE_H

#include <string>
#include <variant>
#include <utility>

namespace Audio::Dsp {
    class ParamValue {
    public:
        ParamValue() : _v(0.0) {}
        ParamValue(double d) : _v(d) {}
        ParamValue(int i) : _v(i) {}
        ParamValue(bool b) : _v(b) {}
        ParamValue(const char *s) : _v(std::string(s)) {}
        ParamValue(std::string s) : _v(std::move(s)) {}

        bool isString() const { return std::holds_alternative<std::string>(_v); }
        bool isBool() const { return std::holds_alternative<bool>(_v); }
        bool isInt() const { return std::holds_alternative<int>(_v); }
        bool isDouble() const { return std::holds_alternative<double>(_v); }

        double asDouble() const {
            if (auto p = std::get_if<double>(&_v)) return *p;
            if (auto p = std::get_if<int>(&_v)) return static_cast<double>(*p);
            if (auto p = std::get_if<bool>(&_v)) return *p ? 1.0 : 0.0;
            return 0.0;
        }

        int asInt() const {
            if (auto p = std::get_if<int>(&_v)) return *p;
            if (auto p = std::get_if<double>(&_v)) return static_cast<int>(*p);
            if (auto p = std::get_if<bool>(&_v)) return *p ? 1 : 0;
            return 0;
        }

        bool asBool() const {
            if (auto p = std::get_if<bool>(&_v)) return *p;
            if (auto p = std::get_if<int>(&_v)) return *p != 0;
            if (auto p = std::get_if<double>(&_v)) return *p != 0.0;
            return false;
        }

        std::string asString() const {
            if (auto p = std::get_if<std::string>(&_v)) return *p;
            return {};
        }

    private:
        std::variant<double, int, bool, std::string> _v;
    };
}

#endif
