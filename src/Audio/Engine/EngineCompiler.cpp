#include "EngineCompiler.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AudioEngine.h"
#include "GraphCompiler.h"

namespace Audio::Engine {
    using namespace Audio::Dsp;

    void EngineCompiler::build(AudioEngine &e, const EngineDef &def) {
        e._sampleRate = def.sampleRate;

        e._transport = Transport{};
        e._transport.tempo = def.transport.tempo;
        e._transport.timeSigNum = def.transport.timeSigNum;
        e._transport.timeSigDen = def.transport.timeSigDen;

        e._master = Master{};
        e._master.gain = def.master.gain;

        e._transport.sampleRate = def.sampleRate;

        // Preserve handle values for names that survive this recompile.
        const std::unordered_map<std::string, Handle> prevNames = e._names;

        e._channels.clear();
        e._buses.clear();
        e._busOrder.clear();
        e._nodeTargets.clear();
        e._names.clear();
        e._targets.clear();
        e._handleToName.clear();
        e._midiToChannel.clear();
        e._channelByHandle.clear();
        // NOTE: _router (MIDI sources/clips/rolls) is intentionally NOT cleared — routing is set up
        // via the facade and survives recompiles (handles are stable).

        auto registerTarget = [&](const std::string &name, ParamTarget *t) -> Handle {
            Handle h;
            auto prev = prevNames.find(name);
            if (!name.empty() && prev != prevNames.end()) h = prev->second;
            else h = e._nextHandle++;
            e._targets[h] = t;
            if (!name.empty()) {
                e._names[name] = h;
                e._handleToName[h] = name;
            }
            return h;
        };
        auto registerNode = [&](const std::string &name, SignalProcessor *p) {
            e._nodeTargets.push_back(std::make_unique<NodeParamTarget>(p));
            registerTarget(name, e._nodeTargets.back().get());
        };

        registerTarget("transport", &e._transport);
        registerTarget(def.master.name, &e._master);

        if (!def.master.postFx.empty()) {
            CompiledGraph mp = compileGraph(def.master.postFx, 2, 1, def.sampleRate);
            e._master.postFx = std::move(mp.pipeline);
            e._master.hasPostFx = true;
            for (auto &n: mp.nodes) registerNode(n.first, n.second);
        }

        // Bus name -> slot index (resolve forward references before configuring).
        std::unordered_map<std::string, int> busIndex;
        for (int i = 0; i < static_cast<int>(def.buses.size()); ++i) {
            if (i >= def.maxBuses)
                throw std::runtime_error("EngineCompiler: more buses than maxBuses");
            busIndex[def.buses[i].name] = i;
        }

        auto resolveTarget = [&](const std::string &name) -> int {
            if (name.empty() || name == "master" || name == def.master.name) return -1;
            auto it = busIndex.find(name);
            if (it == busIndex.end())
                throw std::runtime_error("EngineCompiler: unknown routing target '" + name + "'");
            return it->second;
        };

        e._buses.reserve(static_cast<std::size_t>(def.maxBuses));
        for (int i = 0; i < def.maxBuses; ++i)
            e._buses.push_back(std::make_unique<Bus>());

        for (int i = 0; i < static_cast<int>(def.buses.size()); ++i) {
            const BusDef &bd = def.buses[i];
            Bus *bus = e._buses[i].get();
            bus->configure(Bus::parseType(bd.type), bd.gain, resolveTarget(bd.output));
            if (!bd.postFx.empty()) {
                CompiledGraph pf = compileGraph(bd.postFx, 2, 1, def.sampleRate);
                bus->setPostFx(std::move(pf.pipeline));
                for (auto &n: pf.nodes) registerNode(n.first, n.second);
            }
            registerTarget(bd.name, bus);
        }

        e._channels.reserve(static_cast<std::size_t>(def.maxChannels));
        for (int i = 0; i < def.maxChannels; ++i)
            e._channels.push_back(std::make_unique<Channel>());

        int slot = 0;
        for (const ChannelDef &cd: def.channels) {
            if (slot >= def.maxChannels)
                throw std::runtime_error("EngineCompiler: more channels than maxChannels");

            Channel *ch = e._channels[slot].get();
            ch->configure(cd.midiChannel, cd.gain, cd.pan, cd.mute, cd.solo);
            ch->setRouteTarget(resolveTarget(cd.routeTo));
            ch->activate();
            e._midiToChannel[cd.midiChannel] = slot;

            const SynthDef &sd = cd.synth;
            CompiledGraph vg = compileGraph(sd.voiceGraph, sd.maxVoices, 1, def.sampleRate);

            std::string envName;
            for (const NodeDef &n: sd.voiceGraph.nodes) {
                if (n.type == "adsr") {
                    envName = n.name;
                    break;
                }
            }
            SignalProcessor *env = envName.empty() ? nullptr : vg.find(envName);
            if (env) {
                env->setParam("attack", sd.attack);
                env->setParam("decay", sd.decay);
                env->setParam("sustain", sd.sustain);
                env->setParam("release", sd.release);
            }

            ch->synth().configure(std::move(vg.pipeline), sd.maxVoices, def.sampleRate,
                                  sd.attack, sd.decay, sd.sustain, sd.release,
                                  sd.velocitySensitivity, env);

            const Handle channelHandle = registerTarget(cd.name, ch);
            e._channelByHandle[channelHandle] = ch;
            registerTarget(sd.name, &ch->synth());
            for (auto &n: vg.nodes) registerNode(n.first, n.second);

            if (!cd.postFx.empty()) {
                CompiledGraph pf = compileGraph(cd.postFx, 1, 1, def.sampleRate);
                ch->setPostFx(std::move(pf.pipeline));
                for (auto &n: pf.nodes) registerNode(n.first, n.second);
            }

            for (const SendDef &snd: cd.sends) {
                const int target = resolveTarget(snd.target);
                if (target < 0)
                    throw std::runtime_error("EngineCompiler: send '" + snd.name + "' must target a bus");
                auto send = std::make_unique<Send>();
                send->configure(target, snd.tap == "pre", snd.gain);
                Send *raw = send.get();
                ch->addSend(std::move(send));
                registerTarget(snd.name, raw);
            }

            ++slot;
        }

        // Topological order of defined buses: a bus is processed before its output bus.
        {
            const int n = static_cast<int>(def.buses.size());
            std::vector<int> indeg(n, 0);
            for (int i = 0; i < n; ++i) {
                const int t = e._buses[i]->outputTarget();
                if (t >= 0 && t < n) ++indeg[t];
            }
            std::vector<int> ready;
            for (int i = 0; i < n; ++i) if (indeg[i] == 0) ready.push_back(i);
            while (!ready.empty()) {
                const int u = ready.back();
                ready.pop_back();
                e._busOrder.push_back(u);
                const int t = e._buses[u]->outputTarget();
                if (t >= 0 && t < n) if (--indeg[t] == 0) ready.push_back(t);
            }
            if (static_cast<int>(e._busOrder.size()) != n)
                throw std::runtime_error("EngineCompiler: bus routing has a cycle");
        }

        e._busAccum.assign(static_cast<std::size_t>(def.maxBuses), Audio::AudioSample(0.0, 0.0));
        e._busCount.assign(static_cast<std::size_t>(def.maxBuses), 0);

        // Re-apply live parameter settings for surviving names (structure comes from the
        // def; scalar settings applied via set() are preserved across recompiles).
        for (const auto &entry: e._overrides) {
            auto nameIt = e._names.find(entry.first);
            if (nameIt == e._names.end()) continue;
            auto targetIt = e._targets.find(nameIt->second);
            if (targetIt == e._targets.end()) continue;
            for (const auto &pv: entry.second)
                targetIt->second->setParam(pv.first, pv.second);
        }

        e._def = def;
    }
}
