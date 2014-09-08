#ifndef _PARTITIONER_H
#define _PARTITIONER_H

#include <vector>
#include "Graphics.h"
#include "IRealtime.h"

class Node;
class Light;
class Camera;

class IPartitioner:
    public IRealtime
{
    public:
        IPartitioner() {}
        virtual ~IPartitioner() {}

        virtual void partition(const Node* root) = 0;
        virtual const std::vector<const Light*>& visible_lights() const = 0;
        virtual const std::vector<const Node*>& visible_nodes() const = 0;
        virtual const std::vector<const Node*>& visible_nodes_from(
            const Light* light
        ) const = 0;
        
        virtual void camera(Camera* camera) = 0;
        virtual const Camera* camera() const = 0;
        virtual Camera* camera() = 0;

        virtual void logic(Freq::Time) = 0;

        virtual boost::signals2::connection on_collision(
            const std::shared_ptr<Node>& a,
            const std::shared_ptr<Node>& b,
            std::function<void(Node*, Node*)> cb
        ) = 0;
        
    private:
};

#endif

