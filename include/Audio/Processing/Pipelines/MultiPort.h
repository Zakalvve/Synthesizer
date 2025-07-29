//
// Created by david on 29/07/2025.
//

#ifndef MULTIPORT_H
#define MULTIPORT_H
#include <vector>

#include "IPortOwner.h"
#include "PipelinesUtils.h"
#include "PortBase.h"
#include "Sink.h"
#include "Source.h"

namespace Audio::Processing::Pipelines {
    template<typename T>
    class MultiPort : public PortBase, public Sink<T>, public Source<std::vector<T>> {
    public:
        explicit MultiPort(IPortOwner* owner) : owner(owner) {
            if (!owner) throw std::runtime_error("MultiPort must have an owner");
        }

        void consume(const T& val) override {
            PIPELINE_LOG("MultiPort consuming: " << to_string_fallback(val));
            buffer.push_back(val);
            if (buffer.size() == connections && owner)
                owner->onPortFilled();
        }

        void produce(Sink<std::vector<T>>* consumer) override {
            if (!buffer.empty())
                consumer->consume(buffer);
        }

        void connect(Sink<std::vector<T>>* sink) {
            PIPELINE_LOG("Connecting vector sink to multiport");
            sinks.push_back(sink);
        }

        void registerInput() { ++connections; }

        bool isFilled() const override { return buffer.size() == connections; }
        void flush() override {
            PIPELINE_LOG("Flushing multiport");
            buffer.clear();
        }

        const std::vector<T>& getAll() const { return buffer; }

    private:
        std::vector<T> buffer;
        int connections = 0;
        std::vector<Sink<std::vector<T>>*> sinks;
        IPortOwner* owner;
    };
}

#endif //MULTIPORT_H
