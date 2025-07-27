//
// Created by david on 27/07/2025.
//

#ifndef MULTINODE_H
#define MULTINODE_H

#include "AbstractNode.h"
#include "MultiBuffer.h"

namespace Audio::Processing::Pipelines {
    template<typename TIn, typename TOut>
    class MultiNode : public AbstractNode<MultiBuffer<TIn>, TIn, std::vector<TIn>, TOut> {
        using Base = AbstractNode<MultiBuffer<TIn>, TIn, std::vector<TIn>, TOut>;

    public:
        explicit MultiNode(std::unique_ptr<Processor<std::vector<TIn>, TOut>> processor = nullptr)
            : Base(std::move(processor)) {}

        void addParent() override {
            this->inputBuffer->addParent();
        }
    };
}

#endif //MULTINODE_H
