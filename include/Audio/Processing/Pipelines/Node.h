//
// Created by david on 29/07/2025.
//

#ifndef NODE_H
#define NODE_H
#include <memory>

#include "IPortOwner.h"
#include "PortBank.h"
#include "Processor.h"

namespace Audio::Processing::Pipelines {
    class PortBank;

    template<typename TIn, typename TOut>
    class Node : public IPortOwner {
    public:
        explicit Node(std::unique_ptr<Processor<TIn, TOut>> processor)
            : processor(std::move(processor)),
              inputBank(this),
              outputBank(this) {}

        template<typename T>
        void addInput(const std::string& name) {
            inputBank.addPort<T>(name);
        }

        template<typename T>
        void addOutput(const std::string& name) {
            outputBank.addPort<T>(name);
        }

        template<typename T>
        void connectOutputTo(const std::string& outputName, Node& target, const std::string& targetInputName) {
            outputBank.getPort<T>(outputName)->connect(target.inputBank.getPort<T>(targetInputName));
        }

        template<typename T, typename TargetNode>
        void connectOutputTo(const std::string& outputName, TargetNode& target, const std::string& targetInputName) {
            outputBank.getPort<T>(outputName)->connect(target.template inputs().template getPort<T>(targetInputName));
        }

        template<typename T>
        void feedInput(const std::string& name, const T& value) {
            PIPELINE_LOG("Feeding input " << name << ": " << to_string_fallback(value));
            inputBank.getPort<T>(name)->consume(value);
        }

        template<typename T>
        T getOutputValue(const std::string& name) const {
            return outputBank.getPort<T>(name)->get();
        }

        PortBank& inputs() { return inputBank; }
        const PortBank& inputs() const { return inputBank; }

        PortBank& outputs() { return outputBank; }
        const PortBank& outputs() const { return outputBank; }

        void onPortFilled() override {
            if (!inputBank.allFilled() || !processor) return;

            PIPELINE_LOG("All inputs filled. Executing processor...");
            TIn in = readInputs();
            TOut out = processor->process(in);
            writeOutputs(out);
            inputBank.flushAll();
        }

    private:
        PortBank inputBank;
        PortBank outputBank;
        std::unique_ptr<Processor<TIn, TOut>> processor;

        TIn readInputs();
        void writeOutputs(const TOut&);
    };

    template<typename TIn, typename TOut>
    TIn Node<TIn, TOut>::readInputs() {
        if constexpr (std::is_same_v<TIn, std::tuple<>>) {
            return {};
        } else if constexpr (requires { std::tuple_size<TIn>::value; }) {
            return unpackPorts<TIn>(inputBank, std::make_index_sequence<std::tuple_size<TIn>::value>{});
        } else {
            return inputBank.template getPort<TIn>("in")->get();
        }
    }

    template<typename TIn, typename TOut>
    void Node<TIn, TOut>::writeOutputs(const TOut& value) {
        outputBank.template getPort<TOut>("out")->consume(value);
    }
}

#endif //NODE_H
