#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "Common.h"
#include <glm/glm.hpp>
#include <string>
#include "kit/kit.h"
#include "kit/log/errors.h"
#include "Filesystem.h"
#include "ITexture.h"
#include "Graphics.h"
#include "Pass.h"
#include "kit/cache/icache.h"

class Texture:
    public ITexture
{
    public:

        enum Flags {
            TRANSPARENT = kit::bit(0),
            MIPMAP  = kit::bit(1),
            CLAMP = kit::bit(2),
            FILTER = kit::bit(3)
            //FLIP = BIT(3)
        };

        static unsigned DEFAULT_FLAGS;
        static float ANISOTROPY;

        Texture(unsigned int _m_ID = 0):
            m_ID(_m_ID) {}
        
        Texture(Texture&& t):
            m_ID(t.leak()),
            m_Filename(std::move(t.m_Filename))
        {}
        Texture(const Texture&) = delete;
        Texture& operator=(Texture&& t) {
            unload();
            m_ID = t.leak();
            m_Filename = std::move(t.m_Filename);
            return *this;
        }
        Texture& operator=(const Texture&) = delete;

        Texture(const std::string& fn, unsigned int flags = DEFAULT_FLAGS);
        Texture(const std::tuple<std::string, ICache*>& args):
            Texture(std::get<0>(args))
        {}
        void unload();
        virtual ~Texture();

        /*
         * Returns true if texture id is non-zero
         * Should only false after Texture has been leak()ed
         */
        bool good() const { return m_ID; }
        virtual operator bool() const override { return m_ID; }

        /*
         * Return OpenGL Texture ID for the given pass
         */
        //virtual unsigned int id(Pass* pass = nullptr) const override {
        //    return m_ID;
        //}
        virtual void bind(Pass* pass, unsigned slot=0) const override {
            pass->texture(m_ID, slot);
        }
        virtual unsigned int& id_ref() {
            return m_ID;
        }

        /*
         * Release the responsibility of the ID
         */
        unsigned int leak() {
            unsigned int id = m_ID;
            m_ID = 0;
            return id;
        }

        glm::uvec2 size() const { return m_Size; }
        void size(unsigned w, unsigned h) { m_Size=glm::uvec2(w,h); }

        std::string filename() const {
            return m_Filename;
        }

        static void set_default_flags(unsigned);
        static void set_anisotropy(float a);
        
    protected:
        
        unsigned int load(std::string fn, unsigned int flags = DEFAULT_FLAGS);
        unsigned int m_ID = 0;
        std::string m_Filename;
        //ResourceCache<Texture>* m_Cache = nullptr;
        glm::uvec2 m_Size;
};

#endif

