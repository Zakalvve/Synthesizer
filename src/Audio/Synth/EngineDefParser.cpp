#include "EngineDefParser.h"

#include <stdexcept>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

namespace Audio::Synth {
    namespace {
        using rapidjson::Value;

        double getDouble(const Value &o, const char *k, double d) {
            return (o.HasMember(k) && o[k].IsNumber()) ? o[k].GetDouble() : d;
        }

        int getInt(const Value &o, const char *k, int d) {
            if (!o.HasMember(k) || !o[k].IsNumber()) return d;
            return o[k].IsInt() ? o[k].GetInt() : static_cast<int>(o[k].GetDouble());
        }

        bool getBool(const Value &o, const char *k, bool d) {
            return (o.HasMember(k) && o[k].IsBool()) ? o[k].GetBool() : d;
        }

        std::string getStr(const Value &o, const char *k, const std::string &d) {
            return (o.HasMember(k) && o[k].IsString()) ? std::string(o[k].GetString()) : d;
        }

        ParamValue toParamValue(const Value &v) {
            if (v.IsBool()) return ParamValue(v.GetBool());
            if (v.IsInt()) return ParamValue(v.GetInt());
            if (v.IsNumber()) return ParamValue(v.GetDouble());
            if (v.IsString()) return ParamValue(std::string(v.GetString()));
            return ParamValue{};
        }

        GraphDef parseGraph(const Value &g) {
            GraphDef gd;

            if (g.HasMember("nodes") && g["nodes"].IsArray()) {
                for (const Value &n: g["nodes"].GetArray()) {
                    NodeDef nd;
                    nd.name = getStr(n, "name", "");
                    nd.type = getStr(n, "type", "");
                    if (n.HasMember("params") && n["params"].IsObject()) {
                        for (const auto &m: n["params"].GetObject())
                            nd.params.emplace_back(m.name.GetString(), toParamValue(m.value));
                    }
                    gd.nodes.push_back(std::move(nd));
                }
            }

            if (g.HasMember("connections") && g["connections"].IsArray()) {
                for (const Value &c: g["connections"].GetArray()) {
                    ConnectionDef cd;
                    cd.from = getStr(c, "from", "");
                    cd.outPort = getInt(c, "out", 0);
                    cd.to = getStr(c, "to", "");
                    cd.inPort = getInt(c, "in", 0);
                    cd.delayed = getBool(c, "delayed", false);
                    gd.connections.push_back(std::move(cd));
                }
            }

            if (g.HasMember("outputs") && g["outputs"].IsArray()) {
                for (const Value &o: g["outputs"].GetArray()) {
                    OutputDef od;
                    od.node = getStr(o, "node", "");
                    od.port = getInt(o, "port", 0);
                    gd.outputs.push_back(std::move(od));
                }
            }

            return gd;
        }

        SynthDef parseSynth(const Value &s) {
            SynthDef sd;
            sd.name = getStr(s, "name", "");
            sd.maxVoices = getInt(s, "maxVoices", 16);
            if (s.HasMember("params") && s["params"].IsObject()) {
                const Value &p = s["params"];
                sd.attack = getDouble(p, "attack", 0.0);
                sd.decay = getDouble(p, "decay", 0.0);
                sd.sustain = getDouble(p, "sustain", 0.0);
                sd.release = getDouble(p, "release", 0.0);
                sd.velocitySensitivity = getBool(p, "velocitySensitivity", false);
            }
            if (s.HasMember("voiceGraph") && s["voiceGraph"].IsObject())
                sd.voiceGraph = parseGraph(s["voiceGraph"]);
            return sd;
        }

        SendDef parseSend(const Value &s) {
            SendDef sd;
            sd.name = getStr(s, "name", "");
            sd.target = getStr(s, "target", "");
            sd.tap = getStr(s, "tap", "post");
            sd.gain = getDouble(s, "gain", 1.0);
            return sd;
        }

        ChannelDef parseChannel(const Value &c) {
            ChannelDef cd;
            cd.name = getStr(c, "name", "");
            cd.midiChannel = getInt(c, "midiChannel", 0);
            cd.gain = getDouble(c, "gain", 1.0);
            cd.pan = getDouble(c, "pan", 0.0);
            cd.mute = getBool(c, "mute", false);
            cd.solo = getBool(c, "solo", false);
            cd.routeTo = getStr(c, "routeTo", "master");
            if (c.HasMember("synth") && c["synth"].IsObject())
                cd.synth = parseSynth(c["synth"]);
            if (c.HasMember("postFx") && c["postFx"].IsObject())
                cd.postFx = parseGraph(c["postFx"]);
            if (c.HasMember("sends") && c["sends"].IsArray())
                for (const Value &s: c["sends"].GetArray()) cd.sends.push_back(parseSend(s));
            return cd;
        }

        BusDef parseBus(const Value &b) {
            BusDef bd;
            bd.name = getStr(b, "name", "");
            bd.type = getStr(b, "type", "sum");
            bd.gain = getDouble(b, "gain", 1.0);
            bd.output = getStr(b, "output", "master");
            if (b.HasMember("postFx") && b["postFx"].IsObject())
                bd.postFx = parseGraph(b["postFx"]);
            return bd;
        }
    }

    EngineDef parseEngineDef(const std::string &json) {
        rapidjson::Document doc;
        doc.Parse(json.c_str());

        if (doc.HasParseError())
            throw std::runtime_error(std::string("parseEngineDef: JSON parse error: ") +
                                     rapidjson::GetParseError_En(doc.GetParseError()));
        if (!doc.IsObject())
            throw std::runtime_error("parseEngineDef: root is not an object");

        EngineDef def;
        def.sampleRate = getInt(doc, "sampleRate", 44100);
        def.maxChannels = getInt(doc, "maxChannels", 16);
        def.maxBuses = getInt(doc, "maxBuses", 8);

        if (doc.HasMember("master") && doc["master"].IsObject()) {
            const Value &m = doc["master"];
            def.master.name = getStr(m, "name", "master");
            def.master.gain = getDouble(m, "gain", 1.0);
            if (m.HasMember("postFx") && m["postFx"].IsObject())
                def.master.postFx = parseGraph(m["postFx"]);
        }

        if (doc.HasMember("transport") && doc["transport"].IsObject()) {
            const Value &t = doc["transport"];
            def.transport.tempo = getDouble(t, "tempo", 120.0);
            def.transport.timeSigNum = getInt(t, "timeSigNum", 4);
            def.transport.timeSigDen = getInt(t, "timeSigDen", 4);
        }

        if (doc.HasMember("buses") && doc["buses"].IsArray())
            for (const Value &b: doc["buses"].GetArray()) def.buses.push_back(parseBus(b));

        if (doc.HasMember("channels") && doc["channels"].IsArray())
            for (const Value &c: doc["channels"].GetArray()) def.channels.push_back(parseChannel(c));

        return def;
    }
}
