#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <vector>
#include "Texture.h"
#include "kit/cache/cache.h"

class Material:
    public ITexture
{
    public:
        
        Material(
            const std::string& fn,
            Cache<Resource, std::string>* cache = nullptr
        );
        Material(const std::tuple<std::string, ICache*>& args):
            Material(
                std::get<0>(args),
                (Cache<Resource, std::string>*) std::get<1>(args)
            )
        {}
        virtual ~Material();
        //virtual unsigned int id(Pass* pass = nullptr) const override;
        virtual void bind(Pass* pass) const override;

        static bool supported(std::string fn);
        
    private:
        
        void load_json(std::string fn);
        void load_mtllib(std::string fn, std::string emb);
        
        ICache* m_pCache;
        Cache<Resource, std::string>* cache = nullptr;
        
        std::string m_Filename;
        std::vector<std::shared_ptr<ITexture>> m_Textures;
};

#endif

