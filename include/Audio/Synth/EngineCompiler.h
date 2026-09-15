#ifndef AUDIO_SYNTH_ENGINECOMPILER_H
#define AUDIO_SYNTH_ENGINECOMPILER_H

#include "EngineDef.h"

namespace Audio::Synth {
    class AudioEngine;

    class EngineCompiler {
    public:
        static void build(AudioEngine &engine, const EngineDef &def);
    };
}

#endif
