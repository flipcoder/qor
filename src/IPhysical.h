#ifndef _IPHYSICAL_H
#define _IPHYSICAL_H

#include <cassert>

class Physics;

class IPhysical
{
    public:
        virtual Physics* physics() { assert(false); return nullptr; }
        virtual Physics* physics() const { assert(false); return nullptr; }
};

#endif

