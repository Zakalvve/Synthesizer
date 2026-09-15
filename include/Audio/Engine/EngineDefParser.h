#ifndef AUDIO_ENGINE_ENGINEDEFPARSER_H
#define AUDIO_ENGINE_ENGINEDEFPARSER_H

#include <string>

#include "EngineDef.h"

namespace Audio::Engine {
    EngineDef parseEngineDef(const std::string &json);
}

#endif
