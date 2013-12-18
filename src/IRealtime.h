#ifndef _IREALTIME_H
#define _IREALTIME_H

#include <chrono>
#include <SDL2/SDL.h>
#include "kit/freq/freq.h"

class IRealtime
{
    public:
        virtual ~IRealtime() {}
        virtual void logic(Freq::Time t) {}
};

#endif

