#ifndef _BASICPIPELINE_H_2M5917IG
#define _BASICPIPELINE_H_2M5917IG

#include "Node.h"
#include "Window.h"
#include "GLTask.h"
#include "Graphics.h"
#include "Pass.h"
#include "PipelineShader.h"
#include "kit/cache/cache.h"
#include <functional>
class BasicPartitioner;

class Camera;
class Window;
class Pipeline:
    virtual public kit::mutexed<std::recursive_mutex>
{
    public:

        enum AttributeFlags {
            NONE = 0,
            VERTEX = kit::bit(0),
            WRAP = kit::bit(1),
            NORMAL = kit::bit(2),
            TANGENT = kit::bit(3),
            COLOR = kit::bit(4),
            MASK = kit::mask(5)
        };
        
        enum class AttributeID : unsigned
        {
            VERTEX,
            WRAP,
            NORMAL,
            TANGENT,
            COLOR,
            MAX
        };
        
        Pipeline(
            Window* window,
            Cache<Resource, std::string>* cache = nullptr
        );
        virtual ~Pipeline();

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
            Node* root, Camera* camera);//, std::function<void(Pass*)> with_pass =
        //        std::function<void(Pass*)>()
        //);

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

        void light(const Light* light);
        const Light* light() const { return m_pLight; }
        //std::shared_ptr<Node> root() { return m_pRoot.lock(); }

        //Partitioner* partitioner() { return m_pPartitioner.get(); }
        
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
        
        virtual void ortho(bool b, float fov = m_DefaultFOV);
        
        //virtual void root(const std::shared_ptr<Node>& root) {
        //    m_pRoot = root;
        //}
        
        virtual void shader(
            std::shared_ptr<Program> p
        );
        virtual void shader(std::nullptr_t);
        virtual std::shared_ptr<Program> shader(unsigned slot) const;
        
        unsigned layout(unsigned attrs);
        
        bool blend() const {
            return m_bBlend;
        }
        void blend(bool b) {
            m_bBlend = b;
        }
        
        boost::signals2::signal<void(Pass* pass)> on_pass;
        
    private:

        //unsigned m_OpenTextureSlots = 0;
        
        void shader(
            PassType style = PassType::NORMAL,
            std::shared_ptr<Program> p = std::shared_ptr<Program>()
        );

        //std::shared_ptr<Program> m_pCurrentShader;
        //std::shared_ptr<Program> m_pUserShader;

        // Called from ctor, may be run in GL task thread
        void load_shaders(std::vector<std::string> name);

        std::vector<std::shared_ptr<PipelineShader>> m_Shaders;
        //std::weak_ptr<Node> m_pRoot;
        //std::weak_ptr<Node> m_pCamera;
        const Light* m_pLight = nullptr;
        std::shared_ptr<BasicPartitioner> m_pPartitioner;
        PassType m_ActiveShader = PassType::NONE;
        Color m_BGColor;
        constexpr static float m_DefaultFOV = 80.0f;
        bool m_bBlend = false;

        Window* m_pWindow;

        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        //glm::mat4 m_ModelMatrix;
        glm::mat4 m_ModelViewMatrix;
        //glm::mat4 m_ViewProjectionMatrix;
        glm::mat4 m_ModelViewProjectionMatrix;
        glm::mat4 m_NormalMatrix;
        // TODO: texture matrix

        Cache<Resource, std::string>* m_pCache;
        Pass* m_pPass = nullptr;
        
        const static std::vector<std::string> s_TextureUniformNames;
        const static std::vector<std::string> s_AttributeNames;
};

#endif

