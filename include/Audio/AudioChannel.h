#ifndef AUDIOCHANNEL_H
#define AUDIOCHANNEL_H

#include <queue>
#include <vector>
#include <memory>
#include <string>

#include "AudioSample.h"
#include "Oscillator.h"
#include "AudioNote.h"
#include "ADSRProfile.h"
#include "Constants.h"

namespace Audio{
    class AudioChannel {
        public:
            AudioChannel(int sampleRate);
            virtual ~AudioChannel();

            //Returns a stereo signal
            AudioSample sample(int t);
            bool isActive();
            void playNote(std::string note);
            void stopNote(std::string note);
        private:
            std::shared_ptr<Oscillator> osc;
            std::shared_ptr<ADSRProfile> adsr;
            std::vector<std::shared_ptr<AudioNote>> notes;

            std::queue<std::string> scale;

            int sample_rate;
            double pan;
            double volume;

            //effects?? HPF etc

            void tickRoll(int t);
            double combineActiveNotes();
            AudioSample panMonoSample(double monoSample);
            double calculateFrequency(std::string note) {
                return MusicalConstants::C4_FREQUENCY * pow(2.0, MusicalConstants::noteToSemitone.at(note) / 12.0);
            }
    };
}
#endif // AUDIOCHANNEL_H
