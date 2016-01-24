#include "Material.h"
#include "Filesystem.h"
#include "kit/log/log.h"
#include <boost/filesystem.hpp>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
using namespace std;
namespace fs = boost::filesystem;

const std::vector<std::string> Material :: s_ExtraMapNames = {
    "NRM",
    "DISP",
    "SPEC",
    "OCC"
};

Material :: Material(
    const std::string& fn,
    Cache<Resource, std::string>* cache
):
    m_Filename(fn),
    m_pCache(cache)
{
    string fn_real = Filesystem::cutInternal(fn);
    string ext = Filesystem::getExtension(fn_real);
    string cut = Filesystem::cutExtension(fn_real);
    string emb = Filesystem::getInternal(fn);
    if(ext == "mtl")
        load_mtllib(fn_real, emb);
    else if(ext == "json")
        load_json(fn);
    else {
        static unsigned class_id = cache->class_id("texture");
        m_Textures.push_back(make_shared<Texture>(
            tuple<string, ICache*>(fn, cache)
        ));
        for(auto&& t: s_ExtraMapNames) {
            auto tfn = cut + "_" + t + "." + ext;
            tfn = cache->transform(tfn);
            if(fs::exists(
                fs::path(tfn)
            )){
                m_Textures.push_back(make_shared<Texture>(
                    tuple<string, ICache*>(tfn, cache)
                ));
            }else{
                //break;
            }
        }
        LOGf("textures: %s", m_Textures.size());
    }
}

void Material :: load_json(string fn)
{
    // ??? m_bComposite = true;
}

void Material :: load_mtllib(string fn, string material)
{
    m_bComposite = true;
            
    fstream f(fn);
    if(!f.good()) {
        ERROR(READ, Filesystem::getFileName(fn) + ":" + material);
    }
    
    string itr_material;
    string line;
    while(getline(f, line))
    {
        istringstream ss(line);
        string nothing;
        ss >> nothing;
        if(boost::starts_with(line, "newmtl")) {
            ss >> itr_material;
            continue;
        }
        if(material != itr_material)
            continue;
        if(boost::starts_with(line, "map_Kd"))
        {
            string tfn;
            ss >> tfn;
            tfn = Filesystem::getFileName(tfn);
            
            auto tex = m_pCache->cache_as<ITexture>(tfn);
            // should throw instead of returning null
            assert(tex);
            m_Textures.push_back(tex);
        }
        else if(boost::starts_with(line, "K"))
        {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            char ch = line.at(1);
            if(ch == 'a')
                m_Ambient = v;
            else if(ch == 'd')
            {
                LOG(Color(v).string());
                m_Diffuse = v;
            }
            else if(ch == 's')
                m_Specular = v;
        }
    }
}

Material :: ~Material()
{
}

//unsigned int Material :: id(Pass* pass) const
//{
//    return 0;
//}

void Material :: bind(Pass* pass, unsigned slot) const
{
    const unsigned sz = m_Textures.size();
    //const unsigned sz = max<unsigned>(1, m_Textures.size());
    // prevents pointless texture_slots state change for proxy material
    if(!m_bComposite) {
        pass->texture_slots(0);
        //unsigned slot_bits = 0;
        //for(unsigned i=0; i<sz; ++i) {
        //    if(m_Textures[i])
        //        slot_bits |= 1 << i;
        //}
        //pass->texture_slots(slot_bits);
    }
    if(not (pass->flags() & Pass::BASE))
        pass->material(m_Ambient, m_Diffuse, m_Specular);
    if(sz){
        for(unsigned i=0; i<sz; ++i) {
            if(m_Textures[i]) {
                m_Textures[i]->bind(pass, i);
            } else {
                pass->texture(0,i);
                break;
            }
        }
    }else{
        pass->texture(0,0);
    }
}

/*static*/ bool Material :: supported(
    string fn,
    Cache<Resource, std::string>* cache
){
    string fn_real = Filesystem::cutInternal(Filesystem::getFileName(fn));
    string ext = Filesystem::getExtension(fn_real);
    string cut = Filesystem::cutExtension(fn_real);
    string emb = Filesystem::getInternal(fn);
    
    if(ext=="mtllib")
        return true;
    else if(ext=="json")
        return true;

    unsigned compat = 0U;
    for(auto&& t: s_ExtraMapNames) {
        auto tfn = cut + "_" + t + "." + ext;
        tfn = cache->transform(tfn);
        if(fs::exists(
            fs::path(tfn)
        )){
            ++compat;
        }
    }
    return compat;
    //if(compat == s_ExtraMapNames.size())
    //    return true;
    // partial compatibility probably means user forgot one, so we'll warn
    //if(compat)
    //{
    //    // TODO: remove this warning
    //    //WARNINGf("Material \"%s\" is missing %s out of %s detail maps",
    //    //    Filesystem::getFileName(fn) %
    //    //    (s_ExtraMapNames.size() - compat) %
    //    //    s_ExtraMapNames.size()
    //    //);
    //}
    //return false;
}

