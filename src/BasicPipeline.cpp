#include "BasicPipeline.h"
#include "BasicPartitioner.h"
#include "Window.h"
#include <cassert>
#include <cmath>
#include <glm/glm.hpp>
#include "GLTask.h"
#include "Camera.h"
//#include <glm/gtc/matrix_transform.hpp>

using namespace std;

BasicPipeline :: BasicPipeline(
    Window* window,
    const shared_ptr<Node>& root,
    const shared_ptr<Node>& camera
):
    m_pWindow(window),
    m_pRoot(root),
    m_pCamera(camera)
{
    assert(m_pWindow);
    //assert(m_pCamera.lock());
    //assert(m_pRoot.lock());

    m_pPartitioner = make_shared<BasicPartitioner>();

    //load_shaders(vector<string> {"basic", "bw"});

    GL_TASK_START()
        load_shaders(vector<string> {"base", "basic"});

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_MULTISAMPLE);
        //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_CULL_FACE);
        
        for(auto&& slot: m_Shaders) {
            slot.m_ModelViewProjectionID = slot.m_pShader->uniform(
                "ModelViewProjection"
            );
            slot.m_TextureID = slot.m_pShader->uniform("Texture");
            //LOGf("%s", slot.m_TextureID);
        }
    GL_TASK_END()
    //glEnable(GL_POLYGON_SMOOTH); // don't use this

    ortho(true);

    //float* f =  glm::value_ptr(m_ProjectionMatrix);
    //f[0] = std::round(f[i]);

    //m_ProjectionMatrix = glm::perspective(
    //    80.0f,
    //    16.0f / 9.0f,
    //    -101.0f,
    //    100.0f
    //);
}

void BasicPipeline :: load_shaders(vector<string> names)
{
    m_Shaders.clear();

    const string path = "shaders/";
    for(auto&& name: names) {
        ShaderSlot s;
        // below might throw
        s.m_pShader = make_shared<Program>(
            make_shared<Shader>(path + name + ".vp", Shader::VERTEX),
            make_shared<Shader>(path + name + ".fp", Shader::FRAGMENT)
        );
        m_Shaders.push_back(std::move(s));
    }
    m_ActiveSlot = Style::NORMAL;
    m_Shaders.at((unsigned)m_ActiveSlot).m_pShader->use();
}

void BasicPipeline :: matrix(Pass* pass, const glm::mat4* m)
{
    // current shaders expect one MVP matrix, so we'll do this CPU side
    m_ModelViewProjectionMatrix = m_ViewProjectionMatrix * *m;
    GL_TASK_START()
        m_Shaders.at((unsigned)m_ActiveSlot).m_pShader->uniform(
            m_Shaders.at((unsigned)m_ActiveSlot).m_ModelViewProjectionID,
            m_ModelViewProjectionMatrix
        );
    GL_TASK_END()
}

void BasicPipeline :: texture(
    unsigned id, unsigned slot
){
    GL_TASK_START()
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
        m_Shaders.at((unsigned)m_ActiveSlot).m_pShader->uniform(
            m_Shaders.at((unsigned)m_ActiveSlot).m_TextureID,
            (int)slot
        );
    GL_TASK_END()
}

void BasicPipeline :: render()
{
    assert(m_pWindow);
    if(!m_pRoot.lock())
        return;
    if(!m_pCamera.lock())
        return;

    m_ViewMatrix = glm::inverse(*m_pCamera.lock()->matrix_c(Space::WORLD));
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    
    GL_TASK_START()

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(m_BGColor.r(), m_BGColor.g(), m_BGColor.b(), 1.0f);
    
    glDisable(GL_BLEND);
    Pass base_pass(m_pPartitioner.get(), this, Pass::RECURSIVE | Pass::BASE);
    shader(Style::BASE);
    m_pRoot.lock()->render(&base_pass);
    
    // TODO: For each light...
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_BLEND);
    Pass pass(m_pPartitioner.get(), this, Pass::RECURSIVE);
    shader(Style::NORMAL);
    m_pRoot.lock()->render(&pass);

    GL_TASK_END()
}

void BasicPipeline :: ortho(bool o)
{
    auto camera = dynamic_pointer_cast<Camera>(m_pCamera.lock());
    if(o)
    {
        float aspect_ratio = static_cast<float>(m_pWindow->aspect_ratio());
        m_ProjectionMatrix = glm::ortho(
            0.0f,
            static_cast<float>(m_pWindow->size().x),
            0.0f,
            static_cast<float>(m_pWindow->size().y),
            -100.0f,
            100.0f
        );
    }
    else
    {
        float aspect_ratio = static_cast<float>(m_pWindow->aspect_ratio());
        m_ProjectionMatrix = glm::perspective(
            camera ? camera->fov() : m_DefaultFOV,
            16.0f / 9.0f,
            0.1f,
            100.0f
        );
    }
}

void BasicPipeline :: shader(
    Style style,
    std::shared_ptr<Program> shader
){
    GL_TASK_START()
        m_Shaders.at((unsigned)m_ActiveSlot).m_pShader->use();
    GL_TASK_END()
    
    //if(style != m_ActiveSlot || (shader && shader != m_pCurrentShader)) {
    //    if(!shader)
    //        m_pCurrentShader = m_Shaders.at((unsigned)m_ActiveSlot).m_pShader;
    //    else
    //        m_pCurrentShader = shader;
    //    m_ActiveSlot = style;
    //    m_OpenTextureSlots =
    //        m_Shaders.at((unsigned)m_ActiveSlot).m_TextureSlots;
    //    GL_TASK_START()
    //        for(int i=0; i<m_OpenTextureSlots; ++i)
    //            texture(i, 0);
    //        m_pCurrentShader->use();
    //    GL_TASK_END()
    //}
}

void BasicPipeline :: shader(std::shared_ptr<Program> p)
{
    shader(m_ActiveSlot, p);
}

void BasicPipeline :: shader(std::nullptr_t)
{
    shader(std::shared_ptr<Program>());
}

std::shared_ptr<Program> BasicPipeline :: shader(unsigned slot) const
{
    return m_Shaders.at(slot).m_pShader;
}

