#ifndef _HEADLESS_H_1VI18RQT
#define _HEADLESS_H_1VI18RQT

struct Headless {
    static bool s_Headless;
    static bool s_Server;
    static bool enabled() {
        return s_Headless;
    }
    static bool server() {
        return s_Server;
    }
    static void enable() {
        s_Headless = true;
    }
    static void server(bool b) {
        s_Headless = b;
    }
};

#endif

