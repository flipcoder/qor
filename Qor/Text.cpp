#include "Text.h"
#include <boost/lexical_cast.hpp>
#include "kit/log/log.h"
#include "ResourceCache.h"
using namespace std;
using namespace glm;

static int g_Init = 0;

Font :: Font(const std::string& fn, ICache* c)
{
    auto cache = (ResourceCache*)c;
    
    auto split_point = fn.find(":");
    int size = 12;
    if(split_point == string::npos)
        split_point = fn.size();
    else
        size = boost::lexical_cast<int>(fn.substr(split_point+1));
    m_pFont = TTF_OpenFont((fn.substr(0,split_point)).c_str(), size);
    if(!m_pFont){
        K_ERRORf(READ, "font \"%s\"", fn);
    }

    //std::vector<string> tokens;
    //string res = cache->config()->meta("video")->at<string>("resolution", "1920x1080");
    //boost::split(tokens, res, boost::is_any_of("x"));
    //assert(tokens.size() == 2);
    //m_WindowSize.x = boost::lexical_cast<int>(tokens[0]);
    //m_WindowSize.y = boost::lexical_cast<int>(tokens[1]);
    
    // TODO: resolution on_change event should mark font dirty
}

Font :: Font(const std::tuple<std::string, ICache*>& args):
    Font(std::get<0>(args), std::get<1>(args))
{}

Font :: ~Font()
{
    if(TTF_WasInit()){
        if(m_pFont)
            TTF_CloseFont(m_pFont);
    }
}

void Text :: init()
{
    if(g_Init==0)
    {
        TTF_Init();
    }
    ++g_Init;
}
    
void Text :: deinit()
{
    if(g_Init==1)
    {
        TTF_Quit();
    }
    --g_Init;
}

Text :: Text(const std::shared_ptr<Font>& font):
    m_pFont(font)
{
    init();
}

Text :: ~Text()
{
    if(m_pSurface){
        SDL_FreeSurface(m_pSurface);
        m_pSurface = nullptr;
    }
    
    deinit();
}

void Text :: redraw()
{
    if(m_pSurface){
        SDL_FreeSurface(m_pSurface);
        m_pSurface = nullptr;
    }
    
    SDL_Color color;
    color.r = Uint8(m_Color.r() * 0xFF);
    color.g = Uint8(m_Color.g() * 0xFF);
    color.b = Uint8(m_Color.b() * 0xFF);
    color.a = Uint8(m_Color.a() * 0xFF);
    m_pSurface = TTF_RenderText_Solid(m_pFont->font(),m_Text.c_str(),color);
    assert(m_pSurface);

    m_pTexture = nullptr;
    
    GLuint m_ID;
    
    GL_TASK_START()
        glGenTextures(1, &m_ID);
        glBindTexture(GL_TEXTURE_2D, m_ID);
        int mode = GL_RGB;
        //if(m_pSurface->format->BytesPerPixel == 4)
            mode = GL_RGBA;
        
        SDL_Surface* tmp = SDL_CreateRGBSurface(0,m_pSurface->w, m_pSurface->h,
            32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
        SDL_BlitSurface(m_pSurface, NULL, tmp, NULL);
        glTexImage2D(GL_TEXTURE_2D, 0, mode, tmp->w, tmp->h,
            0, mode, GL_UNSIGNED_BYTE, tmp->pixels);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        SDL_FreeSurface(tmp);
    GL_TASK_END()
    
    m_pTexture = make_shared<Texture>(m_ID);

    glm::vec2 vs,ve;
    if(m_Align == LEFT){
        vs = vec2(0.0f, 0.0f);
        ve = vec2(m_pSurface->w, m_pSurface->h);
    }else if(m_Align == RIGHT){
        vs = vec2(-m_pSurface->w, -m_pSurface->h);
        ve = vec2(0.0f, 0.0f);
    }else{
        vs = vec2(-m_pSurface->w / 2.0f, -m_pSurface->h / 2.0f);
        ve = vec2(m_pSurface->w / 2.0f, m_pSurface->h / 2.0f);
    }
    m_pMesh = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vs, ve)),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(vec2(0.0f,1.0f), vec2(1.0f,0.0f)))
        }, std::make_shared<MeshMaterial>(m_pTexture)
    );

    add(m_pMesh);
}

void Text :: logic_self(Freq::Time t)
{
    Node::logic_self(t);
    
    if(m_bDirty) {
        redraw();
        m_bDirty = false;
    }
}

void Text :: set(std::string tx)
{
    m_Text = tx;
    m_bDirty = true;
}

void Text :: align(Align a)
{
    m_Align = a;
    m_bDirty = true;
}

void Text :: color(Color c)
{
    m_Color = c;
    m_bDirty = true;
}


