#ifndef _SCREENFADER_H
#define _SCREENFADER_H

#include <functional>
#include "kit/freq/freq.h"

std::function<void(Freq::Time)> screen_fader(
    Freq::Timeline* tl,
    std::function<void(Freq::Time, float)> tick,
    std::function<bool(Freq::Time)> should_advance,
    std::function<void(Freq::Time)> done
);

#endif

