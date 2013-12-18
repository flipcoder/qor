#include <iostream>
#include <memory>
#include <thread>
#include "Qor.h"
#include "ScriptState.h"
#include "BasicState.h"
#include "Interpreter.h"
using namespace std;

int main(int argc, const char** argv)
{
    //try{
        auto engine = std::make_shared<Qor>(argc, argv);
        //engine->states().register_class<BasicState>();
        engine->states().register_class<ScriptState>();
        engine->run();
    //}catch(const std::exception& e){
    //    assert(false);
    //    return 1;
    //}
    return 0;
}

