#ifndef AUDIO_SYNTH_GRAPHCOMPILER_H
#define AUDIO_SYNTH_GRAPHCOMPILER_H

#include <string>
#include <utility>
#include <vector>

#include "AudioPipeline.h"
#include "SignalProcessor.h"
#include "EngineDef.h"

namespace Audio::Synth {
    struct CompiledGraph {
        Audio::Processing::AudioPipeline pipeline;
        std::vector<std::pair<std::string, Audio::Processing::SignalProcessor *> > nodes;

        Audio::Processing::SignalProcessor *find(const std::string &name) const {
            for (const auto &n: nodes) if (n.first == name) return n.second;
            return nullptr;
        }
    };

    CompiledGraph compileGraph(const GraphDef &def, int maxInstances, int blockSize, int sampleRate);
}

#endif
