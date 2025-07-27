//
// Created by david on 27/07/2025.
//

#ifndef FLOWNODE_H
#define FLOWNODE_H

#include "Sink.h"

namespace Audio::Processing::Pipelines {

    template<typename TIn>
    class FlowNode : public Sink<TIn> {
    public:
        virtual void addParent() = 0;

        template<typename T>
        void addParentTyped() {
            // fallback to untyped unless overridden
            this->addParent();
        }

        virtual ~FlowNode() = default;
    };

}

#endif //FLOWNODE_H
