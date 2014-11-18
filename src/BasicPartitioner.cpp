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
    }, Node::Each::RECURSIVE);
    
    if(node_idx >= sz)
        m_Nodes.resize(max<unsigned>(512, sz*2));
    if(light_idx >= lsz)
        m_Lights.resize(max<unsigned>(16, lsz*2));

    stable_sort(m_Nodes.begin(), m_Nodes.begin() + node_idx,
        [](const Node* a, const Node* b){
            if(not floatcmp(a->layer(), b->layer()))
                return a->layer() < b->layer();
            return false;
        }
    );
    m_Nodes.at(node_idx) = nullptr;
    m_Lights.at(light_idx) = nullptr;
}

void BasicPartitioner :: logic(Freq::Time t)
{
    for(
        auto itr = m_Collisions.begin();
        itr != m_Collisions.end();
    ){
        auto a = itr->nodes[0].lock();
        if(not a) {
            itr = m_Collisions.erase(itr);
            continue;
        }
        auto b = itr->nodes[1].lock();
        if(not b) {
            itr = m_Collisions.erase(itr);
            continue;
        }
        
        if(a->world_box().collision(b->world_box())) {
            (*itr->on_collision)(a.get(), b.get());
            if(not itr->collision) {
                itr->collision = true;
                (*itr->on_enter)(a.get(), b.get());
            }
        } else {
            (*itr->on_no_collision)(a.get(), b.get());
            if(itr->collision) {
                itr->collision = false;
                (*itr->on_leave)(a.get(), b.get());
            }
        }
        
        if(a.unique() || b.unique()) {
            itr = m_Collisions.erase(itr);
            continue;
        }
        ++itr;
    }
}

void BasicPartitioner :: on_collision(
    const std::shared_ptr<Node>& a,
    const std::shared_ptr<Node>& b,
    std::function<void(Node*, Node*)> col,
    std::function<void(Node*, Node*)> no_col,
    std::function<void(Node*, Node*)> enter,
    std::function<void(Node*, Node*)> leave
){
    auto pair = Pair(a,b);
    //auto con = std::get<2>(t)->connect(cb);
    if(col) pair.on_collision->connect(col);
    if(no_col) pair.on_no_collision->connect(no_col);
    if(enter) pair.on_enter->connect(enter);
    if(leave) pair.on_leave->connect(leave);
    m_Collisions.push_back(std::move(pair));
}

vector<Node*> BasicPartitioner :: get_collisions_for(Node* n)
{
    vector<Node*> r;
    for(
        auto itr = m_Collisions.begin();
        itr != m_Collisions.end();
    ){
        auto a = itr->nodes[0].lock();
        if(not a) {
            itr = m_Collisions.erase(itr);
            continue;
        }
        
        if(a.get() == n)
        {
            auto b = itr->nodes[1].lock();
            if(not b) {
                itr = m_Collisions.erase(itr);
                continue;
            }
            
            if(a->world_box().collision(b->world_box()))
                r.push_back(b.get());
        }
        
        ++itr;
    }

    return r;
}

