#ifndef NODE_H
#define NODE_H

#include <vector>
#include <concepts>
#include <iostream>

#include "Sink.h"
#include "Source.h"
#include "InputBuffer.h"

#include "AbstractNode.h"
#include "InputBuffer.h"

namespace Audio::Processing::Pipelines {

    template<typename TIn, typename TOut>
    using Node = AbstractNode<InputBuffer<TIn>, TIn, TIn, TOut>;

}

#endif //NODE_H