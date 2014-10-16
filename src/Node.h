#ifndef _NODE_H_1F2C5N9A
#define _NODE_H_1F2C5N9A

#include <functional>
#include <queue>
#include <stack>
#include <vector>
#include <unordered_set>
#include <memory>
#include <glm/glm.hpp>
#include "IPartitioner.h"
#include "kit/math/angle.h"
#include "kit/meta/meta.h"
#include "kit/cache/cache.h"
#include "kit/reactive/reactive.h"
#include "IRealtime.h"
#include "IRenderable.h"
#include "Graphics.h"
#include "Pass.h"
#include "Actuation.h"
#include <boost/optional.hpp>
#include <boost/signals2.hpp>

class PhysicsObject;

class Node:
    public Actuation,
    public IPipelineRenderable,
    //public Meta::Serializable,
    public std::enable_shared_from_this<Node>
{       
    public:

        enum Physics {
            NONE = 0,
            STATIC,
            DYNAMIC,
            ACTOR
        };
        
        struct Snapshot {
            Snapshot(
                glm::mat4 transform,
                glm::mat4 world_transform,
                Box box,
                Box world_box
            ):
                transform(transform),
                world_transform(world_transform),
                box(box),
                world_box(world_box)
            {}
            //~Snapshot(){
            //    TRY(remove());
            //}
            glm::mat4 transform;
            glm::mat4 world_transform;
            Box box;
            Box world_box;
        };
        
    private:

        mutable kit::lazy<glm::mat4> m_WorldTransform;
        mutable kit::lazy<Box> m_WorldBox;
        Box m_LastWorldBox;
        std::vector<Snapshot> m_Snapshots;

        std::string m_Name;
        
        Node* m_pParent = nullptr;
        std::vector<std::shared_ptr<Node>> m_Children;

        // for modification while iterating (see logic_self)
        std::vector<std::shared_ptr<Node>> m_ChildrenCopy;
        
        unsigned int m_Type = 0;
        bool m_bVisible = true;

        //std::shared_ptr<Meta> m_pMeta;
        std::unordered_set<std::string> m_Tags;

        glm::vec3 to_world(glm::vec3 point, Space s) const;
        glm::vec3 from_world(glm::vec3 point, Space s) const;
        
        Box calculate_world_box();
        
    protected:

        mutable glm::mat4 m_Transform;
        Box m_Box;
        //mutable kit::lazy<Box> m_Box;
        std::shared_ptr<Meta> m_pConfig;
        std::shared_ptr<Meta> m_pAttributes;
        std::string m_Filename;

        glm::vec3 m_Velocity; // local
        glm::vec3 m_Acceleration; // local
        Space m_VelocitySpace = Space::LOCAL;

        // only visible when attached to current camera?
        //bool m_bViewModel = false;
    
    public:
        
        boost::signals2::signal<void()> on_pend;
        boost::signals2::signal<void()> before_render_self;
        boost::signals2::signal<void()> after_render_self;
        boost::signals2::signal<void()> before_render;
        boost::signals2::signal<void()> after_render;
        //boost::signals2::signal<void(
        //    Freq::Time,//t
        //    float,//mass
        //    glm::vec3,//force
        //    glm::vec3,// omega,
        //    glm::vec3,//torque,
        //    glm::vec3,//velocity,
        //    unsigned* //flags
        //)> on_physics_tick;

        Node():
            m_pConfig(std::make_shared<Meta>())
        {init();}
        
        Node(const Node&) = delete;
        Node(Node&&) = delete;
        Node& operator=(const Node&) = delete;
        Node& operator=(Node&&) = delete;

        // Simply set filename and load (optional) config
        Node(const std::string& fn);
        Node(const std::shared_ptr<Meta>& meta):
            m_pConfig(meta)
        {
            init();
        }

        // ctors to be called by a Node factory
        Node(const std::string& fn, IFactory* factory, ICache* cache);
        Node(const std::tuple<std::string, IFactory*, ICache*>& args):
            Node(std::get<0>(args), std::get<1>(args), std::get<2>(args))
        {init();}
        
        Node(const glm::vec3& pos){
            init();
            Node::move(pos);
        }
        Node(
            glm::mat4 transform,
            glm::mat4 transform_cache,
            //bool transform_pending_cache,
            // don't to copy need parent
            std::vector<std::shared_ptr<Node>> children,
            unsigned type = 0
        ):
            m_Transform(transform),
            //m_WorldTransform(transform_cache.get()),
            // TODO: add child deep copy flag
            //m_Children(children),
            m_Type(type)
        {init();}

        void init();
        void snapshot();
        Snapshot* snapshot(unsigned idx);
        void clear_snapshots();
        void restore_snapshot(unsigned idx);

        // TODO: add child deep copy flag
        //virtual Node* clone() const {
        //    return new Node(
        //        m_Transform,
        //        m_WorldTransform,
        //        m_bWorldTransformPendingCache,
        //        m_Type
        //    );
        //}
        
        virtual ~Node() {}
        
        virtual void sync(const glm::mat4&) {}
        
        virtual std::shared_ptr<Node> as_node() {
            return shared_from_this();
        }

        virtual std::vector<std::shared_ptr<Node>>& children() {
            return m_Children;
        }
        virtual const std::vector<std::shared_ptr<Node>>& children() const {
            return m_Children;
        }
        virtual std::vector<std::shared_ptr<Node>> subnodes() {
            std::vector<std::shared_ptr<Node>> v;
            for(auto&& c: m_Children) {
                v.push_back(c);
                auto sn = c->subnodes();
                v.reserve(v.size() + sn.size());
                v.insert(v.end(), sn.begin(), sn.end());
            }
            return v;
        }

        std::string name() const {
            return m_Name;
        }
        void name(const std::string& n) {
            m_Name = n;
        }
        void type(unsigned int type) { m_Type = type; }
        unsigned int type() const { return m_Type; }
        
        virtual Node::Physics physics() const {
            return Node::Physics::NONE;
        }
        
        //bool viewmodel() const {
        //    return m_bViewModel;
        //}
        //void viewmodel(bool b) {
        //    m_bViewModel = b;
        //}

        void _set_parent(Node* p) { m_pParent = p; }
        Node* parent() { return m_pParent; }
        const Node* parent_c() const { return m_pParent; }
        void parents(std::queue<const Node*>& q, bool include_self = false) const;
        void parents(std::stack<const Node*>& s, bool include_self = false) const;
        void parents(std::queue<Node*>& q, bool include_self = false);
        void parents(std::stack<Node*>& s, bool include_self = false);
        virtual bool visible() const {
            return m_bVisible;
        }
        virtual void visible(bool b) {
            m_bVisible = b;
        }

        void cache_transform() const;
        //bool transform_pending_cache() const {
        //    return m_bWorldTransform.pending();
        //}

        virtual glm::mat4* matrix() const { return &m_Transform; }
        virtual const glm::mat4* matrix_c() const { return &m_Transform; }
        virtual const glm::mat4* matrix_c(Space s) const;
        virtual const glm::mat4* matrix(Space s) const {
            return matrix_c(s);
        }
        
        std::shared_ptr<const Meta> config() const {
            return m_pConfig;
        }
        std::shared_ptr<Meta> config() {
            return m_pConfig;
        }
        
        virtual void pend() const {
            m_WorldTransform.pend();
            m_WorldBox.pend();
            on_pend();
            for(auto c: m_Children)
                const_cast<Node*>(c.get())->pend();
        }

        virtual void reset_translation() {
            Matrix::reset_translation(*matrix());
            pend();
        }
        virtual void reset_orientation() {
            Matrix::reset_orientation(*matrix());
            pend();
        }
        virtual glm::vec3 heading() const { return Matrix::heading(*matrix_c()); }
        virtual glm::vec3 position(Space s = Space::PARENT) const;
        virtual void position(const glm::vec3& v, Space s = Space::PARENT);
        
        virtual void move(const glm::vec3& v, Space s = Space::PARENT);
        
        virtual glm::vec3 velocity();
        virtual void velocity(const glm::vec3& v);
        
        virtual glm::vec3 acceleration();
        virtual void acceleration(const glm::vec3& v);

        size_t num_snapshots() const {
            return m_Snapshots.size();
        }
        
        virtual void rotate(float tau, const glm::vec3& v, Space s = Space::LOCAL);
        virtual void scale(glm::vec3 f);
        virtual void rescale(glm::vec3 f);
        virtual void scale(float f);
        virtual void rescale(float f = 1.0f);

        virtual Node* find(Node* n);
        virtual const Node* find_c(const Node* n) const;

        virtual void logic(Freq::Time t) override;
        virtual void logic_self(Freq::Time t) {}

        enum eRenderFlags
        {
            RENDER_USE_FIXED=kit::bit(0),
            RENDER_SELF_ONLY=kit::bit(1),
            RENDER_AMBIENT_PASS=kit::bit(2),
            RENDER_INDICATORS=kit::bit(3)
        };
        virtual void render_self(Pass* pass) const {}
        virtual void render(Pass* pass) const override;

        //virtual bool in_view(IPartitioner* partitioner = NULL) const {
        //    return true;
        //}

        enum AddFlag {
            ADD_FRONT = kit::bit(0)
        };
        //virtual Node* add(Node* n);
        virtual Node* add(const std::shared_ptr<Node>& n);

        enum RemoveFlag {
            SEARCH_SUBNODES=kit::bit(1) // search subnodes for node to be removed?
        };

        virtual void detach();
        //virtual bool remove();
        virtual bool remove(Node* n, unsigned int flags = 0);
        //virtual std::shared_ptr<Node> preserve(Node* n, unsigned int flags = 0);
        virtual void remove_all(unsigned int flags = 0);

        virtual void collapse(Space s = Space::PARENT, unsigned int flags = 0);

        virtual bool is_light() const {
            return false;
        }
        virtual float mass() const {
            return 0.0f;
        }
        virtual std::shared_ptr<PhysicsObject> body() { return nullptr; }
        virtual std::shared_ptr<const PhysicsObject> body() const { return nullptr; }
        virtual void reset_body() {}
        virtual bool has_children() const {
            return !m_Children.empty();
        }

        template<class T>
        std::vector<std::shared_ptr<T>> children() {
            std::vector<std::shared_ptr<T>> matches;
            for(auto&& c: m_Children) {
                auto p = std::dynamic_pointer_cast<T>(c);
                if(p)
                    matches.push_back(p);
            }
            return matches;
        }

        virtual size_t num_children() const { return m_Children.size(); }
        virtual size_t num_subnodes() const {
            size_t n = num_children();
            for(auto&& c: m_Children)
                n += c->num_subnodes();
            return n;
        }

        void each(std::function<void(Node*)> func);
        void each(std::function<void(const Node*)> func) const;

        //std::vector<Node*> subnodes();
        //std::vector<const Node*> subnodes() const;

        void pend_box() {
            m_WorldBox.pend();
        }
        const Box& box() const {
            return m_Box;
        }
        Box& box() {
            return m_Box;
        }
        const Box& world_box() const;
        Box& world_box() {
            return const_cast<Box&>(
                const_cast<const Node*>(this)->world_box()
            );
        }

        glm::vec3 world_to_object(glm::vec3 point) const {
            return from_world(point, Space::PARENT);
        }
        glm::vec3 object_to_world(glm::vec3 point) const {
            return to_world(point, Space::PARENT);
        }
        glm::vec3 local_to_world(glm::vec3 point) const {
            return Matrix::mult(*matrix(Space::WORLD), point);
            //return to_world(point, Space::LOCAL);
        }
        glm::vec3 world_to_local(glm::vec3 point) const{
            return Matrix::mult(glm::inverse(*matrix(Space::WORLD)), point);
            //return to_world(point, Space::LOCAL);
        }
        Box local_to_world(const Box& b) const {
            Box r(Box::Zero());
            for(auto& v: b.verts())
                r &= Matrix::mult(*matrix(Space::WORLD),v);
            return r;
        }

        bool has_tag(const std::string& t) const {
            return m_Tags.find(t) != m_Tags.end();
        }
        void add_tag(const std::string& t) {
            if(!has_tag(t))
                m_Tags.insert(t);
        }
        void add_tags(const std::vector<std::string>& tags) {
            for(auto&& t: tags)
                if(!has_tag(t))
                    m_Tags.insert(std::move(t));
        }

        void remove_tag(const std::string& t) {
            //m_pMeta->at("tags", std::make_shared<Meta>())->clear();
            m_Tags.erase(t);
        }
        void clear_tags() {
            //m_pMeta->at("tags", std::make_shared<Meta>())->clear();
            m_Tags.clear();
        }
        size_t tag_count() const {
            return m_Tags.size();
            //return m_pMeta->at("tags", std::make_shared<Meta>())->size();
        }
        std::unordered_set<std::string> tags() const {
            return m_Tags;
        }

        boost::signals2::signal<void(Freq::Time)> actuators;

        //std::shared_ptr<Config> config() {
        //    return m_Config;
        //}

        typedef typename std::vector<std::shared_ptr<Node>>::const_iterator
            const_iterator;
        typedef typename std::vector<std::shared_ptr<Node>>::iterator iterator;
        iterator begin() { return m_Children.begin(); }
        iterator end() { return m_Children.end(); }
        const_iterator begin() const { return m_Children.begin(); }
        const_iterator end() const { return m_Children.end(); }
        const_iterator cbegin() const { return m_Children.begin(); }
        const_iterator cend() const { return m_Children.end(); }
        
};

#endif

