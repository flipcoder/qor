#ifndef _BASICPIPELINE_H_2M5917IG
#define _BASICPIPELINE_H_2M5917IG

#include "IPipeline.h"
#include "Node.h"
#include "Window.h"
#include "Camera.h"
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
            Pass* pass,
            unsigned int id
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
            BASE = 0, // for ambient pass
            NORMAL
        };
        
        Style shader() const {
            return (Style)m_ActiveSlot;
        }

        virtual Color bg_color() const override {
            return m_BGColor;
        }
        virtual void bg_color(const Color& c) override {
            m_BGColor = c;
        }
        
        virtual void ortho(bool b) override;
        
        virtual void root(const std::shared_ptr<Camera>& root) {
            m_pRoot = root;
        }
        
    private:

        void shader(Style style = Style::NORMAL) {
            m_ActiveSlot = style;
            GL_TASK_START()
                m_Shaders.at((unsigned)m_ActiveSlot).m_pShader->use();
            GL_TASK_END()
        }

        void load_shaders(std::vector<std::string> name);

        std::vector<ShaderSlot> m_Shaders;
        std::weak_ptr<Node> m_pRoot;
        std::weak_ptr<Node> m_pCamera;
        std::shared_ptr<BasicPartitioner> m_pPartitioner;
        Style m_ActiveSlot=Style::NORMAL;
        Color m_BGColor;

        Window* m_pWindow;

        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        //glm::mat4 m_ModelMatrix;
        //glm::mat4 m_ModelViewMatrix;
        glm::mat4 m_ViewProjectionMatrix;
        glm::mat4 m_ModelViewProjectionMatrix;
};

#endif

