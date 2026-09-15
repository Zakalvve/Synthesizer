#ifndef MIDIJSONFILE_H
#define MIDIJSONFILE_H

#include <cstdint>
#include <memory>

#include "MidiSequence.h"

namespace Audio {
    namespace Midi{

        class MidiJsonFile {
            public:
                MidiJsonFile();
                virtual ~MidiJsonFile();

                bool load(const char* filePath);
                std::unique_ptr<MidiSequence>& getSequence() { return sequence; };

            private:
            const uint16_t bufferSize = 16384;
            std::unique_ptr<MidiSequence> sequence;
        };
    }
}
#endif //MIDIJSONFILE_H
