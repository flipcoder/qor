#ifndef _SHADOW_H
#define _SHADOW_H

#include <vector>
#include "RenderBuffer.h"

class ShadowBuffer
{
    private:

        std::vector<RenderBuffer> m_Buffers;

    public:

        ShadowBuffer() {}
        virtual ~ShadowBuffer() {}
};

#endif

