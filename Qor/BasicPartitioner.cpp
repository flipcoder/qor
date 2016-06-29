#include "BasicPartitioner.h"
#include "Node.h"
#include "Camera.h"
#include <memory>
#include <algorithm>
using namespace std;

#define MIN_NODES 128
#define MIN_LIGHTS 32

BasicPartitioner :: BasicPartitioner()
{
    m_Nodes.reserve(MIN_NODES);
    m_Lights.reserve(MIN_LIGHTS);
}

void BasicPartitioner :: partition(const Node* root)
{
    assert(m_pCamera);
    
    //size_t sz = m_Nodes.size();
    //size_t lsz = m_Lights.size();
    //unsigned node_idx=0;
    //unsigned light_idx=0;
    m_Lights.clear();
    m_Nodes.clear();
    
    Node::LoopCtrl lc = Node::LC_STEP;
    root->each([&](const Node* node) {
        //if(node->is_light())
        //    LOG("light");

        if(node->visible() && node->is_partitioner(m_pCamera))
        {
            auto subnodes = node->visible_nodes(m_pCamera);
            for(unsigned i=0;i<subnodes.size();++i)
            {
                //if(node_idx >= sz) {
                    //sz = max<unsigned>(MIN_NODES, sz*2);
                    //m_Nodes.resize(sz);
                //}
                //m_Nodes[node_idx] = subnodes[i];
                if(node->is_light())
                    m_Lights.push_back((const Light*)node);
                else
                    m_Nodes.push_back(subnodes[i]);
                //++node_idx;
            }
            //lc = Node::LC_SKIP; // skip tree
            return;
        }

        if(not m_pCamera->is_visible(node, &lc)){
            if(not node->visible())
                lc = Node::LC_SKIP;
            return;
        }
        //if(node->is_light())
        //    LOG("(2) light");
        
        // LC_SKIP when visible=true is not impl
        // so we'll reset to LC_STEP here
        lc = Node::LC_STEP;
        
        if(node->is_light()) {
            //if(light_idx >= lsz) {
            //    lsz = max<unsigned>(MIN_LIGHTS, lsz*2);
            //    m_Lights.resize(lsz);
            //}
            //m_Lights[light_idx] = (const Light*)node;
            //++light_idx;
            m_Lights.push_back((const Light*)node);
        } else {
            //if(node_idx >= sz) {
            //    sz = max<unsigned>(MIN_NODES, sz*2);
            //    m_Nodes.resize(sz);
            //}
            //m_Nodes[node_idx] = node;
            //++node_idx;
            m_Nodes.push_back(node);
        }
    }, Node::Each::RECURSIVE | Node::Each::INCLUDE_SELF, &lc);

    //if(node_idx >= sz)
    //    m_Nodes.resize(max<unsigned>(MIN_NODES, sz*2));
    //if(light_idx >= lsz)
    //    m_Lights.resize(max<unsigned>(MIN_LIGHTS, lsz*2));

    stable_sort(ENTIRE(m_Nodes),//.begin(), m_Nodes.begin() + node_idx,
        [](const Node* a, const Node* b){
            if(not a && b)
                return false;
            if(not b && a)
                return true;
            if(not a && not b)
                return true;
            if(not floatcmp(a->layer(), b->layer()))
                return a->layer() < b->layer();
            return false;
        }
    );
    
    // mark endpoints
    //m_Nodes[node_idx] = nullptr;
    //m_Lights[light_idx] = nullptr;
}

void BasicPartitioner :: lazy_logic(Freq:: Time t)
{
    
}

void BasicPartitioner :: logic(Freq::Time t)
{
    ++m_Recur;
    
    vector<shared_ptr<bool>> unset;
    
    // check 1-to-1 collisions
    for(
        auto itr = m_Collisions.begin();
        itr != m_Collisions.end();
    ){
        if(not *itr->recheck){
            ++itr;
            continue;
        }
        
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
                itr->on_touch(a.get(), b.get());
            }
        } else {
            itr->on_no_collision(a.get(), b.get());
            if(itr->collision) {
                itr->collision = false;
                itr->on_untouch(a.get(), b.get());
            }
        }
        
        if(a.unique() || b.unique()) {
            itr = m_Collisions.erase(itr);
            continue;
        }

        *itr->recheck = false;
        ++itr;
    }
    
    // check type collisions
    for(
        auto itr = m_TypedCollisions.begin();
        itr != m_TypedCollisions.end();
    ){
        unsigned type = itr->b;
        if(m_Objects.size() <= type){
            ++itr;
            continue;
        }
        
        if(not *itr->recheck && not *m_Objects[type].recheck){
            ++itr;
            continue;
        }
        unset.push_back(m_Objects[type].recheck);
        
        auto a = itr->a.lock();
        if(not a) {
            itr = m_TypedCollisions.erase(itr);
            continue;
        }

        auto pcs = get_potentials(a.get(), type);
        
        unsigned collisions = 0;
        //for(auto jtr = m_Objects[type].objects.begin();
        //    jtr != m_Objects[type].objects.end();
        //){
        for(auto jtr = pcs.begin(); jtr != pcs.end();)
        {
            auto b = jtr->lock();
            //if(not b) {
            //    jtr = m_Objects[type].objects.erase(jtr);
            //    continue;
            //}
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
                itr->on_touch(a.get(), nullptr);
            else
                itr->on_untouch(a.get(), nullptr);
        }
        
        if(a.unique()) {
            itr = m_TypedCollisions.erase(itr);
            continue;
        }
        
        *itr->recheck = false;
        ++itr;
    }

    // check intertype collisions
    for(
        auto itr = m_IntertypeCollisions.begin();
        itr != m_IntertypeCollisions.end();
    ){
        auto type_a = itr->a;
        auto type_b = itr->b;

        if(not *m_Objects[type_a].recheck && not *m_Objects[type_b].recheck){
            ++itr;
            continue;
        }
        unset.push_back(m_Objects[type_a].recheck);
        unset.push_back(m_Objects[type_b].recheck);
        
        for(auto jtr = m_Objects[type_a].objects.begin();
            jtr != m_Objects[type_a].objects.end();
        ){
            unsigned collisions = 0;
            auto a = jtr->lock();
            if(not a) {
                jtr = m_Objects[type_a].objects.erase(jtr);
                continue;
            }
            auto pcs = get_potentials(a.get(), type_b);
            for(auto htr = pcs.begin();
                htr != pcs.end()
            ;){
            //auto pcs = get_potentials(a.get(), type_b)
            //for(auto htr = m_Objects[type_b].objects.begin();
            //    htr != m_Objects[type_b].objects.end();
            //){
                auto b = htr->lock();
                //if(not b) {
                //    htr = m_Objects[type_b].objects.erase(htr);
                //    continue;
                //}
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
                    itr->on_touch(a.get(), nullptr);
                else
                    itr->on_untouch(a.get(), nullptr);
            }
            ++jtr;
        }
        ++itr;
    }
    
    for(auto& unset_me: unset)
        *unset_me = false;

    --m_Recur;
    
    if(m_Recur == 0){
        for(auto&& func: m_Pending)
            func();
        m_Pending.clear();
    }
}

std::vector<std::weak_ptr<Node>> BasicPartitioner :: get_potentials(
    Node* n, unsigned typ
){
    auto potentials = m_Objects[typ].objects;
    auto pcs_itr = m_Providers.find(typ);
    if(pcs_itr != m_Providers.end()){
        auto more = pcs_itr->second(n->world_box());
        std::copy(ENTIRE(more), back_inserter(potentials));
    }
    return potentials;
}

void BasicPartitioner :: register_provider(unsigned type,
    std::function< 
        std::vector<std::weak_ptr<Node>> (Box)
    > func)
{
    m_Providers[type] = func;
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
            itr = m_Collisions.erase(itr);
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
    auto pcs = get_potentials(n, type);
    for(auto itr = pcs.begin();
        itr != pcs.end();
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
    const std::shared_ptr<Node>& b,
    std::function<void(Node*, Node*)> col,
    std::function<void(Node*, Node*)> no_col,
    std::function<void(Node*, Node*)> touch,
    std::function<void(Node*, Node*)> untouch
){
    auto pair = Pair<weak_ptr<Node>, weak_ptr<Node>>(a,b);
    if(col) pair.on_collision.connect(col);
    if(no_col) pair.on_no_collision.connect(no_col);
    if(touch) pair.on_touch.connect(touch);
    if(untouch) pair.on_untouch.connect(untouch);
    m_Collisions.push_back(std::move(pair));
    
    auto rc = std::weak_ptr<bool>(m_Collisions.back().recheck);
    auto cb = [rc]{ TRY(*std::shared_ptr<bool>(rc) = true;); };
    a->on_pend.connect(cb);
    a->on_free.connect(cb);
    b->on_pend.connect(cb);
    a->on_free.connect(cb);
}

void BasicPartitioner :: on_touch(
    const std::shared_ptr<Node>& a,
    const std::shared_ptr<Node>& b,
    std::function<void(Node*, Node*)> touch
){
    return on_collision(
        a,b,
        std::function<void(Node*, Node*)>(),
        std::function<void(Node*, Node*)>(),
        touch
    );
}

void BasicPartitioner :: on_collision(
    const std::shared_ptr<Node>& a,
    unsigned type,
    std::function<void(Node*, Node*)> col,
    std::function<void(Node*, Node*)> no_col,
    std::function<void(Node*, Node*)> touch,
    std::function<void(Node*, Node*)> untouch
){  
    if(type>=m_Objects.size()) m_Objects.resize(type+1);
    auto pair = Pair<weak_ptr<Node>, unsigned>(a,type);
    if(col) pair.on_collision.connect(col);
    if(no_col) pair.on_no_collision.connect(no_col);
    if(touch) pair.on_touch.connect(touch);
    if(untouch) pair.on_untouch.connect(untouch);
    m_TypedCollisions.push_back(std::move(pair));

    auto rc = std::weak_ptr<bool>(m_TypedCollisions.back().recheck);
    auto cb = [rc]{ TRY(*std::shared_ptr<bool>(rc) = true;); };
    a->on_pend.connect(cb);
    a->on_free.connect(cb);
}

void BasicPartitioner :: on_collision(
    unsigned type_a,
    unsigned type_b,
    std::function<void(Node*, Node*)> col,
    std::function<void(Node*, Node*)> no_col,
    std::function<void(Node*, Node*)> touch,
    std::function<void(Node*, Node*)> untouch
){
    if(type_a>=m_Objects.size() || type_b>=m_Objects.size())
        m_Objects.resize(std::max(type_a, type_b)+1);
    auto pair = Pair<unsigned, unsigned>(type_a,type_b);
    if(col) pair.on_collision.connect(col);
    if(no_col) pair.on_no_collision.connect(no_col);
    if(touch) pair.on_touch.connect(touch);
    if(untouch) pair.on_untouch.connect(untouch);
    m_IntertypeCollisions.push_back(std::move(pair));
}

void BasicPartitioner :: register_object(
    const std::shared_ptr<Node>& a,
    unsigned type
){
    auto func = [&]{
        if(type>=m_Objects.size()) m_Objects.resize(type+1);
        m_Objects[type].objects.emplace_back(a);
        auto rc = std::weak_ptr<bool>(m_Objects[type].recheck);
        auto cb = [rc]{ TRY(*std::shared_ptr<bool>(rc) = true;); };
        a->on_pend.connect(cb);
        a->on_free.connect(cb);
    };
    if(m_Recur)
        m_Pending.push_back(func);
    else
        func();
}

void BasicPartitioner :: deregister_object(
    const std::shared_ptr<Node>& a,
    unsigned type
){
}

void BasicPartitioner :: deregister_object(
    const std::shared_ptr<Node>& a
){
    // remove all object->object collision pairs involving object
    kit::remove_if(m_Collisions, [a](const Pair<std::weak_ptr<Node>, std::weak_ptr<Node>>& p){
        auto ap = p.a.lock();
        auto bp = p.b.lock();
        return
            not ap ||
            not bp ||
            a == ap ||
            a == bp;
            
    });
}

void BasicPartitioner :: after(std::function<void()> func)
{
    m_Pending.push_back(func);
}

