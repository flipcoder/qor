#include <vector>
#include <algorithm>
#include <memory>
#include "Grid.h"
using namespace std;
using namespace glm;

void Grid :: add_tile(std::shared_ptr<Node> n, glm::ivec2 loc)
{
    unsigned ofs = loc.y*m_Size.x+loc.x;
    //LOGf("ofs: %s", ofs);
    //LOGf("ts: %s", m_Tiles.size());
    //assert(m_Tiles.size() > ofs);
    if(m_Tiles.size() <= ofs)
        m_Tiles.resize(ofs + 1);
    //add(n);
    m_Tiles[ofs] = n;
    n->_set_parent(this); // hack: we want to fake this relationship so the
                          // transform comes out right
}

void Grid :: remove_tile(Node* tile)
{
    kit::remove(m_Tiles, tile->as_node());
}

std::vector<const Node*> Grid :: visible_nodes(Camera* camera) const
{
    std::vector<const Node*> r;
    ////r.resize(m_Tiles.size());
    //std::transform(ENTIRE(m_Tiles), back_inserter(r), [](shared_ptr<Node> n){
    //    return n.get();
    //});
    //std::vector<const Node*> r2;
    //r2.resize(r.size());
    //std::copy_if(ENTIRE(r), back_inserter(r2), [](const Node* n){
    //    return not n;
    //});
    //LOG(Vector::to_string(camera->position(Space::WORLD) * vec3(1.0f/m_TileSize.x,1.0f/m_TileSize.y, 1.0f)));
    int xs = camera->ortho_frustum().min().x / m_TileSize.x;
    int ys = camera->ortho_frustum().min().y / m_TileSize.y;
    int xe = camera->ortho_frustum().max().x / m_TileSize.x + 1;
    int ye = camera->ortho_frustum().max().y / m_TileSize.y + 1;
    //LOGf("x,y: %s,%s", xs % ys);
    //LOGf("x,y e: %s,%s", xe % ye);
    for(int j=ys; j<ye; ++j)
        for(int i=xs; i<xe; ++i){
            auto tile = ((Grid*)this)->tile(i,j).get();
            if(tile && camera->is_visible_func(tile,nullptr)){
                r.push_back(tile);
                auto desc = tile->descendants();
                std::copy(ENTIRE(desc), back_inserter(r));
            }
        }
    
    //auto desc = ((Grid*)this)->descendants();
    //std::transform(ENTIRE(desc), back_inserter(r), [](Node* n){
    //    return n;
    //});
    return r;
}

std::vector<Node*> Grid :: all_descendants()
{
    std::vector<Node*> r;
    std::transform(ENTIRE(m_Tiles), back_inserter(r), [](shared_ptr<Node> n){
        return n.get();
    });
    auto desc = descendants();
    std::copy(ENTIRE(desc), back_inserter(r));
    return r;
}

void Grid :: logic_self(Freq::Time t)
{
    Node::logic_self(t);
    //for(auto&& tile: m_Tiles){
    //    if(not tile)
    //        continue;
    //    tile->logic_self(t);
    //}
}

void Grid :: render_self(Pass* pass) const
{
    //Node::render_self(pass);
    //for(auto&& tile: m_Tiles){
    //for(m_Tiles)
    //    if(not tile)
    //        continue;
    //    if(pass->camera()->is_node_visible_func(tile))
    //        tile->render_self(pass);
    //}
}

std::shared_ptr<Node> Grid :: tile(int x, int y)
{
    try{
        return m_Tiles.at(y*m_Size.x+x);
    }catch(const std::out_of_range&){
        return nullptr;
    }
}
