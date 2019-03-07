#include <iostream>
#include <memory>
#include <thread>
#include "kit/kit.h"
#include "Qor.h"
#include "ScriptState.h"
#include "BasicState.h"
#include "Interpreter.h"
#include "Interpreter.h"
#include "Info.h"

#ifdef DEBUG
    #include <backward/backward.cpp>
#endif

using namespace std;
using namespace kit;

int main(int argc, char* argv[])
{
    Args args(argc, (const char**)argv);
    
    auto engine = kit::make_unique<Qor>(args, Info::Program);

    //engine->max_tick(300.0f);
    //engine->max_fps(30.0f);
    
    if(engine->args().value_or("mod", "").empty())
        engine->states().register_class<BasicState>(); // run basic state
    else
        engine->states().register_class<ScriptState>(); // run python mod
    engine->run();
    return 0;
}

