#ifndef AUDIO_SYNTH_ENGINEDEFPARSER_H
#define AUDIO_SYNTH_ENGINEDEFPARSER_H

#include <string>

#include "EngineDef.h"

namespace Audio::Synth {
    EngineDef parseEngineDef(const std::string &json);
}

#endif
