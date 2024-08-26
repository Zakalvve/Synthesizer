#ifndef AUDIOCHANNEL_H
#define AUDIOCHANNEL_H

#include <string>
#include <memory>
#include <unordered_map>
#include <queue>

#include "StereoSample.h"
#include "Oscillator.h"
#include "AudioNote.h"
#include "Constants.h"

namespace Audio {
    class AudioChannel {
        public:
            AudioChannel(int sampleRate);
            virtual ~AudioChannel();

            //Returns a stereo signal
            StereoSample sample();
            bool isActive();
            void playNote(std::string note);
            void releaseNote(std::string note);
        private:
            std::shared_ptr<Oscillator> osc;
            std::unordered_map<std::string, std::shared_ptr<AudioNote>> notes;

            std::queue<std::string> scale;

            int sample_rate;
            double pan;
            double volume;

            //effects?? HPF etc
            double combineActiveNotes();
            StereoSample panMonoSample(double monoSample);
            double calculateFrequency(std::string note) {
                return MusicalConstants::C4_FREQUENCY * pow(2.0, MusicalConstants::noteToSemitone.at(note) / 12.0);
            }
    };
}
#endif // AUDIOCHANNEL_H
