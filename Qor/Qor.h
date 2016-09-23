#ifndef _ENGINE_H_HVSZVL8
#define _ENGINE_H_HVSZVL8

#include <memory>
#include <functional>
#include <deque>
#include <vector>
#include "Nodes.h"
#include "State.h"
#include "StateManager.h"
#include "Window.h"
#include "Canvas.h"
#include "Input.h"
#include "Resource.h"
#include "Interpreter.h"
#include "Texture.h"
#include "Session.h"
#include "Audio.h"
//#include "GUI.h"
#include "TaskHandler.h"
#include "kit/args/args.h"
#include "kit/meta/meta.h"
#include "kit/cache/cache.h"
#include "PipelineShader.h"
#include "ResourceCache.h"

class Qor:
    public StateManager<State>,
    public TaskHandler
    //public StateManager<State, unsigned, Qor*>
{
    public:
                
        Qor(const Args& args, std::string appname);
        Qor(int argc, const char** argv, std::string appname="qor"):
            Qor(Args(argc,argv), appname)
        {}
        virtual ~Qor();
        
        void run(unsigned state_id = 0);
        void run(std::string state);
        void logic();
        void render();

        void quit() { m_bQuit = true; }
        bool quit_flag() const { return m_bQuit; }

        virtual std::shared_ptr<State> new_state(unsigned id) override;

        Input* input() { return m_pInput.get(); }
        const Input* input() const { return m_pInput.get(); }
        Freq* timer() { return m_pTimer.get(); }
        const Freq* timer() const { return m_pTimer.get(); }
        Window* window() { return m_pWindow.get(); }
        const Window* window() const { return m_pWindow.get(); }
        //Canvas* canvas() { return m_pCanvas.get(); }
        //const Canvas* canvas() const { return m_pCanvas.get(); }
        //GUI* gui() { return m_pGUI.get(); }
        //const GUI* gui() const { return m_pGUI.get(); }
        //IPhysics* physics() { return m_pPhysics.get(); }
        //const IPhysics* physics() const { return m_pPhysics.get(); }
#ifndef QOR_NO_AUDIO
        Audio* audio() { return m_pAudio.get(); }
        const Audio* audio() const { return m_pAudio.get(); }
#endif
        Session* session() { return m_pSession.get(); }
        const Session* session() const { return m_pSession.get(); }
        Interpreter* interpreter() { return m_pInterpreter.get(); }
        const Interpreter* interpreter() const { return m_pInterpreter.get(); }
        Pipeline* pipeline() { return m_pPipeline.get(); }
        const Pipeline* pipeline() const { return m_pPipeline.get(); }
        
        //std::shared_ptr<Meta> user_cfg() { return m_pUserCfg; }
        //std::shared_ptr<const Meta> user_cfg() const { return m_pUserCfg; }
        //std::shared_ptr<Meta> mod_cfg() { return m_pModCfg; }
        //std::shared_ptr<const Meta> mod_cfg() const { return m_pModCfg; }

        //const std::shared_ptr<ResourceLocator>& locator() {
        //    return m_pLocator;
        //}
        //const std::shared_ptr<ResourceCache<Texture>>& textures() const {
        //    return m_pTextures;
        //}
        
        ResourceCache* resources() {
            return &m_Resources;
        }
        const ResourceCache* resources() const {
            return &m_Resources;
        }

        template<class T>
        std::shared_ptr<T> make(std::string fn) {
            return std::make_shared<T>(resource_path(fn), &m_Resources);
        }

        //std::shared_ptr<Node> create(std::string type)
        //{
        //    return m_NodeFactory.create(
        //        make_tuple(type, (ICache*)&m_Resources)
        //    );
        //}

        Factory<State, Qor*>& states() {
            return m_StateFactory;
        }
        virtual std::shared_ptr<State> current_state() {
            std::shared_ptr<State> s;
            unsigned id = 0;
            do{
                s = state(id);
                ++id;
            }while(s && s->is_loader());
            return s;
        }
        bool is_loading() const {
            return state()->is_loader();
        }
        //Factory<Node, std::tuple<std::string, ICache*>>& nodes() {
        //    return m_NodeFactory;
        //}

        virtual void clear_tasks() override {
            auto l = std::unique_lock<std::mutex>(m_TasksMutex);
            m_Tasks.clear();
        }
        virtual void wait_task(std::function<void()> func) override;
        virtual void add_task(std::function<void()> func) override {
            auto l = std::unique_lock<std::mutex>(m_TasksMutex);
            m_Tasks.push_front(func);
        }
        virtual void do_tasks() override {
            auto l = std::unique_lock<std::mutex>(m_TasksMutex);
            assert(std::this_thread::get_id() == m_HandlerThreadID);
            while(!m_Tasks.empty()) {
                //assert(glGetError() == GL_NO_ERROR);
                m_Tasks.back()();
                //assert(glGetError() == GL_NO_ERROR);
                m_Tasks.pop_back();
            }
        }
        virtual bool is_handler() const override {
            return std::this_thread::get_id() == m_HandlerThreadID;
        }
        virtual bool has_tasks() const override {
            auto l = std::unique_lock<std::mutex>(m_TasksMutex);
            if(std::this_thread::get_id() != m_HandlerThreadID)
                return false;
            return !m_Tasks.empty();
        }
        
        Args& args() {return m_Args;}
        const Args& args() const {return m_Args;}
        
        std::shared_ptr<Meta> meta() { return m_pMeta; }
        std::shared_ptr<const Meta> meta() const { return m_pMeta; }
        
        //unsigned resolve_node(
        //    const std::tuple<
        //        std::string,
        //        IFactory*,
        //        ICache*
        //    >& args
        //);

        unsigned resolve_resource(
            const std::tuple<
                std::string,
                ICache*
            >& args
        );
        
        std::string resource_path(
            std::string
        );
        bool exists(std::string);
        
        //std::tuple<
        //    std::string,
        //    IFactory*,
        //    ICache*
        //> node_path(const std::tuple<
        //    std::string,
        //    IFactory*,
        //    ICache*
        //>& args);

        std::string mod_name() const {
            return m_Args.value_or("mod","demo");
        }
        
        std::string find_file() const;

        void save_settings();

        void change_state(std::string s) {
            StateManager<State>::change_state(m_StateFactory.class_id(s));
        }
        void change_state(unsigned id) {
            StateManager<State>::change_state(id);
        }

        void set_loading_state(unsigned registered_state) {
            m_LoadingState = registered_state;
        }
        
        float fps() const {
            return m_FPS;
        }

        static Qor* get() { return s_pQor; } // try not to use this ;'(
        
    private:

        mutable std::mutex m_TasksMutex;
        std::thread::id m_HandlerThreadID = std::this_thread::get_id();
        std::deque<std::function<void()>> m_Tasks;
        
        void async_load(State* s)
        {
#ifndef QOR_NO_AUDIO
            m_pAudio->set_context();
#endif
            s->preload();
            s->finish_loading();
        }
        
        // State Factory
        Factory<State, Qor*> m_StateFactory;
        
        // Node Factory
        //Factory<
        //    Node,
        //    std::tuple<
        //        std::string,
        //        ICache*
        //    >
        //> m_NodeFactory;
        
        // Resource Cache+Factory
        ResourceCache m_Resources;
        // cache of paths transformed by resource_path
        std::unordered_map<std::string, std::string> m_Paths;
        
        unsigned m_LoadingState = ~0u;
        std::atomic<bool> m_bQuit = ATOMIC_VAR_INIT(false);

        // Engine components
        std::shared_ptr<Window> m_pWindow;
        //std::unique_ptr<Canvas> m_pCanvas;
        std::shared_ptr<Pipeline> m_pPipeline;
        //std::shared_ptr<GUI> m_pGUI;
        std::shared_ptr<Input> m_pInput;
        std::shared_ptr<Freq> m_pTimer;
        //std::shared_ptr<IPhysics> m_pPhysics;
        std::shared_ptr<Session> m_pSession;
        std::shared_ptr<Interpreter> m_pInterpreter;

#ifndef QOR_NO_AUDIO
        std::shared_ptr<Audio> m_pAudio;
#endif
        
        //std::shared_ptr<Meta> m_pModCfg;
        std::shared_ptr<Meta> m_pMeta = std::make_shared<Meta>();

        //std::shared_ptr<ResourceLocator> m_pLocator;
        //std::shared_ptr<ResourceCache<Texture>> m_pTextures;

        Args m_Args;
        std::string m_Filename;

        std::vector<std::string> m_SearchPaths;

        Freq::Alarm m_FPSAlarm;
        unsigned m_FramesLastSecond = 0;
        float m_FPS = 0.0f;
        std::string m_App;
        
        static Qor* s_pQor;
};

#endif

