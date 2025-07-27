//
// Created by david on 27/07/2025.
//

#ifndef BASEINPUTBUFFER_H
#define BASEINPUTBUFFER_H

#include "Flushable.h"

namespace Audio::Processing::Pipelines {

    template<typename TIn, typename TOut>
    class BaseInputBuffer : public Flushable {
    public:
        virtual ~BaseInputBuffer() = default;

        virtual void add(const TIn& input) = 0;
        virtual bool isFilled() const = 0;

        virtual void flush() override {
            buffer = TOut{}; // default-constructed
        }

        virtual void addParent() {
            // Optional override
        }

        TOut buffer;
    };

}

#endif //BASEINPUTBUFFER_H
