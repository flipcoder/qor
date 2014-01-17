#ifndef _BASICPIPELINE_H_2M5917IG
#define _BASICPIPELINE_H_2M5917IG

#include "Node.h"
#include "Window.h"
#include "GLTask.h"
#include "Graphics.h"
#include "Pass.h"
#include "PipelineShader.h"
#include "kit/cache/cache.h"
class BasicPartitioner;

class Window;
class Pipeline
{
    public:

        enum AttributeFlags {
            NONE = 0,
            VERTEX = kit::bit(0),
            WRAP = kit::bit(1),
            NORMAL = kit::bit(2),
            MASK = kit::mask(3)
        };
        
        enum class AttributeID : unsigned
        {
            VERTEX,
            WRAP,
            NORMAL,
            MAX
        };
        
        Pipeline(
            Window* window,
            Cache<Resource, std::string>* cache = nullptr,
            const std::shared_ptr<Node>& root = std::shared_ptr<Node>(),
            const std::shared_ptr<Node>& camera = std::shared_ptr<Node>()
        );
        virtual ~Pipeline();

        /*
         * Sets the model matrix during a pass
         * To be called inside an Node's render() function before renderSelf()
         */
        virtual void matrix(Pass* pass, const glm::mat4* m);
        virtual void texture(
            unsigned id,
            unsigned slot
        );

        virtual void render();

        //void set_root(Node* node) {
        //    assert(!m_pRoot->parent())
        //    m_pRoot = node;
        //}

        virtual std::shared_ptr<Node> camera() { return m_pCamera.lock(); }
        void camera(const std::shared_ptr<Node>& camera) {
            m_pCamera=camera;
        }
        std::shared_ptr<Node> root() { return m_pRoot.lock(); }

        //Partitioner* partitioner() { return m_pPartitioner.get(); }
        
        PassType slot() const {
            return (PassType)m_ActiveSlot;
        }

        virtual Color bg_color() const {
            return m_BGColor;
        }
        virtual void bg_color(const Color& c) {
            m_BGColor = c;
        }
        
        virtual void ortho(bool b);
        
        virtual void root(const std::shared_ptr<Node>& root) {
            m_pRoot = root;
        }
        
        virtual void shader(
            std::shared_ptr<Program> p
        );
        virtual void shader(std::nullptr_t);
        virtual std::shared_ptr<Program> shader(unsigned slot) const;
        
        void layout(unsigned attrs);
        
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
        std::weak_ptr<Node> m_pRoot;
        std::weak_ptr<Node> m_pCamera;
        std::shared_ptr<BasicPartitioner> m_pPartitioner;
        PassType m_ActiveSlot = PassType::NONE;
        Color m_BGColor;
        const float m_DefaultFOV = 80.0f;

        Window* m_pWindow;

        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        //glm::mat4 m_ModelMatrix;
        glm::mat4 m_ModelViewMatrix;
        //glm::mat4 m_ViewProjectionMatrix;
        glm::mat4 m_ModelViewProjectionMatrix;
        glm::mat4 m_NormalMatrix;
        
        unsigned m_Layout = 0;

        Cache<Resource, std::string>* m_pCache;
};

#endif

