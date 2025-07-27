//
// Created by david on 27/07/2025.
//

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "Sink.h"
#include "Source.h"

namespace Audio::Processing::Pipelines {
    template<typename TIn, typename TOut>
    class Processor : public Sink<TIn> {
    public:
        virtual ~Processor() = default;

        void flow(Source<TIn>* source, Sink<TOut>* sink) {
            this->sink = sink;
            source->produce(this); // `this` is a Sink<TIn>
        }

        void consume(const TIn& value) override {
            sink->consume(process(value));
        }

    protected:
        virtual TOut process(const TIn& value) = 0;

    private:
        Sink<TOut>* sink = nullptr;
    };
}

#endif //PROCESSOR_H
