#ifndef _STATE_H
#define _STATE_H

#include "IRealtime.h"
#include "IRenderable.h"
#include "IPreloadable.h"
#include "ISceneGraph.h"
#include "IPhysical.h"

class State:
    public Actuation,
    public IRenderable,
    public IPreloadable,
    public ISceneGraph,
    public IPhysical
{
    public:
        virtual ~State() {}

        boost::signals2::signal<void()> on_enter;

        std::shared_ptr<Meta> meta() { return m_pMeta; }
        
    private:
        
        std::shared_ptr<Meta> m_pMeta = std::make_shared<Meta>();
};

#endif

