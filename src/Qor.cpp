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
#include "cppa/cppa.hpp"

//#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>
//#include <CEGUI/System.h>
using namespace std;
using namespace boost::filesystem;
using namespace boost::algorithm;
//namespace fs = boost::filesystem;

Qor :: Qor(const Args& args):
    m_Args(args)
{
    m_Filename = args.filename();
    
    {
        m_pUserCfg = make_shared<Meta>();
        //Log::Silencer ls;
        try {
            //m_pUserCfg->merge("settings.json");
            m_pUserCfg->merge(make_shared<Meta>("settings.json"));
        } catch(const Error& e) {}
        make_shared<Schema>("settings.schema.json")->validate(m_pUserCfg);
    }

    srand(time(NULL));
    
    // TODO: open global config and store it here
    
    m_Resources.register_class<Texture>("texture");
    m_Resources.register_class<Material>("material");
    m_Resources.register_class<Audio::Buffer>("audiobuffer");
    m_Resources.register_class<Audio::Stream>("audiostream");
    m_Resources.register_class<Mesh::Data>("meshdata");
    m_Resources.register_class<Scene>("scene");
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
     
    m_SearchPaths.push_back("mods/"+m_Args.value_or("mod","demo")+"/data/");
    m_SearchPaths.push_back("data/");
    m_SearchPaths.push_back("shaders/");
    
    m_pWindow = make_shared<Window>(m_Args, m_pUserCfg);
    
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
    m_LoadingState = m_StateFactory.register_class<LoadingState>();
    
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
    //clear_states_now();
    //m_pPipeline.reset();
}

void Qor :: init_actors()
{
   //m_OpenGLActor = spawn(task_queue_actor);
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

void Qor :: run(unsigned state_id)
{ 
    push_state(state_id);
    while(poll_state())
    {
        if(!state()->finished_loading())
        {
            // Seem like this will be called every loop? Incorrect!
            // Pay attention to the state being pushed here and how
            // it affects the outside loop. Hint: loading screen doesn't require
            // a loading screen ;)
            if(state()->needs_load()) {
                auto old_state = state();
                push_state(m_LoadingState);
                poll_state();
                auto t = thread(bind(&Qor::async_load, old_state.get()));
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
                LOG("entering state");
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
        
    //auto fn_l = to_lower_copy(std::get<0>(args));
    auto fn_cut = to_lower_copy(Filesystem::cutInternal(fn));

    //LOGf("Loading resource \"%s\"...", Filesystem::getFileName(fn));
    //LOG(fn_cut);
    
    if(ends_with(fn_cut, ".json"))
    {
        auto config = make_shared<Meta>(fn);
        //config->deserialize();
        if(config->has(".type"))
            return m_Resources.class_id(
                config->at<string>(".type")
            );
        else if(config->has("type"))
            return m_Resources.class_id(
                config->at<string>("type")
            );
        else
            ERRORf(PARSE, "No value for \".type\" or \"type\" in Resource \"%s\"", fn);
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

    ERRORf(GENERAL, "wtf @ \"%s\"", fn);
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
        if(exists(path(s)))
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

