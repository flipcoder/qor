#ifndef _ISCENEGRAPH_H
#define _ISCENEGRAPH_H

#include "Node.h"
#include "Camera.h"

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
        virtual std::shared_ptr<Node> camera() {
            return std::shared_ptr<Node>();
        }
        virtual std::shared_ptr<const Node> camera() const {
            return std::shared_ptr<Node>();
        }
        virtual void camera(const std::shared_ptr<Node>& camera) {}
};


#endif
