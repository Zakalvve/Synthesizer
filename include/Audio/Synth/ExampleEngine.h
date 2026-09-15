#ifndef AUDIO_SYNTH_EXAMPLEENGINE_H
#define AUDIO_SYNTH_EXAMPLEENGINE_H

#include <string>

namespace Audio::Synth {
    std::string buildExampleEngineJson(int index, bool useVelocity, bool channelPostFx, int sampleRate);

    std::string buildSendDemoEngineJson(int sampleRate);
}

#endif
