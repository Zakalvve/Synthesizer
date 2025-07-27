#ifndef SYNTHESIZER_H
#define SYNTHESIZER_H

#include <unordered_map>

#include "AudioChannel.h"
#include "MidiEventProcessor.h"
#include "MidiRollEvent.h"
#include "MidiRollKeyEvent.h"
#include "MidiRollChannelOnEvent.h"

namespace Audio {
    class Synthesizer : public Midi::MidiEventProcessor {
        public:
            Synthesizer(int sampleRate, double volume);
            ~Synthesizer() = default;
            AudioSample sample();
            bool isPlaying();
            void processMidiEvent(Midi::MidiRollEvent& event) override;
            void processMidiEvent(Midi::MidiRollChannelOnEvent& event) override;
            void processMidiEvent(Midi::MidiRollKeyEvent& event) override;
        private:
            int sample_rate;
            double volume;
            std::unordered_map<int, AudioChannel> channels;
    };
}
#endif //SYNTHESIZER_H