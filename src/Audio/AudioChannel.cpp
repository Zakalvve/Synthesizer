#include <cmath>
#include <numbers>
#include <iostream>
#include <memory>

#include "AudioChannel.h"
#include "Oscillator.h"
#include "StereoSample.h"

namespace Audio {
    AudioChannel::AudioChannel(int sampleRate) : sample_rate(sampleRate), pan(0.0), volume(0.5) {
        osc = std::make_shared<Audio::Oscillator>(Audio::Oscillator::createSawtoothWave(sampleRate));
    }

    AudioChannel::~AudioChannel() { }

    StereoSample AudioChannel::sample() {
        double monoSample = combineActiveNotes();
        StereoSample stereoSample = panMonoSample(monoSample);
        return stereoSample * volume;
    }

    bool AudioChannel::isActive() {
        for(auto& note : notes){
            if (!note.second->isEnded()){
                return true;
            }
        }
        return false;
    }

    void AudioChannel::playNote(std::string note) {
        double frequency = calculateFrequency(note);
        
        if (notes.find(note) == notes.end()){
            // Note not found in map
            double attack_duration = 0.01;
            double decay_duration = 0.03;
            double sustain_threshold = 0.4;
            double release_duration = 0.1;

            std::unique_ptr<AdsrProfile> adsr = std::make_unique<AdsrProfile>(sample_rate, attack_duration, decay_duration, release_duration, sustain_threshold);

            notes[note] = std::make_shared<Audio::AudioNote>(osc, std::move(adsr), frequency, 1);
        }

        notes[note]->play();
    }

    void AudioChannel::releaseNote(std::string note) {
        if (notes.find(note) == notes.end()) return;

        notes[note]->release();
    }
 
    double AudioChannel::combineActiveNotes() {
        double value = 0;

        for (auto& note : notes){
            if (!note.second->isEnded()){
                value += note.second->sample();
            }
        }

        return value;
    }

    StereoSample AudioChannel::panMonoSample(double monoSample) {
        double pan_mapped = ((pan + 1) / 2.0) * (std::numbers::pi / 2.0);

        double left_sample = monoSample * sin(pan_mapped);
        double right_sample = monoSample * cos(pan_mapped);

        return StereoSample(left_sample, right_sample);
    }
}
