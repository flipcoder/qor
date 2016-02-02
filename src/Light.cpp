#include "Light.h"
#include "Util.h"
using namespace std;

Light :: Light(const std::shared_ptr<Meta>& meta):
    Node(meta),
    m_Flags(0)
{
    assert(meta);
    
    m_Type = (Type)meta->at<int>("type", 0); // change to string?

    try {
        m_Atten = to_vec(meta->at<shared_ptr<Meta>>("atten"));
    }catch(const std::out_of_range&){
        m_Atten = glm::vec3(1.0f, 0.0f, 0.0f);
    }
}

//void Light :: bind(unsigned int id)
//{
//    if(visible() && m_Type)
//    {
//        // bind this light to shader uniform here (using id given)
        
//        // determine "vec" to be position or direction based on light type
//        glm::vec4 vec;
//        if(m_Type == Type::POINT || m_Type == Type::SPOT)
//        {
//            vec = glm::vec4(position(Space::WORLD), 1.0);
//        }
//        else if (light_type == Type::DIRECTIONAL)
//        {
//            // direcitonal lights can use local
//            vec = glm::vec4(position(), 1.0);
//        }
//        else {
//            assert(false);
//        }
//        //Renderer::get().bindLight(vec, m_Atten, m_Diffuse, id); // use diffuse light color only right now
//    }
//}

void Light :: bind(Pass* pass) const
{
    int u = pass->shader()->uniform("LightDiffuse");
    if(u >= 0)
        pass->shader()->uniform(u, m_Diffuse.vec3());
}

