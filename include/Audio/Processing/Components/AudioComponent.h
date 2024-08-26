#ifndef AUDIOCOMPONENT_H
#define AUDIOCOMPONENT_H

#include "AudioInput.h"
#include "AudioOutput.h"
#include "AudioProcessingContext.h"

namespace Audio {
    namespace Processing {
        namespace Components {
            template<typename Input, typename Output>
            class AudioComponent : public AudioInput<Input>, public AudioOutput<Output> {
                public:
                    void setInput(const Input& input) override {
                        this->input = input;
                    }

                    const Input& getInput() const override {
                        return input;
                    }

                    virtual Output sample(AudioProcessingContext& context) = 0;

                protected:
                    Input input;
            };
        }
    }
}

#endif //AUDIOCOMPONENT_H