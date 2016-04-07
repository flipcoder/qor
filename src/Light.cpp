#include "Light.h"
#include "Util.h"
using namespace std;
using namespace glm;

Light :: Light(const std::shared_ptr<Meta>& meta):
    Node(meta),
    m_Flags(0)
{
    assert(meta);
    
    //m_Type = (Type)meta->at<int>("type", 1); // change to string?

    //try {
    //    m_Atten = to_vec(meta->at<shared_ptr<Meta>>("atten"));
    //}catch(const std::out_of_range&){
    //    m_Atten = glm::vec3(1.0f, 0.0f, 0.0f);
    //}

    m_Dist = (float)meta->at<double>("distance", 1.0);
    m_Cutoff = (float)meta->at<double>("cutoff", 1.0);
}

void Light :: bind(Pass* pass, unsigned slot) const
{
    int u;

    string sl = to_string(slot);

    u = pass->shader()->uniform(string("LightAmbient[")+sl+"]");
    if(u >= 0)
        pass->shader()->uniform(u, m_Ambient.vec3());
    
    u = pass->shader()->uniform(string("LightDiffuse[")+sl+"]");
    if(u >= 0)
        pass->shader()->uniform(u, m_Diffuse.vec3());

    u = pass->shader()->uniform(string("LightSpecular[")+sl+"]");
    if(u >= 0)
        pass->shader()->uniform(u, m_Specular.vec3());

    u = pass->shader()->uniform(string("LightPos[")+sl+"]");
    if(u >= 0)
    {
        auto pos = position(Space::WORLD);
        pass->shader()->uniform(u, vec4(pos.x,pos.y,pos.z, m_Type==Type::DIRECTIONAL ? 0.0f : 1.0f));
    }
    
    //u = pass->shader()->uniform("LightAtten");
    //if(u >= 0)
    //    pass->shader()->uniform(u, m_Atten);

    u = pass->shader()->uniform(string("LightDist[")+sl+"]");
    if(u >= 0)
        pass->shader()->uniform(u, m_Dist);
}

//m_Box.min = glm::vec3(-0.5f);
void Light :: dist(float f)
{
    m_Dist = f;
    m_Box.min() = glm::vec3(-f, -f, -f);
    m_Box.max() = glm::vec3(f, f, f);
}
//m_Box.max = glm::vec3(0.5f);

