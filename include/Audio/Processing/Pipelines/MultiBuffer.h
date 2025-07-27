//
// Created by david on 27/07/2025.
//

#ifndef MULTIBUFFER_H
#define MULTIBUFFER_H

#include <vector>
#include "BaseInputBuffer.h"

namespace Audio::Processing::Pipelines {
    template<typename TIn>
    class MultiBuffer : public BaseInputBuffer<TIn, std::vector<TIn>> {
    public:
        MultiBuffer() {
            this->buffer.clear();
            parentCount = 0;
        }

        void add(const TIn& input) override {
            this->buffer.push_back(input);
        }

        bool isFilled() const override {
            return static_cast<int>(this->buffer.size()) == parentCount;
        }

        void addParent() override {
            ++parentCount;
        }

        void flush() override {
            this->buffer.clear();
        }

    private:
        int parentCount;
    };
}

#endif //MULTIBUFFER_H
