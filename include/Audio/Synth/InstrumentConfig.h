#ifndef AUDIO_SYNTH_INSTRUMENTCONFIG_H
#define AUDIO_SYNTH_INSTRUMENTCONFIG_H

namespace Audio::Synth {
    enum class OscType { LegacySaw, PhaseSine, PhaseSaw };

    struct InstrumentConfig {
        int sampleRate = 44100;

        OscType oscType = OscType::LegacySaw;

        double attack = 0.01;
        double decay = 0.03;
        double sustain = 0.4;
        double release = 0.1;

        double masterGain = 1.0;
        double velocitySensitivity = 0.0;
        double minFrequency = 20.0;
        double maxFrequency = 20000.0;
        double glideTime = 0.0;
        int maxVoices = 16;
    };
}

#endif
