#ifndef _RESOURCE_H
#define _RESOURCE_H

#include "IRealtime.h"
#include <memory>

class IResource:
    public IRealtime,
    public std::enable_shared_from_this<IResource>
{
    public:
        virtual ~IResource() {}
};

#endif

