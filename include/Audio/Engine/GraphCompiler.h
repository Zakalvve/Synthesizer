#ifndef AUDIO_ENGINE_GRAPHCOMPILER_H
#define AUDIO_ENGINE_GRAPHCOMPILER_H

#include <string>
#include <utility>
#include <vector>

#include "AudioPipeline.h"
#include "SignalProcessor.h"
#include "EngineDef.h"

namespace Audio::Engine {
    struct CompiledGraph {
        Audio::Dsp::AudioPipeline pipeline;
        std::vector<std::pair<std::string, Audio::Dsp::SignalProcessor *> > nodes;

        Audio::Dsp::SignalProcessor *find(const std::string &name) const {
            for (const auto &n: nodes) if (n.first == name) return n.second;
            return nullptr;
        }
    };

    CompiledGraph compileGraph(const GraphDef &def, int maxInstances, int blockSize, int sampleRate);
}

#endif
