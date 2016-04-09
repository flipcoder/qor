#include "Pipeline.h"
#include "BasicPartitioner.h"
#include "Window.h"
#include "Light.h"
#include <cassert>
#include <cmath>
#include <glm/glm.hpp>
#include "GLTask.h"
#include "Camera.h"
#include "Headless.h"
//#include <glm/gtc/matrix_transform.hpp>

using namespace std;

#define MAX_LIGHTS_PER_PASS 8

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
    "Binormal",
    "Color"
};

Pipeline :: Pipeline(
    Window* window,
    Args& args,
    //const std::shared_ptr<Meta>& sys_cfg,
    Cache<Resource, std::string>* cache
):
    m_pWindow(window),
    m_pCache(cache),
    m_pPartitioner(std::make_shared<BasicPartitioner>())
{
    assert(m_pWindow);

    if(Headless::enabled())
        return;
    
    //assert(m_pCamera.lock());
    //assert(m_pRoot.lock());
    
    m_ActiveShader = PassType::NORMAL;
        
    clear_shaders();
    unsigned r = m_Shaders.size();
    load_shaders({"base","basic"});
    m_ShaderOverrides.resize((unsigned)PassType::MAX);
    unsigned i = 0;
    for(unsigned& s: m_ShaderOverrides){
        //s = (unsigned)PassType::NONE;
        s = i;
        ++i;
    }
    
    GL_TASK_START()
        //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_DEPTH_TEST);
        //glDepthFunc(GL_LESS);
        //glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        glEnable(GL_MULTISAMPLE);
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

void Pipeline :: logic(Freq::Time t)
{
    m_pPartitioner->logic(t);
}

unsigned Pipeline :: load_shaders(vector<string> names)
{
    unsigned r = m_Shaders.size();
    GL_TASK_START()
        auto l = this->lock();

        for(auto&& name: names)
        {
            auto shader = m_pCache->cache_cast<PipelineShader>(name+".json");
            m_Shaders.push_back(shader);
            shader->m_pShader->use();
            
            unsigned i = 0;
            for(auto&& attr_name: s_AttributeNames) {
                int attr_id_t = shader->m_pShader->attribute((boost::format("Vertex%s")%
                    attr_name
                ).str());
                if(attr_id_t >= 0){
                    unsigned attr_id = (unsigned)attr_id_t;
                    LOGf("%s shader attr: %s (%s)", name % attr_name % attr_id);
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
        for(auto&& slot: m_Shaders)
        {
            slot->m_pShader->use();
            
            slot->m_MaterialAmbientID = slot->m_pShader->uniform(
                "MaterialAmbient"
            );
            slot->m_MaterialDiffuseID = slot->m_pShader->uniform(
                "MaterialDiffuse"
            );
            slot->m_MaterialSpecularID = slot->m_pShader->uniform(
                "MaterialSpecular"
            );
            slot->m_MaterialEmissiveID = slot->m_pShader->uniform(
                "MaterialEmissive"
            );

            slot->m_ModelViewProjectionID = slot->m_pShader->uniform(
                "ModelViewProjection"
            );
            slot->m_ModelViewID = slot->m_pShader->uniform(
                "ModelView"
            );
            slot->m_ModelID = slot->m_pShader->uniform(
                "Model"
            );
            slot->m_ViewID = slot->m_pShader->uniform(
                "View"
            );
            slot->m_NormalID = slot->m_pShader->uniform(
                "NormalMatrix"
            );
            
            for(int i=0; i < int(s_TextureUniformNames.size() + 1); ++i) {
                int tex_id = slot->m_pShader->uniform(
                    (boost::format("Texture%s")%(
                        i?
                            s_TextureUniformNames.at(i-1)
                        :
                            ""
                    )).str()
                );
                if(tex_id < 0)
                    break;
                slot->m_Textures.resize(i+1);
                slot->m_Textures.at(i) = tex_id;
            }
        }
    GL_TASK_END()
    return r;
}

void Pipeline :: matrix(Pass*, const glm::mat4* m)
{
    auto l = this->lock();
    
    m_ModelViewMatrix = m_ViewMatrix * *m;
    m_NormalMatrix = glm::transpose(glm::inverse(m_ModelViewMatrix));
    m_ModelViewProjectionMatrix = m_ProjectionMatrix * m_ModelViewMatrix;
    //m_ModelViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix * *m;
    
    GL_TASK_START()
        
        // matrices
        m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
            m_Shaders.at((unsigned)m_ActiveShader)->m_ModelViewProjectionID,
            m_ModelViewProjectionMatrix
        );
        m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
            m_Shaders.at((unsigned)m_ActiveShader)->m_ModelViewID,
            m_ModelViewMatrix
        );
        m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
            m_Shaders.at((unsigned)m_ActiveShader)->m_ModelID,
            m_ModelMatrix
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
            int u = m_Shaders.at((unsigned)m_ActiveShader)->m_Textures.at(slot);
            if(u != -1)
                m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
                    u, (int)slot
                );
        }catch(...){
            //assert(false);
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

void Pipeline :: render(
    Node* root, Camera* camera, IPartitioner* partitioner,
    unsigned flags
){
    auto l = this->lock();
    assert(m_pWindow);
    if(not partitioner)
        partitioner = m_pPartitioner.get();
    assert(partitioner);
    if(!root)
        return;
    if(!camera)
        return;
    //if(!m_pRoot.lock())
    //    return;
    //if(!m_pCamera.lock())
    //    return;

    m_ViewMatrix = camera->view();
    m_ProjectionMatrix = camera->projection();
    //m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    
    //l.unlock();
    
    GL_TASK_START()
        auto l = this->lock();

        //std::shared_ptr<Node> root = m_pRoot.lock();
        assert(glGetError() == GL_NO_ERROR);

        // set up initial state
        //glViewport(0,0,m_pWindow->size().x/2,m_pWindow->size().y/2);
        if(not (flags & NO_CLEAR)) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(m_BGColor.r(), m_BGColor.g(), m_BGColor.b(), 1.0f);
        }
        
        Pass pass(partitioner, this, Pass::BASE);
        this->pass(&pass);
        pass.camera(camera);
        //pass.visibility_func(std::bind(&Camera::is_visible, camera, std::placeholders::_1));
        partitioner->camera(camera);
        partitioner->partition(root);
        //bool has_lights = false;
        bool has_lights = flags & LIGHTS;
        //pass.flags(pass.flags() & ~Pass::RECURSIVE);
        //LOGf("visible lights: %s", partitioner->visible_lights().size());
        //if(not partitioner->visible_lights().empty() &&
        //    partitioner->visible_lights()[0] // not null
        //){
        //    pass.flags(pass.flags() & ~Pass::RECURSIVE);
        //    has_lights = true;
        //}

        if(has_lights)
            pass.flags(pass.flags() & ~Pass::RECURSIVE);
        
        if(m_bBlend || (flags & NO_DEPTH))
            glDisable(GL_DEPTH_TEST);
        else
            glEnable(GL_DEPTH_TEST);
        
        if(not m_bBlend && has_lights)
        {
            if(not (flags & NO_DEPTH))
                glEnable(GL_DEPTH_TEST);
            else
                glDisable(GL_DEPTH_TEST);

            glDisable(GL_BLEND);

            assert(glGetError() == GL_NO_ERROR);
            if(m_ShaderOverrides.at((unsigned)PassType::BASE) == (unsigned)PassType::NONE)
                shader(PassType::BASE);
            else
                shader((PassType)(m_ShaderOverrides.at((unsigned)PassType::BASE)));
            assert(glGetError() == GL_NO_ERROR);

            // render base ambient pass
            on_pass(&pass);
            if(pass.flags() & Pass::RECURSIVE)
            {
                root->render(&pass);
            }
            else
            {
                auto rng = partitioner->visible_nodes();
                for(const auto& node: rng) {
                    if(!node)
                        break;
                    node->render(&pass);
                }
            }
        }

        // set up multi-pass state
        if(not has_lights){
            if(m_bBlend) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }else{
                //glDisable(GL_BLEND);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
        }else{
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glEnable(GL_BLEND);
            //glDepthMask(false);
            glDepthFunc(GL_EQUAL);
        }
        
        pass.flags(pass.flags() & ~Pass::BASE);

        assert(glGetError() == GL_NO_ERROR);
        if(m_ShaderOverrides.at((unsigned)PassType::NORMAL) == (unsigned)PassType::NONE)
            shader(PassType::NORMAL);
        else
            shader((PassType)(m_ShaderOverrides.at((unsigned)PassType::NORMAL)));
        assert(glGetError() == GL_NO_ERROR);
        
        if(!has_lights)
        {
            on_pass(&pass);

            // render detail pass (no lights)
            if(pass.flags() & Pass::RECURSIVE)
            {
                root->render(&pass);
            }
            else
            {
                unsigned n = 0;
                auto& rng = partitioner->visible_nodes();
                for(const auto& node: rng) {
                    if(!node)
                        break;
                    node->render(&pass);
                    ++n;
                }
                //LOGf("rendered %s nodes", n);
            }
        }
        else
        {
            on_pass(&pass);

            unsigned l = 0;
            auto visible_lights = partitioner->visible_lights();
            auto visible_nodes = partitioner->visible_nodes();
            
            int i = 0;
            int passes = 0;
            bool break_outer = false;
            while(not break_outer && visible_lights.size()) {
                
                auto light_batch = kit::slice(visible_lights, 0, MAX_LIGHTS_PER_PASS);
                visible_lights = kit::slice(visible_lights, MAX_LIGHTS_PER_PASS);
            
                int i = 0;
                for(auto&& light: light_batch){
                    if(not light){
                        break_outer = true;
                        break;
                    }
                    this->light(light, i);
                    ++i;
                }
                if(i == 0) // no lights this pass
                    break;
                
                int u = m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform("NumLights");
                if(u >= 0)
                    m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(u, i);
                
                for(const auto& node: visible_nodes) {
                    if(not node)
                        break;
                    node->render(&pass);
                }
                ++passes;
            }
            //LOGf("rendered %s passes", passes);
        }

        if(has_lights){
            glDepthMask(true);
            glDepthFunc(GL_LEQUAL);
        }
        if(m_bBlend && not (flags & NO_DEPTH))
            glEnable(GL_DEPTH_TEST);

        //this->light(nullptr);
        this->pass(nullptr);
    GL_TASK_END()
}

//void Pipeline :: ortho(bool origin_bottom)
//{
//    auto l = this->lock();
//    m_ProjectionMatrix = glm::ortho(
//        0.0f,
//        static_cast<float>(m_pWindow->size().x),
//        origin_bottom ? 0.0f : static_cast<float>(m_pWindow->size().y),
//        origin_bottom ? static_cast<float>(m_pWindow->size().y) : 0.0f,
//        -100.0f,
//        100.0f
//        //origin_bottom ? -100.0f : 100.0f,
//        //origin_bottom ? 100.0f : -100.0f
//    );
//    winding(!origin_bottom);
//}

//void Pipeline :: perspective(float fov)
//{
//    auto l = this->lock();
//    float aspect_ratio = static_cast<float>(m_pWindow->aspect_ratio());
//    m_ProjectionMatrix = glm::perspective(
//        fov,
//        aspect_ratio,
//        0.01f,
//        1000.0f
//    );
//    winding(false);
//}

void Pipeline :: winding(bool cw)
{
    GL_TASK_START()
        glFrontFace(cw ? GL_CW : GL_CCW);
    GL_TASK_END()
}

void Pipeline :: shader(
    PassType type
){
    auto l = this->lock();

    if(Headless::enabled())
        return;
    
    //LOGf("style: %s", (unsigned)style);
    GL_TASK_START()
        auto l = this->lock();
        assert(glGetError() == GL_NO_ERROR);
        m_ActiveShader = type;
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

std::shared_ptr<Program> Pipeline :: shader(unsigned slot)
{
    if(Headless::enabled())
        return nullptr;
    auto l = this->lock();
    return m_Shaders.at(slot)->m_pShader;
}

std::shared_ptr<Program> Pipeline :: shader() {
    if(Headless::enabled())
        return nullptr;
    auto l = this->lock();
    return m_Shaders.at((unsigned)m_ActiveShader)->m_pShader;
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
            
            cur_layout ^= bit;
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

void Pipeline :: light(const Light* light, unsigned slot)
{
    auto l = this->lock();
    if(light)
        light->bind(m_pPass, slot);
}

void Pipeline :: material(Color a, Color d, Color s, Color e)
{
    auto l = this->lock();
    GL_TASK_START()

        // materials
        m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
            m_Shaders.at((unsigned)m_ActiveShader)->m_MaterialAmbientID,
            a.vec3()
        );
        m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
            m_Shaders.at((unsigned)m_ActiveShader)->m_MaterialDiffuseID,
            d.vec3()
        );
        m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
            m_Shaders.at((unsigned)m_ActiveShader)->m_MaterialSpecularID,
            s.vec3()
        );
        m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
            m_Shaders.at((unsigned)m_ActiveShader)->m_MaterialEmissiveID,
            e.vec3()
        );
        //m_Shaders.at((unsigned)m_ActiveShader)->m_pShader->uniform(
        //    m_Shaders.at((unsigned)m_ActiveShader)->m_MaterialShininessID,
        //    s.vec3()
        //);
        
    GL_TASK_END()
}

void Pipeline :: override_shader(PassType p, unsigned id)
{
    auto l = lock();
    m_ShaderOverrides.at((unsigned)p) = id;
}

void Pipeline :: clear_shader_overrides()
{
    auto l = lock();
    m_ShaderOverrides.clear();
}

