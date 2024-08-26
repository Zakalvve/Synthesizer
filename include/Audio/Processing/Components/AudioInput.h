#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

namespace Audio {
    namespace Processing {
        namespace Components {
            template<typename T>
            class AudioInput {
                public:
                virtual void setInput(const T& input) = 0;
                virtual const T& getInput() const = 0;
                virtual ~AudioInput() = default;
            };
        }
    }
}

#endif //AUDIOINPUT_H