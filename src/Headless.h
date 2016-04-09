#ifndef _HEADLESS_H_1VI18RQT
#define _HEADLESS_H_1VI18RQT

struct Headless {
    static bool s_Headless;
    static bool enabled() {
        return s_Headless;
    }
    static void enable() {
        s_Headless = true;
    }
};

#endif

