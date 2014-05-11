#ifndef _ACTUATION_H
#define _ACTUATION_H

#include <chrono>
#include <SDL2/SDL.h>
#include "kit/freq/freq.h"
#include <boost/signals2.hpp>

class Actuation:
    public IRealtime
{
    public:
        virtual ~Actuation() {}
        virtual void logic(Freq::Time t) override {
            actuators(t);
        }
        boost::signals2::signal<void(Freq::Time)> actuators;
};

#endif

