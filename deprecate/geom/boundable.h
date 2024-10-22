#ifndef BOUNDABLE_H
#define BOUNDABLE_H

#include "bounding_box.h"

namespace cblt
{
    class Boundable
    {
        public:
        virtual BoundingBox GetBounds() = 0;
    };
}
#endif  // BOUNDABLE_H