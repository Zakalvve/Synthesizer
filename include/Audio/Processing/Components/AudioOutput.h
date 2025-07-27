#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

namespace Audio {
    namespace Processing {
        namespace Components {
            template<typename T>
            class AudioOutput {
                public:
                virtual T sample() = 0;
                virtual ~AudioOutput() = default;
            };
        }
    }
}

#endif //AUDIOOUTPUT_H