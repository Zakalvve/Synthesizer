#ifndef AUDIONOTE_H
#define AUDIONOTE_H

#include <memory>

#include "ADSRProfile.h"
#include "Oscillator.h"

namespace Audio {
    class AudioNote {
        public:
            AudioNote(std::shared_ptr<Oscillator> osc, std::unique_ptr<ADSRProfile> adsrProfile, double frequency, double strength);
            virtual ~AudioNote() = default;

            double sample();
            void play();
            void release();
            bool isEnded();
        protected:
            
        private:
            int harmonics = 12;
            std::shared_ptr<Oscillator> osc;
            std::unique_ptr<ADSRProfile> adsr;
            double frequency;
            double strength;
            int t;
            int isPlaying;
    };
}

#endif // AUDIONOTE_H
