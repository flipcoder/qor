#ifndef _CAMERA_H_K8EZAF7N
#define _CAMERA_H_K8EZAF7N

#include "Tracker.h"
#include "kit/kit.h"
#include "kit/freq/animation.h"
#include "kit/factory/factory.h"
#include "kit/cache/cache.h"
#include <boost/signals2.hpp>
#include "Audio.h"

class Window;
class Camera:
    public Tracker
{
    public:
        typedef std::function<bool(const Node*, Node::LoopCtrl* lc)> NodeVisibleFunc_t;

        Camera(const std::string& fn, IFactory* factory, ICache* cache);
        Camera(const std::tuple<std::string, IFactory*, ICache*>& args):
            Camera(std::get<0>(args), std::get<1>(args), std::get<2>(args))
        {
            init();
        }

        //Camera() {init();}
        Camera(Cache<Resource, std::string>* cache, Window* w):
            m_pResources(cache)
        {
            init();
            window(w);
        }
        //Camera(
        //    const std::shared_ptr<Node>& target
        //):
        //    Tracker(target)
        //{
        //    init();
        //}

        bool is_ortho() const { return m_bOrtho; }
        void ortho(bool origin_bottom = false);
        void perspective(float fov);
        void perspective() {
            perspective(m_FOV);
        }
        void window(Window* window);

        virtual ~Camera() {}

        virtual void logic_self(Freq::Time t) override;

        void fov(float f) {
            m_FOV=f;
            recalculate_projection();
        }
        float fov() const {
            return m_FOV;
        }

        bool in_frustum(const Box& box) const;
        bool in_frustum(glm::vec3 point) const;

        const glm::mat4& projection() const;
        const glm::mat4& view() const;
        
        enum Flag {
            ORTHO = kit::bit(0),
            BOTTOM_ORIGIN = kit::bit(1)
        };
        
        const unsigned DEFAULT_FLAGS = ORTHO;
        
        bool listens() const { return m_bListens; }
        void listen(bool b = true) { m_bListens = b; }
        
        void range(float n, float f);

        bool has_node_visible_func() const {
            return bool(m_IsNodeVisible);
        }
        void set_node_visible_func(NodeVisibleFunc_t f) {
            m_IsNodeVisible = f;
        }
        void clear_node_visible_func() {
            m_IsNodeVisible = NodeVisibleFunc_t();
        }
        
        //bool is_self_visible(const Node* n) const;
        bool is_visible(const Node* n, Node::LoopCtrl* lc = nullptr) const;

        void calculate_perspective_frustum();
        
        virtual std::string type() const override { return "camera"; }

        Box ortho_frustum() const {
            return m_OrthoFrustum();
        }

        bool is_visible_func(const Node* n, Node::LoopCtrl* lc) {
            if(m_IsNodeVisible)
                return m_IsNodeVisible(n, lc);
            return true;
        }

        //bool is_node_visible_func(Node* n) {
        //    if(m_IsNodeVisible)
        //        return m_IsNodeVisible(n);
        //    return true;
        //}
        
    private:

        NodeVisibleFunc_t m_IsNodeVisible;
        
        float m_ZNear = 0.0f;
        float m_ZFar = 0.0f;
        
        unsigned m_Flags = DEFAULT_FLAGS;

        bool m_bInited = false;
        bool m_bListens = false;

        void init();
        
        //void view_update() const;
        //void frustum_update() const;
        
        void recalculate_projection();
        
        float m_FOV = 80.0f;
        
        mutable glm::mat4 m_ProjectionMatrix;
        
        // TODO: cache and update on_pend
        mutable kit::lazy<glm::mat4> m_ViewMatrix;
        
        boost::signals2::scoped_connection m_WindowResize;

        //float m_ZNear;
        //float m_ZFar;

        bool m_bOrtho = true;
        bool m_bBottomOrigin = true;
        glm::ivec2 m_Size;
        //bool m_bWindingCW = false;

        mutable kit::lazy<Box> m_OrthoFrustum;
        
        Audio::Listener m_Listener;
        
        Cache<Resource, std::string>* m_pResources;

        boost::signals2::scoped_connection m_VolumeCon;
};

#endif

