#ifndef _BASICPARTITIONER_H_YPGXO911
#define _BASICPARTITIONER_H_YPGXO911

#include "IPartitioner.h"
#include "kit/kit.h"
#include "kit/reactive/signal.h"
#include "Light.h"
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
        
        virtual void preload() override {
            logic(Freq::Time::ms(0));
        }
        virtual void logic(Freq::Time t) override;
        virtual void lazy_logic(Freq::Time t) override;

        virtual std::vector<Node*> get_collisions_for(Node* n) override;
        virtual std::vector<Node*> get_collisions_for(Node* n, unsigned type) override;
        virtual std::vector<Node*> get_collisions_for(unsigned type_a, unsigned type_b) override;
        
        // 1-1 collision for unique (non-typed) objects
        virtual void on_collision(
            const std::shared_ptr<Node>& a,
            const std::shared_ptr<Node>& b,
            std::function<void(Node*, Node*)> col = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> no_col = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> touch = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> untouch = std::function<void(Node*, Node*)>()
        ) override;
        virtual void on_touch(
            const std::shared_ptr<Node>& a,
            const std::shared_ptr<Node>& b,
            std::function<void(Node*, Node*)> touch = std::function<void(Node*, Node*)>()
        ) override;
        virtual void on_collision(
            const std::shared_ptr<Node>& a,
            unsigned type,
            std::function<void(Node*, Node*)> col = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> no_col = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> touch = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> untouch = std::function<void(Node*, Node*)>()
        ) override;
        virtual void on_collision(
            unsigned type_a,
            unsigned type_b,
            std::function<void(Node*, Node*)> col = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> no_col = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> touch = std::function<void(Node*, Node*)>(),
            std::function<void(Node*, Node*)> untouch = std::function<void(Node*, Node*)>()
        ) override;
        virtual void register_object(
            const std::shared_ptr<Node>& a,
            unsigned type
        ) override;
        virtual void deregister_object(
            const std::shared_ptr<Node>& a,
            unsigned type
        ) override;
        virtual void deregister_object(
            const std::shared_ptr<Node>& a
        ) override;

        virtual void register_provider(
            unsigned type,
            std::function<std::vector<std::weak_ptr<Node>>(Box)> func
        );

        template<class A, class B>
        struct Pair
        {
            Pair(A a, B b):
                a(a),
                b(b)
            {}
            Pair(const Pair&) = default;
            Pair(Pair&&) = default;
            Pair& operator=(const Pair&) = default;
            Pair& operator=(Pair&&) = default;
            
            A a;
            B b;
            kit::signal<void(Node*, Node*)> on_collision = kit::signal<void(Node*, Node*)>();
            kit::signal<void(Node*, Node*)> on_no_collision = kit::signal<void(Node*, Node*)>();
            kit::signal<void(Node*, Node*)> on_touch = kit::signal<void(Node*, Node*)>();
            kit::signal<void(Node*, Node*)> on_untouch = kit::signal<void(Node*, Node*)>();

            std::shared_ptr<bool> recheck = std::make_shared<bool>(true);
            
            //std::unique_ptr<boost::signals2::signal<
            //    void(Node*, Node*)
            //>> on_touch = kit::make_unique<boost::signals2::signal<
            //    void(Node*, Node*)
            //>>();
            //std::unique_ptr<boost::signals2::signal<
            //    void(Node*, Node*)
            //>> on_no_collision = kit::make_unique<boost::signals2::signal<
            //    void(Node*, Node*)
            //>>();
            //std::unique_ptr<boost::signals2::signal<
            //    void(Node*, Node*)
            //>> on_untouch = kit::make_unique<boost::signals2::signal<
            //    void(Node*, Node*)
            //>>();

            bool collision = false;
        };
        
        //virtual void clear_collision(const std::shared_ptr<Node>& n) override;

        virtual void clear() override {
            m_IntertypeCollisions.clear();
            m_TypedCollisions.clear();
            m_Collisions.clear();
        }
        
        virtual bool empty() const override {
            return m_Nodes.empty() &&
                m_Collisions.empty() &&
                m_IntertypeCollisions.empty() &&
                m_TypedCollisions.empty() &&
                m_Lights.empty() &&
                m_Nodes.empty();
        }
        virtual bool has_collisions() const override {
            return (not m_Collisions.empty()) ||
                (not m_TypedCollisions.empty());
        }
            
        //void filter(unsigned typ, std::function<Node*>(Node*) func) {
        //    m_PotentialColliders[typ] = func;
        //}

        std::vector<std::weak_ptr<Node>> get_potentials(
            Node* obj, unsigned typ
        );
        
        void after(std::function<void()> func);
        
    private:

        struct ObjectList
        {
            // all objects in list are the same type
            std::vector<std::weak_ptr<Node>> objects;
            std::shared_ptr<bool> recheck = std::make_shared<bool>(true);
        };

        // type (index) -> objects that can be collided with
        std::vector<ObjectList> m_Objects;
        
        // list of type->type pairs 
        std::vector<Pair<unsigned, unsigned>> m_IntertypeCollisions;
        std::vector<Pair<std::weak_ptr<Node>, unsigned>> m_TypedCollisions;
        std::vector<Pair<std::weak_ptr<Node>, std::weak_ptr<Node>>> m_Collisions;
        std::map<unsigned, std::function<std::vector<std::weak_ptr<Node>>(Box)>> m_Providers;
        
        std::vector<const Node*> m_Nodes;
        std::vector<const Light*> m_Lights;
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

        std::vector<std::function<void()>> m_Pending;
        int m_Recur = 0;
};

#endif

