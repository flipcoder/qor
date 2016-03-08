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
            NO_PHYSICS = 0,
            STATIC,
            DYNAMIC,
            ACTOR,
            GHOST
        };

        enum PhysicsShape {
            NO_SHAPE = 0,
            MESH,
            HULL,
            BOX,
            SPHERE,
            CAPSULE,
            CYLINDER
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
        std::vector<std::unique_ptr<Snapshot>> m_Snapshots;

        Node* m_pParent = nullptr;
        std::vector<std::shared_ptr<Node>> m_Children;

        // for modification while iterating (see logic_self)
        std::vector<std::shared_ptr<Node>> m_ChildrenCopy;
        
        unsigned int m_Type = 0;
        bool m_bVisible = true; // including children
        bool m_bSelfVisible = true;

        //std::shared_ptr<Meta> m_pMeta;
        std::unordered_set<std::string> m_Tags;
        
        Box calculate_world_box();

         // assumes bounding box completely contains children
        bool m_bSkipChildBoxCheck = false;
        
    protected:

        int m_Layer = 0;

        mutable glm::mat4 m_Transform;
        Box m_Box;
        //mutable kit::lazy<Box> m_Box;
        std::shared_ptr<Meta> m_pConfig;
        std::shared_ptr<Meta> m_pProperties;
        std::string m_Name;
        std::string m_Filename;

        glm::vec3 m_Velocity; // local
        glm::vec3 m_Acceleration; // local
        //Space m_VelocitySpace = Space::PARENT;

        // only visible when attached to current camera?
        //bool m_bViewModel = false;     
    
    public:
        
        // on_tick is provided by Actuation
        kit::signal<void()> on_add;
        kit::signal<void()> on_pend;
        kit::signal<void()> on_free; // dtor
        kit::signal<void(Pass*)> before_render_self;
        kit::signal<void(Pass*)> after_render_self;
        kit::signal<void(Pass*)> before_render;
        kit::signal<void(Pass*)> after_render;

        Node() {init();}
        
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
        Node(const std::string& fn, ICache* cache);
        Node(const std::tuple<std::string, ICache*>& args):
            Node(std::get<0>(args), std::get<1>(args))
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

        virtual ~Node() { on_free(); }
        
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
        int layer() const { return m_Layer; }
        void layer(int v) { m_Layer = v; }
        
        virtual Node::Physics physics() const {
            return NO_PHYSICS;
        }
        virtual Node::PhysicsShape physics_shape() const {
            return NO_SHAPE;
        }

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
        virtual bool self_visible() const {
            return m_bSelfVisible;
        }
        virtual void self_visible(bool b) {
            m_bSelfVisible = b;
        }

        void cache() const; // recursive
        
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
        void reload_config(std::string fn);
        
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
        void position(float x, float y, float z, Space s = Space::PARENT) {
            position(glm::vec3(x,y,z), s);
        }
        
        virtual void move(const glm::vec3& v, Space s = Space::PARENT);
        void move(float x, float y, float z, Space s = Space::PARENT) {
            move(glm::vec3(x,y,z), s);
        }
        
        virtual glm::vec3 velocity() const;
        virtual void velocity(const glm::vec3& v);
        void velocity(float x, float y, float z) {
            velocity(glm::vec3(x,y,z));
        }
        
        virtual glm::vec3 acceleration();
        virtual void acceleration(const glm::vec3& v);
        void acceleration(float x, float y, float z) {
            acceleration(glm::vec3(x,y,z));
        }

        size_t num_snapshots() const {
            return m_Snapshots.size();
        }
        
        virtual void rotate(float turns, const glm::vec3& v, Space s = Space::LOCAL);
        virtual void scale(glm::vec3 f, Space s = Space::LOCAL);
        virtual void rescale(glm::vec3 f);
        virtual void scale(float f, Space s = Space::LOCAL);
        virtual void rescale(float f);

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
        virtual void set_render_matrix(Pass* pass) const;

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

        virtual bool detach();
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
        virtual float friction() const {
            return -1.0f;
        }
        virtual bool has_inertia() const {
            return false;
        }
        virtual std::shared_ptr<PhysicsObject> body() { return nullptr; }
        virtual std::shared_ptr<const PhysicsObject> body() const { return nullptr; }
        virtual void reset_body() {}
        virtual void clear_body() {}
        virtual bool has_children() const {
            return !m_Children.empty();
        }

        // If the node's are always completely contained in this node's
        // bounding box, setting this to true will allow optimization of
        // visibility (and other) checks.
        bool skip_child_box_check() const {
            return m_bSkipChildBoxCheck;
        }
        void skip_child_box_check(bool b) {
            m_bSkipChildBoxCheck = b;
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

        enum LoopCtrl {
            LC_STEP = 0, // default behavior
            LC_BREAK, // stop entirely
            LC_SKIP, // skip subtree recursion
        };
        struct Each {
            enum {
                RECURSIVE = kit::bit(0),
                INCLUDE_SELF= kit::bit(1),
                
                STOP_RECURSION = kit::bit(2), // self only
                
                DEFAULT_FLAGS = 0
            };
        };

        void each(const std::function<void(Node*)>& func, unsigned flags = Each::DEFAULT_FLAGS, LoopCtrl* lc = nullptr);
        void each(const std::function<void(const Node*)>& func, unsigned flags = Each::DEFAULT_FLAGS, LoopCtrl* lc = nullptr) const;

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

        Box to_world(const Box& b) const {
            Box r(Box::Zero());
            for(auto& v: b.verts())
                r &= Matrix::mult(*matrix(Space::WORLD),v);
            return r;
        }
        glm::vec3 to_world(glm::vec3 point, Space s = Space::LOCAL) const;
        glm::vec3 from_world(glm::vec3 point, Space s = Space::LOCAL) const;
        glm::vec3 orient_to_world(glm::vec3 vec, Space s = Space::LOCAL) const;
        glm::vec3 orient_from_world(glm::vec3 vec, Space s = Space::LOCAL) const;

        bool has_tag(std::string t) const {
            if(t[0]=='#')
                t = t.substr(1);
            return m_Tags.find(t) != m_Tags.end();
        }
        void add_tag(std::string t) {
            if(t[0]=='#')
                t = t.substr(1);
            if(!has_tag(t))
                m_Tags.insert(t);
        }
        void add_tags(std::string tags);
        void add_tags(std::vector<std::string> tags);

        void remove_tag(std::string t) {
            if(t[0]=='#')
                t = t.substr(1);
            m_Tags.erase(t);
        }
        void clear_tags() {
            m_Tags.clear();
        }
        size_t tag_count() const {
            return m_Tags.size();
        }
        std::unordered_set<std::string> tags() const {
            return m_Tags;
        }

        struct Hook {
            enum {
                RECURSIVE = kit::bit(0),
                INCLUDE_SELF = kit::bit(1),
                REGEX = kit::bit(2),
                //PARENTS = kit::bit(2),
                DEFAULT_FLAGS = RECURSIVE | INCLUDE_SELF
            };
        };
        std::vector<Node*> hook(std::string name, unsigned flags = Hook::DEFAULT_FLAGS);
        std::vector<Node*> hook_if(std::function<bool(Node* n)> cb, unsigned flags = Hook::DEFAULT_FLAGS);
        std::vector<Node*> hook_tag(std::string tag, unsigned flags = Hook::DEFAULT_FLAGS);
        
        template<class T>
        std::vector<T*> hook_type() {
            std::vector<T*> r;
            each([&r](Node* node){
                auto n = dynamic_cast<T*>(node);
                if(n)
                    r.push_back(n);
            }, Node::Each::RECURSIVE);
            return r;
        }

        typedef typename std::vector<std::shared_ptr<Node>>::const_iterator
            const_iterator;
        typedef typename std::vector<std::shared_ptr<Node>>::iterator iterator;
        iterator begin() { return m_Children.begin(); }
        iterator end() { return m_Children.end(); }
        const_iterator begin() const { return m_Children.begin(); }
        const_iterator end() const { return m_Children.end(); }
        const_iterator cbegin() const { return m_Children.begin(); }
        const_iterator cend() const { return m_Children.end(); }
        
        std::string filename() const { return m_Filename; }
        void filename(std::string fn) { m_Filename = fn; }
};

#endif

