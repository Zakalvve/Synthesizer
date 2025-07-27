#ifndef INPUTBUFFER_H
#define INPUTBUFFER_H

#include "BaseInputBuffer.h"

namespace Audio::Processing::Pipelines {

    template<typename T>
    class InputBuffer : public BaseInputBuffer<T, T> {
    public:
        InputBuffer() {
            this->buffer = T{};
            filled = false;
        }

        void add(const T& input) override {
            this->buffer = input;
            filled = true;
        }

        bool isFilled() const override {
            return filled;
        }

        void flush() override {
            filled = false;
        }

    private:
        bool filled = false;
    };

}

#endif //INPUTBUFFER_H