#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <memory>
#include <string>
#include <filesystem>

#include "Node.h"
#include "PipelinesUtils.h"
#include "Processor.h"

#include "WAVFile.h"
#include "AudioSample.h"
#include "MidiRoll.h"
#include "MidiJsonFile.h"
#include "MidiRollTrack.h"
#include "MidiRollEvent.h"
#include "Synthesizer.h"

using namespace std;
using namespace Audio::Processing::Pipelines;

const int sample_rate = 44100;
const int bits_per_sample = 32;
const int max_amplitude = (1u << (bits_per_sample - 1)) - 1;

class AddOne : public Processor<double, double> {
    double process(const double& x) override { return x + 1; }
};

class MultiplyByTwo : public Processor<double, double> {
    double process(const double& x) override { return x * 2; }
};

class MinusOne : public Processor<double, double> {
    double process(const double& x) override { return x - 1; }
};

class HalfInput : public Processor<double, double> {
    double process(const double& x) override { return x / 2.0; }
};

class ToInteger : public Processor<double, int> {
    int process(const double& x) override { return static_cast<int>(x); }
};

template<typename T>
class PassThrough : public Processor<T, T> {
    T process(const T& x) override { return x; }
};

class ConditionalDouble : public Processor<std::tuple<bool, int>, int> {
public:
    int process(const std::tuple<bool, int>& input) override {
        bool condition = std::get<0>(input);
        int value = std::get<1>(input);
        return condition ? value * 2 : value;
    }
};

class AverageProcessor : public Processor<std::vector<double>, double> {
public:
    double process(const std::vector<double>& values) override {
        if (values.empty()) return 0.0;
        double sum = 0.0;
        for (double v : values) sum += v;
        return sum / values.size();
    }
};

// === First: exercise the generic DAG/pipeline machinery ===
void runPipelineTests() {
    try {
        // === First test: simple pipeline arithmatic ===
        Node<double, double> start(std::make_unique<AddOne>());
        Node<double, double> second(std::make_unique<MultiplyByTwo>());
        Node<double, double> third(std::make_unique<MultiplyByTwo>());
        Node<double, double> fourth(std::make_unique<MinusOne>());
        Node<double, double> fifth(std::make_unique<HalfInput>());
        Node<double, int>    sixth(std::make_unique<ToInteger>());

        // Define ports
        start.addInput<double>("in");
        start.addOutput<double>("out");

        second.addInput<double>("in");
        second.addOutput<double>("out");

        third.addInput<double>("in");
        third.addOutput<double>("out");

        fourth.addInput<double>("in");
        fourth.addOutput<double>("out");

        fifth.addInput<double>("in");
        fifth.addOutput<double>("out");

        sixth.addInput<double>("in");
        sixth.addOutput<int>("out");

        // Connect the pipeline
        start.connectOutputTo<double>("out", second, "in");
        second.connectOutputTo<double>("out", third, "in");
        third.connectOutputTo<double>("out", fourth, "in");
        fourth.connectOutputTo<double>("out", fifth, "in");
        fifth.connectOutputTo<double>("out", sixth, "in");

        // Feed initial value
        start.feedInput<double>("in", 1.0);

        // Read final output
        int result = sixth.getOutputValue<int>("out");
        std::cout << "Final result: " << result << std::endl; // Should print 3

        std::cout << std::endl;

        // === Second test: conditional double based on bool ===
        Node<std::tuple<bool, int>, int> logicNode(std::make_unique<ConditionalDouble>());
        logicNode.addInput<bool>("flag");    // instead of "flag"
        logicNode.addInput<int>("number");     // instead of "number"
        logicNode.addOutput<int>("out");

        // Feed inputs
        logicNode.feedInput<int>("number", 7);
        logicNode.feedInput<bool>("flag", false);

        // Read result
        int logicResult = logicNode.getOutputValue<int>("out");
        std::cout << "Conditional logic result (should be 7): " << logicResult << std::endl;

        // === Third test: multi input functionality ===
        Node<std::vector<double>, double> avgNode(std::make_unique<AverageProcessor>());
        avgNode.inputs().addMultiPort<double>("in");
        avgNode.addOutput<double>("out");

        // Create 3 input nodes
        Node<double, double> input1(nullptr);
        input1.addOutput<double>("out");

        Node<double, double> input2(nullptr);
        input2.addOutput<double>("out");

        Node<double, double> input3(nullptr);
        input3.addOutput<double>("out");

        // Connect each output to the avgNode multiport input
        auto* multiPort = avgNode.inputs().getMultiPort<double>("in");
        input1.outputs().getPort<double>("out")->connect(multiPort);
        multiPort->registerInput();

        input2.outputs().getPort<double>("out")->connect(multiPort);
        multiPort->registerInput();

        input3.outputs().getPort<double>("out")->connect(multiPort);
        multiPort->registerInput();

        // Feed inputs
        input1.outputs().getPort<double>("out")->consume(4.0);
        input2.outputs().getPort<double>("out")->consume(8.0);
        input3.outputs().getPort<double>("out")->consume(6.0);

        // Read and print result
        double result2 = avgNode.getOutputValue<double>("out");
        std::cout << "Average result (should be 6): " << result2 << std::endl;

    }
    catch (const std::exception& ex) {
        std::cerr << "[EXCEPTION] " << ex.what() << std::endl;
    }
}

// === Second: synthesize a midi-roll to a WAV file (current procedural path) ===
void synthesizeMidiRoll() {
    std::string filePath = "../data/moonlight-sonata.json";
    Audio::Midi::MidiJsonFile mj;

    if (mj.load(filePath.c_str(), sample_rate)) {
        Audio::Midi::MidiRoll& roll = *(mj.getRoll());
        Audio::WAVFile wav(true, sample_rate, bits_per_sample, max_amplitude);

        Audio::Synthesizer synth(sample_rate, 1.0);

        wav.open("new-test-output.wav");

        int t = 0;

        while(roll.isRolling() || synth.isPlaying()) {

            std::vector<std::unique_ptr<Audio::Midi::MidiRollEvent>> midiEvents = roll.tickRoll(t);

            for(auto& event: midiEvents){
                // Uses a visitor pattern
                event->process(synth);
            }

            Audio::AudioSample steroSample = synth.sample();
            Audio::AudioSample normalizedSample = steroSample * (static_cast<double>(max_amplitude) * 1);
            Audio::AudioSample clampedSample = normalizedSample.clamp(-max_amplitude, max_amplitude);

            wav.sample(clampedSample);

            ++t;
        }

        wav.close();
    }
}

int main() {
    runPipelineTests();
    synthesizeMidiRoll();
    return 0;
}
