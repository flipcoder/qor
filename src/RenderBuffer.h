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
        glGenFramebuffers(1, &m_FrameBufferID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
        glGenRenderbuffers(1, &m_DepthBufferID);
        glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBufferID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBufferID);
        glGenTextures(1, &m_Texture.id_ref());
        glBindTexture(GL_TEXTURE_2D, m_Texture.id_ref());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        // glTexParameters, glGenerateMipmap
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture.id_ref(), 0);

        status();
        
        // throwing here will cause memory leak
        //if(m_Status!=GL_FRAMEBUFFER_COMPLETE)
        //    K_ERROR(GENERAL, "Could not create framebuffer");
        //assert(m_Status == GL_FRAMEBUFFER_COMPLETE);
    }
    virtual ~RenderBuffer() {
        if(m_FrameBufferID)
            glDeleteFramebuffers(1, &m_FrameBufferID);
        if(m_DepthBufferID)
            glDeleteRenderbuffers(1, &m_DepthBufferID);
    }
    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
    }
    static void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void pushViewport(int x, int y, int w, int h){
        bind();
        glPushAttrib(GL_VIEWPORT_BIT);
        glViewport(x,y,w,h);
    }
    void pushViewport(int x = 0, int y = 0) {
        pushViewport(x,y,m_Width,m_Height);
    }
    static void popViewport() {
        glPopAttrib();
        unbind();
    }

    bool status() {
        m_Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        return m_Status == GL_FRAMEBUFFER_COMPLETE;
    }
    bool good() {
        return m_FrameBufferID &&
               m_DepthBufferID &&
               m_Texture.good() &&
               m_Status==GL_FRAMEBUFFER_COMPLETE;
    }
    bool bad() { return !good(); }
    Texture* texture() { return &m_Texture; }
    
private:
    unsigned int m_FrameBufferID = 0;
    unsigned int m_DepthBufferID = 0;
    Texture m_Texture;
    unsigned int m_Status = 0;
    unsigned int m_Width = 0;
    unsigned int m_Height = 0;
};

#endif

