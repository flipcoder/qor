#ifndef _STATE_H
#define _STATE_H

#include "IRealtime.h"
#include "IRenderable.h"
#include "IPreloadable.h"
#include "ISceneGraph.h"

class State:
    public Actuation,
    public IRenderable,
    public IPreloadable,
    public ISceneGraph
{
    public:
        virtual ~State() {}

        boost::signals2::signal<void()> on_enter;
};

#endif

