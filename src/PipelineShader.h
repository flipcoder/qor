#ifndef _PIPELINE_SHADER_H
#define _PIPELINE_SHADER_H

#include "Resource.h"
#include "Shader.h"
#include "kit/cache/cache.h"
#include <string>
#include <tuple>
#include <memory>

// replaces Pipeline::ShaderSlot !

class PipelineShader:
    public Resource
{
    public:
        
        PipelineShader(const std::string& fn);
        PipelineShader(const std::tuple<std::string, ICache*>& args):
            PipelineShader(std::get<0>(args))
        {}
        
        void link();
        bool linked() const;
        
        std::shared_ptr<Program> m_pShader;

        Program::UniformID m_ModelViewProjectionID = -1;
        Program::UniformID m_ModelViewID= -1;
        Program::UniformID m_NormalID = -1;
        std::vector<Program::UniformID> m_Textures;
        std::vector<unsigned> m_Attributes;
        //unsigned m_TextureSlots = 0;

        unsigned m_Layout = 0;
        unsigned m_SupportedLayout = 0;
        unsigned m_ActiveTextureSlots = 0;
};

#endif

