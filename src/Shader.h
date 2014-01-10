#ifndef _SHADER_H
#define _SHADER_H

#include <string>
#include <list>
#include <memory>
#include <string>
#include "kit/math/common.h"
#include "Common.h"
#include "kit/log/errors.h"

class Shader
{
    public:

        enum eType {
            NO_TYPE = 0,
            FRAGMENT,
            VERTEX,
            GEOMETRY
        };

    private:

        std::string checkError(std::string fn);

        unsigned int m_ID = 0;
        eType m_Type = NO_TYPE;

        bool load(std::string fn, eType type, unsigned int flags = 0);

    public:
        
        Shader() = default;
        Shader(std::string fn, eType type, unsigned int flags = 0);
        virtual ~Shader();

        void nullify();

        //enum {
        //    L_COMPILE = BIT(1)
        //};

        //bool compile();
        void unload();

        bool good() const { return m_ID!=0; }
        bool bad() const { return m_ID==0; }

        unsigned int id() const { return m_ID; }
};

class Program
{
    public:

        typedef int UniformID;
        // Note: UniformID is signed, unlike other GL IDs
        static bool isValidUniformID(UniformID u) { return u >= 0; }

    protected:

        unsigned int m_ID = 0;
        std::list<std::shared_ptr<Shader>> m_Shaders;

        bool attach(std::shared_ptr<Shader>& shader);
        bool link();

    public:

        Program(std::shared_ptr<Shader> vp, std::shared_ptr<Shader> fp);
        virtual ~Program();
        bool use();
        void unload();
        
        bool good() const { return m_ID!=0; }
        bool bad() const { return m_ID==0; }

        void nullify();
        unsigned int id() const { return m_ID; }
        

        //void fragdata(std::string name) {
        //    if(!isValidUniform(index)) return;
        //    glBindFragDataLocation(m_ID, 0, "FragColor");
        //}
        void attribute(unsigned int index, std::string name) {
            if(!isValidUniformID(index)) return;
            glBindAttribLocation(m_ID, index, name.c_str());
        }

        UniformID uniform(std::string n) const;
        void uniform(UniformID uid, float v) const;
        void uniform(UniformID uid, float v, float v2) const;
        void uniform(UniformID uid, float v, float v2, float v3) const;
        void uniform(UniformID uid, float v, float v2, float v3, float v4) const;
        void uniform(UniformID uid, int v) const;
        void uniform(UniformID uid, int v, int v2) const;
        void uniform(UniformID uid, int v, int v2, int v3) const;
        void uniform(UniformID uid, int v, int v2, int v3, int v4) const;
        void uniform(UniformID uid, const glm::mat4& matrix) const;
        void uniform(UniformID uid, const glm::vec2& vec) const;
        void uniform(UniformID uid, const glm::vec3& vec) const;
        void uniform(UniformID uid, const glm::vec4& vec) const;

        // glUniform{size}{f|i}({uid},{count},{v})
        void uniform(UniformID uid, unsigned int size, unsigned int count, const int* v) const;
        void uniform(UniformID uid, unsigned int size, unsigned int count, const float* v) const;
        
        static void unuseAll() { glUseProgram(0); }
};

#endif

