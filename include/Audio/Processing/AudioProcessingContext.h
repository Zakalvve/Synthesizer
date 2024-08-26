#ifndef AUDIOPROCESSINGCONTEXT_H
#define AUDIOPROCESSINGCONTEXT_H

namespace Audio {
    namespace Processing {
        struct AudioProcessingContext {
            int track_t;
            int note_t;
            double frequency;
            double volume;
            double pan;
        };
    }
}

#endif //AUDIOPROCESSINGCONTEXT_H