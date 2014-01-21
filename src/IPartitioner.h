#ifndef _PARTITIONER_H
#define _PARTITIONER_H

#include <vector>
#include "Graphics.h"

class Node;
class Light;

class IPartitioner
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

    private:
};

#endif

