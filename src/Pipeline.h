#ifndef _BASICPIPELINE_H_2M5917IG
#define _BASICPIPELINE_H_2M5917IG

#include "Node.h"
#include "Window.h"
#include "GLTask.h"
#include "Graphics.h"
#include "Pass.h"
#include "PipelineShader.h"
#include "kit/cache/cache.h"
#include "kit/args/args.h"
#include "IRealtime.h"
#include <functional>

class BasicPartitioner;
class Camera;
class Window;
class Pipeline:
    virtual public kit::mutexed<std::recursive_mutex>,
    public IRealtime
{
    public:

        enum AttributeFlags {
            ATTR_NONE = 0,
            VERTEX = kit::bit(0),
            WRAP = kit::bit(1),
            NORMAL = kit::bit(2),
            TANGENT = kit::bit(3),
            BINORMAL = kit::bit(4),
            COLOR = kit::bit(5),
            ATTR_MASK = kit::mask(6)
        };

        enum RenderFlags
        {
            RENDER_NONE = 0,
            NO_CLEAR = kit::bit(0),
            NO_DEPTH = kit::bit(1),
            LIGHTS = kit::bit(2),
            RENDER_MASK = kit::mask(2)
        };
        
        enum class AttributeID : unsigned
        {
            VERTEX,
            WRAP,
            NORMAL,
            TANGENT,
            BINORMAL,
            COLOR,
            MAX
        };
        
        Pipeline(
            Window* window,
            Args& args,
            //const std::shared_ptr<Meta>& sys_cfg,
            Cache<Resource, std::string>* cache = nullptr
        );
        virtual ~Pipeline();

        virtual void logic(Freq::Time t) override;

        /*
         * Sets the model matrix during a pass
         * To be called inside an Node's render() function before renderSelf()
         */
        virtual void matrix(Pass* pass, const glm::mat4* m);
        virtual void texture_slots(unsigned slot_flags, unsigned max_tex=2);
        virtual void texture(unsigned id, unsigned slot);
        virtual void texture_nobind(unsigned slot);
        virtual unsigned attribute_id(AttributeID id);

        virtual void render(
            Node* root,
            Camera* camera,
            IPartitioner* partitioner = nullptr,
            unsigned flags = 0
        );

        const BasicPartitioner* partitioner() const {
            return m_pPartitioner.get();
        }
        BasicPartitioner* partitioner() {
            return m_pPartitioner.get();
        }

        //void set_root(Node* node) {
        //    assert(!m_pRoot->parent())
        //    m_pRoot = node;
        //}

        //virtual std::shared_ptr<Node> camera() { return m_pCamera.lock(); }
        //void camera(const std::shared_ptr<Node>& camera) {
        //    m_pCamera=camera;
        //}
        
        void pass(Pass* pass) {m_pPass=pass;}
        Pass* pass() { return m_pPass; }
        const Pass* pass() const { return m_pPass; }

        void light(const Light* light, unsigned slot = 0);
        //const Light* light() const { return m_pLight; }
        //std::shared_ptr<Node> root() { return m_pRoot.lock(); }

        PassType slot() const {
            auto l = this->lock();
            return (PassType)m_ActiveShader;
        }

        virtual Color bg_color() const {
            auto l = this->lock();
            return m_BGColor;
        }
        virtual void bg_color(const Color& c) {
            auto l = this->lock();
            m_BGColor = c;
        }
        
        //void ortho(bool origin_bottom = false);
        //void perspective(float fov = 80.0f);

        void winding(bool cw = false);
        
        //virtual void root(const std::shared_ptr<Node>& root) {
        //    m_pRoot = root;
        //}
        
        //virtual void custom_shader(
        //    std::shared_ptr<Program> p
        //);
        //virtual void reset_shader();
        virtual std::shared_ptr<Program> shader(unsigned slot);
        
        unsigned layout(unsigned attrs);
        
        bool blend() const {
            auto l = this->lock();
            return m_bBlend;
        }
        void blend(bool b) {
            auto l = this->lock();
            m_bBlend = b;
        }

        //PassType detail() const {
        //    auto l = this->lock();
        //    return m_Detail;
        //}
        //void detail(PassType b) {
        //    auto l = this->lock();
        //    m_Detail = b;
        //}
        void material(Color a, Color d, Color s, Color e);

        //void smooth(bool b = true) {
        //    GL_TASK_START()
        //        if(b)
        //            glEnable(GL_POLYGON_SMOOTH);
        //        else
        //            glDisable(GL_POLYGON_SMOOTH);
        //    GL_TASK_END()
        //}

        
        boost::signals2::signal<void(Pass* pass)> on_pass;
        //boost::signals2::signal<void()> on_remove;
        //boost::signals2::signal<void()> on_pend;

        unsigned load_shaders(std::vector<std::string> name);
        void override_shader(PassType p, unsigned id);
        void clear_shader_overrides();
        std::shared_ptr<Program> shader();
        
    private:

        //unsigned m_OpenTextureSlots = 0;
        
        void shader(PassType type);
        void clear_shaders()
        {
            auto l = lock();
            m_Shaders.clear();
        }

        //std::shared_ptr<Program> m_pCurrentShader;
        //std::shared_ptr<Program> m_pUserShader;
        std::vector<std::shared_ptr<PipelineShader>> m_Shaders;
        //std::weak_ptr<Node> m_pRoot;
        //std::weak_ptr<Node> m_pCamera;
        std::shared_ptr<BasicPartitioner> m_pPartitioner;
        const Light* m_pLight = nullptr;
        PassType m_ActiveShader = PassType::NONE;
        Color m_BGColor;
        constexpr static float m_DefaultFOV = 80.0f;
        bool m_bBlend = false;

        Window* m_pWindow;

        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ModelMatrix;
        glm::mat4 m_ModelViewMatrix;
        //glm::mat4 m_ViewProjectionMatrix;
        glm::mat4 m_ModelViewProjectionMatrix;
        glm::mat4 m_NormalMatrix;
        // TODO: texture matrix

        Cache<Resource, std::string>* m_pCache;
        Pass* m_pPass = nullptr;
        //PassType m_Detail = PassType::NORMAL;

        // pass type -> new (user) pass type id
        std::vector<unsigned> m_ShaderOverrides;
        
        const static std::vector<std::string> s_TextureUniformNames;
        const static std::vector<std::string> s_AttributeNames;
};

#endif

