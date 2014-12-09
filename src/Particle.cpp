#include "Particle.h"

Particle :: Particle(std::string fn, Cache<Resource, std::string>* cache)
{
    
}

void Particle :: logic_self(Freq::Time t)
{
    
}

void Particle :: render_self(Pass* pass) const
{
    
}

Particle :: ~Particle()
{
    
}

ParticleSystem :: ParticleSystem(std::string fn, Cache<Resource, std::string>* cache)
{
    
}

void ParticleSystem :: logic_self(Freq::Time t)
{
    m_Accum += t;
}

void ParticleSystem :: lazy_logic(Freq::Time t)
{
    // ...
    
    m_Accum = Freq::Time(0);
}

void ParticleSystem :: render_self(Pass* pass) const
{
    auto t = m_Timeline.logic(m_Accum);
    const_cast<ParticleSystem*>(this)->lazy_logic(t);
}

ParticleSystem :: ~ParticleSystem()
{
    
}

