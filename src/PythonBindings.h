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
#include "Sound.h"
#include "TileMap.h"
#include "Graphics.h"
#include "Camera.h"
#include "Light.h"
#include "NodeInterface.h"
#include "PlayerInterface3D.h"
#include "kit/log/log.h"
#include "kit/log/errors.h"
#include "kit/freq/animation.h"

using namespace boost::python;

namespace Scripting
{

    Qor* qor() {
        return (Qor*)Interpreter::current()->user_data();
    }

    //struct NodeGroup
    //{
    //    //std::vector
    //};

    struct NodeBind
    {
        std::shared_ptr<Node> n;
        //std::weak_ptr<Node> p;
        NodeBind():
            n(std::make_shared<Node>())
        {}
        NodeBind(std::nullptr_t) {}
        NodeBind(Node* n):
            n(n->as_node()) // Yes, this allows n==null
        {}
        NodeBind(const NodeBind& rhs) = default;
        NodeBind& operator=(const NodeBind& rhs) = default;
        explicit NodeBind(const std::shared_ptr<Node>& p):
            n(p)
        {}
        NodeBind as_node() {
            return NodeBind(n);
        }
        virtual ~NodeBind() {}
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
        //void rescale(float f) { n->rescale(f); }
        void scale(float f) { n->scale(f); }
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
            pend();
        }
        void set_position(list t, Space s = Space::LOCAL) {
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
        void detach() { n->detach(); }
        void collapse(Space s) {n->collapse(s);}
        //virtual std::string type() const {
        //    return "node";
        //}
        void add(NodeBind nh) { n->add(nh.n); }
        void spawn() { detach(); qor()->current_state()->root()->add(n); }
        bool valid() const { return !!n; }
        NodeBind parent() const { return NodeBind(n->parent()); }
        void pend() { n->pend(); }
        void add_tag(std::string tag) { n->add_tag(tag); }
        bool has_tag(std::string tag) { return n->has_tag(tag); }
        void remove_tag(std::string tag) { n->remove_tag(tag); }
        unsigned num_subnodes() const { return n->num_subnodes(); }
        unsigned num_children() const { return n->num_children(); }

        object hook(std::string s) {
            list l;
            auto ns = n->hook(s);
            for(auto&& n: ns)
                l.append<NodeBind>(NodeBind(std::move(n)));
            return l;
        }
        object hook_if(boost::python::object cb) {
            list l;
            auto ns = n->hook_if([cb](Node* n){
                return cb(NodeBind(n));
            });
            for(auto&& n: ns)
                l.append<NodeBind>(NodeBind(std::move(n)));
            return l;
        }

        void on_tick(boost::python::object cb){
            n->on_tick.connect([cb](Freq::Time t){
                cb(t.s());
            });
        }
    };

    struct MeshBind:
        public NodeBind
    {
        MeshBind():
            NodeBind(std::static_pointer_cast<Node>(std::make_shared<Mesh>()))
        {}
        explicit MeshBind(const std::shared_ptr<Mesh>& mesh):
            NodeBind(std::static_pointer_cast<Node>(mesh))
        {}
        MeshBind(std::string fn):
            NodeBind(nullptr)
        {
            n = std::make_shared<Mesh>(
                qor()->resource_path(fn),
                qor()->resources()
            );
            //n = std::make_shared<Mesh>(qor()->nodes().create_as<Mesh::Data>(
            //    std::tuple<
            //        std::string,
            //        IFactory*,
            //        ICache*
            //    >(
            //        fn,
            //        &qor()->nodes(),
            //        qor()->resources()
            //    )
            //));
        }
        virtual ~MeshBind() {}
        Mesh* self() {
            return (Mesh*)n.get();
        }
        MeshBind instance() {
            return MeshBind(self()->instance());
        }
        //virtual std::string type() const {
        //    return "mesh";
        //}
    };

    struct LightBind:
        public NodeBind
    {
        LightBind():
            NodeBind(std::static_pointer_cast<Node>(std::make_shared<Light>()))
        {}
        explicit LightBind(const std::shared_ptr<Light>& p):
            NodeBind(std::static_pointer_cast<Node>(p))
        {}
        virtual ~LightBind() {}
        Light* self() {
            return (Light*)n.get();
        }
        void diffuse(Color c) {self()->diffuse(c);}
        void specular(Color c) {self()->specular(c);}
    };

    struct TrackerBind:
        public NodeBind
    {
        TrackerBind():
            NodeBind(std::static_pointer_cast<Node>(std::make_shared<Tracker>()))
        {}
        explicit TrackerBind(const std::shared_ptr<Tracker>& p):
            NodeBind(std::static_pointer_cast<Node>(p))
        {}
        virtual ~TrackerBind() {}
        Tracker* self() {
            return (Tracker*)n.get();
        }

        void stop() {
            self()->track();
        }
        void track(NodeBind nh) {
            self()->track(nh.n);
        }
    };

    struct CameraBind:
        public TrackerBind
    {
        CameraBind():
            TrackerBind(std::static_pointer_cast<Tracker>(std::make_shared<Camera>(
                qor()->resources(), qor()->window()
            )))
        {}
        explicit CameraBind(const std::shared_ptr<Node>& p):
            TrackerBind(std::static_pointer_cast<Tracker>(p))
        {}
        explicit CameraBind(const std::shared_ptr<Camera>& p):
            TrackerBind(std::static_pointer_cast<Tracker>(p))
        {}
        virtual ~CameraBind() {}
        void set_fov(float f) { self()->fov(f); }
        float get_fov() const { return self()->fov(); }
        Camera* self() {
            return (Camera*)n.get();
        }
        const Camera* self() const {
            return (const Camera*)n.get();
        }
        //Camera* ortho(bool bottom_origin) { n->ortho(bottom_origin); }
        //Camera* perspective() { n->perspective(); }
    };

    struct SoundBind:
        public NodeBind
    {
        //SoundBind():
        //    NodeBind(std::static_pointer_cast<Node>(std::make_shared<Sound>()))
        //{}
        SoundBind(std::string fn):
            NodeBind(std::static_pointer_cast<Node>(std::make_shared<Sound>(
                qor()->resource_path(fn),
                qor()->resources()
            )))
            //NodeBind(std::static_pointer_cast<Node>(std::make_shared<Sound>(
            //    fn,
            //    qor()->resources()
            //)))
        {}
        explicit SoundBind(const std::shared_ptr<Sound>& p):
            NodeBind(std::static_pointer_cast<Node>(p))
        {}
        virtual ~SoundBind() {}
        Sound* self() { return (Sound*)kit::safe_ptr(n.get()); }
        void play() { self()->source()->play(); }
        void stop() { self()->source()->stop(); }
        void pause() { self()->source()->pause(); }
        void refresh() { self()->source()->refresh(); }
        bool playing() { return self()->source()->playing(); }
        void ambient(bool b) { return self()->ambient(b); }
        void detach_on_done() { self()->detach_on_done(); }
        void on_done(boost::python::object cb) {
            this->self()->on_done([cb]{
                cb();
            });
        }
    };

    struct SpriteBind:
        public NodeBind
    {
        //SpriteBind(NodeBind& nh) {
            // TODO: casting
        //}
        SpriteBind(std::string fn):
            NodeBind(std::static_pointer_cast<Node>(std::make_shared<Sprite>(
                qor()->resource_path(fn),
                qor()->resources()
            )))
        {
        }
        SpriteBind(std::string fn, std::string skin):
            NodeBind(std::static_pointer_cast<Node>(std::make_shared<Sprite>(
                qor()->resource_path(fn),
                qor()->resources(),
                skin
            )))
        {}
        SpriteBind(std::string fn, std::string skin, list pos):
            NodeBind(std::static_pointer_cast<Node>(std::make_shared<Sprite>(
                qor()->resource_path(fn),
                qor()->resources(),
                skin,
                glm::vec3(
                    extract<float>(pos[0]),
                    extract<float>(pos[1]),
                    extract<float>(pos[2])
                )
            )))
        {}
        SpriteBind(const std::shared_ptr<Sprite>& mesh):
            NodeBind(std::static_pointer_cast<Node>(mesh))
        {}
        virtual ~SpriteBind() {}
        Sprite* self() {
            return (Sprite*)n.get();
        }
        unsigned state_id(const std::string& s) { return self()->state_id(s); }
        void state(unsigned id) { self()->set_state(id); }
        //void states(list& l) {
        //    std::vector<unsigned> v;
        //    auto length = len(l);
        //    for(unsigned i=0;i<length;++i)
        //        v.push_back(i); // uhh....
        //    self()->set_states_by_id(v);
        //}
    };

    struct NodeInterfaceBind
    {
        std::shared_ptr<NodeInterface> n;
        NodeInterfaceBind() {}
        NodeInterfaceBind(const std::shared_ptr<NodeInterface>& p):
            n(p)
        {}
        virtual ~NodeInterfaceBind() {}
    };

    //struct Player2DBind:
    //    public NodeInterfaceBind
    //{
    //    //Player2DBind():
    //    //    NodeInterfaceBind(std::static_pointer_cast<NodeInterface>(
    //    //        std::make_shared<Player2DInterface>(
                    
    //    //        )
    //    //    ))
    //    //{}
    //    Player2DBind* self() {
    //        return (Player2DBind*)n.get();
    //    }
    //};

    struct Player3DBind:
        public NodeInterfaceBind
    {
        Player3DBind(){}
        Player3DBind(NodeBind nih):
            NodeInterfaceBind(std::static_pointer_cast<NodeInterface>(
                std::make_shared<PlayerInterface3D>(
                    qor()->session()->profile(0)->controller(),
                    nih.n,
                    nih.n
                )
            ))
        {
            ((PlayerInterface3D*)n.get())->plug();
        }
        virtual ~Player3DBind() {}
        PlayerInterface3D* self() {
            return (PlayerInterface3D*)n.get();
        }
        const PlayerInterface3D* self() const {
            return (const PlayerInterface3D*)n.get();
        }
        float get_speed() const {
            return self()->speed();
        }
        void set_speed(float s) {
            self()->speed(s);
        }

    };

    //NodeBind create(std::string type) {
    //    return NodeBind(qor()->make(""));
    //}

    //NodeBind spawn(std::string type) {
    //    auto n = qor()->nodes().create(type);
    //    qor()->current_state()->root()->add(n);
    //    return NodeBind(n);
    //}

    void render_from(NodeBind nh) {
        qor()->current_state()->camera(nh.n);
    }

    CameraBind camera() {
        return CameraBind(std::static_pointer_cast<Node>(
            qor()->current_state()->camera()
        ));
    }

    NodeBind root() {
        assert(qor()->current_state());
        assert(qor()->current_state()->root());
        return NodeBind(qor()->current_state()->root());
        //object main = py::import("__main__");
        //Context c = extract<Context>(main.attr("context"));
    }
    object hook(std::string s) {
        return root().hook(s);
    }

    void relative_mouse(bool b) {qor()->input()->relative_mouse(b);}
    void push_state(unsigned state) { qor()->push_state(state);}
    void pop_state() { qor()->pop_state(); }
    void change_state(unsigned state) { qor()->change_state(state);}
    void quit() { qor()->quit(); }
    void perspective() {
        ((Camera*)qor()->current_state()->camera().get())->perspective();
    }
    void ortho(bool origin_bottom = false) {
        ((Camera*)qor()->current_state()->camera().get())->ortho(origin_bottom);
    }
    //object bg_color() {
    //}
    void bg_color(list rgb) {
        auto c = Color(
            extract<float>(rgb[0]),
            extract<float>(rgb[1]),
            extract<float>(rgb[2])
        );
        qor()->pipeline()->bg_color(c);
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
    void on_enter(boost::python::object cb) {
        qor()->current_state()->on_enter.connect([cb]{ cb(); });
    }
    void on_tick(boost::python::object cb) {
        qor()->current_state()->on_tick.connect([cb](Freq::Time t){ cb(t.s()); });
    }

    //float get_x(glm::vec3 v) { return v.x; }
    //float get_y(glm::vec3 v) { return v.y; }
    //float get_z(glm::vec3 v) { return v.z; }
    //void set_x(glm::vec3 v, float x) { v.x = x; }
    //void set_y(glm::vec3 v, float y) { v.y = y; }
    //void set_z(glm::vec3 v, float z) { v.z = z; }

    //float get_r(Color c) { return c.r; }
    //float get_g(Color c) { return c.g; }
    //float get_b(Color c) { return c.b; }
    //float get_a(Color c) { return c.a; }
    //void set_r(Color c, float r) { c.r = r; }
    //void set_g(Color c, float g) { c.g = g; }
    //void set_b(Color c, float b) { c.b = b; }
    //void set_a(Color c, float b) { c.a = a; }

    //void restart_state() { qor()->restart_state(); }

    BOOST_PYTHON_MODULE(qor)
    {
        register_exception_translator<std::exception>(&script_error);
        
        //def("spawn", spawn, args("name"));
        def("root", root);
        def("camera", camera);
        def("relative_mouse", relative_mouse);
        def("push_state", push_state, args("state"));
        def("pop_state", pop_state);
        def("change_state", change_state, args("state"));
        def("quit", quit);
        def("ortho", ortho, args("origin_bottom"));
        def("perspective", perspective);
        def("bg_color", bg_color, args("rgb"));
        def("render_from", render_from, args("camera"));
        def("screen_w", screen_w);
        def("screen_h", screen_h);
        def("cache", cache, args("fn"));
        def("optimize", optimize);
        def("hook", hook);
        def("on_enter", on_enter);
        def("on_tick", on_tick);

        enum_<Space>("Space")
            .value("LOCAL", Space::LOCAL)
            .value("PARENT", Space::PARENT)
            .value("WORLD", Space::WORLD)
        ;
        enum_<Node::Physics>("PhysicsType")
            .value("NO_PHYSICS", Node::Physics::NO_PHYSICS)
            .value("STATIC", Node::Physics::STATIC)
            .value("DYNAMIC", Node::Physics::DYNAMIC)
            .value("ACTOR", Node::Physics::ACTOR)
            .value("GHOST", Node::Physics::GHOST)
        ;
        enum_<Node::PhysicsShape>("PhysicsShape")
            .value("NO_SHAPE", Node::PhysicsShape::NO_SHAPE)
            .value("MESH", Node::PhysicsShape::MESH)
            .value("HULL", Node::PhysicsShape::HULL)
            .value("BOX", Node::PhysicsShape::BOX)
            .value("SPHERE", Node::PhysicsShape::SPHERE)
            .value("CAPSULE", Node::PhysicsShape::CAPSULE)
            .value("CYLINDER", Node::PhysicsShape::CYLINDER)
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
        //    .add_property("position", &WindowBind::get_position, &WindowBind::set_position)
        //    .add_property("center", &WindowBind::get_position, &WindowBind::set_position)
        
        //class_<ContextBind>("Context", no_init);
        
        class_<glm::vec3>("vec3")
            //.add_property("x", &glm::vec3::x)
            //.add_property("y", &get_y, set_y)
            //.add_property("z", &get_z, set_z)
        ;
        
        class_<Color>("Color")
            .def(init<>())
            .def(init<float>())
            .def(init<float,float,float>())
            .def(init<float,float,float,float>())
            //.def("r", &Color::r)
            //.def("g", &Color::g)
            //.def("b", &Color::b)
            //.def("a", &Color::a)
            .def(self + self)
            .def(self - self)
            .def(self * self)
            .def(self += self)
            .def(self -= self)
            .def(self *= self)
            //.def("saturate", &Color::saturate)
            .def("vec3", &Color::vec3)
            .def("vec4", &Color::vec4)
            .def("hex", &Color::hex)
            .def("string", &Color::string)
        ;
        
        class_<NodeBind>("Node")
            .def(init<>())
            .add_property("position", &NodeBind::get_position, &NodeBind::set_position)
            .add_property("matrix", &NodeBind::get_matrix, &NodeBind::set_matrix)
            .def("rotate", &NodeBind::rotate)
            .def("move", &NodeBind::move)
            .def("scale", &NodeBind::scale)
            //.def("rescale", &NodeBind::rescale)
            .def("__nonzero__", &NodeBind::valid)
            .def("pend", &NodeBind::pend)
            .def("num_subnodes", &NodeBind::num_subnodes)
            .def("num_children", &NodeBind::num_children)
            .def("add", &NodeBind::add)
            .def("parent", &NodeBind::parent)
            .def("spawn", &NodeBind::spawn)
            .def("as_node", &NodeBind::as_node)
            .def("detach", &NodeBind::detach)
            .def("collapse", &NodeBind::collapse, args("space"))
            .def("add_tag", &NodeBind::add_tag)
            .def("has_tag", &NodeBind::has_tag)
            .def("remove_tag", &NodeBind::remove_tag)
            .def("hook", &NodeBind::hook)
            .def("hook_if", &NodeBind::hook_if)
            .def("on_tick", &NodeBind::on_tick)
            //.def_readonly("type", &NodeBind::type)
            //.def("add", &NodeBind::add)
        ;
        class_<MeshBind, bases<NodeBind>>("Mesh")
            .def(init<>())
            .def(init<std::string>())
            .def("instance", &MeshBind::instance)
        ;
        class_<SpriteBind, bases<NodeBind>>("Sprite", init<std::string>())
            .def(init<std::string>())
            .def(init<std::string, std::string>())
            .def(init<std::string, std::string, list>())
            .def("state", &SpriteBind::state)
            //.def("states", &SpriteBind::states)
            .def("state_id", &SpriteBind::state_id)
        ;
        class_<TrackerBind, bases<NodeBind>>("Tracker", init<>())
            .def("stop", &TrackerBind::stop)
            .def("track", &TrackerBind::track, args("node"))
        ;
        class_<CameraBind, bases<TrackerBind>>("Camera", init<>())
            .add_property("fov", &CameraBind::get_fov, &CameraBind::set_fov)
            //.def("ortho", &Camera::ortho, args("origin_bottom"))
            //.def("perspective", &Camera::perspective)
        ;
        class_<LightBind, bases<NodeBind>>("Light", init<>())
            .def("diffuse", &LightBind::diffuse)
            .def("specular", &LightBind::specular)
        ;
        class_<SoundBind, bases<NodeBind>>("Sound", init<std::string>())
            .def(init<std::string>())
            .def("play", &SoundBind::play)
            .def("pause", &SoundBind::pause)
            .def("stop", &SoundBind::stop)
            .def("refresh", &SoundBind::refresh)
            .def("playing", &SoundBind::playing)
            .def("ambient", &SoundBind::ambient)
            .def("detach_on_done", &SoundBind::detach_on_done)
            .def("on_done", &SoundBind::on_done)
        ;
        class_<NodeInterfaceBind>("NodeInterface")
        ;
        //class_<Player2DBind, bases<NodeInterfaceBind>>("Player2D", init<>())
        //;
        class_<Player3DBind, bases<NodeInterfaceBind>>("Player3D", init<NodeBind>())
            .add_property("speed", &Player3DBind::get_speed, &Player3DBind::set_speed)
        ;
    }

}

#endif

