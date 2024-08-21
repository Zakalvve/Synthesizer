#ifndef AUDIONOTE_H
#define AUDIONOTE_H

#include <memory>

#include "ADSRProfile.h"
#include "Oscillator.h"

namespace Audio {
    class AudioNote {
        public:
            AudioNote(std::shared_ptr<Oscillator> oscillator, std::shared_ptr<ADSRProfile> adsrProfile, double frequency, double strength);
            virtual ~AudioNote();

            double sample();
            bool isEnded();
        protected:

        private:
            int t;
            double strength;
            double frequency;
            int total_samples;
            int harmonics = 10;
            std::shared_ptr<Oscillator> osc;
            std::shared_ptr<ADSRProfile> adsr;
    };
}

#endif // AUDIONOTE_H
