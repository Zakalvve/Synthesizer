//
// Created by david on 27/07/2025.
//

#ifndef COMPOSITEINPUTBUFFER_H
#define COMPOSITEINPUTBUFFER_H

#include <any>
#include <tuple>
#include <typeindex>
#include <stdexcept>
#include "BaseInputBuffer.h"

namespace Audio::Processing::Pipelines {

    template<
        typename T1, typename TP1, typename TB1,
        typename T2, typename TP2, typename TB2
    >
    class CompositeInputBuffer : public BaseInputBuffer<std::any, std::tuple<TP1, TP2>> {
    public:
        CompositeInputBuffer() = default;

        void add(const std::any& input) override {
            if (input.type() == typeid(T1)) {
                buffer1.add(std::any_cast<T1>(input));
            }
            else if (input.type() == typeid(T2)) {
                buffer2.add(std::any_cast<T2>(input));
            }
            else {
                throw std::runtime_error("CompositeInputBuffer: invalid input type");
            }

            if (isFilled()) {
                this->buffer = std::make_tuple(buffer1.buffer, buffer2.buffer);
            }
        }

        bool isFilled() const override {
            return buffer1.isFilled() && buffer2.isFilled();
        }

        void flush() override {
            buffer1.flush();
            buffer2.flush();
        }

        void addParent() override {
            // Should not be used — use the typed one below
            throw std::logic_error("CompositeInputBuffer: addParent() requires type info");
        }

        template<typename T>
        void addParentTyped() {
            if constexpr (std::is_same_v<T, T1>) {
                buffer1.addParent();
            }
            else if constexpr (std::is_same_v<T, T2>) {
                buffer2.addParent();
            }
            else {
                static_assert(!sizeof(T), "CompositeInputBuffer: invalid parent type");
            }
        }

    private:
        TB1 buffer1;
        TB2 buffer2;
    };

}

#endif //COMPOSITEINPUTBUFFER_H
