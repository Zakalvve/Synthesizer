//
// Created by david on 29/07/2025.
//

#ifndef PORT_H
#define PORT_H

#include "IPortOwner.h"
#include "PortBase.h"
#include "Sink.h"
#include "Source.h"
#include "PipelinesUtils.h"

namespace Audio::Processing::Pipelines {
    template<typename T>
    class Port : public PortBase, public Sink<T>, public Source<T> {
    public:
        explicit Port(IPortOwner* owner) : owner(owner) {
            if (!owner) throw std::runtime_error("Port must have an owner");
        }

        void consume(const T& val) override {
            PIPELINE_LOG("Port consuming: " << to_string_fallback(val));

            if (filled) return;

            buffer = val;
            filled = true;

            if (owner) owner->onPortFilled();

            for (auto* sink : sinks) {
                PIPELINE_LOG("Propagating to sink...");
                sink->consume(val);
            }
        }

        void produce(Sink<T>* consumer) override {
            if (filled)
                consumer->consume(buffer);
        }

        void connect(Sink<T>* sink) {
            PIPELINE_LOG("Connecting sink to port");
            sinks.push_back(sink);
        }

        bool isFilled() const override { return filled; }
        void flush() override {
            PIPELINE_LOG("Flushing port");
            filled = false;
        }
        const T& get() const { return buffer; }

    private:
        T buffer{};
        bool filled = false;
        std::vector<Sink<T>*> sinks;
        IPortOwner* owner;
    };
}

#endif //PORT_H
