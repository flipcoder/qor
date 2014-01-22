#include "BasicPartitioner.h"
#include "Node.h"

void BasicPartitioner :: partition(const Node* root)
{
    size_t sz = m_Nodes.size();
    m_Nodes.clear();
    //m_Nodes.reserve(sz);
    root->each([this](const Node* node){
        if(node->is_light())
            m_Nodes.push_back(node);
        else
            m_Lights.push_back((Light*)node);
    });
}

