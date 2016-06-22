#include "Texture.h"
//#include <IL/il.h>
//#include <IL/ilu.h>
#include <FreeImage.h>
#include <string>
#include <boost/scope_exit.hpp>
#include "kit/log/errors.h"
#include "kit/log/log.h"
#include "Filesystem.h"
#include "GLTask.h"
using namespace std;

unsigned Texture :: DEFAULT_FLAGS =
    Texture::TRANS |
    Texture::FILTER |
    Texture::CLAMP |
    Texture::MIPMAP;

float Texture :: ANISOTROPY = 1.0f;

void Texture :: set_default_flags(unsigned f)
{
    DEFAULT_FLAGS = f;
}

void Texture :: set_anisotropy(float f)
{
    ANISOTROPY = f;
}

Texture :: Texture(const std::string& fn, unsigned int flags):
    m_Filename(fn)
{
    if(not Headless::enabled()) {
        GL_TASK_START()
        
        {
            auto err = glGetError();
            if(err != GL_NO_ERROR)
                K_ERRORf(GENERAL, "OpenGL Error: %s", err);
        }

        FIBITMAP* tempImage = FreeImage_Load(
            FreeImage_GetFileType(fn.c_str(), 0),
            fn.c_str()
        );
        tempImage = FreeImage_ConvertTo32Bits(tempImage);
        FreeImage_FlipVertical(tempImage);
        BOOST_SCOPE_EXIT_ALL(tempImage) {
            FreeImage_Unload(tempImage);
        };
        m_Size = glm::uvec2(
            FreeImage_GetWidth(tempImage),
            FreeImage_GetHeight(tempImage)
        );
        //ilBindImage(tempImage);
        //if(!ilLoadImage(fn.c_str())){
        //    K_ERROR(READ, Filesystem::getFileName(fn));
        //}

        //glActiveTexture(GL_TEXTURE0);
        int last_id;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_id);
        glGenTextures(1,&m_ID);
        glBindTexture(GL_TEXTURE_2D, m_ID);
        BOOST_SCOPE_EXIT_ALL(last_id) {
            glBindTexture(GL_TEXTURE_2D, last_id);
        };
        
        {
            auto err = glGetError();
            if(err != GL_NO_ERROR)
                K_ERRORf(GENERAL, "OpenGL Error: %s", err);
        }

        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,m_Size.x,m_Size.y,0,
            GL_BGRA,GL_UNSIGNED_BYTE,(void*)FreeImage_GetBits(tempImage));

        float filter = 2.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &filter);
        float aniso = std::min<float>(filter, ANISOTROPY);
        if (filter >= 1.9f)
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

        if(flags & CLAMP)
        {
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        if(flags & FILTER)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            if(flags & MIPMAP)
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            else
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }
        
        if(flags & MIPMAP)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
            
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        {
            auto err = glGetError();
            if(err != GL_NO_ERROR)
                K_ERRORf(GENERAL, "OpenGL Error: %s", err);
        }

        GL_TASK_END()
    }
}

//unsigned int Texture :: load(std::string fn, unsigned int flags)
//{
//    unload();
//    FIBITMAP* img = FreeImage_Load(
//        FreeImage_GetFIFFromFilename(fn.c_str()),
//        fn.c_str()
//    );
//    if(!img)
//    {
//        FreeImage_Unload(img);
//        K_ERROR(READ, Filesystem::getFileName(fn));
//    }

//    unsigned char* buffer = FreeImage_GetBits(img);
//    m_Size = glm::uvec2(FreeImage_GetWidth(img), FreeImage_GetHeight(img));
//    if(!buffer)
//    {
//        FreeImage_Unload(img);
//        K_ERROR(READ, Filesystem::getFileName(fn));
//    }

//    glGenTextures(1,&m_ID);
//    glBindTexture(GL_TEXTURE_2D, m_ID);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Size.x, m_Size.y, 0, GL_RGBA,
//        GL_UNSIGNED_BYTE, buffer);

//    FreeImage_Unload(img);
//    return m_ID;

//    //assert(glGetError() == GL_NO_ERROR);

//    //m_ID = gli::createTexture2D(fn);
//    //if(!m_ID)
//    //    K_ERROR(READ, Filesystem::getFileName(fn));

//    //assert(glGetError() == GL_NO_ERROR);
//    //return m_ID;
//}

void Texture :: unload()
{
    if(m_ID)
    {
        GL_TASK_ASYNC_START()
            glDeleteTextures(1,&m_ID);
        GL_TASK_ASYNC_END()
        m_ID = 0;
    }
}

Texture :: ~Texture()
{
    unload();
}

