#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <vector>
#include "Texture.h"

class Material:
    public ITexture
{
    public:
        
        Material(
            const std::string& fn,
            ICache* cache = nullptr
        );
        Material(const std::tuple<std::string, ICache*>& args):
            Material(std::get<0>(args), std::get<1>(args))
        {}
        virtual ~Material();
        //virtual unsigned int id(Pass* pass = nullptr) const override;
        virtual void bind(Pass* pass) const override;

        static bool supported(const std::string& fn);
        
    private:
        
        ICache* m_pCache;
        std::string m_Filename;
        std::vector<Texture> m_Texture;
};

#endif

