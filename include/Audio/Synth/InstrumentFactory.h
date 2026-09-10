#ifndef AUDIO_SYNTH_INSTRUMENTFACTORY_H
#define AUDIO_SYNTH_INSTRUMENTFACTORY_H

#include "SignalGraph.h"

namespace Audio::Synth {
    struct InstrumentSpec {
        Processing::SignalGraph voiceGraph;
        int maxVoices = 16;
        int attackDur = 0;
        int decayDur = 0;
        int releaseDur = 0;
    };

    InstrumentSpec buildInstrument(int index, int sampleRate);

    Processing::SignalGraph buildExamplePostFx(int sampleRate);
}

#endif
