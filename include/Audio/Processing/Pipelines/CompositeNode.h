//
// Created by david on 27/07/2025.
//

#ifndef COMPOSITENODE_H
#define COMPOSITENODE_H

#include "AbstractNode.h"
#include "CompositeInputBuffer.h"
#include "InputBuffer.h"

namespace Audio::Processing::Pipelines {

    template<typename T1, typename T2, typename TOut>
    class CompositeNode :
        public AbstractNode<
            CompositeInputBuffer<
                T1, T1, InputBuffer<T1>,
                T2, T2, InputBuffer<T2>
            >,
            std::any,
            std::tuple<T1, T2>,
            TOut
        >
    {
        using Base = AbstractNode<
            CompositeInputBuffer<T1, T1, InputBuffer<T1>, T2, T2, InputBuffer<T2>>,
            std::any,
            std::tuple<T1, T2>,
            TOut
        >;

    public:
        explicit CompositeNode(std::unique_ptr<Processor<std::tuple<T1, T2>, TOut>> processor = nullptr)
            : Base(std::move(processor)) {}

        template<typename T>
        void addParentTyped() {
            this->inputBuffer->template addParentTyped<T>();
        }
    };

}

#endif //COMPOSITENODE_H
