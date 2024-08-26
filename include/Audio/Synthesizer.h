#ifndef SYNTHESIZER_H
#define SYNTHESIZER_H

#include <unordered_map>

#include "MidiEventProcessor.h"
#include "StereoSample.h"
#include "MidiRollEvent.h"
#include "MidiRollChannelOnEvent.h"
#include "MidiRollKeyEvent.h"
#include "AudioChannel.h"

namespace Audio {
    class Synthesizer : public Midi::Events::MidiEventProcessor {
        public:
            Synthesizer(int sampleRate, double volume);
            ~Synthesizer() = default;
            StereoSample sample();
            bool isPlaying();
            void processMidiEvent(Midi::Events::MidiRollEvent& event) override;
            void processMidiEvent(Midi::Events::MidiRollChannelOnEvent& event) override;
            void processMidiEvent(Midi::Events::MidiRollKeyEvent& event) override;
        private:
            int sample_rate;
            double volume;
            std::unordered_map<int, AudioChannel> channels;
    };
}
#endif //SYNTHESIZER_H