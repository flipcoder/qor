#include "Mesh.h"
#include "Common.h"
#include "GLTask.h"
#include "kit/log/log.h"
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
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

void MeshGeometry :: cache(IPipeline* pipeline) const
{
    if(m_Vertices.empty())
        return;

    if(!m_VertexBuffer)
    {
        //const_cast<MeshGeometry*>(this)->clear_cache();
        // allocate, bind, and populate VBO
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

        //glEnableVertexAttribArray(pipeline->layout(IPipeline::VERTEX));

        // allocate and bind VAO
        //glGenVertexArrays(1, &m_pData->vertex_array);
        //glBindVertexArray(m_pData->vertex_array);

        // TODO: do I need to do this again?
        //glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

        //glVertexAttribPointer(pipeline->layout(IPipeline::VERTEX), 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
    }
}

void MeshIndexedGeometry :: cache(IPipeline* pipeline) const
{
    if(m_Vertices.empty())
        return;
    if(m_Indices.empty())
        return;

    if(!m_VertexBuffer)
    {
        //const_cast<MeshGeometry*>(this)->clear_cache();
        // allocate, bind, and populate VBO
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

        //glEnableVertexAttribArray(pipeline->layout(IPipeline::VERTEX));

        // allocate and bind VAO
        //glGenVertexArrays(1, &m_pData->vertex_array);
        //glBindVertexArray(m_pData->vertex_array);

        // TODO: do I need to do this again?
        //glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

        //glVertexAttribPointer(pipeline->layout(IPipeline::VERTEX), 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
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

void Wrap :: cache(IPipeline* pipeline) const
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

void MeshGeometry :: apply(Pass* pass) const
{
    if(m_Vertices.empty())
        return;

    IPipeline* pipeline = pass->pipeline();
    cache(pipeline);

    pass->vertex_buffer(m_VertexBuffer);
    
    pass->enable_layout(IPipeline::VERTEX);
    
    glVertexAttribPointer(
        pipeline->layout(IPipeline::VERTEX),
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

    IPipeline* pipeline = pass->pipeline();
    cache(pipeline);

    pass->vertex_buffer(m_VertexBuffer);
    pass->element_buffer(m_IndexBuffer);
    
    pass->enable_layout(IPipeline::VERTEX);
    
    glVertexAttribPointer(
        pipeline->layout(IPipeline::VERTEX),
        3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL
    );
    
    glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, (GLubyte*)NULL);
}


void Wrap :: apply(Pass* pass) const
{
    if(m_UV.empty())
        return;

    IPipeline* pipeline = pass->pipeline();
    cache(pipeline);

    pass->vertex_buffer(m_VertexBuffer);
    pass->enable_layout(IPipeline::WRAP);
    glVertexAttribPointer(pipeline->layout(IPipeline::WRAP), 2, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
}

void Skin :: apply(Pass* pass) const
{
    if(!m_pTexture)
        return;
    m_pTexture->bind(pass);
}

Mesh::Data :: Data(
    std::string fn,
    Cache<Resource, std::string>* cache
):
    Resource(fn),
    cache(cache)
{
    
//    if(!ends_with(to_lower_copy(fn), string(".obj")))
//        ERROR(READ, "invalid format");

    //const aiScene* scene = nullptr;
    //Assimp::Importer importer;
    
//    ifstream f(fn);
//    string line;
//    std::vector<glm::vec3> verts;
//    std::vector<glm::uvec3> indices;
//    std::vector<glm::vec2> wrap;
//    std::vector<glm::vec3> normals;

//    std::vector<glm::vec2> wrap_index;
//    std::vector<glm::vec3> normal_index;
    
//    while(getline(f, line))
//    {
//        if(starts_with(trim_copy(line), "#"))
//            continue;
//        istringstream ss(line);
//        string nothing;
//        ss >> nothing;
//        if(begins_with(line, "v "))
//        {
//            vec3 vec;
//            float* v = glm::value_ptr(v);
//            ss >> v[0];
//            ss >> v[1];
//            ss >> v[2];
//            verts.push_back(vec);
//        }
//        else if(begins_with(line, "vn "))
//        {
//        }
//        else if(begins_with(line, "vt "))
//        {
//            vec2 vec;
//            float* v = glm::value_ptr(v);
//            ss >> v[0];
//            ss >> v[1];
//            wrap_index.push_back(vec);
//        }
//        else if(begins_with(line, "f "))
//        {
//            // NOTE: face index starts at 1
//            uvec3 index;
//            uvec3 uvindex;
//            uvec3 nindex;
//            float* f = glm::value_ptr(v);
            
//            for(unsigned i=0;i<3;++i) {
//                string face;
//                ss >> face;
//                vector<string> tokens;
//                split(tokens, face, is_any_of("/"));
//                index[i] = lexical_cast<unsigned>(tokens.at(0));
//                try{
//                    uvindex[i] = lexical_cast<unsigned>(tokens.at(1));
//                }catch(...){}
//                try{
//                    nindex[i] = lexical_cast<unsigned>(tokens.at(2));
//                }catch(...){}
//            }

//            if(!wrap_index.empty())
//                wrap.push_back(wrap_index[uvindex-1]);
//            if(!normal_index.empty())
//                normals.push_back(normal_index[nindex-1]);

//            //ss >> v[0];
//            //ss >> v[1];
//            //ss >> v[2];
//            //indices.push_back(vec);
//        }

//        //if(word=="mtllib")
//    }
//    verts.shrink_to_fit();
//    indices.shrink_to_fit();
//    wrap.shrink_to_fit();
//    normals.shrink_to_fit();
//    m_pData = make_shared<Data>();
//    m_pData->geometry = make_shared<MeshIndexedGeometry>(verts, indices);
}

Mesh :: Mesh(const std::string& fn, IFactory* factory, ICache* cache):
    Node(fn)
{
    Cache<Resource, std::string>* resources = (Cache<Resource, std::string>*)cache;
    m_pData = resources->cache_as<Mesh::Data>(fn);
    if(m_pData->filename().empty())
        m_pData->filename(fn);
    m_pData->cache = resources;
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

void Mesh :: cache(IPipeline* pipeline) const
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
    if(!m_pData->geometry)
        return;

    IPipeline* pipeline = pass->pipeline();
    cache(pipeline);

    pass->vertex_array(m_pData->vertex_array);
    for(const auto& m: m_pData->mods)
        m->apply(pass);
    m_pData->geometry->apply(pass);

    //glDisableVertexAttribArray(1);
    //glDisableVertexAttribArray(0);
}

