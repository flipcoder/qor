#include "Window.h"
#include "kit/kit.h"
#include "kit/log/log.h"
#include "Info.h"
#include <IL/il.h>
#include <IL/ilu.h>
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>

using namespace std;

Window :: Window(
    const Args& args,
    Cache<Resource, std::string>* resources
):
    m_pResources(resources)
{
    auto vid_section = std::make_shared<MetaBase<kit::optional_mutex<std::recursive_mutex>>>();
    m_pResources->config()->mutex().mutex = vid_section->mutex().mutex;
    auto video_cfg = m_pResources->config()->ensure(
        "video",  vid_section
    );
    
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
        ERROR(LIBRARY, "SDL");

    kit::scoped_dtor<Window> dtor(this);

    glm::ivec2 resolution;
    {
        string res_string = video_cfg->at<string>("resolution", "");
        if(!res_string.empty()) {
            vector<string> tokens;
            boost::algorithm::split(tokens, res_string, boost::is_any_of("x"));
            if(tokens.size() != 2)
                ERRORf(PARSE, "Invalid resolution %s", res_string);
            try{
                for(size_t i=0;i<tokens.size();++i)
                    resolution[i] = boost::lexical_cast<int>(tokens[i]);
            }catch(const boost::bad_lexical_cast&){
                ERRORf(PARSE, "Invalid resolution %s", res_string);
            }
        }else{
            SDL_DisplayMode display;
            int r = SDL_GetCurrentDisplayMode(0, &display);
            if(r != 0)
                ERROR(GENERAL, "Could not get display mode");
            resolution = glm::ivec2(display.w, display.h);
        }
    }
    
    bool fullscreen = !(
        video_cfg->at<bool>("windowed", false) ||
        args.has("-w") ||
        args.has("--windowed")
    );
    
    m_pWindow = SDL_CreateWindow(
        Info::Program,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        resolution.x,
        resolution.y,
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

    if(video_cfg->has("AA")){
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, video_cfg->at<int>("AA"));
    }
    
    if(video_cfg->at("vsync", false))
        SDL_GL_SetSwapInterval(1);

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
}

Window :: ~Window()
{
    if(m_GLContext)
        SDL_GL_DeleteContext(*m_GLContext);
    if(m_pWindow)
        SDL_DestroyWindow(m_pWindow);
    SDL_Quit();
}

boost::signals2::connection Window ::  on_resize(
    const boost::signals2::signal<void()>::slot_type& cb
){
    return m_pResources->config()->meta("video")->on_change("resolution", cb);
}

void Window :: resize(glm::ivec2 v)
{
    m_pResources->config()->meta("video")->set<string>("resolution",
        (boost::format("%sx%s") % v.x % v.y).str()
    );
}

