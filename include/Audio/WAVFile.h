#ifndef WAVFILE_H
#define WAVFILE_H

#include <fstream>
#include <string>

#include "StereoSample.h"

namespace Audio {
    class WAVFile {
        public:
            WAVFile(bool isStereo, int sample_rate, int bits_per_sample, int max_amplitude);
            virtual ~WAVFile();

            void open(const std::string& fileName);
            void close();
            void sample(const Audio::StereoSample& audio_sample);
            const int& getSampleRate();
            const int& getMaxAmplitude();
        private:
            void wab(std::ofstream &file, int value, int byteSize);

            // RIFF sub-chunk
            const std::string chunk_id = "RIFF";
            const std::string chunk_size = "----";
            const std::string format = "WAVE";

            // fmt sub-chunk
            const std::string subchunk1_id = "fmt ";
            const int subchunk1_size = 16;
            const int audio_format = 1;
            int num_channels;
            int sample_rate;
            int byte_rate;
            int block_align;
            int bits_per_sample;

            // data sub-chunk
            const std::string subchunk2_id = "data";
            const std::string subchunk2_size = "----";
            std::ofstream wav;

            int start_audio;
            int end_audio;
            int max_amplitude;
    };
}
#endif // WAVFILE_H
