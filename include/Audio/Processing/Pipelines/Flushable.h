//
// Created by david on 27/07/2025.
//

#ifndef FLUSHABLE_H
#define FLUSHABLE_H

namespace Audio::Processing::Pipelines {

    class Flushable {
    public:
        virtual void flush() = 0;
        virtual ~Flushable() = default;
    };

}

#endif //FLUSHABLE_H
