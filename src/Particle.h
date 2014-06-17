#ifndef PARTICLE_H_FZHEOU7G
#define PARTICLE_H_FZHEOU7G

#include "Node.h"
#include "Graphics.h"
#include "kit/math/common.h"

class Particle:
    public Node
{
    public:
        Particle(std::string fn, Cache<Resource, std::string>* cache);
        virtual ~Particle();

    private:
        
};

#endif

