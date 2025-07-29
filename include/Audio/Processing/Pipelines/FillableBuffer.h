//
// Created by david on 29/07/2025.
//

#ifndef FILLABLEBUFFER_H
#define FILLABLEBUFFER_H

namespace Audio::Processing::Pipelines {
    class FillableBuffer {
    public:
        virtual bool isFilled() const = 0;
        virtual void flush() = 0;
        virtual ~FillableBuffer() = default;
    };
}

#endif //FILLABLEBUFFER_H
