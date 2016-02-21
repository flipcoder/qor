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

    struct NodeHook
    {
        std::shared_ptr<Node> n;
        //std::weak_ptr<Node> p;
        NodeHook():
            n(std::make_shared<Node>())
        {}
        NodeHook(std::nullptr_t) {}
        NodeHook(Node* n):
            n(n->as_node()) // Yes, this allows n==null
        {}
        NodeHook(const NodeHook& rhs) = default;
        NodeHook& operator=(const NodeHook& rhs) = default;
        explicit NodeHook(const std::shared_ptr<Node>& p):
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
        void detach() { n->detach(); }
        void collapse(Space s) {n->collapse(s);}
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
        void pend() {
            n->pend();
        }
        void add_tag(std::string tag) {
            n->add_tag(tag);
        }
        bool has_tag(std::string tag) {
            return n->has_tag(tag);
        }
        void remove_tag(std::string tag) {
            n->remove_tag(tag);
        }
        unsigned num_subnodes() const { return n->num_subnodes(); }
        unsigned num_children() const { return n->num_children(); }

        object hook(std::string s) {
            list l;
            auto ns = n->hook(s);
            for(auto&& n: ns)
                l.append<NodeHook>(NodeHook(std::move(n)));
            return l;
        }
    };

    struct MeshHook:
        public NodeHook
    {
        MeshHook():
            NodeHook(std::static_pointer_cast<Node>(std::make_shared<Mesh>()))
        {}
        explicit MeshHook(const std::shared_ptr<Mesh>& mesh):
            NodeHook(std::static_pointer_cast<Node>(mesh))
        {}
        MeshHook(std::string fn):
            NodeHook(nullptr)
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

    struct LightHook:
        public NodeHook
    {
        LightHook():
            NodeHook(std::static_pointer_cast<Node>(std::make_shared<Light>()))
        {}
        explicit LightHook(const std::shared_ptr<Light>& p):
            NodeHook(std::static_pointer_cast<Node>(p))
        {}
        virtual ~LightHook() {}
        Light* self() {
            return (Light*)n.get();
        }
    };

    struct TrackerHook:
        public NodeHook
    {
        TrackerHook():
            NodeHook(std::static_pointer_cast<Node>(std::make_shared<Tracker>()))
        {}
        explicit TrackerHook(const std::shared_ptr<Tracker>& p):
            NodeHook(std::static_pointer_cast<Node>(p))
        {}
        virtual ~TrackerHook() {}
        Tracker* self() {
            return (Tracker*)n.get();
        }

        void stop() {
            self()->track();
        }
        void track(NodeHook nh) {
            self()->track(nh.n);
        }
    };

    struct CameraHook:
        public TrackerHook
    {
        CameraHook():
            TrackerHook(std::static_pointer_cast<Tracker>(std::make_shared<Camera>(
                qor()->resources(), qor()->window()
            )))
        {}
        explicit CameraHook(const std::shared_ptr<Node>& p):
            TrackerHook(std::static_pointer_cast<Tracker>(p))
        {}
        explicit CameraHook(const std::shared_ptr<Camera>& p):
            TrackerHook(std::static_pointer_cast<Tracker>(p))
        {}
        virtual ~CameraHook() {}
        void set_fov(float f) { self()->fov(f); }
        float get_fov() const { return self()->fov(); }
        Camera* self() {
            return (Camera*)n.get();
        }
        const Camera* self() const {
            return (const Camera*)n.get();
        }
    };

    struct SoundHook:
        public NodeHook
    {
        //SoundHook():
        //    NodeHook(std::static_pointer_cast<Node>(std::make_shared<Sound>()))
        //{}
        SoundHook(std::string fn):
            NodeHook(std::static_pointer_cast<Node>(std::make_shared<Sound>(
                qor()->resource_path(fn),
                qor()->resources()
            )))
            //NodeHook(std::static_pointer_cast<Node>(std::make_shared<Sound>(
            //    fn,
            //    qor()->resources()
            //)))
        {}
        explicit SoundHook(const std::shared_ptr<Sound>& p):
            NodeHook(std::static_pointer_cast<Node>(p))
        {}
        virtual ~SoundHook() {}
        Sound* self() {
            return (Sound*)kit::safe_ptr(n.get());
        }
        void play() {
            self()->source()->play();
        }
        void stop() {
            self()->source()->stop();
        }
        void pause() {
            self()->source()->pause();
        }
        void refresh() {
            self()->source()->refresh();
        }
        bool playing() {
            return self()->source()->playing();
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
                qor()->resource_path(fn),
                qor()->resources()
            )))
        {
        }
        SpriteHook(std::string fn, std::string skin):
            NodeHook(std::static_pointer_cast<Node>(std::make_shared<Sprite>(
                qor()->resource_path(fn),
                qor()->resources(),
                skin
            )))
        {}
        SpriteHook(std::string fn, std::string skin, list pos):
            NodeHook(std::static_pointer_cast<Node>(std::make_shared<Sprite>(
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
        SpriteHook(const std::shared_ptr<Sprite>& mesh):
            NodeHook(std::static_pointer_cast<Node>(mesh))
        {}
        virtual ~SpriteHook() {}
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
                    nih.n,
                    nih.n
                )
            ))
        {
            ((PlayerInterface3D*)n.get())->plug();
        }
        virtual ~Player3DHook() {}
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

    //NodeHook create(std::string type) {
    //    return NodeHook();
    //}

    //NodeHook spawn(std::string type) {
    //    auto n = qor()->nodes().create(type);
    //    qor()->current_state()->root()->add(n);
    //    return NodeHook(n);
    //}

    void render_from(NodeHook nh) {
        qor()->current_state()->camera(nh.n);
    }

    CameraHook camera() {
        return CameraHook(std::static_pointer_cast<Node>(
            qor()->current_state()->camera()
        ));
    }

    NodeHook root() {
        assert(qor()->current_state());
        assert(qor()->current_state()->root());
        return NodeHook(qor()->current_state()->root());
        //object main = py::import("__main__");
        //Context c = extract<Context>(main.attr("context"));
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

    //void restart_state() { qor()->restart_state(); }

    BOOST_PYTHON_MODULE(qor)
    {
        register_exception_translator<std::exception>(&script_error);
        
        //def("spawn", spawn, args("name"));
        //def("create", create, args("name"));
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
        
        //class_<AnimationHook>()
        //    .def(init<>())
        //;
        
        class_<NodeHook>("Node")
            .def(init<>())
            .add_property("position", &NodeHook::get_position, &NodeHook::set_position)
            .add_property("matrix", &NodeHook::get_matrix, &NodeHook::set_matrix)
            .def("rotate", &NodeHook::rotate)
            .def("move", &NodeHook::move)
            .def("scale", &NodeHook::scale)
            //.def("rescale", &NodeHook::rescale)
            .def("__nonzero__", &NodeHook::valid)
            .def("pend", &NodeHook::pend)
            .def("num_subnodes", &NodeHook::num_subnodes)
            .def("num_children", &NodeHook::num_children)
            .def("add", &NodeHook::add)
            .def("parent", &NodeHook::parent)
            .def("spawn", &NodeHook::spawn)
            .def("as_node", &NodeHook::as_node)
            .def("detach", &NodeHook::detach)
            .def("collapse", &NodeHook::collapse, args("space"))
            .def("add_tag", &NodeHook::add_tag)
            .def("has_tag", &NodeHook::has_tag)
            .def("remove_tag", &NodeHook::remove_tag)
            .def("hook", &NodeHook::hook)
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
            //.def("states", &SpriteHook::states)
            .def("state_id", &SpriteHook::state_id)
        ;
        class_<TrackerHook, bases<NodeHook>>("Tracker", init<>())
            .def("stop", &TrackerHook::stop)
            .def("track", &TrackerHook::track, args("node"))
        ;
        class_<CameraHook, bases<TrackerHook>>("Camera", init<>())
            .add_property("fov", &CameraHook::get_fov, &CameraHook::set_fov)
        ;
        class_<LightHook, bases<NodeHook>>("Light", init<>())
        ;
        class_<SoundHook, bases<NodeHook>>("Sound", init<std::string>())
            .def(init<std::string>())
            .def("play", &SoundHook::play)
            .def("pause", &SoundHook::pause)
            .def("stop", &SoundHook::stop)
            .def("refresh", &SoundHook::refresh)
            .def("playing", &SoundHook::playing)
        ;
        class_<NodeInterfaceHook>("NodeInterface")
        ;
        //class_<Player2DHook, bases<NodeInterfaceHook>>("Player2D", init<>())
        //;
        class_<Player3DHook, bases<NodeInterfaceHook>>("Player3D", init<NodeHook>())
            .add_property("speed", &Player3DHook::get_speed, &Player3DHook::set_speed)
        ;
    }

}

#endif

