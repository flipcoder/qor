#include "Material.h"
#include "Filesystem.h"
#include "kit/log/log.h"
#include <boost/filesystem.hpp>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
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
        m_Textures.push_back(cache->cache_as<Texture>(fn));
        for(auto&& t: s_ExtraMapNames) {
            auto tfn = cut + "_" + t + "." + ext;
            tfn = cache->transform(tfn);
            if(fs::exists(
                fs::path(tfn)
            )){
                static unsigned class_id = cache->class_id("texture");
                m_Textures.push_back(static_pointer_cast<Texture>(
                    cache->create(class_id, tuple<string, ICache*>(tfn, cache))
                ));
            }
        }

    }
}

void Material :: load_json(string fn)
{
}

void Material :: load_mtllib(string fn, string material)
{
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
            m_Textures.push_back(m_pCache->cache_as<ITexture>(tfn));
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

void Material :: bind(Pass* pass) const
{
    try{
        kit::safe_ptr(m_Textures.at(0))->bind(pass);
    }catch(...){
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
    // all detail maps exist
    if(compat == s_ExtraMapNames.size())
        return true;
    // partial compatibility probably means user forgot one, so we'll warn
    if(compat)
    {
        // TODO: remove this warning
        //WARNINGf("Material \"%s\" is missing %s out of %s detail maps",
        //    Filesystem::getFileName(fn) %
        //    (s_ExtraMapNames.size() - compat) %
        //    s_ExtraMapNames.size()
        //);
    }
    return false;
}

