#include "Graphics.h"

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
    glm::vec2 max
){
    return {
        glm::vec2(min.x, min.y),
        glm::vec2(max.x, min.y),
        glm::vec2(min.x, max.y),
        glm::vec2(max.x, max.y),
        glm::vec2(min.x, max.y),
        glm::vec2(max.x, min.y)
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
        return std::vector<glm::vec2>{
            glm::vec2(fi + unit.x, fj + unit.y),
            glm::vec2(fi, fj + unit.y),
            glm::vec2(fi + unit.x, fj),
            
            glm::vec2(fi, fj + unit.y),
            glm::vec2(fi, fj),
            glm::vec2(fi + unit.x, fj)
        };
    }
    else
    {
        return std::vector<glm::vec2>{
            glm::vec2(fi, fj + unit.y),
            glm::vec2(fi + unit.x, fj + unit.y),
            glm::vec2(fi, fj),

            glm::vec2(fi + unit.x, fj + unit.y),
            glm::vec2(fi + unit.x, fj),
            glm::vec2(fi, fj)
        };
    }
}

