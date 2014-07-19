#include "BasicPartitioner.h"
#include "Node.h"
#include <memory>
#include <algorithm>
using namespace std;

BasicPartitioner :: BasicPartitioner()
{
    m_Nodes.resize(65536);
    m_Lights.resize(16);
}

void BasicPartitioner :: partition(const Node* root)
{
    size_t sz = m_Nodes.size();
    size_t lsz = m_Lights.size();
    unsigned node_idx=0;
    unsigned light_idx=0;
    root->each([&](const Node* node){
        if(node_idx >= sz) {
            sz = max<unsigned>(65536, sz*2);
            m_Nodes.resize(sz);
        }
        if(light_idx >= lsz) {
            lsz = max<unsigned>(16, lsz*2);
            m_Lights.resize(lsz);
        }
        if(!node->is_light()) {
            m_Nodes.at(node_idx) = node;
            ++node_idx;
        } else {
            m_Lights.at(light_idx) = (const Light*)node;
            ++light_idx;
        }
    });
    
    if(node_idx >= sz)
        m_Nodes.resize(max<unsigned>(256, sz*2));
    if(light_idx >= lsz)
        m_Lights.resize(max<unsigned>(16, lsz*2));

    m_Nodes.at(node_idx) = nullptr;
    m_Lights.at(light_idx) = nullptr;
}

