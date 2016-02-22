#include "Texture.h"
#include <IL/il.h>
#include <IL/ilu.h>
#include <string>
#include <boost/scope_exit.hpp>
#include "kit/log/errors.h"
#include "kit/log/log.h"
#include "Filesystem.h"
#include "GLTask.h"
using namespace std;

unsigned Texture :: DEFAULT_FLAGS =
    Texture::TRANSPARENT |
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

Texture :: Texture(const std::string& fn, unsigned int flags)
{
    GL_TASK_START()
    assert(ilGetError() == IL_NO_ERROR);
    
    {
        auto err = glGetError();
        if(err != GL_NO_ERROR)
            ERRORf(GENERAL, "OpenGL Error: %s", err);
    }

    ILuint tempImage;

    ilGenImages(1,&tempImage);
    BOOST_SCOPE_EXIT_ALL(tempImage) {
        ilGetError();
        ilBindImage(0);
        ilDeleteImages(1,&tempImage);
        assert(ilGetError() == IL_NO_ERROR);
    };
    
    assert(ilGetError() == IL_NO_ERROR);

    ilBindImage(tempImage);
    if(!ilLoadImage(fn.c_str())){
        ERROR(READ, Filesystem::getFileName(fn));
    }
    assert(ilGetError() == IL_NO_ERROR);

    ILinfo ImageInfo;
    iluGetImageInfo(&ImageInfo);
    
    assert(ilGetError() == IL_NO_ERROR);
    {
        auto err = glGetError();
        if(err != GL_NO_ERROR)
            ERRORf(GENERAL, "OpenGL Error: %s", err);
    }
    
    if(!ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE)) {
        ERRORf(ACTION, "texture conversion for \"%s\"", Filesystem::getFileName(fn));
    }
    
    assert(ilGetError() == IL_NO_ERROR);

    if(ImageInfo.Origin == IL_ORIGIN_LOWER_LEFT)
        iluFlipImage();
    
    assert(ilGetError() == IL_NO_ERROR);

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
            ERRORf(GENERAL, "OpenGL Error: %s", err);
    }

    glTexImage2D(GL_TEXTURE_2D,0,ilGetInteger(IL_IMAGE_BPP),ilGetInteger(IL_IMAGE_WIDTH),
        ilGetInteger(IL_IMAGE_HEIGHT),0,ilGetInteger(IL_IMAGE_FORMAT),GL_UNSIGNED_BYTE,ilGetData());

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

    m_Size = glm::uvec2(
        ilGetInteger(IL_IMAGE_WIDTH),
        ilGetInteger(IL_IMAGE_HEIGHT)
    );

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
        // GLU version:
        // gluBuild2DMipmaps(GL_TEXTURE_2D,ilGetInteger(IL_IMAGE_BPP),ilGetInteger(IL_IMAGE_WIDTH),
            //ilGetInteger(IL_IMAGE_HEIGHT),ilGetInteger(IL_IMAGE_FORMAT),GL_UNSIGNED_BYTE,ilGetData());
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
        
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    assert(ilGetError() == IL_NO_ERROR);
    
    {
        auto err = glGetError();
        if(err != GL_NO_ERROR)
            ERRORf(GENERAL, "OpenGL Error: %s", err);
    }

    GL_TASK_END()
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
//        ERROR(READ, Filesystem::getFileName(fn));
//    }

//    unsigned char* buffer = FreeImage_GetBits(img);
//    m_Size = glm::uvec2(FreeImage_GetWidth(img), FreeImage_GetHeight(img));
//    if(!buffer)
//    {
//        FreeImage_Unload(img);
//        ERROR(READ, Filesystem::getFileName(fn));
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
//    //    ERROR(READ, Filesystem::getFileName(fn));

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

