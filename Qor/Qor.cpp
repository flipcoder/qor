#include "Qor.h"
//#include "Nodes.h"
//#include "Sprite.h"
//#include "Grid.h"
#include "Headless.h"
#include "Physics.h"
#include "Light.h"
#include "Node.h"
#include "Sound.h"
#include "Scene.h"
#include "Audio.h"
#include "Particle.h"
#include "Material.h"
#include "LoadingState.h"
#include "Text.h"
//#include "GUI.h"
#include "kit/freq/freq.h"
#include "kit/log/log.h"
#include "kit/args/args.h"
#include "kit/meta/meta.h"
#include "kit/meta/schema.h"
#include <stdexcept>
#include <iostream>
#include <thread>
#include <boost/algorithm/string.hpp>
#include <future>

using namespace std;
using namespace boost::filesystem;
using namespace boost::algorithm;
namespace fs = boost::filesystem;
//namespace fs = boost::filesystem;

Qor* Qor :: s_pQor = nullptr;

Qor :: Qor(const Args& args, std::string appname="qor"):
    m_Args(args),
    m_App(appname)
{
    m_Filename = args.filename();
    s_pQor = this;

    if(m_Args.has('d', "dedicated")||
       m_Args.has('s', "server"))
    {
        Headless::enable();
        Headless::server(true);
        LOG("Running in headless mode");
    }

    if(m_Args.has('h', "headless"))
    {
        Headless::enable();
        LOG("Running in headless mode");
    }
        
    {
        //auto rl = m_Resources.lock();
        try {
            m_Resources.config()->merge(make_shared<Meta>("settings.json"));
        } catch(const Error& e) {}
        make_shared<Schema>("settings.schema.json")->validate(m_Resources.config());
    }

    srand(time(NULL));
    
    m_Resources.register_class<Texture>("texture");
    m_Resources.register_class<Material>("material");
#ifndef QOR_NO_AUDIO
    m_Resources.register_class<Audio::Buffer>("audiobuffer");
    m_Resources.register_class<Audio::Stream>("audiostream");
    m_Resources.register_class<Audio::OggStream>("oggstream");
#endif
    m_Resources.register_class<Mesh::Data>("meshdata");
    //m_Resources.register_class<Particle::Data>("particledata");
    //m_Resources.register_class<ParticleSystem::Data>("particlesystemdata");
    //m_Resources.register_class<Scene>("scene");
    m_Resources.register_class<Font>("font");
    //m_Resources.register_class<GUI::Form>("form");
    m_Resources.register_class<PipelineShader>("shader");
    
    m_Resources.register_resolver(bind(
        &Qor::resolve_resource,
        this,
        std::placeholders::_1
    ));
    m_Resources.register_transformer(bind(
        &Qor::resource_path,
        this,
        std::placeholders::_1
    ));
    m_Resources.register_preserver([](const std::string& s){
        auto ext = Filesystem::getExtension(s);
        if(ext == "ogg")
            return false; // don't cache stream
        if(ext == "json")
        {
            auto json = make_shared<Meta>(s);
            if(json->at<bool>("stream", false))
                return false;
        }
        return true; // cache
    });
    
    //if(m_Args.value_or("mod","").empty())
    //    m_SearchPaths.push_back("mods/demo/data");
    //else
    //    m_SearchPaths.push_back("mods/"+m_Args.value_or("mod","")+"/data/");
    
    m_SearchPaths.push_back("data/");
    m_SearchPaths.push_back("shaders/");

#ifdef _WIN32
    for(string& p: m_SearchPaths)
        boost::replace_all(p,"/","\\");
#endif
        
    m_pWindow = make_shared<Window>(m_Args, &m_Resources);
    m_MaxFPS = m_pWindow->refresh_rate();
    
    //auto& renderer = CEGUI::OpenGLRenderer::bootstrapSystem();
    //CEGUI::OpenGLRenderer::create();
    //CEGUI::System::create(renderer);
    
    m_pInput = make_shared<Input>(m_pWindow.get());
    m_pTimer = make_shared<Freq>();
    m_TPSAlarm.timer(m_pTimer->timeline());
    m_FPSAlarm.timer(m_pTimer->timeline());
    //m_pGUI = make_shared<GUI>(m_pTimer.get(), m_pWindow.get(), &m_Resources);
    //m_pGUI->init();
    
#ifndef QOR_NO_AUDIO
    m_pAudio = make_shared<Audio>();
#endif

    //m_pLocator = make_shared<ResourceLocator>();
    //m_pTextures = make_shared<ResourceCache<Texture>>();
    
    //m_NodeFactory.register_class<Node>("node");
    //m_NodeFactory.register_class<Mesh>("mesh");
    //m_NodeFactory.register_class<Camera>("camera");
    //m_NodeFactory.register_class<Sprite>("sprite");
    //m_NodeFactory.register_class<Sound>("sound");
    
    //m_NodeFactory.register_resolver(bind(
    //    &Qor::resolve_node,
    //    this,
    //    std::placeholders::_1
    //));
    //m_NodeFactory.register_transformer(bind(
    //    &Qor::node_path,
    //    this,
    //    std::placeholders::_1
    //));

    m_pSession = make_shared<Session>(appname, m_pInput.get());
    m_pInterpreter = make_shared<Interpreter>(
        "qor",
        (void*)this,
        vector<string> {
            "mods/"
        }
    );
    
    assert(!TaskHandler::get());
    TaskHandler::get(this);
    assert(TaskHandler::get() == this);
    
    //m_pCanvas = kit::make_unique<Canvas>(m_pWindow->size().x, m_pWindow->size().y);
    m_pPipeline = make_shared<Pipeline>(m_pWindow.get(), m_Args, &m_Resources);
    
    m_TPSAlarm.set(Freq::Time::seconds(1.0f));
    m_FPSAlarm.set(Freq::Time::seconds(1.0f));
}

Qor :: ~Qor()
{
    //assert(TaskHandler::get() == this);
    //TaskHandler::get(this);
    //assert(!TaskHandler::get());
    clear_states_now();
    m_pPipeline.reset();
}

void Qor :: logic()
{
    Freq::Time t;
    if(m_TPSAlarm.elapsed()) {
        m_TPSAlarm.set(Freq::Time::seconds(1.0f));
        m_TPS = m_TicksLastSecond;
        LOGf("Ticks: %s", m_TPS);
        m_TicksLastSecond = 0;
    }
    m_TickAccum = 0;
    while(true)
    {
        // accumulated enough time to advance?
        t = m_pTimer->tick();
        m_TickAccum += t.s();
        m_FrameAccum += t.s();
        if(m_MaxTick < K_EPSILON) // MaxTick==0 for unlimited ticks
            break;
        if((m_TickAccum > 1.0f/m_MaxTick) ||
            (m_MaxFPS > K_EPSILON && m_FrameAccum > 1.0f/m_MaxFPS))
            break;
        try{
            this_thread::yield();
        }catch(...){
            quit();
            return;
        }
    }
    //LOGf("%s", m_TickAccum);
    
    ++m_TicksLastSecond;

    bool pipeline_dirty = m_pPipeline->dirty();
    int pipeline_idlemode = m_pPipeline->idle();
    
    m_pInput->logic(t);
    if(m_pInput->quit_flag())
    {
        quit();
        return;
    }
    //m_pAudio->logic(t.ms());

    // Get pipeline idlemode and dirty state BEFORE logic() (since this changes)
    // Do not execute state logic if pipeline is using logic idle mode
    m_pPipeline->logic(t);
    if(not (pipeline_idlemode & Pipeline::IDLE_LOGIC) || pipeline_dirty)
        if(state()){
            state()->logic(t);
        }
}

void Qor :: render()
{
    if(Headless::enabled())
        return;

    bool pipeline_dirty = m_pPipeline->dirty();
    int pipeline_idlemode = m_pPipeline->idle();
    if((pipeline_idlemode & Pipeline::IDLE_RENDER) && not pipeline_dirty)
        return;

    if(m_FPSAlarm.elapsed()) {
        m_FPSAlarm.set(Freq::Time::seconds(1.0f));
        m_FPS = m_FramesLastSecond;
        LOGf("FPS: %s", m_FPS);
        m_FramesLastSecond = 0;
    }

    if(m_MaxFPS > K_EPSILON) // max fps limit?
        if(m_FrameAccum < 1.0f/m_MaxFPS) // not time for frame
            return; // do more logic() before render
    m_FrameAccum = 0;
    
    ++m_FramesLastSecond;

    if(state())
        state()->render();
    m_pWindow->render();
    //CEGUI::System::getSingleton().renderAllGUIContexts();
}

void Qor :: run(string state)
{
    run(state.empty() ? 0 : m_StateFactory.class_id(state));
}

void Qor :: run(unsigned state_id)
{ 
    if(m_LoadingState == ~0u)
        m_LoadingState = m_StateFactory.register_class<LoadingState>();
    
    push_state(state_id);
    while(poll_state())
    {
        if(!state()->finished_loading())
        {
            if(state()->needs_load()) {
                auto old_state = state();
                push_state(m_LoadingState);
                poll_state();
                auto t = thread(bind(&Qor::async_load, this, old_state.get()));
                t.detach();
                continue;
            }else{
                state()->preload();
                state()->finish_loading();
                continue;
            }
        }
        
        if(state())
        {
            //if(is_new_state()) {
            //    state()->start();
            //    LOG("state starting");
            //}
            if(is_entering_state()) {
                //LOG("entering state");
                state()->enter();
                state()->on_enter();
            }
        }

        logic();

        if(quit_flag())
            break;

        if(!pending())
            render();

        if(quit_flag())
            break;
    }
    
    clear_states_now();
}

std::shared_ptr<State> Qor :: new_state(unsigned id) {
    return m_StateFactory.create(id, this);
}

//unsigned Qor :: resolve_node(
//    const std::tuple<
//        string,
//        IFactory*,
//        ICache*
//    >& args
//){
//    auto fn = std::get<0>(args);
//    auto fn_l = to_lower_copy(std::get<0>(args));
//    auto ext = Filesystem::getExtension(fn);
    
//    if(ext == "json")
//    {
//        auto config = make_shared<Meta>(fn);
//        try{
//            return m_NodeFactory.class_id(
//                config->at<string>("type")
//            );
//        }catch(const std::out_of_range&){
//            K_ERRORf(PARSE, "No value for \"type\" in Node \"%s\"", fn);
//            //throw std::numeric_limits<unsigned>::max();
//        }
//    }
//    if(ext == "wav" || ext == "ogg"){
//        static unsigned class_id = m_NodeFactory.class_id("sound");
//        return class_id;
//    }
//    if(ext == "obj"){
//        static unsigned class_id = m_NodeFactory.class_id("mesh");
//        return class_id;
//    }
//    // TODO: ...
    
//    return std::numeric_limits<unsigned>::max();
//}

unsigned Qor :: resolve_resource(
    const std::tuple<
        string,
        ICache*
    >& args
){
    auto fn = std::get<0>(args);
    auto fn_cut = Filesystem::cutInternal(fn);

    //LOGf("Loading resource \"%s\"...", Filesystem::getFileName(fn));
    
    if(ends_with(fn_cut, ".json"))
    {
        auto config = make_shared<Meta>(fn_cut);
        //config->deserialize();
        //if(config->has(".type"))
        //    return m_Resources.class_id(
        //        config->at<string>(".type")
        //    );
        if(config->has("type"))
        {
            auto ttype = config->at<string>("type");
            if(ttype == "scene")
                return m_Resources.class_id("meshdata");
            return m_Resources.class_id(ttype);
        }
        else
        {
            K_ERRORf(PARSE, "No value for \".type\" or \"type\" in Resource \"%s\"", fn);
        }
    }
    // TODO: eventually we may want a hashtable of supported extensions instead
    if(ends_with(fn_cut, ".png") || ends_with(fn_cut, ".jpg")) {
        if(Material::supported(fn, &m_Resources)) {
            static unsigned class_id = m_Resources.class_id("material");
            return class_id;
        }else{
            static unsigned class_id = m_Resources.class_id("texture");
            return class_id;
        }
    }
    if(ends_with(fn_cut, ".mtl")) {
        static unsigned class_id = m_Resources.class_id("material");
        return class_id;
    }
    if(ends_with(fn_cut, ".obj")) {
        static unsigned class_id = m_Resources.class_id("meshdata");
        return class_id;
    }
    if(ends_with(fn_cut, ".ase")) {
        static unsigned class_id = m_Resources.class_id("meshdata");
        return class_id;
    }
#ifndef QOR_NO_AUDIO
    if(ends_with(fn_cut, ".wav")) {
        static unsigned class_id = m_Resources.class_id("audiobuffer");
        return class_id;
    }
    if(ends_with(fn_cut, ".ogg")) {
        static unsigned class_id = m_Resources.class_id("oggstream");
        return class_id;
    }
#endif
    if(ends_with(fn_cut, ".ttf")) {
        static unsigned class_id = m_Resources.class_id("font");
        return class_id;
    }
    //if(ends_with(fn_cut, ".rml")) {
    //    static unsigned class_id = m_Resources.class_id("form");
    //    return class_id;
    //}

    WARNINGf("wtf @ %s", fn);
    return std::numeric_limits<unsigned>::max();
}

string Qor :: resource_path(
    string s
){
    // did i already transform this? if so, return the same thing
    string r = s;

    // filename includes path?
    string sfn = Filesystem::getFileName(s);
    if(s.length() != sfn.length())
    {
        if(fs::exists(path(s)))
            return s; // if it exists, we're good
        s = std::move(sfn);
    }
        
    // the recursive dir search for filename 's'
    string internals = Filesystem::getInternal(s);
    string s_cut = Filesystem::cutInternal(s);
    string ext = Filesystem::getExtension(s_cut);
    
    auto itr = m_Paths.find(s_cut);
    if(itr != m_Paths.end())
    {
        if(fs::exists(path(itr->second))){
            if(internals.empty())
                return itr->second;
            else
                return itr->second + ":" + internals;
        }else{
            m_Paths.erase(itr);
        }
    }

    const path fn = path(s_cut);
    const recursive_directory_iterator end;
    for(const string& p: m_SearchPaths) {
        try{
            const auto it = find_if(
                recursive_directory_iterator(path(p), symlink_option::recurse),
                end,
                [&fn](const directory_entry& e) {
                    return e.path().filename() == fn;
                }
            );
            if(it != end)
            {
                auto ns = it->path().string();
                if(internals.empty())
                {
                    r = ns;
                    break;
                }
                else
                {
                    r = ns + ":" + internals;
                    break;
                }
            }
        }catch(boost::filesystem::filesystem_error&){}
    }
    if(r!=s) // found?
    {
        if(ext != "json")
        {
            // try to load accompanying json file instead (same dir only)
            auto chng = Filesystem::changeExtension(s_cut, "json");
            if(fs::exists(chng))
                r = chng + ":" + internals;
        }
        m_Paths[s_cut] = Filesystem::cutInternal(r); // cache for later
    }
    return r;
    //return std::string();
}

bool Qor :: exists(std::string s)
{
    return resource_path(s) != s;
    //return not resource_path(s).empty();
}

//tuple<
//    string,
//    IFactory*,
//    ICache*
//> Qor :: node_path(const tuple<
//    string,
//    IFactory*,
//    ICache*
//>& args) {
//    return tuple<
//        string,
//        IFactory*,
//        ICache*
//    >(
//        resource_path(std::get<0>(args)),
//        std::get<1>(args),
//        std::get<2>(args)
//    );
//}

void Qor :: wait_task(std::function<void()> func)
{
    // wrap task so we can get the exception out
    auto l = std::unique_lock<std::mutex>(m_TasksMutex);
    auto cbt = packaged_task<void()>(std::move(func));
    auto fut = cbt.get_future();
    auto cbc = std::make_shared<packaged_task<void()>>(std::move(cbt));
    m_Tasks.push_front([cbc]{
        (*cbc)();
    });
    l.unlock();
    while(true) {
        auto l2 = std::unique_lock<std::mutex>(m_TasksMutex);
        if(m_Tasks.empty())
            break;
        l2.unlock();
        std::this_thread::yield();
    }
    fut.get(); // pump exception
}

void Qor :: save_settings()
{
    m_Resources.config()->serialize("settings.json");
}

