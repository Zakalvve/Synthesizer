#ifndef AUDIO_ENGINE_ENGINECOMPILER_H
#define AUDIO_ENGINE_ENGINECOMPILER_H

#include "EngineDef.h"

namespace Audio::Engine {
    class AudioEngine;

    class EngineCompiler {
    public:
        static void build(AudioEngine &engine, const EngineDef &def);
    };
}

#endif
