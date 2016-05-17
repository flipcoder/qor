#include "Grid.h"

void Grid :: add_tile(std::shared_ptr<Node> n, glm::ivec2 loc)
{
    unsigned ofs = loc.y*m_Size.x+loc.x;
    //LOGf("ofs: %s", ofs);
    //LOGf("ts: %s", m_Tiles.size());
    //assert(m_Tiles.size() > ofs);
    if(m_Tiles.size() <= ofs)
        m_Tiles.resize(ofs + 1);
    add(n);
    m_Tiles[ofs] = n.get();
}

std::vector<const Node*> Grid :: visible_nodes(Camera* camera) const
{
    std::vector<const Node*> r;
    r.resize(m_Tiles.size());
    std::transform(ENTIRE(m_Tiles), r.begin(), [](Node* n){
        return n;
    });
    return r;
    //std::vector<const Node*> r2;
    //r2.resize(r.size());
    //std::copy_if(ENTIRE(r), r2.begin(), [](const Node* n){
    //    return false;
    //});
    //return r2;
}

