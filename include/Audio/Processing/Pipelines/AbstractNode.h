//
// Created by david on 27/07/2025.
//

#ifndef ABSTRACTNODE_H
#define ABSTRACTNODE_H

#include <vector>
#include <memory>
#include "Processor.h"
#include "FlowNode.h"
#include "Source.h"
#include "BaseInputBuffer.h"
#include "FlowNodeAdapter.h"

namespace Audio::Processing::Pipelines {

    template<
        typename TBuffer,
        typename TNodeIn,
        typename TProcessIn,
        typename TOut
    >
    class AbstractNode :
        public FlowNode<TNodeIn>,
        public Source<TProcessIn> {

        static_assert(std::is_base_of_v<BaseInputBuffer<TNodeIn, TProcessIn>, TBuffer>,
                      "TBuffer must derive from BaseInputBuffer<TNodeIn, TProcessIn>");

    public:
        explicit AbstractNode(std::unique_ptr<Processor<TProcessIn, TOut>> processor = nullptr)
            : processor(std::move(processor)), inputBuffer(std::make_unique<TBuffer>()) {}

        // Consumes input values
        void consume(const TNodeIn& value) override {
            inputBuffer->add(value);

            if (!inputBuffer->isFilled() || !processor)
                return;

            for (auto* node : nextNodes)
                processor->flow(this, node);

            inputBuffer->flush();
        }

        // Supplies processed data to a downstream sink
        void produce(Sink<TProcessIn>* consumer) override {
            consumer->consume(getData());
        }

        // Connects to downstream node (same TOut)
        template<typename TNext>
        void connect(TNext* nextNode) {
            static_assert(std::is_base_of_v<FlowNode<TOut>, TNext>, "TNext must be a FlowNode of TOut");

            nextNodes.push_back(nextNode);

            // Try to call addParentTyped<TOut>() if it exists, fallback to addParent()
            if constexpr (requires { nextNode->template addParentTyped<TOut>(); }) {
                nextNode->template addParentTyped<TOut>();
            } else {
                nextNode->addParent(); // untyped fallback
            }
        }

        template<typename T>
        void connectToObjectSink(FlowNode<std::any>* nextNode) {
            nextNodes.push_back(new FlowNodeAdapter<T>(nextNode));

            // Try to call addParentTyped<T>() if it exists
            if constexpr (requires { nextNode->template addParentTyped<T>(); }) {
                nextNode->template addParentTyped<T>();
            } else {
                nextNode->addParent(); // fallback
            }
        }

        TProcessIn getData() const {
            return inputBuffer->buffer;
        }

        // Optional for multi-input nodes
        void addParent() override {
            // no-op
        }

    protected:
        std::unique_ptr<TBuffer> inputBuffer;
        std::unique_ptr<Processor<TProcessIn, TOut>> processor;
        std::vector<FlowNode<TOut>*> nextNodes;
    };

}

#endif //ABSTRACTNODE_H
