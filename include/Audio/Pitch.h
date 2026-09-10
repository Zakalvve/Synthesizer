#ifndef AUDIO_PITCH_H
#define AUDIO_PITCH_H

#include <cmath>
#include <string>

#include "Constants.h"

namespace Audio {
    inline double noteToFrequency(const std::string &note) {
        return MusicalConstants::C4_FREQUENCY
               * std::pow(2.0, MusicalConstants::noteToSemitone.at(note) / 12.0);
    }
}

#endif
