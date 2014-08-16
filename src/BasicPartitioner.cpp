#include "BasicPartitioner.h"
#include "Node.h"
#include <memory>
#include <algorithm>
using namespace std;

BasicPartitioner :: BasicPartitioner()
{
    m_Nodes.resize(512);
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
            sz = max<unsigned>(512, sz*2);
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
        m_Nodes.resize(max<unsigned>(512, sz*2));
    if(light_idx >= lsz)
        m_Lights.resize(max<unsigned>(16, lsz*2));

    m_Nodes.at(node_idx) = nullptr;
    m_Lights.at(light_idx) = nullptr;
}

void BasicPartitioner :: logic(Freq::Time t)
{
    for(
        auto itr = m_Collisions.begin();
        itr != m_Collisions.end();
    ){
        auto a = std::get<0>(*itr).lock();
        if(not a) {
            itr = m_Collisions.erase(itr);
            continue;
        }
        auto b = std::get<1>(*itr).lock();
        if(not b) {
            itr = m_Collisions.erase(itr);
            continue;
        }
        
        if(a->world_box().collision(b->world_box())) {
            auto& sig = (*std::get<2>(*itr));
            sig(a.get(), b.get());
        
            if(a.unique() || b.unique()) {
                itr = m_Collisions.erase(itr);
                continue;
            }
        }
        
        ++itr;
    }
}

boost::signals2::connection BasicPartitioner :: on_collision(
    std::shared_ptr<Node>& a,
    std::shared_ptr<Node>& b,
    std::function<void(Node*, Node*)> cb
){
    auto t = make_tuple(
        weak_ptr<Node>(a),
        weak_ptr<Node>(b),
        kit::make_unique<boost::signals2::signal<void(Node*,Node*)>>()
    );
    auto con = std::get<2>(t)->connect(cb);
    m_Collisions.push_back(std::move(t));
    return con;
}

