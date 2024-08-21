#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <memory>
#include <string>
#include <filesystem> // C++17 and later

#include "WAVFile.h"
#include "AudioSample.h"
#include "AudioChannel.h"
#include "MidiRoll.h"
#include "MidiJsonFile.h"
#include "MidiRollTrack.h"
#include "MidiRollEvent.h"
using namespace std;

const int sample_rate = 44100;
const int bits_per_sample = 32;
const int max_amplitude = (1u << (bits_per_sample - 1)) - 1;

int main() {

    std::string filePath = "../data/roll.json";
    Audio::Midi::MidiJsonFile mj;

    if (mj.load(filePath.c_str())) {
        const Audio::Midi::MidiRoll& roll = *(mj.getRoll());
    }

    Audio::WAVFile wav(true, sample_rate, bits_per_sample, max_amplitude);
    Audio::AudioChannel cnl(sample_rate);

    wav.open("test.wav");

    int t = 0;

    while(cnl.isActive()) {

        Audio::AudioSample steroSample = cnl.sample(t);
        Audio::AudioSample normalizedSample = steroSample * (static_cast<double>(max_amplitude) * 1);
        Audio::AudioSample clampedSample = normalizedSample.clamp(-max_amplitude, max_amplitude);

        wav.sample(clampedSample);

        ++t;
    }

    wav.close();

    return 0;
}
