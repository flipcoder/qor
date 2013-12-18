#ifndef _STATE_H
#define _STATE_H

#include "IRealtime.h"
#include "IRenderable.h"
#include "IPreloadable.h"
#include "ISceneGraph.h"
#include "IPipeline.h"

class State:
    public IRealtime,
    public IRenderable,
    public IPreloadable,
    public ISceneGraph
{
    public:
        virtual ~State() {}
        
        virtual std::shared_ptr<IPipeline> pipeline() {
            return std::shared_ptr<IPipeline>();
        }
        virtual std::shared_ptr<const IPipeline> pipeline() const {
            return std::shared_ptr<IPipeline>();
        }
};

#endif

