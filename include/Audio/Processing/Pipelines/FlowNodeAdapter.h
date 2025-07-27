//
// Created by david on 27/07/2025.
//

#ifndef FLOWNODEADAPTER_H
#define FLOWNODEADAPTER_H

#include "FlowNode.h"
#include <any>

namespace Audio::Processing::Pipelines {

    template<typename T>
    class FlowNodeAdapter : public FlowNode<T> {
    public:
        explicit FlowNodeAdapter(FlowNode<std::any>* target)
            : target(target) {}

        void consume(const T& value) override {
            target->consume(std::any(value));
        }

        void addParent() override {
            target->addParent();
        }

        template<typename TOut>
        void addParentTyped() {
            target->template addParentTyped<TOut>();
        }

    private:
        FlowNode<std::any>* target;
    };

}

#endif //FLOWNODEADAPTER_H
