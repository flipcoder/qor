#include <SDL2/SDL.h>
#include "ScreenFader.h"
#include "Pipeline.h"
#include "kit/freq/freq.h"
#include "kit/freq/animation.h"
using namespace std;

std::function<void(Freq::Time)> screen_fader(
    Freq::Timeline* tl,
    std::function<void(Freq::Time, float)> tick,
    std::function<bool(Freq::Time)> should_advance,
    std::function<void(Freq::Time)> done
){
    auto fade = make_shared<Animation<float>>(tl);
    fade->frame(Frame<float>(
        1.0f,
        Freq::Time::seconds(1.0f),
        INTERPOLATE(out_sine<float>)
    ));
    
    return [fade, tick, should_advance, done](Freq::Time t) {

        tick(t, fade->get());
        
        if(fade->elapsed())
        {
            if(floatcmp(fade->get(), 1.0f)) // faded in
            {
                if(should_advance(t)){
                    fade->frame(Frame<float>(
                        0.0f,
                        Freq::Time::seconds(1.0f),
                        INTERPOLATE(out_sine<float>)
                    ));
                }
            }
            else // faded out
            {
                done(t);
            }
        }
    };
}

