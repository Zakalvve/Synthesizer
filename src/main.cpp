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
#include "MidiRoll.h"
#include "MidiJsonFile.h"
#include "MidiRollTrack.h"
#include "MidiRollEvent.h"

#include "InstrumentConfig.h"
#include "PipelineSynthesizer.h"

using namespace std;
using namespace Audio::Processing;

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

void renderRoll(const std::string& outputName, Audio::Synth::PipelineSynthesizer& synth) {
    std::string filePath = "../data/moonlight-sonata.json";
    Audio::Midi::MidiJsonFile mj;
    if (!mj.load(filePath.c_str(), sample_rate)) return;

    Audio::Midi::MidiRoll& roll = *(mj.getRoll());
    Audio::WAVFile wav(true, sample_rate, bits_per_sample, max_amplitude);
    wav.open(outputName);

    int t = 0;
    while (roll.isRolling() || synth.isPlaying()) {
        std::vector<std::unique_ptr<Audio::Midi::MidiRollEvent>> midiEvents = roll.tickRoll(t);
        for (auto& event : midiEvents) event->process(synth);

        Audio::AudioSample stereoSample = synth.sample();
        Audio::AudioSample normalizedSample = stereoSample * (static_cast<double>(max_amplitude) * 1);
        Audio::AudioSample clampedSample = normalizedSample.clamp(-max_amplitude, max_amplitude);

        wav.sample(clampedSample);
        ++t;
    }
    wav.close();
}

int main() {
    runGraphSmokeTest();

    Audio::Synth::PipelineSynthesizer synth(sample_rate, 1.0);
    renderRoll("pipeline-output.wav", synth);

    Audio::Synth::InstrumentConfig altered;
    altered.sampleRate = sample_rate;
    altered.oscType    = Audio::Synth::OscType::PhaseSine;
    altered.glideTime  = 0.02;
    Audio::Synth::PipelineSynthesizer alteredSynth(sample_rate, 1.0, altered,  true,  true);
    renderRoll("altered-output.wav", alteredSynth);
    std::cout << "[RENDER] pipeline-output.wav (default) and altered-output.wav (sine + glide + delay + low-pass + velocity)" << std::endl;

    return 0;
}
