#ifndef _ACTUATION_H
#define _ACTUATION_H

#include <chrono>
#include <SDL2/SDL.h>
#include "kit/freq/freq.h"

class Actuation:
    public IRealtime
{
    public:
        virtual ~Actuation() {}
        virtual void logic(Freq::Time t) override {
            on_tick(t);
        }
        kit::signal<void(Freq::Time)> on_tick;
        std::unordered_map<std::string, kit::signal<void(std::shared_ptr<Meta>)>> event;
};

#endif

