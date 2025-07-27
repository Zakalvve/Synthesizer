#ifndef SOURCE_H
#define SOURCE_H

#include "Sink.h"

namespace Audio::Processing::Pipelines {

    template<typename T>
    class Source {
    public:
        virtual void produce(Sink<T>* consumer) = 0;
        virtual ~Source() = default;
    };

}

#endif // SOURCE_H