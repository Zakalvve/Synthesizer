#include <cmath>
#include <iostream>

#include "AudioChannel.h"
#include "Oscillator.h"

namespace Audio {
    AudioChannel::AudioChannel(int sampleRate) : sample_rate(sampleRate), pan(0.0), volume(1.0) {
        double hold_duration = 2.84;
        double attack_duration = 0.01;
        double decay_duration = 2.83;
        double sustain_threshold = 0.1;
        double release_duration = 0.1;

        osc = std::make_shared<Audio::Oscillator>(Audio::Oscillator::createSawtoothWave(sampleRate));
        adsr = std::make_shared<Audio::ADSRProfile>(sampleRate, attack_duration, decay_duration, hold_duration, release_duration, sustain_threshold);

        std::vector<std::string> scaleNotes = {"C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"};
        for (const std::string& note : scaleNotes) {
            scale.push(note);
        }
    }

    AudioChannel::~AudioChannel() { }

    AudioSample AudioChannel::sample(int t) {
        tickRoll(t);

        double monoSample = combineActiveNotes();
        AudioSample stereoSample = panMonoSample(monoSample);

        return stereoSample;
    }

    bool AudioChannel::isActive() {
        return notes.size() != 0 || !scale.empty();
    }

    void AudioChannel::tickRoll(int t) {
        if (t % (sample_rate * 3) == 0) {
            if (!scale.empty()) {
                std::string note = scale.front();
                playNote(note);
                scale.pop();
            }
        }
    }

    void AudioChannel::playNote(std::string note) {
        double frequency = calculateFrequency(note);

        notes.push_back(std::make_shared<Audio::AudioNote>(osc,
                adsr,
                frequency, 1));
    }

    double AudioChannel::combineActiveNotes() {
        double value = 0;

        for (auto it = notes.begin(); it != notes.end(); ){
            value += (*it)->sample();

            if ((*it)->isEnded()) {
                it = notes.erase(it);
            } else {
                ++it;
            }
        }

        return value;
    }

    AudioSample AudioChannel::panMonoSample(double monoSample) {
        double pan_mapped = ((pan + 1) / 2.0) * (M_PI / 2.0);

        double left_sample = monoSample * sin(pan_mapped) * volume;
        double right_sample = monoSample * cos(pan_mapped) * volume;

        return AudioSample(left_sample, right_sample);
    }
}
