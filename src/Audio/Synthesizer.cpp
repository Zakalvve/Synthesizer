#include <unordered_map>

#include "Synthesizer.h"
#include "AudioChannel.h"
#include "MidiRollEvent.h"
#include "MidiRollKeyEvent.h"
#include "MidiRollChannelOnEvent.h"

namespace Audio {
    Synthesizer::Synthesizer(int sampleRate, double volume) : sample_rate(sampleRate), volume(volume) {}

    AudioSample Synthesizer::sample(int t){
        AudioSample sample = AudioSample(0,0);
        for(auto it = channels.begin(); it != channels.end();){
            sample = sample + it->second.sample(t);
        }
        sample = sample * volume;
        return sample;
    }

    bool Synthesizer::isPlaying() {
        for (auto it = channels.begin(); it != channels.end();){
            if (it->second.isActive()){
                return true;
            }
        }
        return false;
    }

    void Synthesizer::processMidiEvent(Midi::MidiRollEvent& event){ 
        // Do nothing with event types that are not implemented
    }

    void Synthesizer::processMidiEvent(Midi::MidiRollChannelOnEvent& event) {
        int channelIndex = event.getChannel();
        if (channels.find(channelIndex) ==  channels.end()){
            // Channel does not exist
            auto channel = AudioChannel(sample_rate);
            channels[channelIndex] = channel;
        }
    }

    void Synthesizer::processMidiEvent(Midi::MidiRollKeyEvent& event){
        int channelIndex = event.getChannel();
        if (auto search = channels.find(channelIndex); search != channels.end()){
            const std::string& eventType = event.getSubType();
            if (eventType == "keyOn"){
                search->second.playNote(event.getNote()); // Should send the strength as well
            } else if (eventType == "keyOff"){
                search->second.stopNote(event.getNote()); // Should send the strength as well
            }
        }
    }
}