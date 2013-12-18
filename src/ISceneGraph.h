#ifndef _ISCENEGRAPH_H
#define _ISCENEGRAPH_H

#include "Node.h"

class ISceneGraph
{
    public:
        virtual ~ISceneGraph() {}
        
        virtual std::shared_ptr<Node> root() {
            return std::shared_ptr<Node>();
        }
        virtual std::shared_ptr<const Node> root() const {
            return std::shared_ptr<Node>();
        }
};


#endif
