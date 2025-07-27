//
// Created by david on 28/07/2025.
//

#ifndef MULTICOMPOSITENODE_H
#define MULTICOMPOSITENODE_H

#include "AbstractNode.h"
#include "CompositeInputBuffer.h"
#include "MultiBuffer.h"
#include <iostream>

namespace Audio::Processing::Pipelines {

    template<typename T1, typename T2, typename TOut>
    class MultiCompositeNode : public AbstractNode<
        CompositeInputBuffer<
            T1, std::vector<T1>, MultiBuffer<T1>,
            T2, std::vector<T2>, MultiBuffer<T2>
        >,
        std::any,
        std::tuple<std::vector<T1>, std::vector<T2>>,
        TOut
    >
    {
    public:
        explicit MultiCompositeNode(std::unique_ptr<Processor<std::tuple<std::vector<T1>, std::vector<T2>>, TOut>> processor = nullptr)
            : AbstractNode<
                CompositeInputBuffer<
                    T1, std::vector<T1>, MultiBuffer<T1>,
                    T2, std::vector<T2>, MultiBuffer<T2>
                >,
                std::any,
                std::tuple<std::vector<T1>, std::vector<T2>>,
                TOut
            >(std::move(processor))
        {}

        template<typename T>
        void addParentTyped() {
            this->inputBuffer->template addParentTyped<T>();
        }

        void addParent() override {
            std::cout << "addParent" << std::endl;
        }
    };

}

#endif //MULTICOMPOSITENODE_H
