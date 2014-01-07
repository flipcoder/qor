#include "Qor.h"
//#include "Nodes.h"
//#include "Sprite.h"
//#include "Grid.h"
#include "Physics.h"
#include "Node.h"
#include "Scene.h"
#include "Audio.h"
#include "Material.h"
#include "LoadingState.h"
#include "kit/freq/freq.h"
#include "kit/log/log.h"
#include "kit/args/args.h"
#include "kit/meta/meta.h"
#include <stdexcept>
#include <iostream>
#include <thread>
#include <boost/algorithm/string.hpp>

//#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>
//#include <CEGUI/System.h>
using namespace std;
using namespace boost::filesystem;
using namespace boost::algorithm;

Qor :: Qor(int argc, const char** argv):
    m_Args(argc, argv)
{
    if(argc)
        m_Filename = argv[0];
    
    m_pConfig = make_shared<Meta>("settings.json");
    
    // TODO: open global config and store it here
    
    m_Resources.register_class<Texture>("texture");
    m_Resources.register_class<Material>("material");
    m_Resources.register_class<Audio::Buffer>("sound");
    m_Resources.register_class<Audio::Stream>("stream");
    m_Resources.register_class<Mesh::Data>("mesh");
    m_Resources.register_class<Scene>("scene");
    
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
    
    m_pWindow = make_shared<Window>(m_Args, m_pConfig);
    
    //auto& renderer = CEGUI::OpenGLRenderer::bootstrapSystem();
    //CEGUI::OpenGLRenderer::create();
    //CEGUI::System::create(renderer);
    
    m_pInput = make_shared<Input>();
    m_pTimer = make_shared<Freq>();
    m_pGUI = make_shared<GUI>(m_pTimer.get(), m_pWindow.get(), &m_Resources);
    m_pAudio = make_shared<Audio>();

    //m_pLocator = make_shared<ResourceLocator>();
    //m_pTextures = make_shared<ResourceCache<Texture>>();
    m_LoadingState = m_StateFactory.register_class<LoadingState>();

    m_pPhysics = make_shared<Physics>();

    //Nodes::register_class<Sprite>();
    //Nodes::register_class<Grid>();

    m_pSession = make_shared<Session>(m_pInput.get());
    m_pInterpreter = make_shared<Interpreter>(
        "qor",
        (void*)this,
        vector<string> {
            "mods/"
        }
    );
    //push_state(eState::GAME);
    
    assert(!TaskHandler::get());
    TaskHandler::get(this);
    assert(TaskHandler::get() == this);
}

Qor :: ~Qor()
{
    //assert(TaskHandler::get() == this);
    //TaskHandler::get(this);
    //assert(!TaskHandler::get());
    clear_states_now();
}

void Qor :: logic()
{
    Freq::Time t;
    while(!(t = m_pTimer->tick()).ms())
        this_thread::yield();

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
        if(state()->needs_load() && !state()->finished_loading()) {
            auto old_state = state();
            push_state(m_LoadingState);
            poll_state();
            auto t = thread(bind(&Qor::async_load, old_state.get()));
            t.detach();
            continue;
        }
                
        logic();

        if(quit_flag())
            break;

        render();

        if(quit_flag())
            break;
    }
    clear_states_now();
}

std::shared_ptr<State> Qor :: new_state(unsigned id) {
    return m_StateFactory.create(id, this);
}

unsigned Qor :: resolve_resource(
    const std::tuple<
        string,
        ICache*
    >& args
){
    auto fn = std::get<0>(args);
    auto fn_l = to_lower_copy(std::get<0>(args));
    if(ends_with(fn_l, ".json"))
    {
        // TODO: read type from config
        // TODO: composite resource type might be useful at some point
        
        auto config = make_shared<Meta>(fn);
        try{
            return m_Resources.class_id(
                config->at<string>("type")
            );
        }catch(const std::out_of_range&){
            ERRORf(PARSE, "No value for \"type\" in Resource \"%s\"", fn);
            //throw std::numeric_limits<unsigned>::max();
        }
    }
    // TODO: eventually we may want a hashtable of supported extensions instead
    if(ends_with(fn_l, ".png")) {
        if(Material::supported(fn)) {
            static unsigned class_id = m_Resources.class_id("material");
            return class_id;
        }else{
            static unsigned class_id = m_Resources.class_id("texture");
            return class_id;
        }
    }
    if(ends_with(fn_l, ".obj")) {
        static unsigned class_id = m_Resources.class_id("mesh");
        return class_id;
    }
    if(ends_with(fn_l, ".wav")) {
        static unsigned class_id = m_Resources.class_id("sound");
        return class_id;
    }
    if(ends_with(fn_l, ".ogg")) {
        static unsigned class_id = m_Resources.class_id("stream");
        return class_id;
    }
    return 0;
}

string Qor :: resource_path(
    const string& s
){
    const path fn = path(s);
    for(const string& p: m_SearchPaths) {
        const recursive_directory_iterator end;
        const auto it = find_if(
            recursive_directory_iterator(path(p)),
            end,
            [&fn](const directory_entry& e) {
                //LOGf("%s =? %s ", e.path().filename().string() %  fn.string());
                return e.path().filename() == fn;
            }
        );
        if(it != end)
            return it->path().string();
    }
    return s;
}

