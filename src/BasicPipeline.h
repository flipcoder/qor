#ifndef _BASICPIPELINE_H_2M5917IG
#define _BASICPIPELINE_H_2M5917IG

#include "IPipeline.h"
#include "Node.h"
#include "Window.h"
#include "GLTask.h"
#include "Graphics.h"
class BasicPartitioner;

class BasicPipeline:
    public IPipeline
{
    public:

        struct ShaderSlot {
            std::shared_ptr<Program> m_pShader;

            Program::UniformID m_ModelViewProjectionID = 0;
            Program::UniformID m_TextureID = 0;
            unsigned m_TextureSlots = 0;
        };

        BasicPipeline(
            Window* window,
            const std::shared_ptr<Node>& root = std::shared_ptr<Node>(),
            const std::shared_ptr<Node>& camera = std::shared_ptr<Node>()
        );
        virtual ~BasicPipeline() {}

        /*
         * Sets the model matrix during a pass
         * To be called inside an Node's render() function before renderSelf()
         */
        virtual void matrix(Pass* pass, const glm::mat4* m) override;
        virtual void texture(
            unsigned id,
            unsigned slot
        ) override;

        virtual void render() override;

        //void set_root(Node* node) {
        //    assert(!m_pRoot->parent())
        //    m_pRoot = node;
        //}

        virtual std::shared_ptr<Node> camera() override { return m_pCamera.lock(); }
        void camera(const std::shared_ptr<Node>& camera) {
            m_pCamera=camera;
        }
        std::shared_ptr<Node> root() { return m_pRoot.lock(); }

        //Partitioner* partitioner() { return m_pPartitioner.get(); }

        enum class Style {
            NONE = -1,
            BASE = 0, // for ambient pass
            NORMAL,
            LIGHT, // rendering to shadow map
            USER,
        };
        
        Style pass() const {
            return (Style)m_ActiveSlot;
        }

        virtual Color bg_color() const override {
            return m_BGColor;
        }
        virtual void bg_color(const Color& c) override {
            m_BGColor = c;
        }
        
        virtual void ortho(bool b) override;
        
        virtual void root(const std::shared_ptr<Node>& root) {
            m_pRoot = root;
        }
        
        virtual void shader(
            std::shared_ptr<Program> p
        ) override;
        virtual void shader(std::nullptr_t) override;
        virtual std::shared_ptr<Program> shader(unsigned slot) const override;

    private:

        unsigned m_OpenTextureSlots=0;
        
        void shader(
            Style style = Style::NORMAL,
            std::shared_ptr<Program> p = std::shared_ptr<Program>()
        );

        std::shared_ptr<Program> m_pCurrentShader;
        std::shared_ptr<Program> m_pUserShader;

        // Called from ctor, may be run in GL task thread
        void load_shaders(std::vector<std::string> name);

        std::vector<ShaderSlot> m_Shaders;
        std::weak_ptr<Node> m_pRoot;
        std::weak_ptr<Node> m_pCamera;
        std::shared_ptr<BasicPartitioner> m_pPartitioner;
        Style m_ActiveSlot = Style::NONE;
        Color m_BGColor;
        const float m_DefaultFOV = 80.0f;

        Window* m_pWindow;

        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        //glm::mat4 m_ModelMatrix;
        //glm::mat4 m_ModelViewMatrix;
        glm::mat4 m_ViewProjectionMatrix;
        glm::mat4 m_ModelViewProjectionMatrix;
};

#endif

