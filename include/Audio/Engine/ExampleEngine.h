#ifndef AUDIO_ENGINE_EXAMPLEENGINE_H
#define AUDIO_ENGINE_EXAMPLEENGINE_H

#include <string>

namespace Audio::Engine {
    std::string buildExampleEngineJson(int index, bool useVelocity, bool channelPostFx, int sampleRate);

    std::string buildSendDemoEngineJson(int sampleRate);
}

#endif
