//
// Created by david on 28/07/2025.
//

#ifndef INPUTNODE_H
#define INPUTNODE_H

#include "Node.h"
#include "PassThrough.h"

using namespace Audio::Processing::Pipelines::Processors;

namespace Audio::Processing::Pipelines {

    template<typename T>
    class InputNode : public Node<T, T> {
    public:
        InputNode() : Node<T, T>(std::make_unique<PassThrough<T>>()) {}
    };

}

#endif //INPUTNODE_H
