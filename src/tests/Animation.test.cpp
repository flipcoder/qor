#include <functional>
#include <catch.hpp>
#include "../kit/math/common.h"
#include "../Common.h"
#include "../Animation.h"
using namespace std;

TEST_CASE("Animation","[animation]") {

    SECTION("frames") {
        Freq::Timeline timeline;
        Animation<float> anim;
        anim.timeline(&timeline);
        REQUIRE(floatcmp(anim.get(), 0.0f));
        anim.frame(Frame<float>(
            1.0f,
            Freq::Time::seconds(1),
            INTERPOLATE(linear)
        ));
        REQUIRE(anim.size() == 1);
        anim.frame(Frame<float>(
            10.0f,
            Freq::Time::seconds(1),
            INTERPOLATE(linear)
        ));
        REQUIRE(floatcmp(anim.get(), 0.0f));
        REQUIRE(anim.size() == 2);
        timeline.logic(Freq::Time::seconds(1));
        REQUIRE(!anim.elapsed());
        REQUIRE(anim.size() == 1);
        REQUIRE(floatcmp(anim.get(), 1.0f));
        timeline.logic(Freq::Time::ms(500));
        REQUIRE(floatcmp(anim.get(), 5.5f));
        timeline.logic(Freq::Time::ms(500));
        REQUIRE(floatcmp(anim.get(), 10.0f));
        REQUIRE(anim.elapsed());
    }
}

