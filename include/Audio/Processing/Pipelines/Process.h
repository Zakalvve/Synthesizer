#ifndef PROCESS_H
#define PROCESS_H

#include "Sink.h"
#include "Source.h"

namespace Audio {
    namespace Processing {
        namespace Pipelines {
            template<typename In, typename Out>
            class Process : Sink<In> {
                public:
                    virtual void process(Source<In>* source, Sink<Out>* sink){
                        s = sink;
                        source->produce(this);
                    }
                    virtual ~Process() = default;

                protected:
                    Sink<Out>* s;
            };
        }
    }
}

#endif //PROCESS_H