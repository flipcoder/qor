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
#include <boost/algorithm/string.hpp>
#include <glm/glm.hpp>
#include <boost/tokenizer.hpp>
using namespace std;
using namespace glm;
using namespace boost::algorithm;

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

void MeshColors:: cache(Pipeline* pipeline) const
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

void MeshIndexedGeometry :: apply(Pass* pass) const
{
    if(m_Vertices.empty())
        return;
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

unsigned MeshNormals :: layout() const
{
    return Pipeline::NORMAL;
}

unsigned MeshTangents :: layout() const
{
    return Pipeline::TANGENT;
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


void MeshMaterial :: apply(Pass* pass) const
{
    if(!m_pTexture)
        return;
    m_pTexture->bind(pass);
}

bool vec4cmp(const glm::vec4& a,const glm::vec4& b)
{
    if(a.x != b.x)
        return a.x < b.x;
    if(a.y != b.y)
        return a.y < b.y;
    if(a.z != b.z)
        return a.z < b.z;
    return a.w < b.w;
}

bool vec3cmp(const glm::vec3& a,const glm::vec3& b)
{
    if(a.x != b.x)
        return a.x < b.x;
    if(a.y != b.y)
        return a.y < b.y;
    return a.z < b.z;
}

bool vec2cmp(const glm::vec2& a,const glm::vec2& b)
{
    if(a.x != b.x)
        return a.x < b.x;
    return a.y < b.y;
}

// vertex compare that ignores bitangents
struct vertcmp
{
    bool operator()(const std::tuple<
        glm::vec3, // v
        glm::vec2, // w
        glm::vec3, // n
        glm::vec4 // t
    >& a,const std::tuple<
        glm::vec3, // v
        glm::vec2, // w
        glm::vec3, // n
        glm::vec4 // t
    >& b
    ){
        if(std::get<0>(a) != std::get<0>(b))
            return vec3cmp(std::get<0>(a), std::get<0>(b));
        if(std::get<1>(a) != std::get<1>(b))
            return vec2cmp(std::get<1>(a), std::get<1>(b));
        //if(std::get<2>(a) != std::get<2>(b))
        return vec3cmp(std::get<2>(a), std::get<2>(b));
        //return vec4cmp(std::get<3>(a), std::get<3>(b));
    }
};

//bool vertcmp(const std::tuple<
//    glm::vec3, // v
//    glm::vec2, // w
//    glm::vec3 // n
//>& a,const std::tuple<
//    glm::vec3, // v
//    glm::vec2, // w
//    glm::vec3 // n
//>& b
//) {
//    if(std::get<0>(a) != std::get<0>(b))
//        return vec3(std::get<0>(a) < std::get<0>(b));
//    if(std::get<1>(a) != std::get<1>(b))
//        return vec2(std::get<0>(a) < std::get<0>(b));
//    return vec3(std::get<2>(a), std::get<2>(b));
//}

Mesh::Data :: Data(
    std::string fn,
    Cache<Resource, std::string>* cache
):
    Resource(fn),
    cache(cache)
{
    size_t offset = fn.rfind(':');
    string this_object, this_material;
    if(offset != string::npos) {
        vector<string> tokens;
        string fnfn = Filesystem::getFileName(fn);
        boost::split(
            tokens,
            fnfn,
            boost::is_any_of(":")
        );
        this_object = tokens.at(1);
        this_material = tokens.at(2);
    }
    
    //if(!ends_with(to_lower_copy(fn), string(".mesh")))
    //    ERROR(READ, "invalid format");

    //auto data = make_shared<Meta<>>(fn);

    //if(data->meta("metadata")->at<int>("formatVersion") == 3)
    //    ERRORf(PARSE, "Invalid format version for %s", Filesystem::getFileName(fn));
    
    //auto materials = data->meta("materials");
    
    //if(Filesystem::hasExtension(fn, "json"))
    //{
    //    if(m_pConfig->at("composite", false)) {
    //        ERRORf(
    //            READ,
    //            "\"%s\" as composite",
    //            Filesystem::getFileName(fn)
    //        );
    //    }
    //    fn = m_pConfig->at("filename", string());
    //}
    
    fn = Filesystem::cutInternal(fn);
    ifstream f(fn);
    if(!f.good()) {
        ERROR(READ, Filesystem::getFileName(fn));
    }
    string line;
    std::vector<glm::vec3> verts;
    std::vector<glm::vec2> wrap;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> tangents;
    
    std::set<
        std::tuple<
            glm::vec3, // v
            glm::vec2, // w
            glm::vec3, // n
            glm::vec4 // t
        >,
        vertcmp
    > newset;
    std::vector<
        std::tuple<
            glm::vec3, // v
            glm::vec2, // w
            glm::vec3, // n
            glm::vec4 // t
        >
    > newvec;

    std::vector<glm::uvec3> faces;
    std::unordered_map<
        unsigned,
        std::tuple<glm::vec4 /*total*/, unsigned /*count*/>
    > tangent_averages;
    
    string mtllib;
    bool untriangulated = false;
    //std::vector<glm::vec2> wrap_index;
    //std::vector<glm::vec3> normal_index;
    
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
            float* v = glm::value_ptr(vec);
            ss >> v[0];
            ss >> v[1];
            ss >> v[2];
            verts.push_back(vec);
        }
        else if(starts_with(line, "vn "))
        {
            vec3 vec;
            float* v = glm::value_ptr(vec);
            ss >> v[0];
            ss >> v[1];
            ss >> v[2];
            normals.push_back(vec);
        }
        else if(starts_with(line, "vt "))
        {
            vec2 vec;
            float* v = glm::value_ptr(vec);
            ss >> v[0];
            ss >> v[1];
            v[1] = 1.0f - v[1];
            wrap.push_back(vec);
        }
        else if(starts_with(line, "f "))
        {
            assert(!this_object.empty());
            assert(!this_material.empty());
            
            if(this_object != itr_object)
                continue;
            if(this_material != itr_material)
                continue;
            
            //glm::uvec4 index;
            unsigned index[4] = {0};
            tuple<glm::vec3, glm::vec2, glm::vec3, glm::vec4> vert[4] = {};
            unsigned vert_count = 0;
            
            for(unsigned i=0;i<4;++i) {
                vert[i] = tuple<glm::vec3, glm::vec2, glm::vec3, glm::vec4>();
                
                unsigned v[3] = {0};
                
                string face;
                if(!(ss >> face)) {
                    if(i == 3)
                        continue;
                        
                    LOG("not enough vertices");
                    untriangulated = true;
                    continue;
                }
                
                ++vert_count;
                
                vector<string> tokens;
                boost::split(tokens, face, is_any_of("/"));

                try{
                    v[0] = boost::lexical_cast<unsigned>(tokens.at(0)) - 1;
                    std::get<0>(vert[i]) = verts.at(v[0]);
                }catch(...){
                    LOGf("(%s) vertex at index %s",
                        Filesystem::getFileName(fn) % v[0]
                    );
                }
                try{
                    v[1] = boost::lexical_cast<unsigned>(tokens.at(1)) - 1;
                    std::get<1>(vert[i]) = wrap.at(v[1]);
                }catch(...){
                    LOGf("(%s) no wrap (UV) at index %s",
                        Filesystem::getFileName(fn) % v[1]
                    );
                }
                try{
                    v[2] = boost::lexical_cast<unsigned>(tokens.at(2)) - 1;
                    std::get<2>(vert[i]) = normals.at(v[2]);
                }catch(...){
                    LOGf("(%s) no normal at index %s",
                        Filesystem::getFileName(fn) % v[2]
                    );
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
                        std::tuple<glm::vec4, unsigned>& avg =
                            tangent_averages[old_idx];
                        std::get<0>(avg) += glm::vec4();
                        ++std::get<1>(avg);

                        // calc new avg and update in newvec
                        glm::vec4 new_avg;
                        if(std::get<0>(avg) != glm::vec4())
                            new_avg = std::get<0>(avg)/(std::get<1>(avg)*1.0f);
                        std::get<3>(newvec[old_idx]) = new_avg;

                        index[i] = old_idx;
                    }
                    else
                        assert(false);
                }
            }
            
            string another;
            if(ss >> another)
                untriangulated = true;
            
            faces.push_back(glm::uvec3(index[0],index[1],index[2]));
            // triangulate quad
            if(vert_count == 4)
                faces.push_back(glm::uvec3(index[2],index[3],index[0]));
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
            verts.push_back(std::get<0>(v));
            wrap.push_back(std::get<1>(v));
            normals.push_back(std::get<2>(v));
            tangents.push_back(std::get<3>(v));
        }
        geometry = make_shared<MeshIndexedGeometry>(verts, faces);
        assert(!verts.empty());
        assert(!wrap.empty());
        assert(!normals.empty());
        material = make_shared<MeshMaterial>(
            cache->cache_as<ITexture>(mtllib + ":" + this_material)
        );
        mods.push_back(make_shared<Wrap>(wrap));
        mods.push_back(make_shared<MeshNormals>(normals));
        mods.push_back(make_shared<MeshTangents>(tangents));

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

    if(untriangulated)
        WARNINGf("Not triangulated: %s",
            (Filesystem::getFileName(fn) + ":" + this_material)
        );
}

std::vector<std::string> Mesh :: Data :: decompose(std::string fn)
{
    std::vector<std::string> units;
    if(Filesystem::hasExtension(fn, "json"))
    {
        auto config = make_shared<Meta<>>(fn);
        //if(config->at("composite", false) == true)
        //    return true;
        string other_fn = config->at("filename", string());
        if(!other_fn.empty())
            return Mesh::Data::decompose(fn);
    }

    ifstream f(fn);
    string itr_object, itr_material, line;
    while(getline(f, line))
    {
        istringstream ss(line);
        string nothing;
        ss >> nothing;
        
        if(starts_with(line, "o ")) {
            ss >> itr_object;
        }
        else if(starts_with(line, "usemtl ")) {
            ss >> itr_material;
            units.push_back(itr_object + ":" + itr_material);
        }
    }
    return units;
}

Mesh :: Mesh(std::string fn, Cache<Resource, std::string>* cache):
    Node(fn)
{
    //Cache<Resource, std::string>* resources = ();
    if(Filesystem::hasExtension(fn, "json"))
    {
        //if(config->at("composite", false) == true)
        //    return true;
        string other_fn = m_pConfig->at("filename", string());
        if(!other_fn.empty()) {
            fn = other_fn;
        } else {
            ERRORf(PARSE,
                "Unable to locate mesh in \"%s\"",
                Filesystem::hasExtension(fn)
            );
        }
    }
    
    vector<string> units = Mesh::Data::decompose(fn);
    const size_t n_units = units.size();
    //if(n_units == 1)
    //{
    //    m_pData = cache->cache_as<Mesh::Data>(fn);
    //    if(m_pData->filename().empty())
    //        m_pData->filename(fn);
    //    m_pData->cache = cache;
    //}
    //else if(n_units > 1)
    //{
    m_pCompositor = this;
    for(auto&& unit: units) {
        auto m = make_shared<Mesh>(
            cache->cache_as<Mesh::Data>(fn + ":" + unit)
        );
        m->compositor(this);
        add(m);
    }
    m_pData = std::make_shared<Data>();
    //}
}

void Mesh :: clear_cache() const
{
    if(!m_pData)
        return;

    for(const auto& m: m_pData->mods)
        m->clear_cache();

    if(m_pData->vertex_array)
    {
        GL_TASK_START()
            glDeleteVertexArrays(1, &m_pData->vertex_array);
        GL_TASK_END()
        m_pData->vertex_array = 0;
    }
}

void Mesh :: cache(Pipeline* pipeline) const
{
    if(!m_pData->vertex_array) {
        GL_TASK_START()
            glGenVertexArrays(1, &m_pData->vertex_array);
        GL_TASK_END()
    }

    for(const auto& m: m_pData->mods)
        m->cache(pipeline);
    if(m_pData->geometry)
        m_pData->geometry->cache(pipeline);
}

void Mesh :: swap_modifier(
    unsigned int idx,
    std::shared_ptr<IMeshModifier> mod
){
    assert(mod);

    if(m_pData->mods.empty()) {
        assert(false);
        return;
    }

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
        ERRORf(FATAL, "index/size: %s/%s", idx % m_pData->mods.size());
        assert(false); // index incorrect
    }
}

void Mesh :: render_self(Pass* pass) const
{
    assert(m_pData);
    if(!m_pData->geometry)
        return;

    Pipeline* pipeline = pass->pipeline();
    cache(pipeline);
    
    unsigned layout = Pipeline::VERTEX;
    for(const auto& m: m_pData->mods)
        layout |= m->layout();
    
    pass->vertex_array(m_pData->vertex_array);
    layout = pass->layout(layout);
    m_pData->material->apply(pass);
    
    for(const auto& m: m_pData->mods)
        if(layout & m->layout())
            m->apply(pass);
    m_pData->geometry->apply(pass);
}

