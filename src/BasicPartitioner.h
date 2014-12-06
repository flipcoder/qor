#ifndef _BASICPARTITIONER_H_YPGXO911
#define _BASICPARTITIONER_H_YPGXO911

#include "IPartitioner.h"
#include "kit/kit.h"
#include "Light.h"
#include <boost/signals2.hpp>
#include <vector>

class BasicPartitioner:
    public IPartitioner
{
    public:
        BasicPartitioner();
        virtual ~BasicPartitioner() {}

        virtual void partition(const Node* root) override;
        virtual const std::vector<
            const Light*
        >& visible_lights() const override {
            return m_Lights;
        }
        virtual const std::vector<const Node*>& visible_nodes() const override{
            return m_Nodes;
        }
        virtual const std::vector<const Node*>& visible_nodes_from(
            const Light* light
        ) const override {
            return m_Nodes;
        }

        virtual void camera(Camera* camera) override {
            m_pCamera = camera;
        }
        virtual const Camera* camera() const override {
            return m_pCamera;
        }
        virtual Camera* camera() override {
            return m_pCamera;
        }
        
        virtual void logic(Freq::Time t) override;

        virtual std::vector<Node*> get_collisions_for(Node* n) override;
        virtual std::vector<Node*> get_collisions_for(Node* n, unsigned type) override;
        virtual std::vector<Node*> get_collisions_for(unsigned type_a, unsigned type_b) override;
        
        // 1-1 collision for unique (non-typed) objects
        virtual void on_collision(
            const std::shared_ptr<Node>& a,
            const std::shared_ptr<Node>& b,
            std::function<void(Node*, Node*)> col = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> no_col = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> enter = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> leave = std::function<void(Node*, Node*)>()
        ) override;
        virtual void on_collision(
            const std::shared_ptr<Node>& a,
            unsigned type,
            std::function<void(Node*, Node*)> col = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> no_col = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> enter = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> leave = std::function<void(Node*, Node*)>()
        ) override;
        virtual void on_collision(
            unsigned type_a,
            unsigned type_b,
            std::function<void(Node*, Node*)> col = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> no_col = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> enter = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> leave = std::function<void(Node*, Node*)>()
        ) override;
        virtual void set_node_collision_type(
            const std::shared_ptr<Node>& a,
            unsigned type
        ) override;
        virtual void unset_node_collision_type(
            const std::shared_ptr<Node>& a,
            unsigned type
        ) override;
        virtual void unset_node_collision_types(
            const std::shared_ptr<Node>& a
        ) override;

        struct Pair
        {
            Pair(std::weak_ptr<Node> a, std::weak_ptr<Node> b):
                nodes{a,b}{
            }
            Pair(const Pair&) = default;
            Pair(Pair&&) = default;
            Pair& operator=(const Pair&) = default;
            Pair& operator=(Pair&&) = default;
            
            std::vector<std::weak_ptr<Node>> nodes;
            std::unique_ptr<boost::signals2::signal<
                void(Node*, Node*)
            >> on_collision = kit::make_unique<boost::signals2::signal<
                void(Node*, Node*)
            >>();
            std::unique_ptr<boost::signals2::signal<
                void(Node*, Node*)
            >> on_enter = kit::make_unique<boost::signals2::signal<
                void(Node*, Node*)
            >>();
            std::unique_ptr<boost::signals2::signal<
                void(Node*, Node*)
            >> on_no_collision = kit::make_unique<boost::signals2::signal<
                void(Node*, Node*)
            >>();
            std::unique_ptr<boost::signals2::signal<
                void(Node*, Node*)
            >> on_leave = kit::make_unique<boost::signals2::signal<
                void(Node*, Node*)
            >>();

            bool collision = false;
        };

        virtual void clear() override {
            m_TypedCollisions.clear();
            m_Collisions.clear();
            m_Nodes.clear();
            m_Lights.clear();
        }
        
        virtual bool empty() const override {
            return m_Nodes.empty() &&
                m_Collisions.empty() &&
                m_TypedCollisions.empty() &&
                m_Lights.empty() &&
                m_Nodes.empty();
        }
        virtual bool has_collisions() const override {
            return (not m_Collisions.empty()) ||
                (not m_TypedCollisions.empty());
        }
            
    private:

        // type (index) -> set of objects that have that type
        std::vector<std::unordered_set<Node*>> m_TypedCollisions;
        
        std::vector<const Node*> m_Nodes;
        std::vector<const Light*> m_Lights;
        std::vector<Pair> m_Collisions;

        //std::map<
        //    std::tuple<Node*, Node*>,
        //    boost::signals2::signal<
        //        std::function<void(Node*, Node*)>(Node*, Node*),
        //        kit::push_back_values<std::vector<
        //            std::function<void(Node*, Node*)>(Node*, Node*)
        //        >>
        //    >
        //> m_Collisions;
        
        Camera* m_pCamera = nullptr;
};

#endif

