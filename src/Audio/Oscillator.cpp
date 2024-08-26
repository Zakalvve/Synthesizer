#include <memory>
#include <iostream>

#include "Oscillator.h"
#include "IWaveform.h"
#include "SineWave.h"
#include "SawtoothWave.h"

namespace Audio {
    Oscillator::Oscillator(int sampleRate) : Oscillator(std::make_shared<SineWave>(sampleRate)) { }
    Oscillator::Oscillator(std::shared_ptr<IWaveform> waveform) : waveform(waveform) { }

    double Oscillator::sample(int t, double f) const {
        return waveform->sample(t, f);
    }

    std::shared_ptr<IWaveform> Oscillator::createSineWave(int sampleRate) {
        return std::make_shared<SineWave>(sampleRate);
    }
    std::shared_ptr<IWaveform> Oscillator::createSawtoothWave(int sampleRate) {
        return std::make_shared<SawtoothWave>(sampleRate);
    }
    
    void Oscillator::setWaveform(std::shared_ptr<IWaveform> newWaveform) {
        waveform = newWaveform;
    }
}
