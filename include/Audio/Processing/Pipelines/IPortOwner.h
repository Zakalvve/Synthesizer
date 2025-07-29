//
// Created by david on 29/07/2025.
//

#ifndef IPORTOWNER_H
#define IPORTOWNER_H

namespace Audio::Processing::Pipelines {
    class IPortOwner {
    public:
        virtual void onPortFilled() = 0;
        virtual ~IPortOwner() = default;
    };
}

#endif //IPORTOWNER_H
