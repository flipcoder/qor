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

        const static unsigned int DEFAULT_FLAGS =
            TRANSPARENT |
            FILTER |
            MIPMAP;

        Texture(unsigned int _m_ID = 0):
            m_ID(_m_ID) {}
        
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        Texture(const std::string& fn, unsigned int flags = DEFAULT_FLAGS);
        Texture(const std::tuple<std::string, ICache*>& args):
            Texture(std::get<0>(args))
        {}
        virtual ~Texture();

        /*
         * Returns true if texture id is non-zero
         * Should only false after Texture has been leak()ed
         */
        bool good() { return (m_ID!=0); }

        /*
         * Return OpenGL Texture ID for the given pass
         */
        //virtual unsigned int id(Pass* pass = nullptr) const override {
        //    return m_ID;
        //}
        virtual void bind(Pass* pass) const override {
            pass->texture(m_ID);
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

    protected:
        
        unsigned int load(std::string fn, unsigned int flags = DEFAULT_FLAGS);
        unsigned int m_ID = 0;
        std::string m_Filename;
        //ResourceCache<Texture>* m_Cache = nullptr;
        glm::uvec2 m_Size;
};

#endif

