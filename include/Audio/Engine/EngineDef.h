#ifndef AUDIO_ENGINE_ENGINEDEF_H
#define AUDIO_ENGINE_ENGINEDEF_H

#include <string>
#include <utility>
#include <vector>

#include "ParamValue.h"

namespace Audio::Engine {
    using Audio::Dsp::ParamValue;

    using Handle = int;
    inline constexpr Handle InvalidHandle = -1;

    struct NodeDef {
        std::string name;
        std::string type;
        std::vector<std::pair<std::string, ParamValue> > params;
    };

    struct ConnectionDef {
        std::string from;
        int outPort = 0;
        std::string to;
        int inPort = 0;
        bool delayed = false;
    };

    struct OutputDef {
        std::string node;
        int port = 0;
    };

    struct GraphDef {
        std::vector<NodeDef> nodes;
        std::vector<ConnectionDef> connections;
        std::vector<OutputDef> outputs;

        bool empty() const { return nodes.empty(); }
    };

    struct SynthDef {
        std::string name;
        int maxVoices = 16;
        double attack = 0.0;
        double decay = 0.0;
        double sustain = 0.0;
        double release = 0.0;
        bool velocitySensitivity = false;
        GraphDef voiceGraph;
    };

    struct SendDef {
        std::string name;
        std::string target;
        std::string tap = "post";
        double gain = 1.0;
    };

    struct ChannelDef {
        std::string name;
        int midiChannel = 0;
        double gain = 1.0;
        double pan = 0.0;
        bool mute = false;
        bool solo = false;
        std::string routeTo = "master";
        SynthDef synth;
        GraphDef postFx;
        std::vector<SendDef> sends;
    };

    struct BusDef {
        std::string name;
        std::string type = "sum";
        double gain = 1.0;
        GraphDef postFx;
        std::string output = "master";
    };

    struct TransportDef {
        double tempo = 120.0;
        int timeSigNum = 4;
        int timeSigDen = 4;
    };

    struct MasterDef {
        std::string name = "master";
        double gain = 1.0;
        GraphDef postFx;
    };

    struct EngineDef {
        int sampleRate = 44100;
        int maxChannels = 16;
        int maxBuses = 8;
        MasterDef master;
        TransportDef transport;
        std::vector<BusDef> buses;
        std::vector<ChannelDef> channels;
    };

    inline const ParamValue *findParam(const NodeDef &node, const std::string &key) {
        for (const auto &p: node.params) if (p.first == key) return &p.second;
        return nullptr;
    }

    inline double paramOr(const NodeDef &node, const std::string &key, double fallback) {
        const ParamValue *p = findParam(node, key);
        return p ? p->asDouble() : fallback;
    }

    inline int paramOr(const NodeDef &node, const std::string &key, int fallback) {
        const ParamValue *p = findParam(node, key);
        return p ? p->asInt() : fallback;
    }

    inline std::string paramOr(const NodeDef &node, const std::string &key, const std::string &fallback) {
        const ParamValue *p = findParam(node, key);
        return p ? p->asString() : fallback;
    }
}

#endif
