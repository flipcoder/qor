#include "Window.h"
#include "kit/kit.h"
#include "kit/log/log.h"
#include "Info.h"
#include <IL/il.h>
#include <IL/ilu.h>

Window :: Window(const Args& args, const std::shared_ptr<Meta>& config)
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
        ERROR(LIBRARY, "SDL");

    kit::scoped_dtor<Window> dtor(this);

    SDL_DisplayMode display;
    int r = SDL_GetCurrentDisplayMode(0, &display);
    if(r != 0)
        ERROR(GENERAL, "Could not set display mode");

    bool fullscreen = !(
        args.has("-w") ||
        args.has("--windowed") ||
        config->at<bool>("windowed", false)
    );
    
    m_pWindow = SDL_CreateWindow(
        Info::Program,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        display.w,
        display.h,
        SDL_WINDOW_SHOWN |
        SDL_WINDOW_OPENGL |
        (fullscreen ?
            SDL_WINDOW_FULLSCREEN_DESKTOP:
        0) |
        SDL_WINDOW_RESIZABLE
    );

    if(!m_pWindow)
        ERROR(GENERAL, "Could not create window");

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    if(config->has("AA")){
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, config->at<int>("AA"));
    }
    SDL_GL_SetSwapInterval(1); //vsync

    m_GLContext = SDL_GL_CreateContext(m_pWindow);

    if(glewInit() != GLEW_OK)
        ERROR(LIBRARY, "glew");
    //if(!gl3wIsSupported(4,0))
    //    ERROR(OPENGL_VERSION);

    ilInit();
    if(ilGetError() != IL_NO_ERROR)
        ERROR(LIBRARY, "IL");
    iluInit();
    if(ilGetError() != IL_NO_ERROR)
        ERROR(LIBRARY, "ILU");

    dtor.resolve();
}

void Window :: render() const
{
    SDL_GL_SwapWindow(m_pWindow);
    std::this_thread::yield();
}

Window :: ~Window()
{
    if(m_GLContext)
        SDL_GL_DeleteContext(*m_GLContext);
    if(m_pWindow)
        SDL_DestroyWindow(m_pWindow);
    SDL_Quit();
}

