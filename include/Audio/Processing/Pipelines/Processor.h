//
// Created by david on 29/07/2025.
//

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <iostream>
#include <tuple>

#include "PipelinesUtils.h"
#include "Sink.h"
#include "Source.h"

namespace Audio::Processing::Pipelines {
    template<typename TIn, typename TOut>
    class Processor : public Sink<TIn> {
    public:
        virtual TOut process(const TIn& value) = 0;

        void consume(const TIn& value) override {
            if constexpr (requires { std::tuple_size<TIn>::value; }) {
                PIPELINE_LOG("Processor consuming: " << tuple_to_string(value));
            } else {
                PIPELINE_LOG("Processor consuming: " << to_string_fallback(value));
            }
            if (sink)
                sink->consume(process(value));
        }

        void flow(Source<TIn>* source, Sink<TOut>* sink) {
            this->sink = sink;
            source->produce(this);
        }

    protected:
        Sink<TOut>* sink = nullptr;
    };
}

#endif //PROCESSOR_H
