#ifndef SOURCE_H
#define SOURCE_H

#include "Sink.h"

namespace Audio {
    namespace Processing {
        namespace Pipelines {
            template<typename... Ts>
            class Source {
                public:
                    virtual void produce(Sink<Ts...>* consumer) = 0;
                    virtual ~Source() = default;
            };
        }
    }
}

#endif //SOURCE_H