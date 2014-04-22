#ifndef _WINDOW_H_HOFP8EQ7
#define _WINDOW_H_HOFP8EQ7

#include "Common.h"
#include "Shader.h"
#include "Pipeline.h"
#include "kit/args/args.h"
#include "kit/meta/meta.h"
#include <boost/optional.hpp>

class Window
{
    public:
        Window(const Args& args, const std::shared_ptr<Meta<>>& user_cfg);
        virtual ~Window();
        void render() const;

        float aspect_ratio() {
            glm::ivec2 r;
            SDL_GetWindowSize(m_pWindow,&r[0],&r[1]);
            return (1.0f * r.x) / (1.0f * r.y);
        }

        glm::ivec2 size() const {
            glm::ivec2 r;
            SDL_GetWindowSize(m_pWindow,&r[0],&r[1]);
            return r;
        }

        glm::ivec2 center() const {
            return size() / 2;
        }

        SDL_Window* sdl_window() {
            return m_pWindow;
        }
        
        boost::signals2::signal<void(glm::ivec2)> on_resize;
        
    private:

        SDL_Window* m_pWindow = nullptr;
        boost::optional<SDL_GLContext> m_GLContext;
};

#endif

