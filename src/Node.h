#ifndef _INODE_H_1F2C5N9A
#define _INODE_H_1F2C5N9A

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
#include "IRealtime.h"
#include "IRenderable.h"
#include "Graphics.h"
#include "Pass.h"
#include <boost/optional.hpp>
#include <boost/signals2.hpp>
#include <functional>

class Node:
    public IRealtime,
    public IPipelineRenderable,
    //public Meta::Serializable,
    public std::enable_shared_from_this<Node>
{
    private:

        mutable glm::mat4 m_Transform;
        mutable glm::mat4 m_TransformCache;
        mutable bool m_bTransformPendingCache = true;

        Node* m_pParent = nullptr;
        std::vector<std::shared_ptr<Node>> m_Children;
        
        unsigned int m_Type = 0;
        bool m_bVisible = true;

        //std::shared_ptr<Meta<kit::dummy_mutex>> m_pMeta;
        std::unordered_set<std::string> m_Tags;
        
    protected:

        std::shared_ptr<Meta<>> m_pConfig;
        std::string m_Filename;
        
    public:

        Node():
            m_pConfig(std::make_shared<Meta<>>())
        {}
        Node(const Node&) = delete;

        // Simply set filename and load (optional) config
        Node(const std::string& fn);

        // ctors to be called by a Node factory
        Node(const std::string& fn, IFactory* factory, ICache* cache);
        Node(const std::tuple<std::string, IFactory*, ICache*>& args):
            Node(std::get<0>(args), std::get<1>(args), std::get<2>(args))
        {}
        
        Node(const glm::vec3& pos){
            Node::move(pos);
        }
        Node(
            glm::mat4 transform,
            glm::mat4 transform_cache,
            bool transform_pending_cache,
            // don't to copy need parent
            std::vector<std::shared_ptr<Node>> children,
            unsigned type = 0
        ):
            m_Transform(transform),
            m_TransformCache(transform_cache),
            m_bTransformPendingCache(transform_pending_cache),
            // TODO: add child deep copy flag
            //m_Children(children),
            m_Type(type)
        {}

        // TODO: add child deep copy flag
        //virtual Node* clone() const {
        //    return new Node(
        //        m_Transform,
        //        m_TransformCache,
        //        m_bTransformPendingCache,
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

        void type(unsigned int type) { m_Type = type; }
        unsigned int type() const { return m_Type; }

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
        bool transform_pending_cache() const {
            return m_bTransformPendingCache;
        }

        virtual glm::mat4* matrix() const { return &m_Transform; }
        virtual const glm::mat4* matrix_c() const { return &m_Transform; }
        virtual const glm::mat4* matrix_c(Space s) const;
        
        std::shared_ptr<const Meta<>> config() const {
            return m_pConfig;
        }
        std::shared_ptr<Meta<>> config() {
            return m_pConfig;
        }
        
        virtual void pend() const {
            m_bTransformPendingCache = true;
            for(auto c: m_Children)
                const_cast<Node*>(c.get())->pend();
            //m_WorldMatrixCache = m_Transform;
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
        virtual void rotate(float tau, const glm::vec3& v, Space s = Space::LOCAL);
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

        virtual bool in_view(IPartitioner* partitioner = NULL) const {
            return true;
        }

        enum AddFlag {
            ADD_FRONT = kit::bit(0)
        };
        //virtual Node* add(Node* n);
        virtual Node* add(const std::shared_ptr<Node>& n);

        enum RemoveFlag {
            SEARCH_SUBNODES=kit::bit(1), // search subnodes for node to be removed?
        };

        virtual void detach();
        virtual bool remove();
        virtual bool remove(Node* n, unsigned int flags = 0);
        virtual std::shared_ptr<Node> preserve(Node* n, unsigned int flags = 0);
        virtual void remove_all(unsigned int flags = 0);

        virtual void collapse(Space s = Space::PARENT, unsigned int flags = 0);

        virtual bool is_light() const {
            return false;
        }
        virtual unsigned physics() const {
            return 0;
        }
        virtual float mass() const {
            return 0.0f;
        }
        void* body() { return nullptr; }
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

        virtual boost::optional<std::array<glm::vec3,2>> box(
            Space s = Space::PARENT
        ) const {
            return boost::optional<std::array<glm::vec3, 2>>();
        }
        void box(
            std::array<glm::vec3, 2> b
        ) {
        }

        // transforms between world and local space
        virtual glm::vec3 transform_in(glm::vec3 point) const;
        virtual glm::vec3 transform_out(glm::vec3 point) const;

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
            //m_pMeta->at("tags", std::make_shared<Meta<kit::dummy_mutex>>())->clear();
            m_Tags.erase(t);
        }
        void clear_tags() {
            //m_pMeta->at("tags", std::make_shared<Meta<kit::dummy_mutex>>())->clear();
            m_Tags.clear();
        }
        size_t tag_count() const {
            return m_Tags.size();
            //return m_pMeta->at("tags", std::make_shared<Meta<kit::dummy_mutex>>())->size();
        }
        std::unordered_set<std::string> tags() const {
            return m_Tags;
        }

        boost::signals2::signal<void(Freq::Time)> actuators;

        //std::shared_ptr<Config> config() {
        //    return m_Config;
        //}
};

#endif

