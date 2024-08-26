#include <string>
#include <vector>
#include <memory>

#include "MidiJsonFile.h"
#include "MidiRoll.h"
#include "WAVFile.h"
#include "Synthesizer.h"
#include "MidiRollEvent.h"
#include "StereoSample.h"

using namespace std;

const int sample_rate = 44100;
const int bits_per_sample = 32;
const int max_amplitude = (1u << (bits_per_sample - 1)) - 1;

int main() {

    std::string filePath = "../data/moonlight-sonata.json";
    Audio::Midi::MidiJsonFile mj;

    if (mj.load(filePath.c_str(), sample_rate)) {
        Audio::Midi::MidiRoll& roll = *(mj.getRoll());
        Audio::WAVFile wav(true, sample_rate, bits_per_sample, max_amplitude);

        Audio::Synthesizer synth(sample_rate, 1.0);

        wav.open("test.wav");

        int t = 0;

        while(roll.isRolling() || synth.isPlaying()) {

            std::vector<std::unique_ptr<Audio::Midi::Events::MidiRollEvent>> midiEvents = roll.tickRoll(t);

            for(auto& event: midiEvents){
                // Uses a visitor pattern
                event->process(synth);
            }

            Audio::StereoSample steroSample = synth.sample();
            Audio::StereoSample normalizedSample = steroSample * (static_cast<double>(max_amplitude) * 1);
            Audio::StereoSample clampedSample = normalizedSample.clamp(-max_amplitude, max_amplitude);

            wav.sample(clampedSample);

            ++t;
        }

        wav.close();
    }
    
    return 0;
}
