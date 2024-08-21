#ifndef SYNTHESIZER_H
#define SYNTHESIZER_H

#include <unordered_map>

#include "AudioChannel.h"
#include "MidiRollEvent.h"
#include "MidiRollKeyEvent.h"
#include "MidiRollChannelOnEvent.h"

namespace Audio {
    class Synthesizer {
        public:
            Synthesizer(int sampleRate, double volume);
            ~Synthesizer() = default;
            AudioSample sample(int t);
            bool isPlaying();
            void processMidiEvent(Midi::MidiRollEvent& event);
            void processMidiEvent(Midi::MidiRollChannelOnEvent& event);
            void processMidiEvent(Midi::MidiRollKeyEvent& event);
        private:
            int sample_rate;
            double volume;
            std::unordered_map<int, AudioChannel> channels;
    };
}
#endif //SYNTHESIZER_H