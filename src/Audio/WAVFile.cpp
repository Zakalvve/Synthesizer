#include <iostream>
#include <fstream>

#include "WAVFile.h"

namespace Audio {
    WAVFile::WAVFile(bool isStereo, int sampleRate, int bitsPerSample, int max_amplitude) :
        sample_rate(sampleRate),
        bits_per_sample(bitsPerSample),
        num_channels(isStereo ? 2 : 1),
        max_amplitude(max_amplitude)
    {
        byte_rate = sampleRate * num_channels * (bits_per_sample / 8);
        block_align = num_channels * (bits_per_sample / 8);
    }

    WAVFile::~WAVFile() {
        close();
    }

    void WAVFile::wab(std::ofstream& file, int value, int byteSize) {
        file.write(reinterpret_cast<const char*>(&value), byteSize);
    }

    void WAVFile::open(const std::string& fileName) {
        wav.open("../output/" + fileName, std::ios::binary);

        if (wav.is_open()) {
            wav << chunk_id;
            wav << chunk_size;
            wav << format;

            wav << subchunk1_id;
            wab(wav, subchunk1_size, 4);
            wab(wav, audio_format, 2);
            wab(wav, num_channels, 2);
            wab(wav, sample_rate, 4);
            wab(wav, byte_rate, 4);
            wab(wav, block_align, 2);
            wab(wav, bits_per_sample, 2);

            wav << subchunk2_id;
            wav << subchunk2_size;

            start_audio = wav.tellp();
        }
    }

    void WAVFile::close() {
        if (wav.is_open()) {

            end_audio = wav.tellp();

            wav.seekp(start_audio - 4);
            wab(wav, end_audio - start_audio, 4);

            wav.seekp(4, std::ios::beg);
            wab(wav, end_audio - 8, 4);
            wav.close();
        }
    }

    void WAVFile::sample(const AudioSample& as) {
        wab(wav, as.c1, bits_per_sample / 8);
        wab(wav, as.c2, bits_per_sample / 8);
    }

    const int& WAVFile::getSampleRate() {
        return sample_rate;
    }

    const int& WAVFile::getMaxAmplitude() {
        return max_amplitude;
    }
}
