#include <memory>
//#include <boost/regex.hpp>
#include <regex>
#include "Node.h"
#include "Common.h"
#include "Filesystem.h"
using namespace std;

Node :: Node(const std::string& fn):
    m_Filename(fn)
{
    if(Filesystem::getExtension(fn)=="json")
    {
        try {
            m_pConfig = make_shared<Meta>(fn);
        } catch(const Error& e) {}
    }

    init();
}

Node :: Node(const std::string& fn, ICache* cache):
    Node(fn)
{
    if(Filesystem::getExtension(fn)=="json")
    {
        try {
            m_pConfig = make_shared<Meta>(fn);
        } catch(const Error& e) {}
    }

    init();
}

void Node :: filename(const std::string& fn)
{
    m_Filename = fn;
    if(Filesystem::getExtension(fn)=="json")
    {
        try {
            m_pConfig = make_shared<Meta>(fn);
        } catch(const Error& e) {}
    }
}

void Node :: init()
{
    if(not m_pConfig)
        m_pConfig = make_shared<Meta>();
    if(m_Name.empty())
        TRY(m_Name = m_pConfig->at<string>("name"));
        
    m_WorldTransform = [this]() -> glm::mat4 {
        if(parent_c())
            return *parent_c()->matrix_c(Space::WORLD) * *matrix_c();
        else
            return *matrix_c();
    };
    m_WorldBox = std::bind(&Node::calculate_world_box, this);
}

void Node :: clear_snapshots()
{
    m_Snapshots.clear();
}

void Node :: snapshot()
{
    m_Snapshots.emplace_back(kit::make_unique<Snapshot>(
        m_Transform,
        m_WorldTransform.get(),
        m_Box,
        m_WorldBox.get()
    ));
}

Node::Snapshot* Node :: snapshot(unsigned idx)
{
    if(m_Snapshots.empty() || idx >= m_Snapshots.size())
        return nullptr;
    return m_Snapshots[idx].get();
}

void Node :: restore_snapshot(unsigned idx)
{
    if(m_Snapshots.empty() || idx >= m_Snapshots.size())
        return;
    m_Transform = m_Snapshots[idx]->transform;
    clear_snapshots();
}

void Node :: parents(std::queue<const Node*>& q, bool include_self) const
{
    const Node* parent = m_pParent;
    if(include_self)
        q.push(this);
    while(parent)
    {
        q.push(parent);
        parent = parent->parent_c();
    }
}

void Node :: parents(std::stack<const Node*>& s, bool include_self) const
{
    const Node* parent = m_pParent;
    if(include_self)
        s.push(this);
    while(parent)
    {
        s.push(parent);
        parent = parent->parent_c();
    }
}

void Node :: parents(std::queue<Node*>& q, bool include_self)
{
    Node* parent = m_pParent;
    if(include_self)
        q.push(this);
    while(parent)
    {
        q.push(parent);
        parent = parent->parent();
    }
}

void Node :: parents(std::stack<Node*>& s, bool include_self)
{
    Node* parent = m_pParent;
    if(include_self)
        s.push(this);
    while(parent)
    {
        s.push(parent);
        parent = parent->parent();
    }
}

const glm::mat4* Node :: matrix_c(Space s) const
{
    assert(s != Space::LOCAL); // this would be identity

    if(s == Space::PARENT)
        return matrix_c();

    return &m_WorldTransform();
}


glm::vec3 Node :: position(Space s) const
{
    assert(s != Space::LOCAL);
    if(s == Space::PARENT)
        return Matrix::translation(m_Transform);
    else if (s == Space::WORLD)
        return Matrix::translation(*matrix_c(Space::WORLD));
    assert(false);
    return glm::vec3(0.0f);
}

void Node :: position(const glm::vec3& v, Space s)
{
    assert(s != Space::LOCAL); // didn't you mean parent?
    assert(s != Space::WORLD);
    Matrix::translation(m_Transform, v);
    pend();
    on_move();
}

void Node :: move(const glm::vec3& v, Space s)
{
    if(s == Space::LOCAL)
        Matrix::translate(m_Transform, Matrix::orientation(m_Transform) * v);
    else if(s == Space::WORLD)
        Matrix::translate(m_Transform, orient_from_world(v, Space::LOCAL));
    else
        Matrix::translate(m_Transform, v);
    pend();
    on_move();
}

glm::vec3 Node :: velocity() const
{
    return m_Velocity;
}

void Node :: velocity(const glm::vec3& v)
{
    m_Velocity = v;
}

glm::vec3 Node :: acceleration()
{
    return m_Acceleration;
}

void Node :: acceleration(const glm::vec3& v)
{
    m_Acceleration = v;
}

void Node :: scale(glm::vec3 v, Space s)
{
    assert(s != Space::WORLD);
    switch(s)
    {
        case Space::LOCAL:
            m_Transform *= glm::scale(v);
            break;
        case Space::PARENT:
            m_Transform = glm::scale(v) * m_Transform;
            break;
        default:
            break;
    }
    pend();
    on_move();
}

void Node :: rescale(glm::vec3 v)
{
    Matrix::rescale(m_Transform, v);
    pend();
    on_move();
}

void Node :: scale(float f, Space s)
{
    scale(glm::vec3(f,f,f), s);
}

void Node :: rescale(float f)
{
    Matrix::rescale(m_Transform, f);
    pend();
    on_move();
}

void Node :: rotate(float turns, const glm::vec3& v, Space s)
{
    assert(s != Space::WORLD);
    switch(s)
    {
        case Space::LOCAL:
            m_Transform *= glm::rotate(turns * float(K_TAU), v);
            break;
        case Space::PARENT:
            m_Transform = glm::rotate(turns * float(K_TAU), v) * m_Transform;
            break;
        default:
            break;
    }
    pend();
    on_move();
}

void Node :: set_render_matrix(Pass* pass) const
{
    pass->matrix(matrix_c(Space::WORLD));
}

void Node :: render(Pass* pass) const
{
    if(m_bVisible)
    {
        bool self_vis = m_bSelfVisible;
        if(self_vis)
        {
            set_render_matrix(pass);
            before_render(pass);
            before_render_self(pass);
            render_self(pass);
            after_render_self(pass);
        }

        // render children
        if(pass && pass->recursive())
            for(const auto& c: m_Children)
                c->render(pass);
        if(self_vis)
            after_render(pass);
    }
}

void Node :: logic(Freq::Time t)
{
    if(m_bDetach) {
        detach();
        return;
    }
    auto self(shared_from_this()); // protect on_tick detach() calls killing pointer
    Actuation::logic(t);
    if(self.unique())
        return;
    kit::clear(self);
    
    logic_self(t);

    glm::vec3 new_vel;
    bool accel = false;
    if(m_Acceleration != glm::vec3(0.0f)){
        new_vel = m_Velocity;
        m_Velocity += m_Acceleration/2.0f * t.s();
        accel = true;
        new_vel += m_Acceleration * t.s();
    }
    
    if(m_Velocity != glm::vec3(0.0f)) {
        //clear_snapshots();
        //snapshot();
        move(m_Velocity * t.s());
    }

    if(accel)
        m_Velocity = new_vel;
    
    m_ChildrenCopy = m_Children;
    for(const auto& c: m_ChildrenCopy)
        c->logic(t);
    m_ChildrenCopy.clear();
}

//void Node :: render_from(const glm::mat4& view_matrix, IPartitioner* partitioner, unsigned int flags) const
//{
//    // Grab queue of parents connected this node
//    std::queue<const Node*> parents;
//    parents(parents);

//    // Multiply GL matrix by inverse of self matrix
//    //glm::mat4 m(glm::mat4::INVERSE, *matrix_c());
//    glm::mat4 m = glm::inverse(*matrix_c());
//    glMultMatrixf(glm::value_ptr(m));

//    // Now go through parents in order from this node (camera) to world space
//    const Node* parent = nullptr;
//    while(!parents.empty())
//    {
//        parent = parents.front();
//        parents.pop();

//        // multiply by inverse of each parent
//        //m.clear(glm::mat4::INVERSE, *parent->matrix_c());
//        m = glm::inverse(*parent->matrix_c());
//        glMultMatrixf(glm::value_ptr(m));
//    }

//    //if(partitioner)
//    //    partitioner->refreshFrustum();
//}

const Node* Node :: find_c(const Node* n) const
{
    assert(n);
    if(n->parent_c() == this)
        return n;

    for(auto itr = m_Children.cbegin();
        itr != m_Children.cend();
        ++itr)
    {
        const Node* temp = (*itr)->find_c(n);
        if(temp)
            return temp;
    }

    //foreach(const Node* child, m_Children)
    //    child->find_c(n);

    return nullptr;
}

Node* Node :: find(Node* n)
{
    assert(n);
    if(n->parent_c() == this)
        return n;

    for(auto itr = m_Children.begin();
        itr != m_Children.end();
        ++itr)
    {
        Node* temp = (*itr)->find(n);
        if(temp)
            return temp;
    }

    //foreach(Node* child, m_Children)
    //    child->find(n);
    return nullptr;
}

//Node* Node ::add(Node* n)
//{
//    std::shared_ptr<Node> np(n);
//    return add(np);
//}

Node* Node :: add(const std::shared_ptr<Node>& n)
{
    assert(n);
    assert(this != n.get()); // can't add to self
    if(n->parent())
        n->detach();

    n->_set_parent(this);
    n->layer(m_Layer);
    m_Children.push_back(n);
    n->pend();
    n->on_add();
    return n.get();
}

Node* Node :: stick(const std::shared_ptr<Node>& n)
{
    add(n);
    n->collapse(Space::WORLD);
    return n.get();
}

bool Node :: remove(Node* n, unsigned int flags)
{
    //assert(flags & PRESERVE); // deprecated: use preserve() instead

    for(auto itr = m_Children.begin();
        itr != m_Children.end();
    ){
        if(itr->get() == n)
        {
            //if(!(flags & PRESERVE))
            //    (*itr)->remove_all();

            //_onRemove(itr->get());
            //Node* delete_me = itr->get();
            itr = m_Children.erase(itr);

            //if(!(flags & PRESERVE))
            //    delete delete_me;

            return true;
        }
        else if(flags & SEARCH_SUBNODES)
        {
            // recursively search subnodes for node to remove
            if((*itr)->remove(n,flags))
                return true;
        }
        ++itr;
    }

    return false;
}

//std::shared_ptr<Node> Node :: preserve(Node* n, unsigned int flags)
//{
//    std::shared_ptr<Node> preserved_node;

//    for(auto itr = m_Children.begin();
//        itr != m_Children.end();
//        ++itr)
//    {
//        if(itr->get() == n)
//        {
//            (*itr)->remove_all(); // remove children
//            //_onRemove(itr->get());
//            (*itr)->_set_parent(nullptr);
//            preserved_node = *itr;
//            m_Children.erase(itr);
//            return preserved_node;
//        }
//        else if(flags & SEARCH_SUBNODES)
//        {
//            // recursively search subnodes for node to remove
//            if((preserved_node = (*itr)->preserve(n)))
//                return preserved_node;
//        }
//    }

//    return preserved_node;
//}

//unsigned int Node :: remove(Node* n, unsigned int flags)
//{
//    // keep track of # of removed nodes
//    int removed = 0;

//    for(list<Node*>::iterator itr = m_Children.begin();
//        itr != m_Children.end();
//        )
//    {
//        // SEARCH_SUBNODES flag: search subnodes for node to remove
//        if(*itr != n)
//        {
//            if(flags & SEARCH_SUBNODES)
//                removed += (*itr)->remove(n,flags);
//        }
//        else
//        {
//            (*itr)->removeAll();
//            onRemove(*itr);
//            //(*itr)->removeReference();
//            itr = m_Children.erase(itr);
//            removed++;
//
//            continue; // avoid itr++
//        }

//        ++itr; // increment here to avoid erase() itr invalidation
//    }

//    return removed;
//}

void Node :: remove_all(unsigned int flags)
{
   //assert(! (flags & PRESERVE));

   for(auto itr = m_Children.begin();
       itr != m_Children.end();
       )
   {
       //_onRemove(itr->get());
       //Node* delete_me = itr->get();
       itr = m_Children.erase(itr);
       //delete delete_me;
   }

   m_Children.clear();
}

//void Node :: removeAll(list<Node*>& removed_nodes, unsigned int flags)
//{
//    assert(flags & PRESERVE);

//    for(auto itr = m_Children.begin();
//        itr != m_Children.end();
//        )
//    {
//        _onRemove(itr->get());
//        removed_nodes.push_back(itr->get()); // preserve (still contains subnodes!)
//        itr = m_Children.erase(itr);
//    }

//    m_Children.clear();
//}


//void Node :: removeAll(list<shared_ptr<Node>>& removed_nodes, unsigned int flags)
//{
//    assert(flags & PRESERVE);

//    for(auto itr = m_Children.begin();
//        itr != m_Children.end();
//        )
//    {
//        _onRemove(itr->get());
//        removed_nodes.push_back(itr); // preserve (still contains subnodes!)
//        itr = m_Children.erase(itr);
//    }

//    m_Children.clear();
//}

void Node :: safe_detach()
{
    m_bDetach = true;
}

bool Node :: detach()
{
    bool b = false;
    if(m_pParent) {
        b = m_pParent->remove(this);
        m_pParent = nullptr;
    }
    return b;
}

void Node :: collapse(Space s, unsigned int flags)
{
    if(s == Space::PARENT)
    {
        if(!parent()) {// if this node is root, exit
            WARNING("attempt to collapse root node");
            return;
        }
        if(!parent()->parent()) {// if parent is root, exit
            WARNING("node already collapsed");
            return;
        }

        // dettach self from parent
        //parent()->remove(this, PRESERVE);
        //std::shared_ptr<Node> preserve_me = parent()->preserve(this);
        // Combine transformations
        auto self(shared_from_this());
        m_Transform = *parent()->matrix() * m_Transform;
        auto old_parent = self->parent();
        self->detach();

        // reassign parent
        old_parent->parent()->add(self);

        //_setParent(parent()->parent()); // bad

        assert(m_pParent); // if this node becomes root, we've done something wrong
    }
    else if(s == Space::WORLD)
    {
        if(!parent()){
            WARNING("node already collapsed");
            return;
        }

        while(parent()->parent())
            collapse(Space::PARENT);
    }
    else {
        WARNING("Collapsing a node to local space has no effect.");
    }
}

//bool Node :: remove()
//{
//    if(m_pParent)
//    {
//        m_pParent->remove(this);
//        return true;
//    }
//    return false;
//}

glm::vec3 Node :: to_world(glm::vec3 point, Space s) const
{
    assert(s != Space::WORLD);
    glm::vec3 r;
    if(s == Space::LOCAL)
        r = Matrix::mult(*matrix(Space::WORLD), point);
    else if(s == Space::PARENT)
    {
        glm::mat4 pmat = m_pParent ?
            *m_pParent->matrix(Space::WORLD) :
            glm::mat4(1.0f);
        r = Matrix::mult(pmat, point);
    }
    //std::queue<const Node*> ps;
    //parents(ps, s == Space::LOCAL);
    //while(!ps.empty())
    //{
    //    point = Matrix::mult(*ps.front()->matrix_c(), point);
    //    ps.pop();
    //}
    return r;
}

glm::vec3 Node :: from_world(glm::vec3 point, Space s) const
{
    assert(s != Space::WORLD);
    glm::vec3 r;
    if(s == Space::LOCAL)
        r = Matrix::mult(glm::inverse(*matrix(Space::WORLD)), point);
    else if(s == Space::PARENT)
    {
        glm::mat4 pmat = m_pParent ?
            *m_pParent->matrix(Space::WORLD) :
            glm::mat4(1.0f);
        r = Matrix::mult(glm::inverse(pmat), point);
    }
    //assert(s != Space::WORLD);
    //std::stack<const Node*> ps;
    //parents(ps, s == Space::LOCAL);
    //while(!ps.empty())
    //{
    //    point = Matrix::mult(glm::inverse(*ps.top()->matrix_c()), point);
    //    ps.pop();
    //}
    //return point;
    return r;
}

glm::vec3 Node :: orient_to_world(glm::vec3 vec, Space s) const
{
    assert(s != Space::WORLD);
    glm::vec3 r;
    if(s == Space::LOCAL)
        r = Matrix::mult(*matrix(Space::WORLD), glm::vec4(vec,0.0f));
    else if(s == Space::PARENT)
    {
        glm::mat4 pmat = m_pParent ?
            *m_pParent->matrix(Space::WORLD) :
            glm::mat4(1.0f);
        r = Matrix::mult(pmat, glm::vec4(vec,0.0f));
    }
    return r;
}

glm::vec3 Node :: orient_from_world(glm::vec3 vec, Space s) const
{
    assert(s != Space::WORLD);
    glm::vec3 r;
    if(s == Space::LOCAL)
        r = Matrix::mult(glm::inverse(*matrix(Space::WORLD)), glm::vec4(vec,0.0f));
    else if(s == Space::PARENT)
    {
        glm::mat4 pmat = m_pParent ?
            *m_pParent->matrix(Space::WORLD) :
            glm::mat4(1.0f);
        r = Matrix::mult(glm::inverse(pmat), glm::vec4(vec,0.0f));
    }
    return r;
}

void Node :: cache() const
{
    m_WorldBox.ensure();
    cache_transform();
    for(const auto& c: m_Children)
        c->cache();
}

void Node :: cache_transform() const
{
    m_WorldTransform.ensure();
}

void Node :: each(const std::function<void(Node*)>& func, unsigned flags, LoopCtrl* lc)
{
    if(lc) *lc = LC_STEP;
    
    if(flags & Each::INCLUDE_SELF)
        func(this);
    
    if((lc && *lc == LC_BREAK) || (flags & Each::STOP_RECURSION))
        return;
    
    if(not lc || *lc != LC_SKIP)
    {
        unsigned new_flags = flags | Each::INCLUDE_SELF;
        if(not (flags & Each::RECURSIVE))
            new_flags |= Each::STOP_RECURSION;

        for(const auto& c: m_Children)
            c->each(func, new_flags, lc);
    }
}

void Node :: each(const std::function<void(const Node*)>& func, unsigned flags, LoopCtrl* lc) const
{
    if(lc) *lc = LC_STEP;
    
    if(flags & Each::INCLUDE_SELF)
        func(this);
    
    if((lc && *lc == LC_BREAK) || (flags & Each::STOP_RECURSION))
        return;
    
    if(not lc || *lc != LC_SKIP)
    {
        unsigned new_flags = flags | Each::INCLUDE_SELF;
        if(not (flags & Each::RECURSIVE))
            new_flags |= Each::STOP_RECURSION;

        for(const auto& c: m_Children)
            ((const Node*)c.get())->each(func, new_flags, lc);
    }
}

const Box& Node :: world_box() const 
{
    return m_WorldBox();
}

Box Node :: calculate_world_box()
{
    if(m_Box.quick_full()) {
        return m_Box;
    } else if(m_Box.quick_zero()) {
        //assert(false);
        return m_Box;
    }
    
    Box world(Box::Zero());
    auto verts = m_Box.verts();
    
    for(auto& v: verts)
    {
        v = Matrix::mult(*matrix_c(Space::WORLD), v);
        world &= v;
    }
    return world;
}

void Node :: reload_config(std::string fn)
{
    m_pConfig = make_shared<Meta>(fn);
}

std::vector<Node*> Node :: hook(std::string name, unsigned flags)
{
    std::vector<Node*> r;
    if(not name.empty() && name[0] == '#')
    {
        std::vector<std::string> tags;
        boost::split(tags, name, boost::is_any_of("#"));
        each([&](Node* n){
            for(auto&& t: tags)
            {
                if(not t.empty() &&
                   n->tags().find(t) != n->tags().end())
                {
                    r.push_back(n);
                    return;
                }
            }
        }, (flags & Hook::RECURSIVE) ?
            (Each::DEFAULT_FLAGS | Each::RECURSIVE) : Each::DEFAULT_FLAGS
        );
    }
    else
    {
        if(flags & Hook::REGEX)
        {
            std::regex reg(name, std::regex_constants::extended);
            each([&](Node* n){
                if(std::regex_match(n->name(), reg))
                    r.push_back(n);
            }, Each::RECURSIVE);
        }
        else
        {
            each([&](Node* n){
                if(n->name() == name)
                    r.push_back(n);
            }, Each::RECURSIVE);
        }
    }
    return r;
}

std::vector<Node*> Node :: hook_tag(std::string tag, unsigned flags)
{
    if(tag[0] != '#')
        tag += "#";
    return hook(tag, flags);
}


std::vector<Node*> Node :: hook_if(std::function<bool(Node* n)> cb, unsigned flags)
{
    std::vector<Node*> r;
    each([&](Node* n){
        if(cb(n))
            r.push_back(n);
    }, (flags & Hook::RECURSIVE) ?
        (Each::DEFAULT_FLAGS | Each::RECURSIVE) : Each::DEFAULT_FLAGS
    );
    return r;
}

void Node :: add_tags(std::string tags)
{
    vector<string> taglist;
    boost::split(taglist, tags, boost::is_any_of("#"));
    add_tags(taglist);
}

void Node :: add_tags(std::vector<std::string> tags)
{
    for(auto&& t: tags)
    {
        if(t.empty())
            continue;
        if(t[0]=='#')
            t = t.substr(1);
        if(!has_tag(t))
            m_Tags.insert(std::move(t));
    }
}

std::vector<const Node*> Node :: visible_nodes(Camera* camera) const
{
    assert(false);
    return std::vector<const Node*>();
}

Node* Node :: subroot()
{
    if(m_pParent == nullptr)
        return this;
    if(m_pParent->parent() == nullptr)
        return this;
    return m_pParent->subroot();
}

Node* Node :: root()
{
    if(m_pParent == nullptr)
        return this;
    return m_pParent->root();
}

std::vector<Node*> Node :: descendants()
{
    std::vector<Node*> r;
    each([&r](Node* n){
        r.push_back(n);
    }, Node::Each::INCLUDE_SELF | Node::Each::RECURSIVE);
    return r;
}

void Node :: lazy_logic(Freq::Time t)
{
    lazy_logic_self(t);
    
    m_ChildrenCopy = m_Children;
    for(const auto& c: m_ChildrenCopy)
        c->lazy_logic(t);
    m_ChildrenCopy.clear();
}

