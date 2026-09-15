#ifndef AUDIO_SYNTH_NODEREGISTRY_H
#define AUDIO_SYNTH_NODEREGISTRY_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "SignalProcessor.h"
#include "EngineDef.h"

namespace Audio::Synth {
    class NodeRegistry {
    public:
        using Factory = std::function<std::unique_ptr<Audio::Processing::SignalProcessor>(const NodeDef &, int sampleRate)>;

        static const NodeRegistry &instance();

        std::unique_ptr<Audio::Processing::SignalProcessor> create(const NodeDef &def, int sampleRate) const;

        bool has(const std::string &type) const;

    private:
        NodeRegistry();

        std::unordered_map<std::string, Factory> _factories;
    };
}

#endif
