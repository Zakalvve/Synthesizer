#ifndef MIDIJSONFILE_H
#define MIDIJSONFILE_H

#include <memory>
#include <cstdint>

#include "MidiRoll.h"

namespace Audio {
    namespace Midi{

        class MidiJsonFile {
            public:
                MidiJsonFile();
                virtual ~MidiJsonFile();

                bool load(const char* filePath, int sampleRate);
                std::unique_ptr<MidiRoll>& getRoll() { return roll; };

            private:
            const uint16_t bufferSize = 16384;
            std::unique_ptr<MidiRoll> roll;
        };
    }
}
#endif //MIDIJSONFILE_H