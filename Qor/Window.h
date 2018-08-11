#ifndef _WINDOW_H_HOFP8EQ7
#define _WINDOW_H_HOFP8EQ7

#include "Common.h"
#include "Shader.h"
#include "Pipeline.h"
#include "kit/args/args.h"
#include "kit/meta/meta.h"
#include <boost/optional.hpp>
#include "Resource.h"
#include "ResourceCache.h"
#include "Headless.h"

class Window
{
    public:
        Window(
            const Args& args,
            ResourceCache* resources
        );
        virtual ~Window();
        void destroy();
        void render() const;

        float aspect_ratio() {
            glm::ivec2 r;
            if(not Headless::enabled())
                SDL_GetWindowSize(m_pWindow,&r[0],&r[1]);
            else
                return 1.0f;
            return (1.0f * r.x) / (1.0f * r.y);
        }

        glm::ivec2 size() const {
            glm::ivec2 r(1,1);
            if(not Headless::enabled())
                SDL_GetWindowSize(m_pWindow,&r[0],&r[1]);
            return r;
        }
        glm::vec2 size2f() const {
            return glm::vec2(size());
        }
        glm::vec3 size3f() const {
            const auto sz = size();
            return glm::vec3(sz.x,sz.y,0.0f);
        }

        glm::ivec2 center() const {
            return size() / 2;
        }
        glm::vec2 center2f() const {
            return glm::vec2(size()) / 2.0f;
        }
        glm::vec3 center3f() const {
            const auto sz = size();
            return glm::vec3(sz.x,sz.y,0.0f) / 2.0f;
        }

        SDL_Window* sdl_window() {
            return m_pWindow;
        }
        
        boost::signals2::connection on_resize(const boost::signals2::signal<void()>::slot_type& cb);
        void resize(glm::ivec2);
        
        boost::signals2::signal<void()> on_delay;

        
    private:
        
        void delay();

        ResourceCache* m_pResources;
        SDL_Window* m_pWindow = nullptr;
        boost::optional<SDL_GLContext> m_GLContext;
        std::string m_Title;
        std::thread m_DelayThread;
        mutable std::condition_variable m_condDelay;
        mutable std::mutex m_DelayMutex;
        mutable bool m_DelayReady = false;
        mutable bool m_DelayDone = false;
        mutable bool m_QuitFlag = false;
};

#endif

