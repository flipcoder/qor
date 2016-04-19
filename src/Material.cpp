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
    //"OCC"
};

Material :: Material(
    const std::string& fn,
    Cache<Resource, std::string>* cache
):
    ITexture(fn),
    m_Filename(fn),
    m_Name(fn),
    m_pCache(cache)
{
    string fn_real = Filesystem::cutInternal(fn);
    string ext = Filesystem::getExtension(fn_real);
    string cut = Filesystem::cutExtension(fn_real);
    string emb = Filesystem::getInternal(fn);

    if(ext == "mtl"){
        load_mtllib(fn_real, emb);
    }
    else if(ext != "json"){
        auto json_name = Filesystem::getFileName(cut)+".json";
        bool b= m_pCache->transform(json_name) != json_name;
        //LOGf("looking up %s: %s", json_name % b)
        auto transformed_json_name = m_pCache->transform(json_name);
        if(transformed_json_name != json_name){ // json exists
            // this will allow us to get json data from config
            Resource::filename(transformed_json_name);
            load_json(fn);
        }else
            load_detail_maps(fn);
    } else //if(ext == "json")
        load_json(fn);
}

void Material :: load_detail_maps(std::string fn)
{
    fn = m_pCache->transform(fn);
    
    string fn_real = Filesystem::cutInternal(fn);
    string ext = Filesystem::getExtension(fn_real);
    string cut = Filesystem::cutExtension(fn_real);
    string emb = Filesystem::getInternal(fn);
     
    m_Textures.push_back(make_shared<Texture>(
        tuple<string,ICache*>(fn,m_pCache)
    ));
    m_Filename = m_Textures[0]->filename();
    //m_Textures.push_back(m_pCache->m_pCache_cast<Texture>(fn));
    for(auto&& t: s_ExtraMapNames) {
        auto tfn = cut + "_" + t + "." + ext;
        tfn = m_pCache->transform(tfn);
        //LOG(tfn);
        if(fs::exists(
            fs::path(tfn)
        )){
            // TODO: material will be cached, so no need to use m_pCache for this
            m_Textures.push_back(make_shared<Texture>(
                tuple<string, ICache*>(tfn, m_pCache)
            ));
        }else{
            break;
        }
    }
    //LOGf("textures: %s", m_Textures.size());
}

void Material :: load_json(string fn)
{
    // ??? m_bComposite = true;
    //m_bComposite = true;
    assert(m_Textures.empty());
    
    if(not m_pConfig || m_pConfig->empty())
        return;

    //LOGf("loading json for %s...", fn)
    
    auto new_fn = Filesystem::getFileNameNoExt(fn)+".png";
    auto s = m_pConfig->at<string>("texture", new_fn);
    load_detail_maps(s);
    
    auto ambient = m_pConfig->meta("ambient", make_shared<Meta>(MetaFormat::JSON, "[1.0, 1.0, 1.0, 1.0]"));
    m_Ambient.set(
        ambient->at<double>(0),
        ambient->at<double>(1),
        ambient->at<double>(2),
        ambient->at<double>(3,1.0)
    );

    auto diffuse = m_pConfig->meta("diffuse", make_shared<Meta>(MetaFormat::JSON, "[1.0, 1.0, 1.0, 1.0]"));
    m_Diffuse.set(
        diffuse->at<double>(0),
        diffuse->at<double>(1),
        diffuse->at<double>(2),
        diffuse->at<double>(3,1.0)
    );

    auto specular = m_pConfig->meta("specular", make_shared<Meta>(MetaFormat::JSON, "[1.0, 1.0, 1.0, 1.0]"));
    m_Specular.set(
        specular->at<double>(0),
        specular->at<double>(1),
        specular->at<double>(2),
        specular->at<double>(3,1.0)
    );
    
    auto emissive = m_pConfig->meta("emissive", make_shared<Meta>(MetaFormat::JSON, "[0.0, 0.0, 0.0, 0.0]"));
    m_Emissive.set(
        emissive->at<double>(0),
        emissive->at<double>(1),
        emissive->at<double>(2),
        emissive->at<double>(3,1.0)
    );
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
            std::getline(ss, tfn);
            boost::trim(tfn);
            tfn = Filesystem::getFileName(tfn);
            //LOG(tfn);
            // if a texture json by this name exists, pull in that data?
            auto json_name = Filesystem::changeExtension(tfn,"json");
            if(m_pCache->transform(json_name) != json_name)
            {
                load_json(json_name);
            }
            else
            {
                load_detail_maps(tfn);
                //auto tex = m_pCache->cache_cast<ITexture>(tfn);
                //m_Textures.push_back(tex);
            }
        }
        else if(boost::starts_with(line, "K"))
        {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            char ch = line.at(1);
            if(ch == 'a')
                m_Ambient = v;
            else if(ch == 'd')
                m_Diffuse = v;
            else if(ch == 's')
                m_Specular = v;
            else if(ch == 'e'){
                //LOG(Vector::to_string(v));
                m_Emissive = v;
            }
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
    before(pass);
    //if(!m_bComposite) {
        pass->texture_slots(0);
        //unsigned slot_bits = 0;
        //for(unsigned i=0; i<sz; ++i) {
        //    if(m_Textures[i])
        //        slot_bits |= 1 << i;
        //}
        //pass->texture_slots(slot_bits);
    //}
    //if(not (pass->flags() & Pass::BASE))
    
    pass->material(m_Ambient, m_Diffuse, m_Specular, m_Emissive);
    
    if(sz){
        //if(m_Emissive.r() > 0.9f){
        //    LOGf("emissive bind %s", Vector::to_string(m_Emissive.vec3()));
        //}
        for(unsigned i=0; i<sz; ++i) {
            if(m_Textures[i]) {
                m_Textures[i]->bind_nomaterial(pass, i);
            } else {
                pass->texture(0,i);
                break;
            }
        }
    }else{
        pass->texture(0,0);
    }
    after(pass);
}

/*static*/ bool Material :: supported(
    string fn,
    Cache<Resource, std::string>* cache
){
    string fn_real = Filesystem::cutInternal(Filesystem::getFileName(fn));
    string ext = Filesystem::getExtension(fn_real);
    string cut = Filesystem::cutExtension(fn_real);
    string emb = Filesystem::getInternal(fn);
    //LOG("material transform");
    
    if(ext=="mtllib")
        return true;
    else if(ext=="json")
        return true;

    auto json_name = cut + ".json";
    if(cache->transform(json_name) != json_name){
        LOGf("detected json for %s", fn)
        return true;
    }

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

Material :: operator bool() const
{
    return true;
}

