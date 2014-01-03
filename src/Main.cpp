#include <iostream>
#include <memory>
#include <thread>
#include "kit/kit.h"
#include "Qor.h"
#include "ScriptState.h"
#include "BasicState.h"
#include "Interpreter.h"
using namespace std;
using namespace kit;

int main(int argc, const char** argv)
{
    auto engine = kit::make_unique<Qor>(argc, argv);
    engine->states().register_class<ScriptState>();
    engine->run();
    return 0;
}

