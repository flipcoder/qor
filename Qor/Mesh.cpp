#include "Mesh.h"
#include "Common.h"
#include "GLTask.h"
#include "Filesystem.h"
#include "kit/log/log.h"
#include <fstream>
#include <sstream>
#include <set>
#include <tuple>
#include <algorithm>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <glm/glm.hpp>
#include <boost/tokenizer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "ResourceCache.h"
#include "Material.h"
using namespace std;
using namespace glm;
using namespace boost::algorithm;
using std::tuple;
using std::shared_ptr;
using std::get;

vector<vec3> MeshIndexedGeometry :: ordered_verts() const
{
    vector<vec3> r;
    for(uvec3 face: m_Indices)
        for(unsigned i=0; i<3; ++i)
            r.push_back(m_Vertices.at(face[i]));

    return r;
}

void MeshGeometry :: clear_cache()
{
    if(m_VertexBuffer)
    {
        GL_TASK_START()
            glDeleteBuffers(1, &m_VertexBuffer);
            m_VertexBuffer = 0;
        GL_TASK_END()
    }
}

void MeshIndexedGeometry :: clear_cache()
{
    if(m_VertexBuffer)
    {
        GL_TASK_START()
            glDeleteBuffers(1, &m_VertexBuffer);
            m_VertexBuffer = 0;
        GL_TASK_END()
    }
    if(m_IndexBuffer)
    {
        GL_TASK_START()
            glDeleteBuffers(1, &m_IndexBuffer);
            m_IndexBuffer = 0;
        GL_TASK_END()
    }
}

void Wrap :: clear_cache()
{
    if(m_VertexBuffer)
    {
        GL_TASK_START()
            glDeleteBuffers(1, &m_VertexBuffer);
            m_VertexBuffer = 0;
        GL_TASK_END()
    }
}

void MeshColors :: clear_cache()
{
    if(m_VertexBuffer)
    {
        GL_TASK_START()
            glDeleteBuffers(1, &m_VertexBuffer);
            m_VertexBuffer = 0;
        GL_TASK_END()
    }
}

void MeshFade :: clear_cache()
{
    if(m_VertexBuffer)
    {
        GL_TASK_START()
            glDeleteBuffers(1, &m_VertexBuffer);
            m_VertexBuffer = 0;
        GL_TASK_END()
    }
}

void MeshNormals :: clear_cache()
{
    if(m_VertexBuffer)
    {
        GL_TASK_START()
            glDeleteBuffers(1, &m_VertexBuffer);
            m_VertexBuffer = 0;
        GL_TASK_END()
    }
}

void MeshTangents :: clear_cache()
{
    if(m_VertexBuffer)
    {
        GL_TASK_START()
            glDeleteBuffers(1, &m_VertexBuffer);
            m_VertexBuffer = 0;
        GL_TASK_END()
    }
}

void MeshBinormals :: clear_cache()
{
    if(m_VertexBuffer)
    {
        GL_TASK_START()
            glDeleteBuffers(1, &m_VertexBuffer);
            m_VertexBuffer = 0;
        GL_TASK_END()
    }
}

void MeshGeometry :: cache(Pipeline* pipeline) const
{
    if(m_Vertices.empty())
        return;

    if(!m_VertexBuffer)
    {
        GL_TASK_START()
            glGenBuffers(1, &m_VertexBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
            glBufferData(
                GL_ARRAY_BUFFER,
                m_Vertices.size() * 3 * sizeof(float),
                &m_Vertices[0],
                GL_STATIC_DRAW
            );
        GL_TASK_END()
    }
}

void MeshIndexedGeometry :: cache(Pipeline* pipeline) const
{
    if(m_Vertices.empty())
        return;
    if(m_Indices.empty())
        return;

    if(!m_VertexBuffer)
    {
        GL_TASK_START()
            glGenBuffers(1, &m_VertexBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
            glBufferData(
                GL_ARRAY_BUFFER,
                m_Vertices.size() * 3 * sizeof(float),
                &m_Vertices[0],
                GL_STATIC_DRAW
            );
        GL_TASK_END()
    }
    if(!m_IndexBuffer)
    {
        GL_TASK_START()
            glGenBuffers(1, &m_IndexBuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                m_Indices.size() * 3 * sizeof(unsigned),
                &m_Indices[0],
                GL_STATIC_DRAW
            );
        GL_TASK_END()
    }
}

void Wrap :: cache(Pipeline* pipeline) const
{
    if(m_UV.empty())
        return;

    if(!m_VertexBuffer)
    {
        GL_TASK_START()
            glGenBuffers(1, &m_VertexBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
            glBufferData(
                GL_ARRAY_BUFFER,
                m_UV.size() * 2 * sizeof(float),
                &m_UV[0],
                GL_STATIC_DRAW
            );
        GL_TASK_END()
    }
}

void MeshColors :: cache(Pipeline* pipeline) const
{
    if(m_Colors.empty())
        return;

    if(!m_VertexBuffer)
    {
        GL_TASK_START()
            glGenBuffers(1, &m_VertexBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
            glBufferData(
                GL_ARRAY_BUFFER,
                m_Colors.size() * 4 * sizeof(float),
                &m_Colors[0],
                GL_STATIC_DRAW
            );
        GL_TASK_END()
    }
}

void MeshFade :: cache(Pipeline* pipeline) const
{
    if(m_Fade.empty())
        return;

    if(!m_VertexBuffer)
    {
        GL_TASK_START()
            glGenBuffers(1, &m_VertexBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
            glBufferData(
                GL_ARRAY_BUFFER,
                m_Fade.size() * sizeof(float),
                &m_Fade[0],
                GL_STATIC_DRAW
            );
        GL_TASK_END()
    }
}

void MeshNormals :: cache(Pipeline* pipeline) const
{
    if(m_Normals.empty())
        return;

    if(!m_VertexBuffer)
    {
        GL_TASK_START()
            glGenBuffers(1, &m_VertexBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
            glBufferData(
                GL_ARRAY_BUFFER,
                m_Normals.size() * 3 * sizeof(float),
                &m_Normals[0],
                GL_STATIC_DRAW
            );
        GL_TASK_END()
    }
}

void MeshTangents :: cache(Pipeline* pipeline) const
{
    if(m_Tangents.empty())
        return;

    if(!m_VertexBuffer)
    {
        GL_TASK_START()
            glGenBuffers(1, &m_VertexBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
            glBufferData(
                GL_ARRAY_BUFFER,
                m_Tangents.size() * 4 * sizeof(float),
                &m_Tangents[0],
                GL_STATIC_DRAW
            );
        GL_TASK_END()
    }
}

void MeshBinormals :: cache(Pipeline* pipeline) const
{
    if(m_Binormals.empty())
        return;

    if(!m_VertexBuffer)
    {
        GL_TASK_START()
            glGenBuffers(1, &m_VertexBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
            glBufferData(
                GL_ARRAY_BUFFER,
                m_Binormals.size() * 4 * sizeof(float),
                &m_Binormals[0],
                GL_STATIC_DRAW
            );
        GL_TASK_END()
    }
}

void MeshGeometry :: apply(Pass* pass) const
{
    if(m_Vertices.empty())
        return;

    Pipeline* pipeline = pass->pipeline();
    cache(pipeline);

    pass->vertex_buffer(m_VertexBuffer);
    pass->element_buffer(0);
    
    glVertexAttribPointer(
        pass->attribute_id((unsigned)Pipeline::AttributeID::VERTEX),
        3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL
    );
    glDrawArrays(GL_TRIANGLES, 0, m_Vertices.size());
}

void MeshGeometry :: append(std::vector<glm::vec3> verts)
{
    m_Vertices.insert(m_Vertices.end(), ENTIRE(verts));
    clear_cache();
}

void MeshIndexedGeometry :: apply(Pass* pass) const
{
    //if(m_Vertices.empty())
    //    return;
    if(m_Indices.empty())
        return;

    Pipeline* pipeline = pass->pipeline();
    cache(pipeline);

    pass->vertex_buffer(m_VertexBuffer);
    pass->element_buffer(m_IndexBuffer);
    
    glVertexAttribPointer(
        pass->attribute_id((unsigned)Pipeline::AttributeID::VERTEX),
        3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL
    );
    
    glDrawElements(GL_TRIANGLES, 3 * m_Indices.size(), GL_UNSIGNED_INT, (GLubyte*)NULL);
}

unsigned Wrap :: layout() const
{
    return Pipeline::WRAP;
}

unsigned MeshColors :: layout() const
{
    return Pipeline::COLOR;
}

unsigned MeshFade :: layout() const
{
    return Pipeline::FADE;
}

unsigned MeshNormals :: layout() const
{
    return Pipeline::NORMAL;
}

unsigned MeshTangents :: layout() const
{
    return Pipeline::TANGENT;
}

unsigned MeshBinormals :: layout() const
{
    return Pipeline::BINORMAL;
}

void Wrap :: apply(Pass* pass) const
{
    if(m_UV.empty())
        return;

    Pipeline* pipeline = pass->pipeline();
    cache(pipeline);

    pass->vertex_buffer(m_VertexBuffer);
    glVertexAttribPointer(
        pass->attribute_id((unsigned)Pipeline::AttributeID::WRAP),
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        (GLubyte*)NULL
    );
}

void Wrap :: append(std::vector<glm::vec2> data)
{
    m_UV.insert(m_UV.end(), ENTIRE(data));
    clear_cache();
}

void MeshColors :: apply(Pass* pass) const
{
    if(m_Colors.empty())
        return;

    Pipeline* pipeline = pass->pipeline();
    cache(pipeline);

    pass->vertex_buffer(m_VertexBuffer);
    glVertexAttribPointer(
        pass->attribute_id((unsigned)Pipeline::AttributeID::COLOR),
        4,
        GL_FLOAT,
        GL_FALSE,
        0,
        (GLubyte*)NULL
    );
}

void MeshFade :: apply(Pass* pass) const
{
    if(m_Fade.empty())
        return;

    Pipeline* pipeline = pass->pipeline();
    cache(pipeline);

    pass->vertex_buffer(m_VertexBuffer);
    glVertexAttribPointer(
        pass->attribute_id((unsigned)Pipeline::AttributeID::FADE),
        1,
        GL_FLOAT,
        GL_FALSE,
        0,
        (GLubyte*)NULL
    );
}

void MeshNormals :: apply(Pass* pass) const
{
    if(m_Normals.empty())
        return;

    Pipeline* pipeline = pass->pipeline();
    cache(pipeline);

    pass->vertex_buffer(m_VertexBuffer);
    glVertexAttribPointer(
        pass->attribute_id((unsigned)Pipeline::AttributeID::NORMAL),
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        (GLubyte*)NULL
    );
}

void MeshTangents :: apply(Pass* pass) const
{
    if(m_Tangents.empty())
        return;

    Pipeline* pipeline = pass->pipeline();
    cache(pipeline);

    pass->vertex_buffer(m_VertexBuffer);
    glVertexAttribPointer(
        pass->attribute_id((unsigned)Pipeline::AttributeID::TANGENT),
        4,
        GL_FLOAT,
        GL_FALSE,
        0,
        (GLubyte*)NULL
    );
}

void MeshBinormals :: apply(Pass* pass) const
{
    if(m_Binormals.empty())
        return;

    Pipeline* pipeline = pass->pipeline();
    cache(pipeline);

    pass->vertex_buffer(m_VertexBuffer);
    glVertexAttribPointer(
        pass->attribute_id((unsigned)Pipeline::AttributeID::BINORMAL),
        4,
        GL_FLOAT,
        GL_FALSE,
        0,
        (GLubyte*)NULL
    );
}

void MeshMaterial :: apply(Pass* pass) const
{
    if(!m_pTexture){
        pass->material(m_Ambient, m_Diffuse, m_Specular, m_Emissive);
        return;
    }else{
        pass->material(
            m_pTexture->ambient() * m_Ambient,
            m_pTexture->diffuse() * m_Diffuse, 
            m_pTexture->specular() * m_Specular,
            m_pTexture->emissive().a() > m_Emissive.a() ?
                m_pTexture->emissive() :
                m_Emissive
        );
    }
    m_pTexture->bind(pass);
}

bool vec4cmp(const vec4& a,const vec4& b)
{
    if(a.x != b.x)
        return a.x < b.x;
    if(a.y != b.y)
        return a.y < b.y;
    if(a.z != b.z)
        return a.z < b.z;
    return a.w < b.w;
}

bool vec3cmp(const vec3& a,const vec3& b)
{
    if(a.x != b.x)
        return a.x < b.x;
    if(a.y != b.y)
        return a.y < b.y;
    return a.z < b.z;
}

bool vec2cmp(const vec2& a,const vec2& b)
{
    if(a.x != b.x)
        return a.x < b.x;
    return a.y < b.y;
}

// vertex compare that ignores bitangents
struct vertcmp
{
    bool operator()(const tuple<
        vec3, // v
        vec2, // w
        vec3, // n
        vec4 // t
    >& a,const tuple<
        vec3, // v
        vec2, // w
        vec3, // n
        vec4 // t
    >& b
    ){
        if(get<0>(a) != get<0>(b))
            return vec3cmp(get<0>(a), get<0>(b));
        if(get<1>(a) != get<1>(b))
            return vec2cmp(get<1>(a), get<1>(b));
        //if(get<2>(a) != get<2>(b))
        return vec3cmp(get<2>(a), get<2>(b));
        //return vec4cmp(get<3>(a), get<3>(b));
    }
};

//bool vertcmp(const tuple<
//    vec3, // v
//    vec2, // w
//    vec3 // n
//>& a,const tuple<
//    vec3, // v
//    vec2, // w
//    vec3 // n
//>& b
//) {
//    if(get<0>(a) != get<0>(b))
//        return vec3(get<0>(a) < get<0>(b));
//    if(get<1>(a) != get<1>(b))
//        return vec2(get<0>(a) < get<0>(b));
//    return vec3(get<2>(a), get<2>(b));
//}

Mesh::Data :: Data(
    string fn,
    Cache<Resource, string>* cache
):
    Resource(fn),
    cache(cache)
{
    //auto t = std::chrono::high_resolution_clock::now();
    
    size_t offset = fn.rfind(':');
    string this_object, this_material;
    string fn_base = Filesystem::getFileNameNoInternal(fn);
    //LOG(fn_base);
    if(offset != string::npos) {
        vector<string> tokens;
        boost::split(
            tokens,
            fn_base,
            boost::is_any_of(":")
        );
        this_object = tokens.at(1);
        TRY(this_material = tokens.at(2));
    }
    
    //if(!ends_with(to_lower_copy(fn), string(".mesh")))
    //    K_ERROR(READ, "invalid format");

    //auto data = make_shared<Meta>(fn);

    //if(data->meta("metadata")->at<int>("formatVersion") == 3)
    //    K_ERRORf(PARSE, "Invalid format version for %s", Filesystem::getFileName(fn));
    
    //auto materials = data->meta("materials");
    
    //if(Filesystem::hasExtension(fn, "json"))
    //{
    //    if(m_pConfig->at("composite", false)) {
    //        K_ERRORf(
    //            READ,
    //            "\"%s\" as composite",
    //            Filesystem::getFileName(fn)
    //        );
    //    }
    //    fn = m_pConfig->at("filename", string());
    //}
    
    fn = Filesystem::cutInternal(fn);
    fn = Filesystem::cutInternal(fn);
    string ext = Filesystem::getExtension(fn);
    //LOGf("getExtension: %s", ext)
    if(ext == "obj")
        load_obj(fn, this_object, this_material);
    else if(ext == "json")
        load_json(fn, this_object, this_material);

    //calculate_tangents();
    calculate_box();

    //auto t2 = std::chrono::high_resolution_clock::now();
    
    //LOGf(
    //    "Loaded %s in %sms",
    //    Filesystem::getFileName(fn) %
    //    chrono::duration_cast<chrono::milliseconds>(t2-t).count()
    //);
}

void Mesh::Data :: load_json(string fn, string this_object, string this_material)
{
    // this_object and this_material

    // TODO: cache these files
    
    //LOGf("load_json fn %s obj %s mat %s", fn % this_object % this_material);
    
    auto doc = ((ResourceCache*)cache)->config(fn)->meta("data")->meta(
        this_object + ":" + this_material
    );

    //auto t = std::chrono::high_resolution_clock::now();
    
    std::vector<glm::uvec3> indices;
    std::vector<glm::vec3> verts;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> wrap;
    std::vector<glm::vec4> tangents;
    std::vector<float> fade;

    auto indices_d = doc->at<string>("indices", string());
    auto verts_d = doc->at<string>("vertices", string());
    auto wrap_d = doc->at<string>("wrap", string());
    auto normals_d = doc->at<string>("normals", string());
    auto tangents_d = doc->at<string>("tangents", string());
    auto fade_d = doc->at<string>("fade", string());
    //LOGf("indices: %s", (indices_d->size() / 3));
    //LOGf("vertices: %s", (verts_d->size() / 3));

    int a,b,c;
    float x,y,z,w;
    
    indices.reserve(doc->at<int>("num_indices"));
    std::stringstream ss(indices_d);
    while(ss >> a){
        ss >> b >> c;
        indices.push_back(uvec3(a,b,c));
    }
    
    verts.reserve(doc->at<int>("num_vertices"));
    ss.str(verts_d);
    while(ss >> x){
        ss >> y >> z;
        verts.push_back(vec3(x,y,z));
    }
    
    wrap.reserve(doc->at<int>("num_wrap"));
    ss.str(wrap_d);
    while(ss >> x){
        ss >> y;
        wrap.push_back(vec2(x,y));
    }
    
    normals.reserve(doc->at<int>("num_normals"));
    ss.str(normals_d);
    while(ss >> x){
        ss >> y >> z; 
        normals.push_back(vec3(x,y,z));
    }
    
    tangents.reserve(doc->at<int>("num_tangents"));
    ss.str(tangents_d);
    while(ss >> x){
        ss >> y >> z >> w;
        tangents.push_back(vec4(x,y,z,w));
    }
    
    fade.reserve(doc->at<int>("num_fade"));
    ss.str(fade_d);
    while(ss >> x)
        fade.push_back(x);
    
    if(indices.empty())
        geometry = make_shared<MeshGeometry>(verts);
    else
        geometry = make_shared<MeshIndexedGeometry>(verts, indices);
    if(not wrap.empty())
        mods.push_back(make_shared<Wrap>(wrap));
    if(not normals.empty())
        mods.push_back(make_shared<MeshNormals>(normals));
    if(not tangents.empty())
        mods.push_back(make_shared<MeshTangents>(tangents));
    if(not fade.empty())
        mods.push_back(make_shared<MeshFade>(fade));
    
    auto tex = doc->at<string>("image", string());
    if(Filesystem::getExtension(tex).empty())
        tex += ".png";
    if(not tex.empty() && tex!=".png")
        material = make_shared<MeshMaterial>(cache->cache_cast<ITexture>(tex));
    
    //auto t2 = std::chrono::high_resolution_clock::now();
    //LOGf(
    //    "Loaded json data for %s in %sms",
    //    Filesystem::getFileName(fn) %
    //    chrono::duration_cast<chrono::milliseconds>(t2-t).count()
    //);

    //LOG("done loading json");
}

//void Mesh::Data :: load_assimp(string fn, string this_object, string this_material)
//{
//    auto_ptr<Assimp::Importer> importer(new Assimp::Importer());
//    unsigned int flags = aiProcess_Triangulate|
//        aiProcess_CalcTangentSpace|
//        aiProcess_GenSmoothNormals;//GenNormals
//        //aiProcess_GenSmoothNormals;

//    const aiScene* aiscene = importer->ReadFile(fn, flags);
//    std::string err = importer->GetErrorString();

//    if(err != "" || 
//        aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
//        aiscene->mFlags & AI_SCENE_FLAGS_VALIDATED)
//    {
//        return false;
//    }
    
//    assert(aiscene->mNumMeshes >= 1);
    
//    for(unsigned int i=0; i<aiscene->mNumMaterials; i++)
//    {
        
//    }
//}

void Mesh::Data :: load_obj(string fn, string this_object, string this_material)
{
    ifstream f(fn);
    if(!f.good()) {
        K_ERROR(READ, Filesystem::getFileName(fn));
    }
    string line;
    vector<vec3> verts;
    vector<vec2> wrap;
    vector<vec3> normals;
    //vector<vec4> tangents;
    //vector<vec4> binormals;
    
    set<
        tuple<
            vec3, // v
            vec2, // w
            vec3, // n
            vec4 // t
        >,
        vertcmp
    > newset;
    vector<
        tuple<
            vec3, // v
            vec2, // w
            vec3, // n
            vec4 // t
        >
    > newvec;

    vector<uvec3> faces;
    unordered_map<
        unsigned,
        tuple<vec4 /*total*/, unsigned /*count*/>
    > tangent_averages;
    
    string mtllib;
    //bool untriangulated = false;
    //vector<vec2> wrap_index;
    //vector<vec3> normal_index;
    
    string itr_object;
    string itr_material;
        
    while(getline(f, line))
    {
        if(starts_with(trim_copy(line), "#"))
            continue;

        istringstream ss(line);
        string nothing;
        ss >> nothing;
        
        if(starts_with(line, "mtllib ")) {
            ss >> mtllib;
            continue;
        }
        else if(starts_with(line, "o ")){
            ss >> itr_object;
            continue;
        }
        else if(starts_with(line, "usemtl ")) {
            ss >> itr_material;
            continue;
        }

        if(starts_with(line, "v "))
        {
            vec3 vec;
            float* v = value_ptr(vec);
            ss >> v[0];
            ss >> v[1];
            ss >> v[2];
            verts.push_back(vec);
        }
        else if(starts_with(line, "vn "))
        {
            vec3 vec;
            float* v = value_ptr(vec);
            ss >> v[0];
            ss >> v[1];
            ss >> v[2];
            normals.push_back(vec);
        }
        else if(starts_with(line, "vt "))
        {
            vec2 vec;
            float* v = value_ptr(vec);
            ss >> v[0];
            ss >> v[1];
            v[1] = 1.0f - v[1];
            wrap.push_back(vec);
        }
        else if(starts_with(line, "f "))
        {
            //assert(!this_object.empty());
            //assert(!this_material.empty());
            
            if(this_object != itr_object)
                continue;
            if(this_material != itr_material)
                continue;
            
            //uvec4 index;
            unsigned index[4] = {0};
            tuple<vec3, vec2, vec3, vec4> vert[4] = {};
            unsigned vert_count = 0;
            
            for(unsigned i=0;i<4;++i) {
                vert[i] = tuple<vec3, vec2, vec3, vec4>();
                
                unsigned v[3] = {0};
                
                string face;
                if(!(ss >> face)) {
                    if(i == 3)
                        continue;
                        
                    //WARNING("not enough vertices");
                    //ERROR(GENERAL, "not triangulated");
                    WARNING("not triangulated");
                    //untriangulated = true;
                    continue;
                }
                
                ++vert_count;
                
                vector<string> tokens;
                boost::split(tokens, face, is_any_of("/"));

                try{
                    v[0] = boost::lexical_cast<unsigned>(tokens.at(0)) - 1;
                    get<0>(vert[i]) = verts.at(v[0]);
                }catch(...){
                    LOGf("(%s) vertex at index %s",
                        Filesystem::getFileName(fn) % v[0]
                    );
                }
                try{
                    v[1] = boost::lexical_cast<unsigned>(tokens.at(1)) - 1;
                    get<1>(vert[i]) = wrap.at(v[1]);
                }catch(...){
                    //LOGf("(%s) no wrap (UV) at index %s",
                    //    Filesystem::getFileName(fn) % v[1]
                    //);
                }
                try{
                    v[2] = boost::lexical_cast<unsigned>(tokens.at(2)) - 1;
                    get<2>(vert[i]) = normals.at(v[2]);
                }catch(...){
                    //LOGf("(%s) no normal at index %s",
                    //    Filesystem::getFileName(fn) % v[2]
                    //);
                }
            }

            for(unsigned i=0;i<vert_count;++i) {
                // TODO: calc tangents?
                
                // attempt to add
                if(newset.insert(vert[i]).second)
                {
                    // new index
                    newvec.push_back(vert[i]);
                    index[i] = newvec.size()-1;
                }
                else
                {
                    // already added
                    // average the tangent
                    auto itr = std::find(ENTIRE(newvec), vert[i]);
                    size_t old_idx = std::distance(newvec.begin(), itr);
                    if(old_idx != newvec.size())
                    {
                        //tuple<vec4, unsigned>& avg =
                        //    tangent_averages[old_idx];
                        //get<0>(avg) += vec4();
                        //++get<1>(avg);

                        // calc new avg and update in newvec
                        //vec4 new_avg;
                        //if(get<0>(avg) != vec4())
                        //    new_avg = get<0>(avg)/(get<1>(avg)*1.0f);
                        //get<3>(newvec[old_idx]) = new_avg;

                        index[i] = old_idx;
                    }
                    else
                        assert(false);
                }
            }
            
            string another;
            if(ss >> another) {
                //ERROR(GENERAL, "not triangulated");
                WARNING("not triangulated");
                //untriangulated = true;
            }
            
            faces.push_back(uvec3(index[0],index[1],index[2]));
            // triangulate quad
            if(vert_count == 4)
                faces.push_back(uvec3(index[2],index[3],index[0]));
        }
        else
        {
            // ignore line
            //LOGf("\"%s\": ignoring line: \n\t%s",
            //    Filesystem::getFileName(fn) % line
            //);
        }
    }
    
    if(!newvec.empty())
    {
        verts.clear();
        verts.reserve(newvec.size());
        wrap.clear();
        wrap.reserve(newvec.size());
        normals.clear();
        normals.reserve(newvec.size());
        for(auto&& v: newvec) {
            verts.push_back(get<0>(v));
            wrap.push_back(get<1>(v));
            normals.push_back(get<2>(v));
            //tangents.push_back(get<3>(v));
        }
        geometry = make_shared<MeshIndexedGeometry>(verts, faces);
        assert(!verts.empty());
        assert(!wrap.empty());
        assert(!normals.empty());
        //LOGf("grab %s:%s from cache", mtllib%this_material);
        try{
            material = make_shared<MeshMaterial>(
                cache->cache_cast<ITexture>(mtllib + ":" + this_material)
            );
        }catch(const std::out_of_range&){
            WARNINGf("Texture unit %s:%s had problems loading.", mtllib % this_material);
        }
        mods.push_back(make_shared<Wrap>(wrap));
        mods.push_back(make_shared<MeshNormals>(normals));
        //mods.push_back(make_shared<MeshTangents>(tangents));

        //LOGf("%s polygons loaded on \"%s:%s:%s\"",
        //    faces.size() %
        //    Filesystem::getFileName(fn) % this_object % this_material
        //);
    }
    else
    {
        WARNINGf(
            "No mesh data available for \"%s:%s:%s\"",
            Filesystem::getFileName(fn) % this_object % this_material
        );
    }

    //if(untriangulated)
    //    WARNINGf("Not triangulated: %s",
    //        (Filesystem::getFileName(fn) + ":" + this_material)
    //    );
}

vector<string> Mesh :: Data :: decompose(string fn, Cache<Resource, string>* cache)
{
    //LOGf("decompose %s", fn);
    vector<string> units;
    
    auto internal = Filesystem::getInternal(fn);
    auto fn_cut = Filesystem::cutInternal(fn);
    
    if(Filesystem::hasExtension(fn_cut, "json"))
    {
        auto config = ((ResourceCache*)cache)->config(fn_cut);
        for(auto& e: *config->meta("data"))
        {
            if(boost::starts_with(e.key, internal + ":")){
                //LOGf("unit %s", e.key);
                units.push_back(e.key);
            }
        }
    }
    else
    {
        //LOGf("decomposing obj %s", fn);
        ifstream f(fn);
        string itr_object, itr_material, line;
        while(getline(f, line))
        {
            //LOG("decomposing line");
            istringstream ss(line);
            string nothing;
            ss >> nothing;
            
            if(starts_with(line, "o ")) {
                ss >> itr_object;
                //LOGf("found obj %s", itr_object);
            }
            else if(starts_with(line, "usemtl ")) {
                ss >> itr_material;
                //LOG("decomposing mat");
                auto id = itr_object + ":" + itr_material;
                //LOGf("unit %s", id);
                if(std::find(ENTIRE(units), id) == units.end())
                    units.push_back(id);
            }
        }
    }

    return units;
}

void Mesh :: Data :: calculate_tangents()
{
    //LOG(filename());
    if(geometry && not geometry->empty())
    {
        // assert wrap map exists
        //LOG("calc tangents");
        shared_ptr<Wrap> wrap;
        shared_ptr<MeshNormals> normals;
        for(auto&& mod: mods)
        {
            auto w = dynamic_pointer_cast<Wrap>(mod);
            if(w)
                wrap = w;
            auto n = dynamic_pointer_cast<MeshNormals>(mod);
            if(n)
                normals = n;
        }
        if(not wrap || not normals)
            return;
        //LOG("has uv and normals");
        
        size_t sz = geometry->size();
        //if(sz % 3 == 0){
        //    LOGf("%s not triangulated", filename());
        //    return;
        //}
        auto&& v = geometry->verts();
        auto&& uv = wrap->data();
        auto&& n = normals->data();
        vector<vec4> tangents(sz);
        vector<vec3> tan1(sz);
        vector<vec3> tan2(sz);

        for(auto&& e: tan1)
            e = vec3(0.0f);
        for(auto&& e: tan2)
            e = vec3(0.0f);
        
        if(not geometry->indexed())
        {
            for(int i=0; i<sz; i+=3)
            {
                const vec3& v1 = v[i];
                const vec3& v2 = v[i+1];
                const vec3& v3 = v[i+2];
                
                const vec2& w1 = uv[i];
                const vec2& w2 = uv[i+1];
                const vec2& w3 = uv[i+2];

                float x1 = v2.x - v1.x;
                float x2 = v3.x - v1.x;
                float y1 = v2.y - v1.y;
                float y2 = v3.y - v1.y;
                float z1 = v2.z - v1.z;
                float z2 = v3.z - v1.z;
                
                float s1 = w2.x - w1.x;
                float s2 = w3.x - w1.x;
                float t1 = w2.y - w1.y;
                float t2 = w3.y - w1.y;
                
                float r = 1.0f / (s1 * t2 - s2 * t1);
                vec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                    (t2 * z1 - t1 * z2) * r);
                vec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                    (s1 * z2 - s2 * z1) * r);
                
                tan1[i] += sdir;
                tan1[i+1] += sdir;
                tan1[i+2] += sdir;
                
                tan2[i] += tdir;
                tan2[i+1] += tdir;
                tan2[i+2] += tdir;
            }
            for(int i=0; i<sz; ++i)
            {
                // each vertex
                tangents[i] = vec4(normalize(tan1[i] - n[i] * glm::dot(n[i], tan1[i])), 1.0f);
                tangents[i].w = (glm::dot(glm::cross(n[i],tan1[i]),tan2[i]) < 0.0f) ? -1.0f : 1.0f;
                //LOG(Vector::to_string(tangents[i]));
            }
            mods.push_back(make_shared<MeshTangents>(tangents));
        }
    }
}

void Mesh :: Data :: calculate_box()
{
    if(geometry && not geometry->empty())
    {
        box = Box::Zero();
        if(geometry)
            for(auto& v: geometry->verts())
                box &= v;
    }
    //LOGf("box: %s", string(box));
}

Mesh :: Mesh(aiMesh* mesh, Cache<Resource, string>* cache, vector<shared_ptr<MeshMaterial>>& materials):
    Node((std::string)mesh->mName.data), // OSX complains w/o cast
    m_pCache(cache)
{
    vector<vec3> verts;
    vector<vec2> wrap;
    vector<vec3> normals;
    vector<vec4> tangents;
    vector<uvec3> indices;
    m_pData = make_shared<Mesh::Data>();
    
    if(mesh->mNumVertices)
    {
        verts.reserve(mesh->mNumVertices);
        
        for(unsigned int j=0; j<mesh->mNumVertices; j++)
            verts.emplace_back(
                (float)mesh->mVertices[j].x,
                (float)mesh->mVertices[j].y,
                (float)mesh->mVertices[j].z
            );
        
        if(mesh->mTangents)
        {
            tangents.reserve(mesh->mNumVertices);
            for(unsigned int j=0; j<mesh->mNumVertices; j++)
                tangents.emplace_back(
                    (float)mesh->mTangents[j].x,
                    (float)mesh->mTangents[j].y,
                    (float)mesh->mTangents[j].z,
                    0.0f
                );
            add_modifier(make_shared<MeshTangents>(tangents));
        }
        if(mesh->mNormals)
        {
            normals.reserve(mesh->mNumVertices);
            for(unsigned int j=0; j<mesh->mNumVertices; j++)
                normals.emplace_back(
                    (float)mesh->mNormals[j].x,
                    (float)mesh->mNormals[j].y,
                    (float)mesh->mNormals[j].z
                );
            add_modifier(make_shared<MeshNormals>(normals));
        }
        if(mesh->HasTextureCoords(0))
        {
            wrap.reserve(mesh->mNumVertices);
            for(unsigned int j=0; j<mesh->mNumVertices; j++)
                wrap.emplace_back(
                    (float)mesh->mTextureCoords[0][j].x,
                    1.0f - (float)mesh->mTextureCoords[0][j].y
                );
            add_modifier(make_shared<Wrap>(wrap));
        }
    }
    if(mesh->mNumFaces)
    {
        indices.reserve(mesh->mNumFaces * 3);
        for(unsigned int j=0; j<mesh->mNumFaces; j++)
        {
            indices.emplace_back(
                (int)mesh->mFaces[j].mIndices[0],
                (int)mesh->mFaces[j].mIndices[1],
                (int)mesh->mFaces[j].mIndices[2]
            );
        }
        set_geometry(make_shared<MeshIndexedGeometry>(verts, indices));
    }

    material(materials.at(mesh->mMaterialIndex));
}

Mesh :: Mesh(string fn, Cache<Resource, string>* cache):
    Node(fn),
    m_pCache(cache)
{
    string fn_real = Filesystem::cutInternal(fn);
    string ext = Filesystem::getExtension(fn_real);
    if(ext != "json")
    {
        load_assimp(fn);
    }
    else
    {
        //Cache<Resource, string>* resources = ();
        //if(Filesystem::hasExtension(fn, "json"))
        //{
        //    //if(config->at("composite", false) == true)
        //    //    return true;
        //    string other_fn = m_pConfig->at("filename", string());
        //    if(!other_fn.empty()) {
        //        fn = other_fn;
        //    } else {
        //        K_ERRORf(PARSE,
        //            "Unable to locate mesh in \"%s\"",
        //            Filesystem::hasExtension(fn)
        //        );
        //    }
        //}
        //LOGf("mesh ctor: %s", fn);
        
        vector<string> units = Mesh::Data::decompose(fn, cache);
        const size_t n_units = units.size();
        if(n_units == 0){
            K_ERRORf(GENERAL, "%s contains 0 mesh units.", fn);
        }
        fn = Filesystem::cutInternal(fn); // prevent redundant object names
        //LOGf("%s mesh units", n_units);
        
        //if(n_units == 1)
        //{
        //    m_pData = cache->cache_cast<Mesh::Data>(fn);
        //    if(m_pData->filename().empty())
        //        m_pData->filename(fn);
        //    m_pData->cache = cache;
        //}
        //else if(n_units > 1)
        //{
        m_pCompositor = this;
        for(auto&& unit: units) {
            auto m = make_shared<Mesh>(
                cache->cache_cast<Mesh::Data>(fn + ":" + unit)
            );
            m->compositor(this);
            add(m);
        }
        m_pData = make_shared<Data>();
        update();
        //}
    }
    
    auto _this = this;
    on_pend.connect([_this]{
        _this->pend_callback();
    });
    process_material_settings();
}

void Mesh :: load_assimp(std::string fn)
{
    assert(not fn.empty());
    m_pData = make_shared<Data>();
    m_pCompositor = this;
    
    auto importer = kit::make_unique<Assimp::Importer>();
    unsigned int flags = aiProcess_Triangulate|
        aiProcess_CalcTangentSpace;
        //aiProcess_GenSmoothNormals; // GenNormals

    const aiScene* aiscene = importer->ReadFile(fn, flags);
    std::string err = importer->GetErrorString();

    if(not err.empty() || 
        aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
        //aiscene->mFlags & AI_SCENE_FLAGS_VALIDATED)
    {
        K_ERROR(READ, err);
    }
    
    assert(aiscene->mNumMeshes >= 1);
    
    std::vector<shared_ptr<MeshMaterial>> materials;
    for(unsigned int i=0; i<aiscene->mNumMaterials; i++)
    {
        aiTextureType type = aiTextureType_DIFFUSE;
        unsigned int ntex = aiscene->mMaterials[i]->GetTextureCount(type);
        aiString texpath;
            aiTextureMapping mapping;
            unsigned int uvindex;
            float blend;
            aiTextureOp op;
            aiTextureMapMode mapmode[3];
            aiscene->mMaterials[i]->GetTexture(
                type,
                0,
                &texpath,
                &mapping,
                &uvindex,
                &blend,
                &op,
                mapmode
            );
            auto aimat = aiscene->mMaterials[i];
        //assert(texpath.length);
            
        if(texpath.length){
            auto mat_fn = Filesystem::getFileName(string(texpath.data));
            if(Filesystem::getExtension(mat_fn).empty())
                mat_fn += ".png";
            auto mat = std::make_shared<MeshMaterial>(
                m_pCache->cache_as<Material>(mat_fn)
            );
            aiColor3D col(0.f,0.f,0.f);
            
            aimat->Get(AI_MATKEY_COLOR_DIFFUSE, col);
            ((Material*)mat->texture())->diffuse(Color(col.r, col.g, col.b));
            
            materials.push_back(mat);
        }else{
            auto mat = make_shared<MeshMaterial>(make_shared<Material>());
            aiColor3D col(0.f,0.f,0.f);
            aimat->Get(AI_MATKEY_COLOR_DIFFUSE, col);
            ((Material*)mat->texture())->diffuse(Color(col.r, col.g, col.b));
            materials.push_back(mat);
        }
    }
    
    for(unsigned int i = 0; i < aiscene->mNumMeshes; i++)
    {
        auto m = make_shared<Mesh>(aiscene->mMeshes[i], m_pCache, materials);
        m->compositor(this);
        add(m);
    }
    update();
}

void Mesh :: process_material_settings()
{
    if(m_pData && m_pData->material)
    {
       auto cfg = m_pData->material->texture()->config();
#ifndef QOR_NO_PHYSICS
       if(cfg->at<string>("physics", "default") == "")
           disable_physics();
#endif
    }
}

void Mesh :: clear_cache() const
{
    //if(!m_pData)
    //    return;

    for(const auto& m: m_pData->mods)
        m->clear_cache();

    //if(m_pData->vertex_array)
    //{
    //    GL_TASK_START()
    //        glDeleteVertexArrays(1, &m_pData->vertex_array);
    //    GL_TASK_END()
    //    m_pData->vertex_array = 0;
    //}
}

void Mesh :: cache(Pipeline* pipeline) const
{
    //if(!m_pData)
    //    return;
    //if(!m_pData->vertex_array) {
    //    GL_TASK_START()
    //        glGenVertexArrays(1, &m_pData->vertex_array);
    //    GL_TASK_END()
    //}

    for(const auto& m: m_pData->mods)
        m->cache(pipeline);
    if(m_pData->geometry)
        m_pData->geometry->cache(pipeline);
}

void Mesh :: swap_modifier(
    unsigned int idx,
    shared_ptr<IMeshModifier> mod
){
    assert(mod);

    //if(m_pData->mods.empty()) {
    //    assert(false);
    //    return;
    //}

    if(idx == m_pData->mods.size()) // one after end
    {
        m_pData->mods.push_back(mod); // add to end
        clear_cache();
    }
    else if(idx < m_pData->mods.size()) // already exists
    {
        if(m_pData->mods.at(idx) != mod)
        {
            m_pData->mods[idx] = mod;
            clear_cache();
        }
    }
    else
    {
        K_ERRORf(FATAL, "index/size: %s/%s", idx % m_pData->mods.size());
        assert(false); // index incorrect
    }
}

void Mesh :: render_self(Pass* pass) const
{
    //assert(m_pData);
    if(empty())
        return;

    Pipeline* pipeline = pass->pipeline();
    cache(pipeline);
    
    unsigned layout = Pipeline::VERTEX;
    for(const auto& m: m_pData->mods) {
        layout |= m->layout();
    }
    
    //pass->vertex_array(m_pData->vertex_array);
    layout = pass->layout(layout);
    if(m_pData->material)
        m_pData->material->apply(pass);
    
    for(const auto& m: m_pData->mods)
        if(layout & m->layout())
            m->apply(pass);
    m_pData->geometry->apply(pass);
    
    //pass->layout(0);
}

bool Mesh :: bake_one(
    Node* n,
    map<shared_ptr<MeshMaterial>, shared_ptr<Mesh>>& meshes,
    vector<Node*>& old_nodes,
    std::function<bool(Node*)>& predicate
){
    int src_mesh_count = 0;
    Mesh* m = dynamic_cast<Mesh*>(n);
    if(not m) return false;
    //if(not m->bakeable()) return;
    if(predicate && not predicate(n))
        return false;
    auto mat = m->internals()->material;
    if(not *mat)
        return false;
    shared_ptr<Mesh> target;
    auto* src_mesh_data = m->internals().get();
    if(not src_mesh_data->geometry ||
        src_mesh_data->geometry->verts().empty()
    )
        return false;
    if(meshes.find(mat) == meshes.end()) {
        // create new target mesh
        target = make_shared<Mesh>();
        target->internals()->material = mat; // weak texture cpy
        target->internals()->geometry = make_shared<MeshGeometry>(); // #1
        target->internals()->mods.push_back(make_shared<Wrap>());
        meshes[mat] = target;
    }
    auto src_verts = src_mesh_data->geometry->ordered_verts();
    auto src_wrap = ((Wrap*)(src_mesh_data->mods.at(0).get()))->data();
    // safe if above is type MeshGeometry (see #1 above)
    auto* dest_mesh_geom = (MeshGeometry*)(
        meshes[mat]->internals()->geometry.get()
    );
    auto* dest_mesh_wrap = (Wrap*)(
        meshes[mat]->internals()->mods.at(0).get()
    );
    
    // TODO: this bakes into world space
    for(auto& v: src_verts)
        v = Matrix::mult(*n->matrix_c(Space::WORLD), v); // collapse
    dest_mesh_geom->append(std::move(src_verts));
    dest_mesh_wrap->append(std::move(src_wrap));
    // TODO: transform UVs and all that
    old_nodes.push_back(n);
    //m->clear();
    //m->visible(false);
    return true;
}

void Mesh :: bake(
    shared_ptr<Node> root,
    Pipeline* pipeline,
    std::function<bool(Node*)> predicate
){
    map<shared_ptr<MeshMaterial>, shared_ptr<Mesh>> meshes;
    vector<Node*> old_nodes;
    unsigned src_mesh_count = 0;
    root->each([&](Node* n){
        src_mesh_count += bake_one(n, meshes, old_nodes, predicate)?1:0;
    },
        ((Node::Each::DEFAULT_FLAGS
            | Node::Each::RECURSIVE)
            & ~Node::Each::INCLUDE_SELF)
    );

    for(auto& mp: meshes)
    {
        auto m = mp.second;
        m->update();
        root->add(m);
        if(pipeline) m->cache(pipeline);
    }
    for(auto& n: old_nodes){
        //n->detach();
        //((Mesh*)n)->clear();
        //((Mesh*)n)->visible(false);
    }

    //if(src_mesh_count)
    //    LOGf("Baking %s -> %s meshes", src_mesh_count % meshes.size());
}

void Mesh :: bake(
    shared_ptr<Node> root,
    vector<Node*> nodes,
    Pipeline* pipeline,
    function<bool(Node*)> predicate
){
    map<shared_ptr<MeshMaterial>, shared_ptr<Mesh>> meshes;
    vector<Node*> old_nodes;
    unsigned src_mesh_count = 0;
    for(Node* n: nodes)
        src_mesh_count += bake_one(n, meshes, old_nodes, predicate)?1:0;

    for(auto& mp: meshes)
    {
        auto m = mp.second;
        m->update();
        root->add(m);
        if(pipeline)
            m->cache(pipeline);
    }
    for(auto& n: old_nodes){
        //n->detach();
        //((Mesh*)n)->clear();
        //((Mesh*)n)->visible(false);
    }

    //if(src_mesh_count)
    //    LOGf("Baking %s -> %s meshes", src_mesh_count % meshes.size());
}

void Mesh :: update()
{
    if(m_pCompositor)
    {
        auto _this = this;
        m_Box.zero();
        //LOGf("initial zero box: %s", m_Box.string())
        Box& box = m_Box;
        each([&box, _this](Node* n){
            auto m = dynamic_cast<Mesh*>(n);
            if(m && m->compositor() == _this){
                //LOG("composite mesh update");
                //m->update();
                if(m->box()) {
                    //LOGf("before box: %s", _this->m_Box.string())
                    //LOGf("merging with box: %s", m->box().string())
                    _this->m_Box &= m->box();
                    //LOGf("after box: %s", _this->m_Box.string())
                } else {
                    //LOGf("invalid box: %s", m->box().string())
                }
            }
        });
        skip_child_box_check(true);
    }
    else
    {
        m_pData->calculate_box();
        m_Box = m_pData->box;
        //LOGf("meshdata calculated box: %s", m_Box.string())
    }
    pend();
}

#ifndef QOR_NO_PHYSICS

    void Mesh :: set_physics(Node::Physics s, bool recursive)
    {
        m_Physics = s;
        if(recursive)
            each([s](Node* n){
                auto m = std::dynamic_pointer_cast<Mesh>(n->as_node());
                if(m)
                    m->set_physics(s, true);
            });
    }

    void Mesh :: reset_translation() {
        Matrix::reset_translation(*matrix());
        pend();
        update_body();
    }
    void Mesh :: reset_orientation() {
        Matrix::reset_orientation(*matrix());
        pend();
        update_body();
    }
    void Mesh :: position(const glm::vec3& v, Space s) {
        Node::position(v,s);
        pend();
        update_body();
    }
    void Mesh :: move(const glm::vec3& v, Space s) {
        Node::move(v,s);
        pend();
        update_body();
    }
    void Mesh :: velocity(const glm::vec3& v) {
        if(m_pBody)
        {
            auto body = ((btRigidBody*)m_pBody->body());
            if(physics() == Node::KINEMATIC)
            {
                Node::velocity(v);
            }else{
                if(mass() > K_EPSILON)
                    body->setLinearVelocity(::Physics::toBulletVector(v));
            }
        }
        else
            Node::velocity(v);
    }
    glm::vec3 Mesh :: velocity() const {
        if(m_pBody) {
            if(mass() > K_EPSILON){
                return ::Physics::fromBulletVector(
                    ((btRigidBody*)m_pBody->body())->getLinearVelocity()
                );
            }
        }
        return Node::velocity();
    }

    void Mesh :: update_body()
    {
        if(m_pBody) {
            if(physics() == Node::KINEMATIC)
            {
                //teleport(position(Space::WORLD));
            }
            else if(mass() > K_EPSILON)
            {
                auto body = (btRigidBody*)m_pBody->body();
                //auto world = m_pBody->system()->world();
                //m_pBody->system()->world()->removeCollisionObject(body);
                body->setWorldTransform(::Physics::toBulletTransform(
                    *matrix_c(Space::WORLD)
                ));
                //body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
                //world->addRigidBody(body);
                //body->activate(true);
            }else{
                teleport(position(Space::WORLD));
            }
        }
    }

#endif

void Mesh :: teleport(glm::vec3 pos)
{
    #ifndef QOR_NO_PHYSICS
    if(m_pBody){
        auto body = (btRigidBody*)m_pBody->body();
        *matrix() = Matrix::with_translation(*matrix(), pos);
        //position(pos);
        pend();
        body->proceedToTransform(::Physics::toBulletTransform(*matrix_c(Space::WORLD)));
        on_move();
    }else{
        Node::position(pos);
    }
    #else
        Node::position(pos);
    #endif
}

void Mesh :: teleport(glm::mat4 mat)
{
    #ifndef QOR_NO_PHYSICS
    if(m_pBody){
        auto body = (btRigidBody*)m_pBody->body();
        //set_matrix(mat);
        *matrix() = mat;
        pend();
        body->proceedToTransform(::Physics::toBulletTransform(*matrix_c()));
        on_move();
    }else{
        *matrix() = mat;
        pend();
    }
    #else
        *matrix() = mat;
        pend();
    #endif
    
    Node::on_move();
}

void Mesh :: impulse(glm::vec3 imp)
{
    #ifndef QOR_NO_PHYSICS
    if(m_pBody)
        ((btRigidBody*)(m_pBody->body()))->applyCentralImpulse(
            ::Physics::toBulletVector(imp)
        );
    #endif
}

void Mesh :: material(std::string fn, Cache<Resource, std::string>* cache)
{
    if(not m_pCompositor || m_pCompositor != this)
        m_pData->material = std::make_shared<MeshMaterial>(
            cache->cache_cast<ITexture>(fn)
        );
    else
        each([&fn,cache](Node* n){
            auto mesh = dynamic_cast<Mesh*>(n);
            if(mesh)
                mesh->material(fn,cache);
        });

}

void Mesh :: swap_material(std::string from, std::string to, Cache<Resource, std::string>* cache)
{
    if(not m_pCompositor || m_pCompositor != this)
    {
        //LOG(from);
        //LOGf("tex: %s", m_pData->material->texture()->filename());
        if(Filesystem::getFileName(m_pData->material->texture()->filename()) == from)
            material(to, cache);
    }
    else
    {
        each([&to,&from,cache](Node* n){
            auto mesh = dynamic_cast<Mesh*>(n);
            if(mesh)
                mesh->swap_material(from,to,cache);
        });
    }
}

void Mesh :: gravity(glm::vec3 g)
{
    #ifndef QOR_NO_PHYSICS
    if(m_pBody)
        ((btRigidBody*)(m_pBody->body()))->setGravity(
            ::Physics::toBulletVector(g)
        );
    #endif
}

void Mesh :: pend_callback()
{
    #ifndef QOR_NO_PHYSICS
    if(m_pBody){
        //auto body = ((btRigidBody*)m_pBody->body());
        //if(physics() == Node::KINEMATIC)
        //    update_body();
        //    teleport(*matrix_c(Space::WORLD));
        //}
    }
    #endif
}


void Mesh :: set_matrix(glm::mat4 m)
{
    teleport(m);
}


std::shared_ptr<Mesh> Mesh :: line(glm::vec3 start, glm::vec3 end, shared_ptr<Texture> tex, float width)
{
    vec3 v = end-start;
    auto mesh = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(0.0f,-width/2.0f),vec2(1.0f,width/2.0f))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(
                glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f)
            ))
        },
        make_shared<MeshMaterial>(tex)
    );
    mesh->position(start);
    auto len = glm::length(v);
    v = normalize(v);
    mesh->rotate(atan(v.y,v.x)/K_TAU, Axis::Z);
    mesh->scale(vec3(len, 1.0f, 1.0f));
    return mesh;
}

