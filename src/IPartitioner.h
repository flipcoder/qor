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

    //virtual std::vector<Light*> getViewableLights(Node* root) = 0;
    //virtual std::vector<Node*> getViewableNodes(Node* root) = 0;
    //virtual std::vector<Node*> getLitObjects(Light* light, Node* root) = 0;
    //virtual std::vector<Node*> getLitObjects(Light* light, std::vector<Node*>& node_vector) = 0;

    //virtual Space getBoundingSpace() { return Space::LOCAL; }
private:
};

#endif

