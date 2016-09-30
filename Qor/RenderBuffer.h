#ifndef _RENDERBUFFER_H
#define _RENDERBUFFER_H

#include "Common.h"
#include "Texture.h"
#include "kit/log/log.h"

class RenderBuffer
{
public:
    RenderBuffer(int width, int height):
        m_Width(width),
        m_Height(height)
    {
        GLuint id;
        
        //SDL_Surface* tmp = SDL_CreateRGBSurface(0, width, height,
        //    32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
        
        //GL_TASK_START()
        //    glGenTextures(1, &id);
        //    glBindTexture(GL_TEXTURE_2D, id);
        //    int mode = GL_RGBA;
            
        //    glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height,
        //        0, mode, GL_UNSIGNED_BYTE, 0);
            
        //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //GL_TASK_END()
        
        //SDL_FreeSurface(tmp);
        
        m_pTexture = std::make_shared<Texture>();
        
        GL_TASK_START()
            glGenFramebuffers(1, &m_FrameBufferID);
            glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
            glGenRenderbuffers(1, &m_DepthBufferID);
            glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBufferID);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBufferID);
            
            glGenTextures(1, &m_pTexture->id_ref());
            glBindTexture(GL_TEXTURE_2D, m_pTexture->id_ref());
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
            // glTexParameters, glGenerateMipmap
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_pTexture->id_ref(), 0);
            
            GLenum bufs[1] = {GL_COLOR_ATTACHMENT0};
            glDrawBuffers(1, bufs);
            
            unbind();
        GL_TASK_END()

        if(not status()){
            destroy();
            K_ERROR(GENERAL, "framebuffer");
        }
    }
    virtual ~RenderBuffer() {
        destroy();
    }
    void destroy() {
        GL_TASK_START()
            if(m_FrameBufferID)
                glDeleteFramebuffers(1, &m_FrameBufferID);
            if(m_DepthBufferID)
                glDeleteRenderbuffers(1, &m_DepthBufferID);
        GL_TASK_END()
    }
    void bind() {
        GL_TASK_START()
            glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
        GL_TASK_END()
    }
    static void unbind() {
        GL_TASK_START()
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        GL_TASK_END()
    }
    void push(int x=0, int y=0, int w=0, int h=0){
        bind();
        GL_TASK_START()
            glPushAttrib(GL_VIEWPORT_BIT);
            glViewport(x, y, w==0?m_Width:w, h==0?m_Height:h);
        GL_TASK_END()
    }
    static void pop() {
        GL_TASK_START()
            glPopAttrib();
        GL_TASK_END()
        unbind();
    }

    bool status() const {
        unsigned status = 0;
        GL_TASK_START()
            status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        GL_TASK_END()
        return status == GL_FRAMEBUFFER_COMPLETE;
    }
    bool good() const {
        return m_pTexture->good();
    }
    bool bad() const { return !good(); }
    std::shared_ptr<Texture> texture() { return m_pTexture; }
    
private:
    unsigned int m_FrameBufferID = 0;
    unsigned int m_DepthBufferID = 0;
    unsigned int m_Width = 0;
    unsigned int m_Height = 0;
    std::shared_ptr<Texture> m_pTexture;
};

#endif

