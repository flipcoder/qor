#include "Light.h"

Light :: Light(const std::shared_ptr<Meta>& meta):
    Node(meta)
{
    
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
    
}

