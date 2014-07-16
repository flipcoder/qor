#include "Pipeline.h"
#include "BasicPartitioner.h"
#include "Window.h"
#include "Light.h"
#include <cassert>
#include <cmath>
#include <glm/glm.hpp>
#include "GLTask.h"
#include "Camera.h"
//#include <glm/gtc/matrix_transform.hpp>

using namespace std;

const std::vector<std::string> Pipeline :: s_TextureUniformNames = {
    "Nrm",
    "Disp",
    "Spec",
    "Occ"
};

const std::vector<std::string> Pipeline :: s_AttributeNames = {
    "Position",
    "Wrap",
    "Normal",
    "Tangent",
    "Color"
};

Pipeline :: Pipeline(
    Window* window,
    Args& args,
    //const std::shared_ptr<Meta<kit::dummy_mutex>>& sys_cfg,
    Cache<Resource, std::string>* cache
):
    m_pWindow(window),
    m_pCache(cache)
{
    assert(m_pWindow);
    //assert(m_pCamera.lock());
    //assert(m_pRoot.lock());

    m_pPartitioner = make_shared<BasicPartitioner>();

    m_ActiveShader = PassType::NORMAL;
    GL_TASK_START()
        
        // these should line up with Graphics.h's PassTypes
        load_shaders({
            args.value_or("base_shader", "base"),
            args.value_or("basic_shader", "basic")
        });
        
        glEnable(GL_MULTISAMPLE);
        //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        
        for(auto&& slot: m_Shaders) {
            slot->m_ModelViewProjectionID = slot->m_pShader->uniform(
                "ModelViewProjection"
            );
            slot->m_ModelViewID = slot->m_pShader->uniform(
                "ModelView"
            );
            slot->m_ViewID = slot->m_pShader->uniform(
                "View"
            );
            slot->m_NormalID = slot->m_pShader->uniform(
                "NormalMatrix"
            );
            
            for(int i=0;i < int(s_TextureUniformNames.size() + 1);++i) {
                int tex_id = slot->m_pShader->uniform(
                    (boost::format("Texture%s")%(
                        i?
                            s_TextureUniformNames.at(i-1)
                        :
                            ""
                    )).str()
                );
                if(tex_id == -1)
                    break;
                slot->m_Textures.resize(i+1);
                slot->m_Textures.at(i) = tex_id;
            }
        }
        
        ortho(true);
         
        //glEnable(GL_POLYGON_SMOOTH); // don't use this for 2D
        assert(glGetError() == GL_NO_ERROR);
        
        
    GL_TASK_END()
}

Pipeline :: ~Pipeline()
{
    //auto l = this->lock();
    
    //GL_TASK_START()
    //    auto l = this->lock();
    //    layout(0);
    //    texture_slots(~0,8);
    //GL_TASK_END()
}

void Pipeline :: load_shaders(vector<string> names)
{
    auto l = this->lock();
    
    m_Shaders.clear();

    for(auto&& name: names)
    {
        auto shader = m_pCache->cache_as<PipelineShader>(name+".json");
        m_Shaders.push_back(shader);
        
        unsigned layout = 0;
        unsigned i = 0;
        for(auto&& attr_name: s_AttributeNames) {
            auto attr_id = shader->m_pShader->attribute((boost::format("Vertex%s")%
                attr_name
            ).str());
            if(attr_id != (unsigned)-1) {
                LOGf("attr: %s (%s)", attr_name % attr_id);
                shader->m_Attributes.resize(i+1);
                shader->m_Attributes.at(i) = attr_id;
                shader->m_SupportedLayout |= (1 << i);
            }
            else {
                WARNINGf("missing attribute %s", attr_name);
            }
            
            ++i;
        }
    }
    //m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->use();
}

void Pipeline :: matrix(Pass* pass, const glm::mat4* m)
{
    auto l = this->lock();
    
    m_ModelViewMatrix = m_ViewMatrix * *m;
    m_NormalMatrix = glm::transpose(glm::inverse(m_ModelViewMatrix));
    m_ModelViewProjectionMatrix = m_ProjectionMatrix * m_ModelViewMatrix;
    //m_ModelViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix * *m;
    
    GL_TASK_START()
        auto l = this->lock();
        m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
            m_Shaders.at((unsigned)m_ActiveShader)->m_ModelViewProjectionID,
            m_ModelViewProjectionMatrix
        );
        m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
            m_Shaders.at((unsigned)m_ActiveShader)->m_ModelViewID,
            m_ModelViewMatrix
        );
        m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
            m_Shaders.at((unsigned)m_ActiveShader)->m_ViewID,
            m_ViewMatrix
        );
        m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
            m_Shaders.at((unsigned)m_ActiveShader)->m_NormalID,
            m_NormalMatrix
        );
    GL_TASK_END()
}

void Pipeline :: texture(
    unsigned id, unsigned slot
){
    GL_TASK_START()
        auto l = this->lock();
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
        try{
            m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
                m_Shaders.at((unsigned)m_ActiveShader)->m_Textures.at(slot),
                (int)slot
            );
        }catch(...){
            assert(false);
        }
    GL_TASK_END()
}

void Pipeline :: texture_nobind(
    unsigned slot
){
    auto l = this->lock();
    GL_TASK_START()
        auto l = this->lock();
        glActiveTexture(GL_TEXTURE0 + slot);
    GL_TASK_END()
}

void Pipeline :: render(Node* root, Camera* camera)
{
    auto l = this->lock();
    assert(m_pWindow);
    if(!root)
        return;
    if(!camera)
        return;
    //if(!m_pRoot.lock())
    //    return;
    //if(!m_pCamera.lock())
    //    return;

    m_ViewMatrix = glm::inverse(*camera->matrix_c(Space::WORLD));
    //m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    
    //l.unlock();
    
    GL_TASK_START()
        auto l = this->lock();

        //std::shared_ptr<Node> root = m_pRoot.lock();
        assert(glGetError() == GL_NO_ERROR);

        // set up initial state
        //glViewport(0,0,m_pWindow->size().x/2,m_pWindow->size().y/2);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(m_BGColor.r(), m_BGColor.g(), m_BGColor.b(), 1.0f);
        
        Pass pass(m_pPartitioner.get(), this, Pass::BASE | Pass::RECURSIVE);
        this->pass(&pass);
        m_pPartitioner->camera(camera);
        m_pPartitioner->partition(root);
        bool has_lights = false;
        try{
            if(m_pPartitioner->visible_lights().at(0)) {
                pass.flags(pass.flags() & ~Pass::RECURSIVE);
                has_lights = true;
            }
        }catch(const std::out_of_range&){}
        
        if(m_bBlend)
        {
            glDisable(GL_DEPTH_TEST);
        }
        else
        {
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);

            shader(PassType::BASE);

            // render base ambient pass
            on_pass(&pass);
            if(pass.flags() & Pass::RECURSIVE)
                root->render(&pass);
            else
                for(const auto& node: m_pPartitioner->visible_nodes()) {
                    if(!node)
                        break;
                    node->render(&pass);
                }
        }

        // set up multi-pass state
        if(m_bBlend) {
            //glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        }else{
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        
        glEnable(GL_BLEND);
        
        pass.flags(pass.flags() & ~Pass::BASE);

        shader(m_Detail);
        if(!has_lights)
        {
            on_pass(&pass);

            // render detail pass (no lights)
            if(pass.flags() & Pass::RECURSIVE)
                root->render(&pass);
            else
                for(const auto& node: m_pPartitioner->visible_nodes()) {
                    if(!node)
                        break;
                    node->render(&pass);
                }
        }
        else
        {
            on_pass(&pass);

            // render each light pass
            for(const auto& light: m_pPartitioner->visible_lights()) {
                if(!light)
                    break;
                this->light(light);
                for(const auto& node: m_pPartitioner->visible_nodes_from(light)) {
                    if(!node)
                        break;
                    node->render(&pass);
                }
            }
        }

        this->light(nullptr);
        this->pass(nullptr);
    GL_TASK_END()
}

void Pipeline :: ortho(bool origin_bottom)
{
    auto l = this->lock();
    m_ProjectionMatrix = glm::ortho(
        0.0f,
        static_cast<float>(m_pWindow->size().x),
        origin_bottom ? 0.0f : static_cast<float>(m_pWindow->size().y),
        origin_bottom ? static_cast<float>(m_pWindow->size().y) : 0.0f,
        -100.0f,
        100.0f
        //origin_bottom ? -100.0f : 100.0f,
        //origin_bottom ? 100.0f : -100.0f
    );
    GL_TASK_START()
        glFrontFace(origin_bottom ? GL_CCW : GL_CW);
    GL_TASK_END()
}

void Pipeline :: perspective(float fov)
{
    auto l = this->lock();
    float aspect_ratio = static_cast<float>(m_pWindow->aspect_ratio());
    m_ProjectionMatrix = glm::perspective(
        fov,
        aspect_ratio,
        0.01f,
        1000.0f
    );
    GL_TASK_START()
        glFrontFace(GL_CCW);
    GL_TASK_END()
}

void Pipeline :: shader(
    PassType style,
    std::shared_ptr<Program> shader
){
    auto l = this->lock();
    
    //LOGf("style: %s", (unsigned)style);
    GL_TASK_START()
        auto l = this->lock();
        assert(glGetError() == GL_NO_ERROR);
        m_ActiveShader = style;
        m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->use();
        assert(glGetError() == GL_NO_ERROR);
    GL_TASK_END()
    
    //if(style != m_ActiveShader || (shader && shader != m_pCurrentShader)) {
    //    if(!shader)
    //        m_pCurrentShader = m_Shaders.at((unsigned)m_ActiveShader)->m_pShader;
    //    else
    //        m_pCurrentShader = shader;
    //    m_ActiveShader = style;
    //    m_OpenTextureSlots =
    //        m_Shaders.at((unsigned)m_ActiveShader).m_TextureSlots;
    //    GL_TASK_START()
    //        auto l = this->lock();
    //        for(int i=0; i<m_OpenTextureSlots; ++i)
    //            texture(i, 0);
    //        m_pCurrentShader->use();
    //    GL_TASK_END()
    //}
}

void Pipeline :: shader(std::shared_ptr<Program> p)
{
    auto l = this->lock();
    shader(m_ActiveShader, p);
}

void Pipeline :: shader(std::nullptr_t)
{
    shader(std::shared_ptr<Program>());
}

std::shared_ptr<Program> Pipeline :: shader(unsigned slot) const
{
    auto l = this->lock();
    return m_Shaders.at(slot)->m_pShader;
}

unsigned Pipeline :: layout(unsigned attrs)
{
    auto l = this->lock();
    auto& shader = m_Shaders.at((unsigned)m_ActiveShader);
    auto& cur_layout = shader->m_Layout;

    // get compatible layout
    attrs &= shader->m_SupportedLayout;
    
    for(unsigned i=0; i < (unsigned)AttributeID::MAX; ++i)
    //for(unsigned i=0; cur_layout!=attrs; ++i)
    {
        unsigned bit = 1U << i;
        //assert(bit < (unsigned)AttributeID::MAX);
        unsigned abit = attrs & bit;
        //unsigned abit = 1;
        
        if((cur_layout & bit) != abit)
        {
            if(abit) {
                try{
                    glEnableVertexAttribArray(shader->m_Attributes.at(i));
                    //LOGf("enable: %s", i);
                    cur_layout |= bit;
                }catch(...){}
                //glEnableVertexAttribArray(i);
            } else {
                try{
                    glDisableVertexAttribArray(shader->m_Attributes.at(i));
                    //LOGf("disable: %s", i);
                    cur_layout &= ~bit;
                }catch(...){}
                //glDisableVertexAttribArray(i);
            }
            
            //cur_layout ^= bit;
        }
    }
    
    return attrs;
    
    //m_Layout = attrs;
}

void Pipeline :: texture_slots(unsigned slot_flags, unsigned max_tex)
{
    assert(max_tex);
    
    auto l = this->lock();
    
    auto& shader = m_Shaders.at((unsigned)m_ActiveShader);
    auto& cur_slots = shader->m_ActiveTextureSlots;
    
    GL_TASK_START()
        auto l = this->lock();
        texture(0,0);
        //for(unsigned i = max_tex-1; cur_slots != slot_flags; --i) {
        //    const unsigned bit = 1 << i;
        //    const unsigned new_state = slot_flags & bit;
            
        //    // intended state differs?
        //    if((cur_slots & bit) != new_state) {
        //        if(!new_state)
        //            texture(0, i);
        //        //else
        //        //    texture_nobind(i);
                
        //        // update bit for slot, possible early termination
        //        cur_slots ^= bit;
        //    }
        //}
    GL_TASK_END()
}

unsigned Pipeline :: attribute_id(AttributeID id)
{
    auto l = this->lock();
    return m_Shaders.at((unsigned)m_ActiveShader)->
        m_Attributes.at((unsigned)id);
}

void Pipeline :: light(const Light* light)
{
    auto l = this->lock();
    m_pLight = light;
    if(light)
        light->bind(m_pPass);
}

