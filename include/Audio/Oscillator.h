#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <memory>

#include "IWaveform.h"

namespace Audio {
    class Oscillator {
        public:
            Oscillator(int sampleRate);
            Oscillator(std::shared_ptr<IWaveform> waveform);
            virtual ~Oscillator();

            double sample(int t, double f) const;

            static std::shared_ptr<IWaveform> createSineWave(int sampleRate);
            static std::shared_ptr<IWaveform> createSawtoothWave(int sampleRate);
            void setWaveform(std::shared_ptr<IWaveform> newWaveform);
        private:
            std::shared_ptr<IWaveform> waveform;
    };
}
#endif // OSCILLATOR_H
