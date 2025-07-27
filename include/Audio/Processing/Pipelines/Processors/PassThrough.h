//
// Created by david on 28/07/2025.
//

#ifndef PASSTHROUGH_H
#define PASSTHROUGH_H

#include "Processor.h"

namespace Audio::Processing::Pipelines::Processors {

    template<typename T>
    class PassThrough : public Processor<T, T> {
    protected:
        T process(const T& value) override {
            std::cout << "[PassThrough] " << value << "\n";
            return value;
        }
    };

}

#endif //PASSTHROUGH_H
