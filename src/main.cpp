#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <memory>
#include <string>
#include <filesystem>

#include "Signal.h"
#include "AudioContext.h"
#include "SignalProcessor.h"
#include "SignalGraph.h"
#include "AudioPipeline.h"

#include "WAVFile.h"
#include "AudioSample.h"
#include "MidiChannelOnMessage.h"
#include "MidiNoteMessage.h"

#include "AudioEngine.h"
#include "EngineDefParser.h"
#include "ExampleEngine.h"

using namespace std;
using namespace Audio::Dsp;

const int sample_rate = 44100;
const int bits_per_sample = 32;
const int max_amplitude = (1u << (bits_per_sample - 1)) - 1;


class ConstProcessor : public SignalProcessor {
    double _value;
public:
    explicit ConstProcessor(double value) : _value(value) {}
    void process(AudioContext& ctx) const override {
        Signal* out = ctx.output(0);
        for (int f = 0; f < ctx.blockSize; ++f) out[f] = _value;
    }
    int inputCount()  const override { return 0; }
    int outputCount() const override { return 1; }
};

class GainProcessor : public SignalProcessor {
    double _gain;
public:
    explicit GainProcessor(double gain) : _gain(gain) {}
    void process(AudioContext& ctx) const override {
        const Signal* in = ctx.input(0);
        Signal* out = ctx.output(0);
        for (int f = 0; f < ctx.blockSize; ++f) out[f] = in[f] * _gain;
    }
    int inputCount()  const override { return 1; }
    int outputCount() const override { return 1; }
};

class SumProcessor : public SignalProcessor {
    int _inputs;
public:
    explicit SumProcessor(int inputs) : _inputs(inputs) {}
    void process(AudioContext& ctx) const override {
        Signal* out = ctx.output(0);
        for (int f = 0; f < ctx.blockSize; ++f) {
            double sum = 0.0;
            for (int i = 0; i < _inputs; ++i) sum += ctx.input(i)[f];
            out[f] = sum;
        }
    }
    int inputCount()  const override { return _inputs; }
    int outputCount() const override { return 1; }
};

class CounterProcessor : public SignalProcessor {
public:
    void process(AudioContext& ctx) const override {
        double* st = ctx.state();
        Signal* out = ctx.output(0);
        for (int f = 0; f < ctx.blockSize; ++f) out[f] = st[0]++;
    }
    int inputCount()  const override { return 0; }
    int outputCount() const override { return 1; }
    std::size_t stateSize() const override { return 1; }
    void resetState(double* st) const override { st[0] = 0.0; }
};

void runGraphSmokeTest() {
    SignalGraph graph;
    auto c2    = graph.add(std::make_unique<ConstProcessor>(2.0));
    auto gain  = graph.add(std::make_unique<GainProcessor>(3.0));
    auto c10   = graph.add(std::make_unique<ConstProcessor>(10.0));
    auto count = graph.add(std::make_unique<CounterProcessor>());
    auto sum   = graph.add(std::make_unique<SumProcessor>(3));

    graph.connect(c2,   0, gain, 0);
    graph.connect(gain, 0, sum,  0);
    graph.connect(c10,  0, sum,  1);
    graph.connect(count,0, sum,  2);
    graph.setOutput(sum, 0);

    AudioPipeline pipeline = graph.compile( 2,  1);

    AudioContext ctx;
    const double i0a = pipeline.renderInstance(0, ctx)[0];
    const double i0b = pipeline.renderInstance(0, ctx)[0];
    const double i1a = pipeline.renderInstance(1, ctx)[0];
    const double i0c = pipeline.renderInstance(0, ctx)[0];
    const double i1b = pipeline.renderInstance(1, ctx)[0];

    bool pass = true;
    auto check = [&](const char* label, double got, double want) {
        const bool ok = (got == want);
        pass = pass && ok;
        std::cout << "  [" << (ok ? "ok" : "XX") << "] " << label
                  << " = " << got << " (want " << want << ")\n";
    };
    std::cout << "[SMOKE TEST] audio graph core\n";
    check("chain + fan-in, instance0 #1", i0a, 16);
    check("state persists,  instance0 #2", i0b, 17);
    check("instance isolated, instance1 #1", i1a, 16);
    check("state persists,  instance0 #3", i0c, 18);
    check("instance isolated, instance1 #2", i1b, 17);
    std::cout << "[SMOKE TEST] " << (pass ? "PASS" : "FAIL") << std::endl;
}

static double renderEnergy(Audio::Engine::AudioEngine& engine, int samples) {
    engine.play();
    Audio::Midi::MidiChannelOnMessage on(0, "meta", "channelOn", 0);
    engine.visit(on);
    Audio::Midi::MidiNoteMessage key(0, "channel", "keyOn", 0, "C4", 100);
    engine.visit(key);

    double acc = 0.0;
    for (int i = 0; i < samples; ++i) acc += std::abs(engine.sample().c1);
    return acc;
}

void runFacadeTests() {
    using namespace Audio::Engine;

    bool pass = true;
    auto check = [&](const char* label, bool ok) {
        pass = pass && ok;
        std::cout << "  [" << (ok ? "ok" : "XX") << "] " << label << "\n";
    };

    std::cout << "[FACADE TEST] json-defined engine\n";

    AudioEngine engine(parseEngineDef(buildExampleEngineJson(0, false, false, sample_rate)));

    const Handle hSynth = engine.handle("synth0");
    const Handle hOsc = engine.handle("synth0.osc");
    const Handle hChan = engine.handle("channel0");
    check("handle('synth0') resolves", hSynth != InvalidHandle);
    check("handle('synth0.osc') resolves", hOsc != InvalidHandle);
    check("handle('channel0') resolves", hChan != InvalidHandle);
    check("handle('missing') is invalid", engine.handle("missing") == InvalidHandle);

    check("channel gain default 0.5", engine.get(hChan, "gain").asDouble() == 0.5);
    engine.set(hChan, "gain", 0.25);
    check("channel gain set to 0.25", engine.get(hChan, "gain").asDouble() == 0.25);

    engine.set("synth0.osc", "waveform", std::string("sine"));
    check("osc waveform set to sine", engine.get(hOsc, "waveform").asString() == "sine");

    engine.set("synth0", "attack", 0.5);
    check("synth attack set", std::abs(engine.get(hSynth, "attack").asDouble() - 0.5) < 1e-12);
    check("adsr node attack synced from synth", std::abs(engine.get("synth0.adsr", "attack").asDouble() - 0.5) < 1e-12);

    // set() changes rendered output: louder gain -> more energy.
    {
        AudioEngine quiet(parseEngineDef(buildExampleEngineJson(0, false, false, sample_rate)));
        AudioEngine loud(parseEngineDef(buildExampleEngineJson(0, false, false, sample_rate)));
        quiet.set("channel0", "gain", 0.2);
        loud.set("channel0", "gain", 0.8);
        const double eQuiet = renderEnergy(quiet, 3000);
        const double eLoud = renderEnergy(loud, 3000);
        check("output non-silent", eQuiet > 0.0 && eLoud > 0.0);
        check("higher gain -> more energy", eLoud > eQuiet);
    }

    // Recompile (stopped): structural change keeps handles stable and preserves settings.
    {
        AudioEngine e(parseEngineDef(buildExampleEngineJson(0, false, false, sample_rate)));
        const Handle hc = e.handle("channel0");
        const Handle ho = e.handle("synth0.osc");
        e.set("channel0", "gain", 0.33);
        e.compile(parseEngineDef(buildExampleEngineJson(0, true, true, sample_rate)));  // adds post-fx nodes
        check("handle('channel0') stable across recompile", e.handle("channel0") == hc);
        check("handle('synth0.osc') stable across recompile", e.handle("synth0.osc") == ho);
        check("channel gain preserved across recompile", e.get("channel0", "gain").asDouble() == 0.33);
        check("newly added node gets a handle", e.handle("channel0.fx.delay") != InvalidHandle);
    }

    // Send exposes a gain slider addressable by name.
    {
        AudioEngine e(parseEngineDef(buildSendDemoEngineJson(sample_rate)));
        const Handle hs = e.handle("echoSend");
        check("send handle resolves", hs != InvalidHandle);
        check("send gain default 0.6", std::abs(e.get(hs, "gain").asDouble() - 0.6) < 1e-12);
        e.set(hs, "gain", 0.3);
        check("send gain set to 0.3", std::abs(e.get(hs, "gain").asDouble() - 0.3) < 1e-12);
    }

    // compile() is rejected while playing.
    {
        AudioEngine e(parseEngineDef(buildExampleEngineJson(0, false, false, sample_rate)));
        e.play();
        bool threw = false;
        try {
            e.compile(parseEngineDef(buildExampleEngineJson(0, false, false, sample_rate)));
        } catch (const std::exception&) {
            threw = true;
        }
        check("compile() throws while playing", threw);
    }

    // Transport musical position.
    {
        AudioEngine e(parseEngineDef(buildExampleEngineJson(0, false, false, sample_rate)));
        e.set("transport", "tempo", 120.0);
        e.set("transport", "samplePosition", sample_rate);  // one second
        check("transport positionSeconds", std::abs(e.get("transport", "positionSeconds").asDouble() - 1.0) < 1e-9);
        check("transport beat (120bpm, 1s -> 2)", std::abs(e.get("transport", "beat").asDouble() - 2.0) < 1e-9);
    }

    std::cout << "[FACADE TEST] " << (pass ? "PASS" : "FAIL") << std::endl;
}

static double midiEnergy(Audio::Engine::AudioEngine& engine, int frames) {
    engine.rewind();
    engine.play();
    std::vector<Audio::AudioSample> buf(frames, Audio::AudioSample(0.0, 0.0));
    const int n = engine.render(buf.data(), frames);
    double acc = 0.0;
    for (int i = 0; i < n; ++i) acc += std::abs(buf[i].c1);
    engine.stop();
    return acc;
}

void runMidiRoutingTests() {
    using namespace Audio::Engine;
    using namespace Audio::Engine::Midi;

    bool pass = true;
    auto check = [&](const char* label, bool ok) {
        pass = pass && ok;
        std::cout << "  [" << (ok ? "ok" : "XX") << "] " << label << "\n";
    };

    std::cout << "[MIDI TEST] router + sources\n";

    const std::string midi = "../data/moonlight-sonata.json";

    // exportWav() contract
    {
        AudioEngine e(parseEngineDef(buildExampleEngineJson(0, false, false, sample_rate)));
        check("exportWav() rejects non-.wav path", e.exportWav("nope.mp3") == false);
        check("exportWav() accepts .wav path", e.exportWav("../output/_miditest.wav") == true);
    }

    // A clip offset delays the source's events.
    {
        AudioEngine a(parseEngineDef(buildExampleEngineJson(0, false, false, sample_rate)));
        const SourceId sa = a.addMidiFile(midi);
        a.midiRouter().addClip(sa, 0, a.handle("channel0"), 0);
        const double e0 = midiEnergy(a, sample_rate);

        AudioEngine b(parseEngineDef(buildExampleEngineJson(0, false, false, sample_rate)));
        const SourceId sb = b.addMidiFile(midi);
        b.midiRouter().addClip(sb, 0, b.handle("channel0"), 2L * sample_rate);  // +2s
        const double eDelayed = midiEnergy(b, sample_rate);

        check("midi file loads", sa != InvalidSource && sb != InvalidSource);
        check("offset 0 sounds in first second", e0 > 0.0);
        check("offset +2s is silent in first second", eDelayed == 0.0);
    }

    // A clip on a channel the source doesn't contain produces nothing.
    {
        AudioEngine e(parseEngineDef(buildExampleEngineJson(0, false, false, sample_rate)));
        const SourceId s = e.addMidiFile(midi);
        e.midiRouter().addClip(s, 5, e.handle("channel0"), 0);  // file has no channel 5
        check("empty source channel is silent", midiEnergy(e, sample_rate) == 0.0);
    }

    // Routing survives a (stopped) recompile.
    {
        AudioEngine e(parseEngineDef(buildExampleEngineJson(0, false, false, sample_rate)));
        const SourceId s = e.addMidiFile(midi);
        const Handle hc = e.handle("channel0");
        e.midiRouter().addClip(s, 0, hc, 0);
        e.stop();
        e.compile(parseEngineDef(buildExampleEngineJson(0, true, true, sample_rate)));  // structural change
        check("channel handle stable after recompile", e.handle("channel0") == hc);
        check("routing still plays after recompile", midiEnergy(e, sample_rate) > 0.0);
    }

    std::cout << "[MIDI TEST] " << (pass ? "PASS" : "FAIL") << std::endl;
}

void renderConfig(Audio::Engine::AudioEngine& engine, const std::string& outputName) {
    Audio::Engine::Midi::SourceId src = engine.addMidiFile("../data/moonlight-sonata.json");
    engine.midiRouter().addClip(src, 0, engine.handle("channel0"), 0);
    engine.exportWav("../output/" + outputName);
}

int main() {
    runGraphSmokeTest();
    runFacadeTests();
    runMidiRoutingTests();

    {
        Audio::Engine::AudioEngine engine(Audio::Engine::parseEngineDef(
            Audio::Engine::buildExampleEngineJson(0, false, false, sample_rate)));
        renderConfig(engine, "synth0-saw.wav");
    }
    {
        Audio::Engine::AudioEngine engine(Audio::Engine::parseEngineDef(
            Audio::Engine::buildExampleEngineJson(1, false, false, sample_rate)));
        renderConfig(engine, "synth1-sine.wav");
    }
    {
        Audio::Engine::AudioEngine engine(Audio::Engine::parseEngineDef(
            Audio::Engine::buildExampleEngineJson(0, true, true, sample_rate)));
        renderConfig(engine, "synth0-saw-postfx.wav");
    }
    {
        Audio::Engine::AudioEngine engine(Audio::Engine::parseEngineDef(
            Audio::Engine::buildSendDemoEngineJson(sample_rate)));
        renderConfig(engine, "synth0-saw-send.wav");
    }

    std::cout << "[RENDER] synth0-saw.wav, synth1-sine.wav, synth0-saw-postfx.wav, synth0-saw-send.wav" << std::endl;

    return 0;
}
