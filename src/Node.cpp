#include "Node.h"
#include "Common.h"
using namespace std;

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

    // TODO
    // assuming world space...

    if(m_bTransformPendingCache)
    {
        // if not root
        if(parent_c())
            m_TransformCache = *parent_c()->matrix_c(Space::WORLD) * *matrix_c();
            //m_TransformCache = *matrix_c() * *parent_c()->matrix_c(Space::WORLD); 
        else
            m_TransformCache = *matrix_c();

        m_bTransformPendingCache = false;

        //glm::vec3 pos = position(Space::WORLD);
        //cout << "pending " <<
        //    pos.x << ", " <<
        //    pos.y << ", " <<
        //    pos.z <<
        //    endl;
    }

    return &m_TransformCache;
    //stack<const Node*> parents;
    //parents(parents, INCLUDE_SELF);
    //Node* parent = m_pParent;
    //glm::mat4 matrix;
    //while(parent != nullptr)
    //{
    //    foreach(const Node* n, *parents)
    //}
}


glm::vec3 Node :: position(Space s) const
{
    assert(s != Space::LOCAL); // alert, you probably meant PARENT

    if(s == Space::PARENT)
        return Matrix::translation(m_Transform);
    else if (s == Space::WORLD)
    {
        return Matrix::translation(*matrix_c(Space::WORLD));

        //// TODO: Move this local-to-world crap to an actual Node method
        ////       and fix it because it looks broken

        ////glm::vec3 v(0.0, 0.0, 0.0); // [Note] shouldn't this be pos?
        //queue<const Node*> parents;
        //parents(parents);

        ////glm::mat4 m(glm::mat4::INVERSE, *matrix_c());
        //glm::mat4 m = glm::inverse(*matrix_c());
        //// Now go through parents in order from local to world
        //const Node* parent = nullptr;
        //while(!parents.empty())
        //{
        //    parent = parents.front();
        //    parents.pop();

        //    // multiply by inverse of each parent
        //    //m.clear(glm::mat4::INVERSE, *parent->matrix_c());
        //    m *= glm::inverse(*parent->matrix_c());
        //}
        ////v *= m;
        //return Matrix::mult(glm::vec3(), m);
        ////matrix()->translate(v); <- to move in world space, do this
    }
    assert(false);
    return glm::vec3();
}

void Node :: position(const glm::vec3& v, Space s)
{
    assert(s != Space::LOCAL); // didn't you mean parent?
    assert(s != Space::WORLD); // not yet implemented

    Matrix::translation(m_Transform, v);
    pend();
}

void Node :: move(const glm::vec3& v, Space s)
{
    assert(s != Space::LOCAL); // didn't you mean parent?
    assert(s != Space::WORLD); // not yet implemented

    m_Transform = glm::translate(m_Transform, v);
    pend();

    //Matrix::translate(m_Transform, v);
    //pend();
}

void Node :: rotate(float tau, const glm::vec3& v, Space s)
{
    assert(s != Space::WORLD); // not yet implemented

    switch(s)
    {
        case Space::LOCAL:
            m_Transform *= glm::rotate(tau * 360.0f, v);
            break;
        case Space::PARENT:
            m_Transform = glm::rotate(tau * 360.0f, v) * m_Transform;
            break;
        default:
            assert(false);
    }
    pend();
}

void Node :: render(Pass* pass) const
{
    bool visible = true; // TODO: replace with function to check flag

    // render self
    if(visible)
    {
        pass->matrix(matrix_c(Space::WORLD));
        render_self(pass);
    }

    // render children
    if(pass && pass->recursive())
        for(const auto& c: m_Children)
            c->render(pass);
}

void Node :: logic(Freq::Time t)
{
     logic_self(t);

     for(const auto& c: m_Children)
        c->logic(t);
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

Node* Node ::add(std::shared_ptr<Node> n)
{
    assert(n);
    assert(this != n.get()); // can't add to self
    assert(n->parent() == nullptr); // node we're trying to add has no existing parent

    n->_set_parent(this);
    m_Children.push_back(n);
    return n.get();
}

bool Node :: remove(Node* n, unsigned int flags)
{
    //assert(flags & PRESERVE); // deprecated: use preserve() instead

    for(auto itr = m_Children.begin();
        itr != m_Children.end();
        ++itr)
    {
        if(itr->get() == n)
        {
            //if(!(flags & PRESERVE))
            (*itr)->remove_all();

            //_onRemove(itr->get());
            //Node* delete_me = itr->get();
            m_Children.erase(itr);

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
    }

    return false;
}

std::shared_ptr<Node> Node :: preserve(Node* n, unsigned int flags)
{
    std::shared_ptr<Node> preserved_node;

    for(auto itr = m_Children.begin();
        itr != m_Children.end();
        ++itr)
    {
        if(itr->get() == n)
        {
            (*itr)->remove_all(); // remove children
            //_onRemove(itr->get());
            (*itr)->_set_parent(nullptr);
            preserved_node = *itr;
            m_Children.erase(itr);
            return preserved_node;
        }
        else if(flags & SEARCH_SUBNODES)
        {
            // recursively search subnodes for node to remove
            if((preserved_node = (*itr)->preserve(n)))
                return preserved_node;
        }
    }

    return preserved_node; // should be nullptr at this point
}

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


void Node :: collapse(Space s, unsigned int flags)
{
    assert(s != Space::LOCAL);

    if(s == Space::PARENT)
    {
        if(!parent()) // if this node is root, exit
            return;
        if(!parent()->parent()) // if parent is root, exit
            return;

        // dettach self from parent
        //parent()->remove(this, PRESERVE);
        std::shared_ptr<Node> preserve_me = parent()->preserve(this);
        // Combine transformations
        m_Transform = *parent()->matrix() * m_Transform;

        // reassign parent
        parent()->parent()->add(preserve_me);

        //_setParent(parent()->parent()); // bad

        assert(m_pParent); // if this node becomes root, we've done something wrong
    }
    else if(s == Space::WORLD)
    {
        if(!parent())
            return;

        while(parent()->parent())
            collapse(Space::PARENT);
    }
}

bool Node :: remove()
{
    if(m_pParent)
    {
        m_pParent->remove(this);
        return true;
    }
    return false;
}


glm::vec3 Node :: transform_in(glm::vec3 point) const
{
    std::stack<const Node*> ps;
    parents(ps, true); // include self
    while(!ps.empty())
    {
        point = Matrix::mult(point, *ps.top()->matrix_c());
        ps.pop();
    }

    return point;
}

glm::vec3 Node :: transform_out(glm::vec3 point) const
{
    std::queue<const Node*> ps;
    parents(ps, true); // include self

    while(!ps.empty())
    {
        point = Matrix::mult(glm::inverse(*ps.front()->matrix_c()), point);
        ps.pop();
    }

    //const Node* parent = m_pParent;
    //while(parent != nullptr)
    //{
    //    point *= parent->matrix_c();
    //    parent = parent->getParent_c();
    //}
    return point;
}

void Node :: cache_transform() const
{
    if(m_bTransformPendingCache)
    {
        matrix_c(Space::WORLD);
        for(auto c: m_Children)
            const_cast<Node*>(c.get())->cache_transform();
        m_bTransformPendingCache = false;
    }
}

