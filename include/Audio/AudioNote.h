#ifndef AUDIONOTE_H
#define AUDIONOTE_H

#include <memory>

#include "Oscillator.h"
#include "AdsrProfile.h"

namespace Audio {
    class AudioNote {
        public:
            AudioNote(std::shared_ptr<Oscillator> osc, std::unique_ptr<AdsrProfile> adsrProfile, double frequency, double strength);
            virtual ~AudioNote() = default;

            double sample();
            void play();
            void release();
            bool isEnded();
        protected:
            
        private:
            int harmonics = 1;
            std::shared_ptr<Oscillator> osc;
            std::unique_ptr<AdsrProfile> adsr;
            double frequency;
            double strength;
            int t;
            int isPlaying;
    };
}

#endif // AUDIONOTE_H
