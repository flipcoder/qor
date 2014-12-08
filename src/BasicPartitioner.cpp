#include "BasicPartitioner.h"
#include "Node.h"
#include "Camera.h"
#include <memory>
#include <algorithm>
using namespace std;

#define MIN_NODES 65536
#define MIN_LIGHTS 16

BasicPartitioner :: BasicPartitioner()
{
    m_Nodes.resize(MIN_NODES);
    m_Lights.resize(MIN_LIGHTS);
}

void BasicPartitioner :: partition(const Node* root)
{
    assert(m_pCamera);
    
    size_t sz = m_Nodes.size();
    size_t lsz = m_Lights.size();
    unsigned node_idx=0;
    unsigned light_idx=0;
    root->each([&](const Node* node){
        if(!m_pCamera->is_visible(node))
            return;
        if(K_UNLIKELY(node_idx >= sz)) {
            sz = max<unsigned>(MIN_NODES, sz*2);
            m_Nodes.resize(sz);
        }
        if(K_UNLIKELY(light_idx >= lsz)) {
            lsz = max<unsigned>(MIN_LIGHTS, lsz*2);
            m_Lights.resize(lsz);
        }
        if(not node->is_light()) {
            m_Nodes[node_idx] = node;
            ++node_idx;
        } else {
            m_Lights[light_idx] = (const Light*)node;
            ++light_idx;
        }
    }, Node::Each::RECURSIVE);
    
    if(K_UNLIKELY(node_idx >= sz))
        m_Nodes.resize(max<unsigned>(MIN_NODES, sz*2));
    if(K_UNLIKELY(light_idx >= lsz))
        m_Lights.resize(max<unsigned>(MIN_LIGHTS, lsz*2));

    stable_sort(m_Nodes.begin(), m_Nodes.begin() + node_idx,
        [](const Node* a, const Node* b){
            if(not floatcmp(a->layer(), b->layer()))
                return a->layer() < b->layer();
            return false;
        }
    );
    m_Nodes[node_idx] = nullptr;
    m_Lights[light_idx] = nullptr;
}

void BasicPartitioner :: logic(Freq::Time t)
{
    // check 1-to-1 collisions
    for(
        auto itr = m_Collisions.begin();
        itr != m_Collisions.end();
    ){
        auto a = itr->a.lock();
        if(not a) {
            itr = m_Collisions.erase(itr);
            continue;
        }
        auto b = itr->b.lock();
        if(not b) {
            itr = m_Collisions.erase(itr);
            continue;
        }
        
        if(a->world_box().collision(b->world_box())) {
            itr->on_collision(a.get(), b.get());
            if(not itr->collision) {
                itr->collision = true;
                itr->on_enter(a.get(), b.get());
            }
        } else {
            itr->on_no_collision(a.get(), b.get());
            if(itr->collision) {
                itr->collision = false;
                itr->on_leave(a.get(), b.get());
            }
        }
        
        if(a.unique() || b.unique()) {
            itr = m_Collisions.erase(itr);
            continue;
        }
        ++itr;
    }
    
    // check type collisions
    for(
        auto itr = m_TypedCollisions.begin();
        itr != m_TypedCollisions.end();
    ){
        auto a = itr->a.lock();
        if(not a) {
            itr = m_TypedCollisions.erase(itr);
            continue;
        }
        
        unsigned type = itr->b;
        if(m_Objects.size() <= type)
            continue;
        unsigned collisions = 0;
        for(auto jtr = m_Objects[type].begin();
            jtr != m_Objects[type].end();
        ){
            auto b = jtr->lock();
            if(not b) {
                jtr = m_Objects[type].erase(jtr);
                continue;
            }
            if(a->world_box().collision(b->world_box())) {
                itr->on_collision(a.get(), b.get());
                ++collisions;
            } else {
                itr->on_no_collision(a.get(), b.get());
            }
            ++jtr;
        }
        if(itr->collision != (bool)collisions)
        {
            itr->collision = (bool)collisions;
            if(collisions)
                itr->on_enter(a.get(), nullptr);
            else
                itr->on_leave(a.get(), nullptr);
        }
        
        if(a.unique()) {
            itr = m_TypedCollisions.erase(itr);
            continue;
        }
        ++itr;
    }

    // check intertype collisions
    for(
        auto itr = m_IntertypeCollisions.begin();
        itr != m_IntertypeCollisions.end();
    ){
        
        auto type_a = itr->a;
        auto type_b = itr->b;
        for(auto jtr = m_Objects[type_a].begin();
            jtr != m_Objects[type_a].end();
        ){
            unsigned collisions = 0;
            auto a = jtr->lock();
            if(not a) {
                jtr = m_Objects[type_a].erase(jtr);
                continue;
            }
            for(auto htr = m_Objects[type_b].begin();
                htr != m_Objects[type_b].end();
            ){
                auto b = htr->lock();
                if(not b) {
                    htr = m_Objects[type_b].erase(htr);
                    continue;
                }
                if(a == b) // same object
                    goto iter;

                if(a->world_box().collision(b->world_box())) {
                    itr->on_collision(a.get(), b.get());
                    ++collisions;
                } else {
                    itr->on_no_collision(a.get(), b.get());
                }
                
                iter:
                    ++htr;
            }
            if(itr->collision != (bool)collisions)
            {
                itr->collision = (bool)collisions;
                if(collisions)
                    itr->on_enter(a.get(), nullptr);
                else
                    itr->on_leave(a.get(), nullptr);
            }
            ++jtr;
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
    auto pair = Pair<weak_ptr<Node>, weak_ptr<Node>>(a,b);
    if(col) pair.on_collision.connect(col);
    if(no_col) pair.on_no_collision.connect(no_col);
    if(enter) pair.on_enter.connect(enter);
    if(leave) pair.on_leave.connect(leave);
    m_Collisions.push_back(std::move(pair));
}

vector<Node*> BasicPartitioner :: get_collisions_for(Node* n)
{
    vector<Node*> r;
    for(
        auto itr = m_Collisions.begin();
        itr != m_Collisions.end();
        ++itr
    ){
        auto a = itr->a.lock();
        if(not a) {
            // erasing may invalidate iterators of outer loops
            //itr = m_Collisions.erase(itr);
            continue;
        }
        
        if(a.get() == n)
        {
            auto b = itr->b.lock();
            if(not b) {
                itr = m_Collisions.erase(itr);
                continue;
            }
            
            if(a->world_box().collision(b->world_box()))
                r.push_back(b.get());
        }
    }

    return r;
}
std::vector<Node*> BasicPartitioner :: get_collisions_for(Node* n, unsigned type)
{
    std::vector<Node*> r;
    
    if(m_Objects.size() <= type)
        return r;
    for(auto itr = m_Objects[type].begin();
        itr != m_Objects[type].end();
        ++itr
    ){
        auto b = itr->lock();
        if(not b) {
            //itr = m_Objects[type].erase(itr);
            continue;
        }
        if(n->world_box().collision(b->world_box()))
            r.push_back(b.get());
    }

    return r;
}

std::vector<Node*> BasicPartitioner :: get_collisions_for(unsigned type_a, unsigned type_b)
{
    return std::vector<Node*>();
}


void BasicPartitioner :: on_collision(
    const std::shared_ptr<Node>& a,
    unsigned type,
    std::function<void(Node*, Node*)> col,
    std::function<void(Node*, Node*)> no_col,
    std::function<void(Node*, Node*)> enter,
    std::function<void(Node*, Node*)> leave
){
    if(type>=m_Objects.size()) m_Objects.resize(type+1);
    auto pair = Pair<weak_ptr<Node>, unsigned>(a,type);
    if(col) pair.on_collision.connect(col);
    if(no_col) pair.on_no_collision.connect(no_col);
    if(enter) pair.on_enter.connect(enter);
    if(leave) pair.on_leave.connect(leave);
    m_TypedCollisions.push_back(std::move(pair));
}

void BasicPartitioner :: on_collision(
    unsigned type_a,
    unsigned type_b,
    std::function<void(Node*, Node*)> col,
    std::function<void(Node*, Node*)> no_col,
    std::function<void(Node*, Node*)> enter,
    std::function<void(Node*, Node*)> leave
){
    if(type_a>=m_Objects.size() || type_b>=m_Objects.size())
        m_Objects.resize(std::max(type_a, type_b)+1);
    auto pair = Pair<unsigned, unsigned>(type_a,type_b);
    if(col) pair.on_collision.connect(col);
    if(no_col) pair.on_no_collision.connect(no_col);
    if(enter) pair.on_enter.connect(enter);
    if(leave) pair.on_leave.connect(leave);
    m_IntertypeCollisions.push_back(std::move(pair));
}

void BasicPartitioner :: register_object(
    const std::shared_ptr<Node>& a,
    unsigned type
){
    if(type>=m_Objects.size()) m_Objects.resize(type+1);
    m_Objects[type].emplace_back(a);
}

void BasicPartitioner :: deregister_object(
    const std::shared_ptr<Node>& a,
    unsigned type
){
}

void BasicPartitioner :: deregister_object(
    const std::shared_ptr<Node>& a
){
    
}

