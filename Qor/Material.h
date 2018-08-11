#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <vector>
#include "Texture.h"
#include "ResourceCache.h"
#include "kit/reactive/reactive.h"
#include "Graphics.h"

class Material:
    public ITexture
{
    public:
        
        Material();
        Material(
            const std::string& fn,
            ResourceCache* cache = nullptr
        );
        Material(const std::tuple<std::string, ICache*>& args):
            Material(
                std::get<0>(args),
                (ResourceCache*) std::get<1>(args)
            )
        {}
        virtual ~Material();
        //virtual unsigned int id(Pass* pass = nullptr) const override;
        virtual void bind(Pass* pass, unsigned slot = 0) const override;
        virtual void unbind(Pass* pass) const override;

        static bool supported(
            std::string fn,
            ResourceCache* cache
        );

        enum ExtraMap {
            //DIFF = 0,
            NRM,
            DISP,
            FADE,
            SPEC,
            //OCC
        };
        
        virtual operator bool() const override;
        
        virtual glm::uvec2 size() const override { return m_Textures.at(0)->size(); }
        virtual void size(unsigned w, unsigned h) override { m_Textures.at(0)->size(w,h); }
        virtual glm::uvec2 center() const override { return m_Textures.at(0)->center(); }
        
        kit::signal<void(Pass*)> before;
        kit::signal<void(Pass*)> after;
        
        void ambient(Color c) { m_Ambient = c; }
        void diffuse(Color c) { m_Diffuse = c; }
        void specular(Color c) { m_Specular = c; }
        void emissive(Color c) { m_Emissive = c; }
        
        virtual Color ambient() override { return Color::white(1.0f); }
        virtual Color diffuse() override { return Color::white(1.0f); }
        virtual Color specular() override { return Color::white(1.0f); }
        virtual Color emissive() override { return Color::white(0.0f); }

        virtual std::string name() const override {
            return m_Textures.at(0)->filename();
        }

        virtual std::string filename() const override {
            return m_Textures.at(0)->filename();
        }
        
    private:

#ifndef SWIG
        const static std::vector<std::string> s_ExtraMapNames;
#endif
        
        void load_json(std::string fn);
        void load_mtllib(std::string fn, std::string emb);
        void load_detail_maps(std::string fn);
        
        ResourceCache* m_pCache = nullptr;
        
        std::string m_Filename;
        std::string m_Name;
        
        std::vector<std::shared_ptr<ITexture>> m_Textures;
        //bool m_bComposite = false;

        Color m_Ambient = Color::white();
        Color m_Diffuse = Color::white();
        Color m_Specular = Color::white();
        Color m_Emissive = Color::clear();

        glm::uvec2 m_Size;

        bool m_bBackfaces = false;
};

#endif

