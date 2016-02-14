#include "Qor.h"
//#include "Nodes.h"
//#include "Sprite.h"
//#include "Grid.h"
#include "Physics.h"
#include "Light.h"
#include "Node.h"
#include "Sound.h"
#include "Scene.h"
#include "Audio.h"
#include "Particle.h"
#include "Material.h"
#include "LoadingState.h"
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

Qor :: Qor(const Args& args):
    m_Args(args)
{
    m_Filename = args.filename();
    
    {
        //auto rl = m_Resources.lock();
        try {
            m_Resources.config()->merge(make_shared<Meta>("settings.json"));
        } catch(const Error& e) {}
        make_shared<Schema>("settings.schema.json")->validate(m_Resources.config());
    }

    srand(time(NULL));
    
    // TODO: open global config and store it here
    
    m_Resources.register_class<Texture>("texture");
    m_Resources.register_class<Material>("material");
    m_Resources.register_class<Audio::Buffer>("audiobuffer");
    m_Resources.register_class<Audio::Stream>("audiostream");
    m_Resources.register_class<Mesh::Data>("meshdata");
    //m_Resources.register_class<Particle::Data>("particledata");
    //m_Resources.register_class<ParticleSystem::Data>("particlesystemdata");
    //m_Resources.register_class<Scene>("scene");
    //m_Resources.register_class<GUI::Font>("font");
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
    
    if(m_Args.value_or("mod","").empty())
        m_SearchPaths.push_back("mods/demo/data");
    else
        m_SearchPaths.push_back("mods/"+m_Args.value_or("mod","")+"/data/");
    m_SearchPaths.push_back("data/");
    m_SearchPaths.push_back("shaders/");
    
    m_pWindow = make_shared<Window>(m_Args, &m_Resources);
    
    //auto& renderer = CEGUI::OpenGLRenderer::bootstrapSystem();
    //CEGUI::OpenGLRenderer::create();
    //CEGUI::System::create(renderer);
    
    m_pInput = make_shared<Input>(m_pWindow.get());
    m_pTimer = make_shared<Freq>();
    m_FPSAlarm.timer(m_pTimer->timeline());
    //m_pGUI = make_shared<GUI>(m_pTimer.get(), m_pWindow.get(), &m_Resources);
    //m_pGUI->init();
    m_pAudio = make_shared<Audio>();

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

    m_pSession = make_shared<Session>(m_pInput.get());
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
    if(m_FPSAlarm.elapsed()) {
        m_FPSAlarm.set(Freq::Time::seconds(1.0f));
        m_FPS = m_FramesLastSecond;
        LOGf("FPS: %s", m_FPS);
        m_FramesLastSecond = 0;
    }
    while(!(t = m_pTimer->tick()).ms())
        this_thread::yield();
    //t = m_pTimer->tick();
    ++m_FramesLastSecond;

    m_pInput->logic(t);
    if(m_pInput->quit_flag())
    {
        quit();
        return;
    }
    //m_pAudio->logic(t.ms());

    if(state())
        state()->logic(t);
}

void Qor :: render()
{
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
//            ERRORf(PARSE, "No value for \"type\" in Node \"%s\"", fn);
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

    LOGf("Loading resource \"%s\"...", Filesystem::getFileName(fn));
    //LOG(fn_cut);
    
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
            ERRORf(PARSE, "No value for \".type\" or \"type\" in Resource \"%s\"", fn);
        }
    }
    // TODO: eventually we may want a hashtable of supported extensions instead
    if(ends_with(fn_cut, ".png")) {
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
    if(ends_with(fn_cut, ".wav")) {
        static unsigned class_id = m_Resources.class_id("audiobuffer");
        return class_id;
    }
    if(ends_with(fn_cut, ".ogg")) {
        static unsigned class_id = m_Resources.class_id("audiostream");
        return class_id;
    }
    if(ends_with(fn_cut, ".otf")) {
        static unsigned class_id = m_Resources.class_id("font");
        return class_id;
    }
    if(ends_with(fn_cut, ".rml")) {
        static unsigned class_id = m_Resources.class_id("form");
        return class_id;
    }

    WARNINGf("wtf @ %s", fn);
    return std::numeric_limits<unsigned>::max();
}

string Qor :: resource_path(
    string s
){
    // search current directory first

    // filename includes path?
    string sfn = Filesystem::getFileName(s);
    if(s.length() != sfn.length())
    {
        if(fs::exists(path(s)))
            return s; // if it exists, we're good
        s = std::move(sfn); // otherwise, remove path for search
    }
    
    // the recursive dir search for filename 's'
    string internals = Filesystem::getInternal(s);
    const path fn = path(Filesystem::cutInternal(s));
    const recursive_directory_iterator end;
    for(const string& p: m_SearchPaths) {
        try{
            const auto it = find_if(
                recursive_directory_iterator(path(p)),
                end,
                [&fn](const directory_entry& e) {
                    return e.path().filename() == fn;
                }
            );
            if(it != end)
            {
                auto ns = it->path().string();
                if(internals.empty())
                    return ns;
                else
                    return ns + ":" + internals;
            }
        }catch(boost::filesystem::filesystem_error&){}
    }
    return s;
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
    auto cbc = kit::move_on_copy<packaged_task<void()>>(std::move(cbt));
    m_Tasks.push_front([cbc]{
        cbc.get()();
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

