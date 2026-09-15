#include "ExampleEngine.h"

#include <string>

namespace Audio::Engine {
    std::string buildExampleEngineJson(int index, bool useVelocity, bool channelPostFx, int sampleRate) {
        const bool sine = (index == 1);
        const std::string wave = sine ? "sine" : "saw";
        const double attack = sine ? 0.02 : 0.01;
        const double decay = sine ? 0.06 : 0.03;
        const double sustain = sine ? 0.5 : 0.4;
        const double release = sine ? 0.25 : 0.1;
        const int capacity = sampleRate;
        const int delay = sampleRate * 3 / 8;

        const std::string velFlag = useVelocity ? "true" : "false";

        std::string postFx;
        if (channelPostFx) {
            postFx = std::string(",\n      \"postFx\": {\n") +
                     "        \"nodes\": [\n" +
                     "          { \"name\": \"channel0.fx.in\", \"type\": \"input\" },\n" +
                     "          { \"name\": \"channel0.fx.delay\", \"type\": \"delay\", \"params\": { \"capacity\": " +
                     std::to_string(capacity) + ", \"delay\": " + std::to_string(delay) +
                     ", \"feedback\": 0.55, \"wet\": 0.6, \"dry\": 1.0 } },\n" +
                     "          { \"name\": \"channel0.fx.filter\", \"type\": \"filter\", \"params\": { \"cutoff\": 1200.0, \"mode\": \"lowpass\" } }\n" +
                     "        ],\n" +
                     "        \"connections\": [\n" +
                     "          { \"from\": \"channel0.fx.in\", \"out\": 0, \"to\": \"channel0.fx.delay\", \"in\": 0 },\n" +
                     "          { \"from\": \"channel0.fx.delay\", \"out\": 0, \"to\": \"channel0.fx.filter\", \"in\": 0 }\n" +
                     "        ],\n" +
                     "        \"outputs\": [ { \"node\": \"channel0.fx.filter\", \"port\": 0 } ]\n" +
                     "      }";
        }

        std::string json = std::string("{\n") +
            "  \"sampleRate\": " + std::to_string(sampleRate) + ",\n" +
            "  \"maxChannels\": 16,\n" +
            "  \"maxBuses\": 8,\n" +
            "  \"master\": { \"gain\": 1.0 },\n" +
            "  \"transport\": { \"tempo\": 120.0 },\n" +
            "  \"channels\": [\n" +
            "    {\n" +
            "      \"name\": \"channel0\",\n" +
            "      \"midiChannel\": 0,\n" +
            "      \"gain\": 0.5,\n" +
            "      \"pan\": 0.0,\n" +
            "      \"synth\": {\n" +
            "        \"name\": \"synth0\",\n" +
            "        \"maxVoices\": 16,\n" +
            "        \"params\": { \"attack\": " + std::to_string(attack) + ", \"decay\": " + std::to_string(decay) +
            ", \"sustain\": " + std::to_string(sustain) + ", \"release\": " + std::to_string(release) +
            ", \"velocitySensitivity\": " + velFlag + " },\n" +
            "        \"voiceGraph\": {\n" +
            "          \"nodes\": [\n" +
            "            { \"name\": \"synth0.osc\", \"type\": \"oscillator\", \"params\": { \"waveform\": \"" + wave + "\" } },\n" +
            "            { \"name\": \"synth0.adsr\", \"type\": \"adsr\" },\n" +
            "            { \"name\": \"synth0.vca\", \"type\": \"vca\" }\n" +
            "          ],\n" +
            "          \"connections\": [\n" +
            "            { \"from\": \"synth0.osc\", \"out\": 0, \"to\": \"synth0.vca\", \"in\": 0 },\n" +
            "            { \"from\": \"synth0.adsr\", \"out\": 0, \"to\": \"synth0.vca\", \"in\": 1 }\n" +
            "          ],\n" +
            "          \"outputs\": [ { \"node\": \"synth0.vca\", \"port\": 0 } ]\n" +
            "        }\n" +
            "      }" + postFx + "\n" +
            "    }\n" +
            "  ]\n" +
            "}\n";

        return json;
    }

    std::string buildSendDemoEngineJson(int sampleRate) {
        const int capacity = sampleRate;
        const int delay = sampleRate / 4;

        std::string json = std::string("{\n") +
            "  \"sampleRate\": " + std::to_string(sampleRate) + ",\n" +
            "  \"maxChannels\": 16,\n" +
            "  \"maxBuses\": 8,\n" +
            "  \"master\": { \"gain\": 1.0 },\n" +
            "  \"transport\": { \"tempo\": 120.0 },\n" +
            "  \"buses\": [\n" +
            "    {\n" +
            "      \"name\": \"fxBus\",\n" +
            "      \"type\": \"sum\",\n" +
            "      \"gain\": 0.8,\n" +
            "      \"output\": \"master\",\n" +
            "      \"postFx\": {\n" +
            "        \"nodes\": [\n" +
            "          { \"name\": \"fxBus.in\", \"type\": \"input\" },\n" +
            "          { \"name\": \"fxBus.delay\", \"type\": \"delay\", \"params\": { \"capacity\": " +
            std::to_string(capacity) + ", \"delay\": " + std::to_string(delay) +
            ", \"feedback\": 0.5, \"wet\": 1.0, \"dry\": 0.0 } },\n" +
            "          { \"name\": \"fxBus.filter\", \"type\": \"filter\", \"params\": { \"cutoff\": 2000.0, \"mode\": \"lowpass\" } }\n" +
            "        ],\n" +
            "        \"connections\": [\n" +
            "          { \"from\": \"fxBus.in\", \"out\": 0, \"to\": \"fxBus.delay\", \"in\": 0 },\n" +
            "          { \"from\": \"fxBus.delay\", \"out\": 0, \"to\": \"fxBus.filter\", \"in\": 0 }\n" +
            "        ],\n" +
            "        \"outputs\": [ { \"node\": \"fxBus.filter\", \"port\": 0 } ]\n" +
            "      }\n" +
            "    }\n" +
            "  ],\n" +
            "  \"channels\": [\n" +
            "    {\n" +
            "      \"name\": \"channel0\",\n" +
            "      \"midiChannel\": 0,\n" +
            "      \"gain\": 0.5,\n" +
            "      \"pan\": 0.0,\n" +
            "      \"routeTo\": \"master\",\n" +
            "      \"synth\": {\n" +
            "        \"name\": \"synth0\",\n" +
            "        \"maxVoices\": 16,\n" +
            "        \"params\": { \"attack\": 0.010000, \"decay\": 0.030000, \"sustain\": 0.400000, \"release\": 0.100000, \"velocitySensitivity\": false },\n" +
            "        \"voiceGraph\": {\n" +
            "          \"nodes\": [\n" +
            "            { \"name\": \"synth0.osc\", \"type\": \"oscillator\", \"params\": { \"waveform\": \"saw\" } },\n" +
            "            { \"name\": \"synth0.adsr\", \"type\": \"adsr\" },\n" +
            "            { \"name\": \"synth0.vca\", \"type\": \"vca\" }\n" +
            "          ],\n" +
            "          \"connections\": [\n" +
            "            { \"from\": \"synth0.osc\", \"out\": 0, \"to\": \"synth0.vca\", \"in\": 0 },\n" +
            "            { \"from\": \"synth0.adsr\", \"out\": 0, \"to\": \"synth0.vca\", \"in\": 1 }\n" +
            "          ],\n" +
            "          \"outputs\": [ { \"node\": \"synth0.vca\", \"port\": 0 } ]\n" +
            "        }\n" +
            "      },\n" +
            "      \"sends\": [\n" +
            "        { \"name\": \"echoSend\", \"target\": \"fxBus\", \"tap\": \"post\", \"gain\": 0.6 }\n" +
            "      ]\n" +
            "    }\n" +
            "  ]\n" +
            "}\n";

        return json;
    }
}

