#ifndef QOR_PYTHON_BINDINGS_H
#define QOR_PYTHON_BINDINGS_H

#include <boost/python.hpp>
#include "Node.h"
#include "Mesh.h"
#include "Graphics.h"
#include "Window.h"
#include "Qor.h"
#include "Interpreter.h"
#include "Sprite.h"
#include "TileMap.h"
#include "Graphics.h"
#include "Camera.h"
#include "NodeInterface.h"
#include "PlayerInterface3D.h"
#include "kit/log/log.h"
#include "kit/log/errors.h"

using namespace boost::python;

Qor* qor() {
    return (Qor*)Interpreter::current()->user_data();
}

//struct NodeGroup
//{
//    //std::vector
//};

struct NodeHook
{
    std::shared_ptr<Node> n;
    //std::weak_ptr<Node> p;
    NodeHook():
        n(std::make_shared<Node>())
    {}
    NodeHook(Node* n):
        n(n->as_node()) // Yes, this allows n==null
    {}
    NodeHook(const std::shared_ptr<Node>& p):
        n(p)
    {}
    NodeHook as_node() {
        return NodeHook(n);
    }
    virtual ~NodeHook() {}
    void rotate(float tau, list v) {
        n->rotate(
            tau,
            glm::vec3(
                extract<float>(v[0]),
                extract<float>(v[1]),
                extract<float>(v[2])
            )
        );
    }
    object get_matrix() const {
        list l;
        const float* f = glm::value_ptr(*n->matrix_c());
        for(unsigned i=0;i<16;++i)
            l.append<float>(f[i]);
        return l;
    }
    void set_matrix(list m) {
        float* f = glm::value_ptr(*n->matrix());
        for(unsigned i=0;i<16;++i)
            f[i] = extract<float>(m[i]);
        update();
    }
    void set_position(list t) {
        n->position(glm::vec3(
            extract<float>(t[0]),
            extract<float>(t[1]),
            extract<float>(t[2])
        ));
    }
    object get_position() const {
        glm::vec3 p = n->position();
        list l;
        for(unsigned i=0;i<3;++i)
            l.append<float>(p[i]);
        return l;
    }
    void move(list t) {
        n->position(n->position() + glm::vec3(
            extract<float>(t[0]),
            extract<float>(t[1]),
            extract<float>(t[2])
        ));
    }
    void detach() {
        auto p = n->parent();
        if(p)
            p->remove(n.get());
        //n = shared_ptr<Node>();
    }
    //void add(NodeHook nh) {
    //}
    //virtual std::string type() const {
    //    return "node";
    //}
    void add(NodeHook nh) {
        n->add(nh.n);
    }
    void spawn() {
        detach();
        qor()->current_state()->root()->add(n);
    }
    bool valid() const {
        return !!n;
    }
    NodeHook parent() const {
        return NodeHook(n->parent());
    }
    void update() {
        n->pend();
    }
    unsigned num_subnodes() const { return n->num_subnodes(); }
    unsigned num_children() const { return n->num_children(); }
};

struct MeshHook:
    public NodeHook
{
    MeshHook():
        NodeHook(std::static_pointer_cast<Node>(std::make_shared<Mesh>()))
    {}
    MeshHook(const std::shared_ptr<Mesh>& mesh):
        NodeHook(std::static_pointer_cast<Node>(mesh))
    {}
    MeshHook(std::string fn):
        NodeHook()
    {
        n = std::make_shared<Mesh>(qor()->nodes().create_as<Mesh::Data>(
            std::tuple<
                std::string,
                IFactory*,
                ICache*
            >(
                fn,
                &qor()->nodes(),
                qor()->resources()
            )
        ));
    }
    virtual ~MeshHook() {}
    Mesh* self() {
        return (Mesh*)n.get();
    }
    MeshHook instance() {
        return MeshHook(self()->instance());
    }
    //virtual std::string type() const {
    //    return "mesh";
    //}
};

struct CameraHook:
    public NodeHook
{
    CameraHook():
        NodeHook(std::static_pointer_cast<Node>(std::make_shared<Camera>()))
    {}
    CameraHook(const std::shared_ptr<Camera>& mesh):
        NodeHook(std::static_pointer_cast<Node>(mesh))
    {}
    virtual ~CameraHook() {}
    Camera* self() {
        return (Camera*)n.get();
    }
};

struct SpriteHook:
    public NodeHook
{
    //SpriteHook(NodeHook& nh) {
        // TODO: casting
    //}
    SpriteHook(std::string fn):
        NodeHook(std::static_pointer_cast<Node>(std::make_shared<Sprite>(
            fn,
            qor()->resources()
        )))
    {}
    SpriteHook(std::string fn, std::string skin):
        NodeHook(std::static_pointer_cast<Node>(std::make_shared<Sprite>(
            fn,
            qor()->resources(),
            skin
        )))
    {}
    SpriteHook(std::string fn, std::string skin, list pos):
        NodeHook(std::static_pointer_cast<Node>(std::make_shared<Sprite>(
            fn,
            qor()->resources(),
            skin,
            glm::vec3(
                extract<float>(pos[0]),
                extract<float>(pos[1]),
                extract<float>(pos[2])
            )
        )))
    {}
    SpriteHook(const std::shared_ptr<Sprite>& mesh):
        NodeHook(std::static_pointer_cast<Node>(mesh))
    {}
    virtual ~SpriteHook() {}
    Sprite* self() {
        return (Sprite*)n.get();
    }
    unsigned state_id(const std::string& s) { return self()->state_id(s); }
    void state(unsigned id) { self()->set_state(id); }
    void states(list& l) {
        std::vector<unsigned> v;
        auto length = len(l);
        for(unsigned i=0;i<length;++i)
            v.push_back(i);
        self()->set_all_states(v);
    }
};

struct NodeInterfaceHook
{
    std::shared_ptr<NodeInterface> n;
    NodeInterfaceHook() {}
    NodeInterfaceHook(const std::shared_ptr<NodeInterface>& p):
        n(p)
    {}
    virtual ~NodeInterfaceHook() {}
};

//struct Player2DHook:
//    public NodeInterfaceHook
//{
//    //Player2DHook():
//    //    NodeInterfaceHook(std::static_pointer_cast<NodeInterface>(
//    //        std::make_shared<Player2DInterface>(
                
//    //        )
//    //    ))
//    //{}
//    Player2DHook* self() {
//        return (Player2DHook*)n.get();
//    }
//};

struct Player3DHook:
    public NodeInterfaceHook
{
    Player3DHook(){}
    Player3DHook(NodeHook nih):
        NodeInterfaceHook(std::static_pointer_cast<NodeInterface>(
            std::make_shared<PlayerInterface3D>(
                qor()->session()->profile(0)->controller(),
                nih.n
            )
        ))
    {
        ((PlayerInterface3D*)n.get())->plug();
    }
    virtual ~Player3DHook() {}
    Player3DHook* self() {
        return (Player3DHook*)n.get();
    }
};

//NodeHook create(std::string type) {
//    return NodeHook();
//}

//NodeHook spawn(std::string type) {
//    auto n = qor()->nodes().create(type);
//    qor()->current_state()->root()->add(n);
//    return NodeHook(n);
//}

void render_from(NodeHook nh) {
    qor()->current_state()->pipeline()->camera(nh.n);
}

NodeHook camera() {
    return NodeHook(qor()->current_state()->pipeline()->camera());
}

NodeHook root() {
    assert(qor()->current_state());
    assert(qor()->current_state()->root());
    return NodeHook(qor()->current_state()->root());
    //object main = py::import("__main__");
    //Context c = extract<Context>(main.attr("context"));
}

void push_state(unsigned state) { qor()->push_state(state);}
void pop_state() { qor()->pop_state(); }
void change_state(unsigned state) { qor()->change_state(state);}
void quit() { qor()->quit(); }
void ortho(bool on) {
    qor()->current_state()->pipeline()->ortho(on);
}
//object bg_color() {
//}
void bg_color(list rgb) {
    auto c = Color(
        extract<float>(rgb[0]),
        extract<float>(rgb[1]),
        extract<float>(rgb[2])
    );
    qor()->current_state()->pipeline()->bg_color(c);
}
unsigned screen_w() {
    return qor()->window()->size().x;
}
unsigned screen_h() {
    return qor()->window()->size().y;
}
void cache(std::string fn) {
    qor()->resources()->cache(fn);
}
void optimize() {
    qor()->resources()->optimize();
}
void script_error(const std::exception& e)
{
    WARNING(e.what());
    PyErr_SetString(PyExc_RuntimeError, e.what());
}

//void restart_state() { qor()->restart_state(); }

BOOST_PYTHON_MODULE(qor)
{
    register_exception_translator<std::exception>(&script_error);
    
    //def("spawn", spawn, args("name"));
    //def("create", create, args("name"));
    def("root", root);
    def("camera", camera);
    def("push_state", push_state, args("state"));
    def("pop_state", pop_state);
    def("change_state", change_state, args("state"));
    def("quit", quit);
    def("ortho", ortho, args("on"));
    def("bg_color", bg_color, args("rgb"));
    def("render_from", render_from, args("camera"));
    def("screen_w", screen_w);
    def("screen_h", screen_h);
    def("cache", cache, args("fn"));
    def("optimize", optimize);

    enum_<Space>("Space")
        .value("LOCAL", Space::LOCAL)
        .value("PARENT", Space::PARENT)
        .value("WORLD", Space::WORLD)
    ;
    
    //enum_<eNode>("NodeType")
    //    .value("NODE", eNode::NODE)
    //    .value("SPRITE", eNode::SPRITE)
    //    .value("ENVIRONMENT", eNode::ENVIRONMENT)
    //    .value("ENTITY", eNode::ENTITY)
    //    .value("ACTOR", eNode::ACTOR)
    //    .value("LIGHT", eNode::LIGHT)
    //    .value("PARTICLE_SYSTEM", eNode::PARTICLE_SYSTEM)
    //    .value("SOUND", eNode::SOUND)
    //    .value("SELECTION", eNode::SELECTION)
    //    .value("TRACKER", eNode::TRACKER)
    //    .value("CAMERA", eNode::CAMERA)
    //    //.value("USER_TYPE", eNode::USER_TYPE)
    //    //.value("MAX", eNode::MAX),
    //;

    //class_<Window>("Window")
    //    .add_property("position", &WindowHook::get_position, &WindowHook::set_position)
    //    .add_property("center", &WindowHook::get_position, &WindowHook::set_position)
    
    //class_<ContextHook>("Context", no_init);
    
    class_<NodeHook>("Node")
        .def(init<>())
        .add_property("position", &NodeHook::get_position, &NodeHook::set_position)
        .add_property("matrix", &NodeHook::get_matrix, &NodeHook::set_matrix)
        .def("rotate", &NodeHook::rotate)
        .def("move", &NodeHook::move)
        .def("__nonzero__", &NodeHook::valid)
        .def("update", &NodeHook::update)
        .def("num_subnodes", &NodeHook::num_subnodes)
        .def("num_children", &NodeHook::num_children)
        .def("add", &NodeHook::add)
        .def("parent", &NodeHook::parent)
        .def("spawn", &NodeHook::spawn)
        .def("as_node", &NodeHook::as_node)
        //.def_readonly("type", &NodeHook::type)
        //.def("add", &NodeHook::add)
    ;
    class_<MeshHook, bases<NodeHook>>("Mesh")
        .def(init<>())
        .def(init<std::string>())
        .def("instance", &MeshHook::instance)
    ;
    class_<SpriteHook, bases<NodeHook>>("Sprite", init<std::string>())
        .def(init<std::string>())
        .def(init<std::string, std::string>())
        .def(init<std::string, std::string, list>())
        .def("state", &SpriteHook::state)
        .def("states", &SpriteHook::states)
        .def("state_id", &SpriteHook::state_id)
    ;
    class_<CameraHook, bases<NodeHook>>("Camera", init<>())
    ;
    class_<NodeInterfaceHook>("NodeInterface")
    ;
    //class_<Player2DHook, bases<NodeInterfaceHook>>("Player2D", init<>())
    //;
    class_<Player3DHook, bases<NodeInterfaceHook>>("Player3D", init<NodeHook>())
    ;
}

#endif

