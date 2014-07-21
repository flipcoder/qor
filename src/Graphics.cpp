#include "Graphics.h"
using namespace std;
using namespace glm;

std::vector<glm::vec3> Prefab :: quad(
    glm::vec2 min,
    glm::vec2 max,
    float depth
){
    return {
        glm::vec3(min.x, min.y, depth),
        glm::vec3(max.x, min.y, depth),
        glm::vec3(min.x, max.y, depth),
        glm::vec3(max.x, min.y, depth),
        glm::vec3(max.x, max.y, depth),
        glm::vec3(min.x, max.y, depth)
    };
}

std::vector<glm::vec2> Prefab :: quad_wrap(
    glm::vec2 min,
    glm::vec2 max,
    glm::vec2 scale,
    glm::vec2 offset
){
    return {
        // version without flipped Y coords
        //glm::vec2(min.x, min.y) * scale + offset,
        //glm::vec2(max.x, min.y) * scale + offset,
        //glm::vec2(min.x, max.y) * scale + offset,
        //glm::vec2(max.x, min.y) * scale + offset,
        //glm::vec2(max.x, max.y) * scale + offset,
        //glm::vec2(min.x, max.y) * scale + offset
        
        glm::vec2(min.x, max.y) * scale + offset,
        glm::vec2(max.x, max.y) * scale + offset,
        glm::vec2(min.x, min.y) * scale + offset,
        glm::vec2(max.x, max.y) * scale + offset,
        glm::vec2(max.x, min.y) * scale + offset,
        glm::vec2(min.x, min.y) * scale + offset
    };
}

std::vector<glm::vec2> Prefab :: tile_wrap(
    glm::uvec2 tile_size,
    glm::uvec2 tileset_size,
    unsigned int index,
    unsigned int flags
){
    auto num_tiles = glm::uvec2(
        (tileset_size.x / tile_size.x),
        (tileset_size.y / tile_size.y)
    );

    if(index >= num_tiles.x*num_tiles.y)
        return std::vector<glm::vec2>();

    auto unit = glm::vec2(
        1.0f / num_tiles.x,
        1.0f / num_tiles.y
    );

    float fi = unit.x * (index % num_tiles.x);
    float fj = unit.y * (index / num_tiles.x);

    if(flags & H_FLIP)
    {
        return quad_wrap(
            glm::vec2(fi + unit.x, fj + unit.y),
            glm::vec2(fi, fj)
        );
    }
    else
    {
        return quad_wrap(
            glm::vec2(fi, fj + unit.y),
            glm::vec2(fi + unit.x, fj)
        );
    }
}


Box :: Box():
    m_Min(std::numeric_limits<float>::max()),
    m_Max(std::numeric_limits<float>::min())
{
    
}

void Box :: zero()
{
    m_Min = glm::vec3(std::numeric_limits<float>::max());
    m_Max = glm::vec3(std::numeric_limits<float>::min());
}

void Box :: full()
{
    m_Min = glm::vec3(std::numeric_limits<float>::min());
    m_Max = glm::vec3(std::numeric_limits<float>::max());
}

Box :: Box(
    glm::vec3 minimum,
    glm::vec3 maximum
):
    m_Min(minimum),
    m_Max(maximum)
{
}

Box :: Box(const std::vector<glm::vec3>& points):
    m_Min(std::numeric_limits<float>::max()),
    m_Max(std::numeric_limits<float>::min())
{
    for(auto& p: points)
        *this &= p;
}

std::vector<glm::vec3> Box :: verts() const
{
    return vector<vec3> {
        glm::vec3(m_Min.x, m_Min.y, m_Min.z),
        glm::vec3(m_Min.x, m_Min.y, m_Max.z),
        glm::vec3(m_Min.x, m_Max.y, m_Min.z),
        glm::vec3(m_Min.x, m_Max.y, m_Max.z),
        glm::vec3(m_Max.x, m_Min.y, m_Min.z),
        glm::vec3(m_Max.x, m_Min.y, m_Max.z),
        glm::vec3(m_Max.x, m_Max.y, m_Min.z),
        glm::vec3(m_Max.x, m_Max.y, m_Max.z)
    };
}

