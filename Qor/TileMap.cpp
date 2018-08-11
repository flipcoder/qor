#include "TileMap.h"
#include "Material.h"
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

const float TileMap :: GROUP_Z_OFFSET = 1.0f;
const float TileMap :: DECAL_Z_OFFSET = K_EPSILON * 10.0f;

MapTile :: MapTile(
    TileBank* bank,
    TileLayer* layer,
    SetTile* settile,
    vec3 pos,
    unsigned orientation,
    xml_node<>* node
    //MapTile::Rotate rotate
):
    m_pBank(bank),
    m_pLayer(layer),
    // get mesh + wrap by instancing the tileset tile mesh
    m_pMesh(safe_ptr(settile)->mesh()->prototype()),
    m_pSetTile(settile)
{
    assert(bank);
    assert(layer);
    assert(settile);
    
    assert(layer->map());
    assert(layer->map()->tile_geometry());
    if(node)
    {
        auto attr = node->first_attribute("name");
        if(attr && attr->value())
            m_Filename = attr->value();
        attr = node->first_attribute("type");
        if(attr && attr->value())
            m_ObjectType = attr->value();
        m_pConfig->merge(TileMap::get_xml_properties("", node));
        
    }
    m_pConfig->merge(settile->config());
    m_pConfig->merge(layer->config());
    //LOGf("maptile serialize: %s",m_pConfig->serialize(MetaFormat::JSON));

    // extract properties from node

    // add texture from set (only applied to this instance)
    add(m_pMesh);

    position(glm::vec3(
        pos.x * settile->size().x,
        pos.y * settile->size().y,
        0.0f
    ));
    scale(glm::vec3(
        settile->size().x * 1.0f,
        settile->size().y * 1.0f,
        1.0f
    ));
    m_pMesh->pend();
    
    if(layer->depth())
        m_pMesh->set_geometry(m_pLayer->map()->tilted_tile_geometry());

    orient(orientation);
}

void MapTile :: orient(unsigned orientation)
{
    if(orientation)
    {
        // create new wrap data from old
        auto wrap =  m_pMesh->fork_modifier<Wrap>()->data();

        if(orientation & (unsigned)Orientation::D)
        {
            rotate(0.25f, glm::vec3(0.0f, 0.0f, 1.0f));
            move(glm::vec3(m_pSetTile->size().x, 0.0f, 0.0f));
        }

        if(orientation & (unsigned)Orientation::H)
        {
            //LOG("h flip");
            
            // flip the higher and lower X values of the UV
            float min = numeric_limits<float>::max();
            float max = numeric_limits<float>::min();
            //for(auto& c: wrap)
            //{
            //    LOGf("wrap: (%s, %s)", c.x % c.y);
            //}

            for(auto& c: wrap)
            {
                if(c.x < min)
                    min = c.x;
                if(c.x > max)
                    max = c.x;
            }

            float mid = (max + min) / 2.0f;

            for(auto& c: wrap)
            {
                if(c.x < mid)
                    c.x = max;
                else
                    c.x = min;
            }
            
            //for(auto& c: wrap)
            //{
            //    LOGf("flipped wrap: (%s, %s)", c.x % c.y);
            //}

        }

        if(orientation & (unsigned)Orientation::V)
        {
            //LOG("v flip");
            
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

            float mid = (max + min) / 2.0f;

            for(auto& c: wrap)
            {
                if(c.y < mid)
                    c.y = max;
                else
                    c.y = min;
            }
            //for(auto& c: wrap)
            //{
            //    LOGf("wrap: (%s, %s)", c.x % c.y);
            //}

        }
        
        m_pMesh->swap_modifier<Wrap>(make_shared<Wrap>(wrap));
    }
}

SetTile :: SetTile(
    TileBank* bank,
    shared_ptr<ITexture> texture,
    vector<glm::vec2> uv,
    vector<glm::vec3> normals,
    vector<glm::vec4> tangents,
    //std::map<string, string>&& properties,
    std::shared_ptr<Meta> config,
    uvec2 size
):
    m_pBank(bank),
    m_pTexture(texture),
    m_pConfig(make_shared<Meta>(config)),
    m_Size(size)
{
    assert(uv.size() == 6);
    // copy the mesh base (to be modified)
    //m_pMesh = make_shared<Mesh>();
    m_pMesh = make_shared<Mesh>(m_pBank->map()->tile_geometry());
    // UV offset is 0, texture is 1
    m_pMesh->add_modifier(std::make_shared<Wrap>(uv));
    if(m_pBank->map()->more_attributes()){
        m_pMesh->add_modifier(std::make_shared<MeshNormals>(normals));
        m_pMesh->add_modifier(std::make_shared<MeshTangents>(tangents));
    }
    m_pMesh->material(make_shared<MeshMaterial>(m_pTexture));
}


void TileBank :: add(
    size_t offset,
    string fn,
    ResourceCache* resources
){
    vector<char> data = Filesystem::file_to_buffer(fn);
    if(data.empty())
        K_ERROR(READ, Filesystem::getFileName(fn));

    xml_document<> doc;
    doc.parse<parse_declaration_node | parse_no_data_nodes>(&data[0]);
    from_xml(fn, doc.first_node("tileset"), resources);
}

void TileBank :: add(
    string fn,
    xml_node<>* xml,
    ResourceCache* resources
){
    assert(xml);

    try{
        from_xml(fn, xml, resources);
    }catch(const null_ptr_exception& e){
        K_ERROR(PARSE, Filesystem::getFileName(m_Name));
    }catch(const boost::bad_lexical_cast& e){
        K_ERROR(PARSE, Filesystem::getFileName(m_Name));
    }
}

void TileBank :: from_xml(
    const string& fn, xml_node<>* xml,
    ResourceCache* resources
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

    //auto image_size = uvec2(
    //    boost::lexical_cast<int>(safe_ptr(
    //        image_node->first_attribute("width"))->value()),
    //    boost::lexical_cast<int>(safe_ptr(
    //        image_node->first_attribute("height"))->value())
    //);

    size_t offset = boost::lexical_cast<size_t>(safe_ptr(
        xml->first_attribute("firstgid"))->value()
    );

    // hold texture here temporarily
    auto tex_fn = Filesystem::getPath(fn) + safe_ptr(
        image_node)->first_attribute("source")->value();
    // load json with same filename
    auto tex_json_fn = Filesystem::changeExtension(tex_fn, "json");
    //LOGf("tileset json: %s", tex_json_fn);
    TRY(m_pConfig = make_shared<Meta>(tex_json_fn));
    if(not m_pConfig) m_pConfig = make_shared<Meta>();
    
    //LOGf("tileset texture: %s", tex_fn);
    shared_ptr<ITexture> texture = resources->cache_as<Material>(tex_fn);
    
    auto image_size = texture->size();

    const auto num_tiles = uvec2(
        image_size.x / m_TileSize.x,
        image_size.y / m_TileSize.y
    );

    // get any tile properties
    std::map<size_t, std::shared_ptr<Meta>> tile_props;
    for(xml_node<>* tile = xml->first_node("tile");
        tile;
        tile = tile->next_sibling("tile"))
    {
        try{
            tile_props[boost::lexical_cast<size_t>(safe_ptr(
                tile->first_attribute("gid"))->value())
            ] = std::move(TileMap::get_xml_properties(fn, tile));
        }catch(const boost::bad_lexical_cast& e){
            WARNINGf(fn + " invalid tile ID %s.", tile->first_attribute("gid"));
        }catch(const null_ptr_exception& e){
            WARNINGf(fn + " invalid tile ID %s.", tile->first_attribute("gid"));
        }
    }

    // yes: the order of this matters (see offset counter below)
    for(size_t j = 0; j < num_tiles.y; j++)
        for(size_t i = 0; i < num_tiles.x; i++)
        {
            auto props = std::make_shared<Meta>();
            try{
                auto cfg = m_pConfig->at<std::shared_ptr<Meta>>(
                    (boost::format("%s,%s") % i % j).str()
                );
                auto cfgsz = cfg->size();
                props->merge(cfg);
            }catch(...){
                TRY(props->merge(m_pConfig->at<std::shared_ptr<Meta>>("default")));
            }
            try{
                props->merge(tile_props.at(offset));
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
                texture,
                vector<vec2>{
                    vec2(fi + K_EPSILON, fj + K_EPSILON),
                    vec2(fi + unit.x - K_EPSILON, fj + K_EPSILON),
                    vec2(fi + K_EPSILON, fj + unit.y - K_EPSILON),
                    
                    vec2(fi + unit.x - K_EPSILON, fj + K_EPSILON),
                    vec2(fi + unit.x - K_EPSILON, fj + unit.y - K_EPSILON),
                    vec2(fi + K_EPSILON, fj + unit.y - K_EPSILON)
                },
                vector<vec3>{
                    vec3(0.0f, 0.0f, 1.0f),
                    vec3(0.0f, 0.0f, 1.0f),
                    vec3(0.0f, 0.0f, 1.0f),
                    vec3(0.0f, 0.0f, 1.0f),
                    vec3(0.0f, 0.0f, 1.0f),
                    vec3(0.0f, 0.0f, 1.0f)
                },
                vector<vec4>{
                    vec4(1.0f, 0.0f, 0.0f, 1.0f),
                    vec4(1.0f, 0.0f, 0.0f, 1.0f),
                    vec4(1.0f, 0.0f, 0.0f, 1.0f),
                    vec4(1.0f, 0.0f, 0.0f, 1.0f),
                    vec4(1.0f, 0.0f, 0.0f, 1.0f),
                    vec4(1.0f, 0.0f, 0.0f, 1.0f)
                },

                props,
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
    //auto staticregion = make_shared<TileRegion>();
    //m_pStaticRegion = staticregion.get();
    //add(staticregion);

    //m_Properties = TileMap::get_xml_properties(fn, node);
    m_pConfig->merge(TileMap::get_xml_properties(fn, node));
    //for(auto&& p: props)
    //    m_pConfig->set<string>(p.first, p.second);

    //auto group_prop = props.find("group");
    string group_name = m_pConfig->at<string>("group", string("0"));

    // default group name needed?
    if(groups.find(group_name) == groups.end())
        groups[group_name] = std::make_shared<TileLayerGroup>(group_name);
    m_pGroup = groups[group_name];
    //LOGf("layer group name: %s", group_name);

    // get width and height in terms of tile count
    try{
        m_Size = uvec2(
            boost::lexical_cast<int>(safe_ptr(
                node->first_attribute("width"))->value()),
            boost::lexical_cast<int>(safe_ptr(
                node->first_attribute("height"))->value())
        );
    }catch(...){}

    Grid::size((ivec2)m_Size);
    Grid::tile_size((ivec2)m_pMap->tile_size());
    
    m_Depth = m_pConfig->has("depth");

    // The branching point for normal "layer"s and "objectgroup" layers
    if(objects)
    {
        for(xml_node<>* obj_node = node->first_node("object");
            obj_node;
            obj_node = obj_node->next_sibling("object"))
        {
            // TODO: object loading
            unsigned id = 0;
            try{
                id = boost::lexical_cast<size_t>(kit::safe_ptr(
                    obj_node->first_attribute("gid"))->value()
                );
            }catch(...){
                WARNING("bad object id");
                continue;
            }

            unsigned orientation = (id & 0xF0000000) >> 28;
            //if(orientation)
            //    LOGf("orient: %s", orientation);
            // unset high nibble
            id &= ~0xF0000000;
            //LOGf("id after: %s", id);

            //LOGf("object: %s", id);
            auto settile = tilemap->bank()->tile(id);
            
            ivec2 sz(
                (boost::lexical_cast<int>(kit::safe_ptr(
                    obj_node->first_attribute("x"))->value()
                )) / settile->size().x,
                (1.0f * boost::lexical_cast<int>(kit::safe_ptr(
                    obj_node->first_attribute("y"))->value()
                )) / settile->size().y - 1.0f
            );
            
            auto m = make_shared<MapTile>(
                tilemap->bank(),
                this,
                settile,
                vec3(sz.x,sz.y,0.0f),
                orientation,
                obj_node
            );
            //if(m->config()->has("static"))
            //    m_pStaticRegion->add(m);
            //else
            add(m);
            //add_tile(m, sz);
        }
        
        return;
    }

    // Normal tile layers continue here...

    // Load layer data here (enforce CSV)
    xml_node<>* data = node->first_node("data");
    if(!data)
        K_ERROR(PARSE, tilemap->name() + " has layer without data.");
    xml_attribute<>* encoding = data->first_attribute("encoding");
    if(!encoding || string(encoding->value()) != "csv")
        K_ERROR(PARSE, tilemap->name() + " must use CSV encoding.");

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
            unsigned orientation = (id & 0xF0000000) >> 28;
            //if(orientation)
            //    LOGf("orient: %s", orientation);
            // unset high nibble
            id &= ~0xF0000000;
            //LOGf("id after: %s", id);

            int x = count % m_Size.x;
            int y = count / m_Size.x;

            if(id) // if not blank area
            {
                auto m = make_shared<MapTile>(
                    tilemap->bank(),
                    this,
                    tilemap->bank()->tile(id),
                    vec3(1.0f*x, 1.0f*y, 0.0f),
                    orientation
                );
                add_tile(m, ivec2(x,y));
                //add(m);
            }

        }catch(const boost::bad_lexical_cast e){
            K_ERROR(PARSE, tilemap->name() + " has invalid tile ID " + *token);
        }catch(const out_of_range& e){
            K_ERROR(PARSE, tilemap->name() + " has invalid tile ID " + *token);
        }

        ++count;
    }
}

TileMap :: TileMap(
    const string& fn,
    ResourceCache* resources
):
    m_Name(Filesystem::getFileName(fn)),
    m_Bank(this)
{

    vector<char> data = Filesystem::file_to_buffer(fn);
    if(data.empty())
        K_ERROR(READ, m_Name);

    xml_document<> doc;
    doc.parse<parse_declaration_node | parse_no_data_nodes>(&data[0]);
    xml_node<>* map_node = doc.first_node("map");

    // build base tile geometry
    m_pBase = make_shared<Mesh>(std::make_shared<MeshGeometry>(
        Prefab::quad()
    ));
    // a skewed base to trick the depth buffer into sorting things properly
    auto tilted =  Prefab::quad();
    for(auto& v: tilted)
        v[2] = (1.0f - v[1]) * 0.5f * GROUP_Z_OFFSET;
    m_pTiltedBase = make_shared<Mesh>(std::make_shared<MeshGeometry>(
        tilted
    ));

    m_Bank.name(m_Name);

    uvec2 tile_size;
    try{
        string s;

        // make sure oritentation is orthogonal
        s = safe_ptr(map_node->first_attribute("orientation"))->value();
        if(s != "orthogonal")
            K_ERROR(PARSE,
                m_Name + " was not marked as orthogonal.");

        m_Size = uvec2(
            boost::lexical_cast<int>(safe_ptr(
                map_node->first_attribute("width"))->value()),
            boost::lexical_cast<int>(safe_ptr(
                map_node->first_attribute("height"))->value())
        );

        // read here for passing into tilebank
        m_TileSize = uvec2(
            boost::lexical_cast<int>(safe_ptr(
                map_node->first_attribute("tilewidth"))->value()),
            boost::lexical_cast<int>(safe_ptr(
                map_node->first_attribute("tileheight"))->value())
        );

    }catch(const boost::bad_lexical_cast& e){
        K_ERROR(PARSE,m_Name + " missing required attributes.");
    }catch(const null_ptr_exception& e){
        K_ERROR(PARSE,m_Name + " missing required attributes.");
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
            K_ERROR(PARSE, m_Name + " tileset information.");
        }catch(const null_ptr_exception& e){
            K_ERROR(PARSE, m_Name + " tileset information.");
        }
    }

    m_pConfig->merge(TileMap::get_xml_properties(fn, map_node));

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
        assert(m->group());
        const bool is_new_group = /*!m->group() ||*/ last_group!=m->group();
        assert(groups.size() > 0);

        last_group = m->group();
        //if(not last_group){
        //    K_ERROR(PARSE, "layer group empty");
        //}else{
        //    //LOGf("layer group not empty: %s", last_group);
        //}
        last_group->level(groups.size()-1);
        decal_count = (is_new_group ? 0 : decal_count + 1);
        last_group->decal_levels(decal_count);
        
        add(m);
        m_Layers.push_back(m);
        m->move(vec3(0.0f,0.0f,
            m->group()->level() * GROUP_Z_OFFSET + decal_count * DECAL_Z_OFFSET
        ));
    }

    for(xml_node<>* node= map_node->first_node("objectgroup");
        node;
        node = node->next_sibling("objectgroup"))
    {
        auto m = make_shared<TileLayer>(this, node, groups, fn, true);
        const bool is_new_group = /*!m->group() ||*/ last_group!=m->group();
        assert(groups.size() > 0);

        last_group = m->group();
        //last_group->decal_levels(last_group->decal_levels() + 1);
        decal_count = last_group->decal_levels() -  1;

        add(m);
        m_ObjectLayers.push_back(m);
        m->move(vec3(0.0f,0.0f,
            m->group()->level() * GROUP_Z_OFFSET + decal_count * DECAL_Z_OFFSET
        ));
    }
}

std::shared_ptr<Meta> TileMap :: get_xml_properties(
    const string& fn,
    xml_node<>* parent
){
    //std::map<string, string> m;
    auto meta = make_shared<Meta>();
    if(parent)
    {
        xml_node<>* props = parent->first_node("properties");
        if(props)
            for(xml_node<>* prop = props->first_node("property");
                prop;
                prop = prop->next_sibling("property"))
            {
                try{
                    meta->set<std::string>(
                        safe_ptr(prop->first_attribute("name"))->value(),
                        safe_ptr(prop->first_attribute("value"))->value()
                    );
                }catch(const null_ptr_exception& e){
                    K_ERROR(PARSE, fn + " " + parent->name() + " properties");
                }
            }
    }
    return meta;
}
std::shared_ptr<Meta> TileMap :: get_xml_attributes(
    const string& fn,
    xml_node<>* parent
){
    //std::map<string, string> r;
    auto meta = make_shared<Meta>();
    if(parent)
    {
        for(xml_attribute<>* attr = parent->first_attribute();
            attr;
            attr = attr->next_attribute())
        {
            meta->set<string>(attr->name(), attr->value());
        }
    }
    return meta;
}

TileMap :: ~TileMap()
{
}


//std::vector<const Node*> TileRegion :: visible_nodes(Camera* camera) const
//{
//    std::vector<const Node*> r;
//    std::transform(ENTIRE(children()), std::back_inserter(r),
//        [](const std::shared_ptr<Node>& n){
//           return n.get();
//        }
//    );
//    return r; // TEMP
//}

