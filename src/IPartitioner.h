#ifndef _PARTITIONER_H
#define _PARTITIONER_H

#include <vector>
#include <functional>
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

        virtual void on_collision(
            const std::shared_ptr<Node>& a,
            const std::shared_ptr<Node>& b,
            std::function<void(Node*, Node*)> col,
            std::function<void(Node*, Node*)> no_col,
            std::function<void(Node*, Node*)> enter,
            std::function<void(Node*, Node*)> leave
        ) = 0;
        virtual void on_collision(
            const std::shared_ptr<Node>& a,
            unsigned type,
            std::function<void(Node*, Node*)> col,
            std::function<void(Node*, Node*)> no_col,
            std::function<void(Node*, Node*)> enter,
            std::function<void(Node*, Node*)> leave
        ) = 0;
        virtual void on_collision(
            unsigned type_a,
            unsigned type_b,
            std::function<void(Node*, Node*)> col,
            std::function<void(Node*, Node*)> no_col,
            std::function<void(Node*, Node*)> enter,
            std::function<void(Node*, Node*)> leave
        ) = 0;
        virtual void set_node_collision_type(
            const std::shared_ptr<Node>& a,
            unsigned type
        ) = 0;
        virtual void unset_node_collision_type(
            const std::shared_ptr<Node>& a,
            unsigned type
        ) = 0;
        virtual void unset_node_collision_types(
            const std::shared_ptr<Node>& a
        ) = 0;
        
        virtual std::vector<Node*> get_collisions_for(Node* n) = 0;
        virtual std::vector<Node*> get_collisions_for(Node* n, unsigned type) = 0;
        virtual std::vector<Node*> get_collisions_for(unsigned type_a, unsigned type_b) = 0;
        
        virtual void clear() = 0;
        virtual bool empty() const = 0;
        virtual bool has_collisions() const = 0;
        
    private:
};

#endif

