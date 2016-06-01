#include <iostream>
#include <memory>
#include "Qor/kit/kit.h"
#include "Qor/Qor.h"
#include "Info.h"
#include "Game.h"
using namespace std;
using namespace kit;

int main(int argc, const char** argv)
{
    
    Args args(argc, argv);
    args.set("mod", PACKAGE);
    args.set("title", PACKAGE);
    
    Texture::DEFAULT_FLAGS = Texture::TRANS | Texture::MIPMAP;
    
#ifndef DEBUG
    try{
#endif
        auto engine = kit::make_unique<Qor>(args);
        engine->states().register_class<Game>("game");
        engine->run("game");
#ifndef DEBUG
    }catch(const Error&){
        // already logged
    }catch(const std::exception& e){
        LOGf("Uncaught exception: %s", e.what());
    }
#endif
    return 0;
}

