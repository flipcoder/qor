#include <rapidxml.hpp>
#include "TileMap.h"
#include <fstream>
#include <cassert>
#include <memory>
#include "Filesystem.h"
#include <boost/lexical_cast.hpp>
#include <glm/glm.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <map>
#include <algorithm>
#include "kit/log/log.h"

using namespace std;
using namespace rapidxml;
using namespace kit;
using namespace glm;
using namespace Filesystem;

MapTile :: MapTile(
    TileBank* bank,
    TileLayer* layer,
    SetTile* settile,
    vec3 pos,
    unsigned orientation
    //MapTile::Rotate rotate
):
    m_pBank(bank),
    m_pLayer(layer),
    // get mesh + wrap by instancing the tileset tile mesh
    m_pMesh(safe_ptr(settile)->mesh()->instance())
{
    assert(layer->map());
    assert(layer->map()->tile_geometry());

    // add texture from set (only applied to this instance)
    add(m_pMesh);

    // TODO: eventually get scaling and position into (this) tile only
    //  It just doesn't work because scales are done in parent space and
    //  doesn't preserve local scale -- so we'll split for now
    position(glm::vec3(
        pos.x * settile->size().x,
        pos.y * settile->size().y,
        0.0f
    ));
    *m_pMesh->matrix() = glm::scale(glm::mat4(), glm::vec3(
        settile->size().x * 1.0f, // 1.0f
        settile->size().y * 1.0f, // 1.0f
        // TODO: apply Z offset from map to Z mesh scale
        layer->depth() ? 
            1.0f :
            1.0f
    ));
    
    if(layer->depth())
        m_pMesh->set_geometry(m_pLayer->map()->tilted_tile_geometry());

    orient(orientation);
}

void MapTile :: orient(unsigned orientation)
{
    if(orientation)
    {
        // create new wrap mod
        auto wrap =  m_pMesh->get_modifier<Wrap>()->data();

        if(orientation & (unsigned)Orientation::D)
        {
            //  flip X and Y values
            for(auto& c: wrap)
                swap(c.x, c.y);
        }

        if(orientation & (unsigned)Orientation::H)
        {
            // flip the higher and lower X values of the UV

            float min = numeric_limits<float>::max();
            float max = numeric_limits<float>::min();
            for(auto& c: wrap)
            {
                if(c.x < min)
                    min = c.x;
                if(c.x > max)
                    max = c.x;
            }

            float mid = max - min;

            for(auto& c: wrap)
            {
                if(c.x < mid)
                    c.x = max;
                else
                    c.x = min;
            }
        }

        if(orientation & (unsigned)Orientation::V)
        {
            // flip the higher and lower Y values of the UV
            float min = numeric_limits<float>::max();
            float max = numeric_limits<float>::min();
            for(auto& c: wrap)
            {
                if(c.y < min)
                    min = c.y;
                if(c.y > max)
                    max = c.y;
            }

            float mid = max - min;

            for(auto& c: wrap)
            {
                if(c.y < mid)
                    c.y = max;
                else
                    c.y = min;
            }

        }
        

        m_pMesh->swap_modifier<Wrap>(make_shared<Wrap>(wrap));
    }
}

SetTile :: SetTile(
    TileBank* bank,
    shared_ptr<Texture> texture,
    vector<glm::vec2> uv,
    std::map<string, string>&& properties,
    uvec2 size
):
    m_pBank(bank),
    m_pTexture(texture),
    m_Properties(properties),
    m_Size(size)
{
    assert(uv.size() == 6);
    // copy the mesh base (to be modified)
    //m_pMesh = make_shared<Mesh>();
    m_pMesh = make_shared<Mesh>(m_pBank->map()->tile_geometry());
    // UV offset is 0, texture is 1
    m_pMesh->add_modifier(std::make_shared<Wrap>(uv));
    m_pMesh->add_modifier(make_shared<Skin>(m_pTexture));
}


void TileBank :: add(
    size_t offset,
    string fn,
    Cache<IResource, std::string>* resources
){
    vector<char> data = Filesystem::file_to_buffer(fn);
    if(data.empty())
        throw Error(ErrorCode::READ, Filesystem::getFileName(fn));

    xml_document<> doc;
    doc.parse<parse_declaration_node | parse_no_data_nodes>(&data[0]);
    from_xml(fn, doc.first_node("tileset"), resources);
}

void TileBank :: add(
    string fn,
    xml_node<>* xml,
    Cache<IResource, std::string>* resources
){
    assert(xml);

    try{
        from_xml(fn, xml, resources);
    }catch(const null_ptr_exception& e){
        throw Error(ErrorCode::PARSE, Filesystem::getFileName(m_Name));
    }catch(const boost::bad_lexical_cast& e){
        throw Error(ErrorCode::PARSE, Filesystem::getFileName(m_Name));
    }
}

void TileBank :: from_xml(
    const string& fn, xml_node<>* xml,
    Cache<IResource, std::string>* resources
){
    xml_node<>* image_node = xml->first_node("image");
    safe_ptr(image_node);

    m_TileSize = uvec2(
        boost::lexical_cast<int>(safe_ptr(
            xml->first_attribute("tilewidth"))->value()
        ),
        boost::lexical_cast<int>(safe_ptr(
            xml->first_attribute("tileheight"))->value()
        )
    );

    auto image_size = uvec2(
        boost::lexical_cast<int>(safe_ptr(
            image_node->first_attribute("width"))->value()),
        boost::lexical_cast<int>(safe_ptr(
            image_node->first_attribute("height"))->value())
    );

    size_t offset = boost::lexical_cast<size_t>(safe_ptr(
        xml->first_attribute("firstgid"))->value()
    );

    // hold texture here temporarily
    auto texture = resources->cache_as<Texture>(
        Filesystem::getPath(fn) + safe_ptr(
        image_node)->first_attribute("source")->value()
    );

    const auto num_tiles = uvec2(
        image_size.x / m_TileSize.x,
        image_size.y / m_TileSize.y
    );

    // get any tile properties
    std::map<size_t, std::map<string, string>> tile_props;
    for(xml_node<>* tile = xml->first_node("tile");
        tile;
        tile = tile->next_sibling("tile"))
    {
        try{
            tile_props[boost::lexical_cast<size_t>(safe_ptr(
                tile->first_attribute("gid"))->value())
            ] = std::move(TileMap::get_xml_properties(fn, tile));
        }catch(const boost::bad_lexical_cast& e){
            throw Error(ErrorCode::PARSE, fn + " invalid tile ID.");
        }catch(const null_ptr_exception& e){
            throw Error(ErrorCode::PARSE, fn + " invalid tile ID.");
        }
    }

    // yes: the order of this matters (see offset counter below)
    for(size_t j = 0; j < num_tiles.y; j++)
        for(size_t i = 0; i < num_tiles.x; i++)
        {
            std::map<string,string> props;
            try{
                props = std::move(tile_props.at(offset));
            }catch(const out_of_range&){} // may not have props

            auto unit = vec2(
                1.0f / num_tiles.x,
                1.0f / num_tiles.y
            );
            float fi = unit.x * i;
            float fj = unit.y * j;

            m_IDs.push_back(offset++); // best approx for gid
            m_Tiles.emplace_back(
                this,
                //m_IDs.size()-1,
                texture,
                vector<vec2>{
                    //vec2(0.0f, 0.0f),
                    //vec2(0.0f, unit.y),
                    //vec2(unit.x, 0.0f),

                    //vec2(unit.x, unit.y),
                    //vec2(unit.x, 0.0f),
                    //vec2(0.0f, unit.y)

                    vec2(fi, fj),
                    vec2(fi, fj + unit.y),
                    vec2(fi + unit.x, fj),

                    vec2(fi + unit.x, fj + unit.y),
                    vec2(fi + unit.x, fj),
                    vec2(fi, fj + unit.y)
                },
                std::move(props),
                m_TileSize
            );
        }
}

TileLayer :: TileLayer(
    TileMap* tilemap,
    xml_node<>* node,
    std::map<string, shared_ptr<TileLayerGroup>>& groups,
    const std::string& fn,
    bool objects=false
):
    m_pMap(tilemap)
{
    assert(tilemap);

    m_Properties = TileMap::get_xml_properties(fn, node);

    auto group_prop = m_Properties.find("group");

    // default group name needed?
    string group_name = group_prop!=m_Properties.end() ? 
        group_prop->second :
        string();
    if(groups.find(group_name) != groups.end())
        groups[group_name] = std::make_shared<TileLayerGroup>(group_name);
    m_pGroup = groups[group_name];

    // get width and height in terms of tile count
    m_Size = uvec2(
        boost::lexical_cast<int>(safe_ptr(
            node->first_attribute("width"))->value()),
        boost::lexical_cast<int>(safe_ptr(
            node->first_attribute("height"))->value())
    );

    if(m_Properties.find("depth") != m_Properties.end())
        m_Depth = true;

    // The branching point for normal "layer"s and "objectgroup" layers
    if(objects)
    {
        for(xml_node<>* obj_node = node->first_node("object");
            obj_node;
            obj_node = obj_node->next_sibling("object"))
        {
            
        }
        
        return;
    }

    // Normal tile layers continue here...

    // Load layer data here (enforce CSV)
    xml_node<>* data = node->first_node("data");
    if(!data)
        throw Error(ErrorCode::PARSE, tilemap->name() + " has layer without data.");
    xml_attribute<>* encoding = data->first_attribute("encoding");
    if(!encoding || string(encoding->value()) != "csv")
        throw Error(ErrorCode::PARSE, tilemap->name() + " must use CSV encoding.");

    string raw = data->value(); // boost: why must I do this
    boost::tokenizer<boost::char_separator<char>> tokens(
        raw,
        boost::char_separator<char>(" ,\t\n\r")
    );

    //for(auto token: tokenizer)
    unsigned int count = 0;
    for(auto token = tokens.begin();
        token != tokens.end();
        ++token)
    {
        try{
            //LOG(*token);
            uint32_t id = boost::lexical_cast<uint32_t>(*token);
            // TODO: preserve the high byte here and translate it
            //   using MapTile::decode_orientation()
            //LOGf("id before: %s", id);
            // unset high nibble
            id &= ~0xF0000000;
            //LOGf("id after: %s", id);

            if(id) // if not blank area
            {
                auto m = make_shared<MapTile>(
                    tilemap->bank(),
                    this,
                    tilemap->bank()->tile(id),
                    vec3(
                        1.0f*(count % m_Size.x),
                        1.0f*(count / m_Size.y),
                        0.0f
                    )
                );
                add(m);
            }

        }catch(const boost::bad_lexical_cast e){
            throw Error(ErrorCode::PARSE, tilemap->name() + " has invalid tile ID " + *token);
        }catch(const out_of_range& e){
            throw Error(ErrorCode::PARSE, tilemap->name() + " has invalid tile ID " + *token);
        }

        ++count;
    }
}

TileMap :: TileMap(
    const string& fn,
    Cache<IResource, std::string>* resources
):
    m_Name(Filesystem::getFileName(fn)),
    m_Bank(this)
{

    vector<char> data = Filesystem::file_to_buffer(fn);
    if(data.empty())
        throw Error(ErrorCode::READ, m_Name);

    xml_document<> doc;
    doc.parse<parse_declaration_node | parse_no_data_nodes>(&data[0]);
    xml_node<>* map_node = doc.first_node("map");

    // build base tile geometry
    m_pBase = make_shared<Mesh>(std::make_shared<MeshGeometry>(
        vector<vec3>{
            vec3(0.0f, 0.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),

            vec3(1.0f, 1.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f)
        }
    ));
    // a skewed base to trick the depth buffer into sorting things properly
    m_pTiltedBase = make_shared<Mesh>(std::make_shared<MeshGeometry>(
        vector<vec3>{
            // where y = 0, z = 1
            vec3(0.0f, 0.0f, 1.0f),
            vec3(0.0f, 1.0f, 0.0f),
            vec3(1.0f, 0.0f, 1.0f),

            vec3(1.0f, 1.0f, 0.0f),
            vec3(1.0f, 0.0f, 1.0f),
            vec3(0.0f, 1.0f, 0.0f)
        }
    ));


    //m_pBase = make_shared<Mesh>(std::make_shared<MeshGeometry>(
    //    vector<vec3>{
    //        vec3(0.0f, 0.0f, 0.0f),
    //        vec3(0.0f, 16.0f, 0.0f),
    //        vec3(16.0f, 0.0f, 0.0f),

    //        vec3(16.0f, 16.0f, 0.0f),
    //        vec3(16.0f, 0.0f, 0.0f),
    //        vec3(0.0f, 16.0f, 0.0f)
    //    }
    //));


    //map<string, string> attributes;
    //for(xml_attribute<> *attr = node->first_attribute();
    //    attr;
    //    attr = attr->next_attribute())
    //{
    //    attributes[attr->name()] = attr->value();
    //}

    m_Bank.name(m_Name);

    uvec2 tile_size;
    try{
        string s;

        // make sure oritentation is orthogonal
        s = safe_ptr(map_node->first_attribute("orientation"))->value();
        if(s != "orthogonal")
            throw Error(ErrorCode::PARSE,
                m_Name + " was not marked as orthogonal.");

        m_Size = uvec2(
            boost::lexical_cast<int>(safe_ptr(
                map_node->first_attribute("width"))->value()),
            boost::lexical_cast<int>(safe_ptr(
                map_node->first_attribute("height"))->value())
        );

        // read here for passing into tilebank
        tile_size = uvec2(
            boost::lexical_cast<int>(safe_ptr(
                map_node->first_attribute("tilewidth"))->value()),
            boost::lexical_cast<int>(safe_ptr(
                map_node->first_attribute("tileheight"))->value())
        );

    }catch(const boost::bad_lexical_cast& e){
        throw Error(ErrorCode::PARSE,m_Name + " missing required attributes.");
    }catch(const null_ptr_exception& e){
        throw Error(ErrorCode::PARSE,m_Name + " missing required attributes.");
    }

    for(xml_node<>* node = map_node->first_node("tileset");
        node;
        node = node->next_sibling("tileset"))
    {
        try{
            // external tilebank
            if(node->first_attribute("source"))
            {
                m_Bank.add(
                    boost::lexical_cast<size_t>(safe_ptr(
                        node->first_attribute("firstgid"))->value()),
                    Filesystem::getPath(fn) + safe_ptr(
                        node->first_attribute("source"))->value(),
                    resources
                );
            }
            else
            {
                // tilebank is embedded in this file
                m_Bank.add(
                    fn, node, resources
                );
            }
        }catch(const boost::bad_lexical_cast& e){
            throw Error(ErrorCode::PARSE, m_Name + " tileset information.");
        }catch(const null_ptr_exception& e){
            throw Error(ErrorCode::PARSE, m_Name + " tileset information.");
        }
    }

    m_Properties = TileMap::get_xml_properties(fn, map_node);

    std::map<string, shared_ptr<TileLayerGroup>> groups;

    //bool once = false;
    std::shared_ptr<TileLayerGroup> last_group;

    // offset level above the current group's base
    unsigned int decal_count = 0;
    for(xml_node<>* node = map_node->first_node("layer");
        node;
        node = node->next_sibling("layer"))
    {
        auto m = make_shared<TileLayer>(this, node, groups, fn);
        const bool is_new_group = /*!m->group() ||*/ last_group!=m->group();
        assert(groups.size() > 0);

        last_group = m->group();
        decal_count = (is_new_group ? 0 : decal_count + 1);

        add(m);
        m->move(vec3(0.0f,0.0f,
            (groups.size()-1) * GROUP_Z_OFFSET - decal_count * DECAL_Z_OFFSET
        ));
    }

    for(xml_node<>* node= map_node->first_node("objectgroup");
        node;
        node = node->next_sibling("objectgroup"))
    {
        // TODO: create object layer (get properties, width height)
        auto m = make_shared<TileLayer>(this, node, groups, fn, true);
        const bool is_new_group = /*!m->group() ||*/ last_group!=m->group();
        assert(groups.size() > 0);

        // TODO: I just added the stuff above to this one, so if anything goes
        //   wrong... look at the below code skeptically
        last_group = m->group();
        decal_count = (is_new_group ? 0 : decal_count + 1);

        add(m);
        m->move(vec3(0.0f,0.0f,
            // groups.size() should probably read from an ordering index instead
            // of just being the full size
            // so object groups don't appear above everything?
            (groups.size()-1) * GROUP_Z_OFFSET - decal_count * DECAL_Z_OFFSET
        ));

        //TODO: do this stuff inside of each TileObjectLayer
        //for(xml_node<>* node = group->first_node("object");
        //    node;
        //    node = node->next_sibling("object"))
        //{
        //    // TODO: attributes name, type, x, y, width, height
        //}
    }
}

std::map<string,string> TileMap :: get_xml_properties(
    const string& fn,
    xml_node<>* parent
){
    std::map<string, string> r;
    if(parent)
    {
        xml_node<>* props = parent->first_node("properties");
        if(props)
            for(xml_node<>* prop = props->first_node("property");
                prop;
                prop = prop->next_sibling("property"))
            {
                try{
                    r[safe_ptr(
                        prop->first_attribute("name"))->value()] = safe_ptr(
                        prop->first_attribute("value"))->value();
                }catch(const null_ptr_exception& e){
                    throw Error(ErrorCode::PARSE,
                        fn + " " + parent->name() + " properties");
                }
            }
    }
    return r;
}

std::map<string,string> TileMap :: get_xml_attributes(
    const string& fn,
    xml_node<>* parent
){
    std::map<string, string> r;
    if(parent)
    {
        for(xml_attribute<>* attr = parent->first_attribute();
            attr;
            attr = attr->next_attribute())
        {
            r[attr->name()] = attr->value();
        }
    }
    return r;
}

TileMap :: ~TileMap()
{
}

