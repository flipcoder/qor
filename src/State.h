#ifndef _STATE_H
#define _STATE_H

#include "IRealtime.h"
#include "IRenderable.h"
#include "IPreloadable.h"
#include "ISceneGraph.h"
#include "Pipeline.h"

class State:
    public IRealtime,
    public IRenderable,
    public IPreloadable,
    public ISceneGraph
{
    public:
        virtual ~State() {}
        
        virtual std::shared_ptr<Pipeline> pipeline() {
            return std::shared_ptr<Pipeline>();
        }
        virtual std::shared_ptr<const Pipeline> pipeline() const {
            return std::shared_ptr<Pipeline>();
        }
};

#endif

