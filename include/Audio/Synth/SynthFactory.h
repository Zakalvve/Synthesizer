#ifndef AUDIO_SYNTH_SYNTHFACTORY_H
#define AUDIO_SYNTH_SYNTHFACTORY_H

#include "SignalGraph.h"

namespace Audio::Synth {
    struct SynthSpec {
        Processing::SignalGraph voiceGraph;
        int maxVoices = 16;
        int attackDur = 0;
        int decayDur = 0;
        int releaseDur = 0;
    };

    SynthSpec buildSynth(int index, int sampleRate);

    Processing::SignalGraph buildExamplePostFx(int sampleRate);
}

#endif
