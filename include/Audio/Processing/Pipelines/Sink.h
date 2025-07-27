#ifndef SINK_H
#define SINK_H

namespace Audio {
    namespace Processing {
        namespace Pipelines {
            template<typename... Ts>
            class Sink {
                public:
                    virtual void consume(Ts... args) = 0;
                    virtual ~Sink() = default;
            };
        }
    }
}

#endif //SINK_H