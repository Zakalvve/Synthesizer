#ifndef SINK_H
#define SINK_H

namespace Audio::Processing::Pipelines {

    template<typename T>
    class Sink {
    public:
        virtual void consume(const T& value) = 0;
        virtual ~Sink() = default;
    };

}

#endif // SINK_H