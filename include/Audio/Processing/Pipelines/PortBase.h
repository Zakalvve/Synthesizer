//
// Created by david on 29/07/2025.
//

#ifndef PORTBASE_H
#define PORTBASE_H

#include "FillableBuffer.h"

namespace Audio::Processing::Pipelines {
    class PortBase : public FillableBuffer {
    public:
        ~PortBase() override = default;
    };
}

#endif //PORTBASE_H
