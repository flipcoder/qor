#ifndef _TILEMAP_H_GNFXFF8Q
#define _TILEMAP_H_GNFXFF8Q

#include "Grid.h"
#include "Texture.h"
#include <rapidxml.hpp>
#include <glm/glm.hpp>
#include "Pass.h"
#include "kit/cache/cache.h"
#include "Mesh.h"
#include <stdexcept>

class TileMap;
class SetTile;
class MapTile;
class TileBank;
class TileLayer;
//class TileObjectLayer;
class TileLayerGroup;

//class ITileLayer:
//    public Node
//{
//    public:
//        virtual ~ITileLayer() {}
//        TileMap* map() { return m_pMap; }
//    private:
//        TileMap* m_pMap;
//};
/*
* The specific tile instance on a map layer
*/
class MapTile:
    public Node
{
public:

    enum class Orientation: uint32_t {
        NONE = 0,
        WTF = kit::bit(0),
        D = kit::bit(1), // swap X and Y values
        V = kit::bit(2), // flip Y values
        H = kit::bit(3) // flip X values
    };

    //enum class Rotate: uint32_t {
        //    NONE = 0,
        //    L = kit::bit(0),
        //    R = kit::bit(1)
        //};

        MapTile(
            TileBank* tilebank,
            TileLayer* layer,
            SetTile* settile,
            glm::vec3 pos,
            unsigned orient = (unsigned)Orientation::NONE,
            rapidxml::xml_node<>* node = nullptr
            //Rotate rotate= Rotate::NONE
        );
        virtual ~MapTile() {}

        std::shared_ptr<Mesh> mesh() {
            return m_pMesh;
        }
        
        std::map<std::string, std::string>& properties() {return m_Properties;}
        const std::map<std::string, std::string>& properties() const {
            return m_Properties;
        }
        
    private:
        /*
         * Transforms UV cordinates based on enum Orientation bits 'orient' 
         * called once during construction (organizational purposes only)
         */
        void orient(unsigned orientation);

        // If static, this is just a pointer to SetTile's mesh
        // If dynamic, this is an instance itself (with shared attributes)
        std::shared_ptr<Mesh> m_pMesh;
        TileBank* m_pBank;
        TileLayer* m_pLayer;
        unsigned m_Orientation;
        std::map<std::string, std::string> m_Properties;

        //void render_self(Pass* pass) const override;
};

/*
 * A tile within a tilebank
 */
class SetTile
{
    public:
        SetTile(
            TileBank* bank,
            //size_t id,
            std::shared_ptr<Texture> texture,
            std::vector<glm::vec2> uv,
            std::map<std::string, std::string>&& properties,
            glm::uvec2 size
        );

        std::shared_ptr<Mesh> mesh() { return m_pMesh; }
        std::shared_ptr<Texture> texture() { return m_pTexture; }
    
        glm::uvec2 size() { return m_Size; }

    private:

        std::shared_ptr<Mesh> m_pMesh; // instance with UV modifier
        std::shared_ptr<Texture> m_pTexture;
        glm::vec2 m_UV;
        std::map<std::string, std::string> m_Properties;
        glm::uvec2 m_Size;
        TileBank* m_pBank;

        // TODO: add geometry here
};

//class TileSet
//{
//    public:
//        TileSet(TileBank* bank, glm::vec2 size):
//            m_pBank(bank)
//        {}
//    private:
//        TileBank* m_pBank;
//        glm::uvec2 m_TileSize;
//};

/*
 * A group of reusable TileBankTiles to be used by a TileMap
 *
 * TODO: make this a composite
 */
class TileBank
{
    public:
        TileBank(TileMap* map):
            m_pMap(map)
        {}
        void name(const std::string& n) { m_Name = n; }

        void add(
            size_t offset,
            std::string fn,
            Cache<Resource, std::string>* resources
        );
        void add(
            std::string fn,
            rapidxml::xml_node<>* xml,
            Cache<Resource, std::string>* resources
        );
        virtual ~TileBank() {}

        /*
         * Grab a tile from the set using id
         *
         * May throw out_of_range
         */
        SetTile* tile(size_t id) {
            // this can be optimized, obviously
            for(size_t i=0; i<m_IDs.size(); ++i)
                if(m_IDs[i] == id)
                    return &m_Tiles.at(i);
            throw std::out_of_range("out of range");
        }

        TileMap* map() { return m_pMap; }

        //std::shared_ptr<Mesh> mesh() { return m_pMesh; }

    private:
        // filename is provided for error messages
        void from_xml(
            const std::string& fn,
            rapidxml::xml_node<>* xml,
            Cache<Resource, std::string>* resources
        );

        glm::uvec2 m_TileSize;

        // The SetTiles hold the textures
        //std::shared_ptr<Texture> m_pTexture;
        std::string m_Name;

        std::vector<SetTile> m_Tiles; // for dynamic tilebank
        std::vector<size_t> m_IDs; // m_Tiles index -> gid
        TileMap* m_pMap;
};

/*
 * A specific group
 */
class TileLayerGroup
{
    public:
        TileLayerGroup(const std::string& name):
            m_Name(name)
        {}
        //virtual ~TileLayerGroup() {}

        int level() const {return m_Level;}
        void level(int v){m_Level=v;}

        int decal_levels() const {return m_DecalLevels;}
        void decal_levels(int v){m_DecalLevels=v;}
        
        std::string name() const {return m_Name;}
        void name(std::string n){m_Name=n;}
        
    private:
        std::string m_Name;

        int m_Level = 0;
        int m_DecalLevels = 0;
        
        // TODO: add order ID here (relative to map)

        // mesh base for single tile (used in dynamic layers only)
        //std::shared_ptr<Mesh> m_pMesh;
};

/*
 * A single layer of a TileMap
 *     Node-parent is guarenteed to be a TileMap
 *     Node-children of this are Tiles
 */
class TileLayer:
    public Grid
    //public ITileLayer
{
    public:
        //TileLayer(TileMap* parent):
        //    m_pTileMap(parent)
        //{}
        //TileLayer(TileMap* parent,std::shared_ptr<TileLayerGroup> group):
        //    m_pTileMap(parent),
        //    m_pGroup(group)
        //{}
        TileLayer(
            TileMap* tilemap,
            rapidxml::xml_node<>* node,
            std::map<std::string, std::shared_ptr<TileLayerGroup>>& groups,
            const std::string& fn,
            bool objects
        );

        virtual ~TileLayer() {}

        //std::shared_ptr<MapTile> spawn();

        // MapTiles are nodes themselves, so no need to do custom render
        //void render_self(Pass* pass) const override;
        bool depth() const { return m_Depth; }

        std::shared_ptr<const TileLayerGroup> group() const {
            return m_pGroup;
        }
        std::shared_ptr<TileLayerGroup> group() { return m_pGroup; }

        /* Used inside the tmx file format only */
        enum class OrientationFlag: uint32_t {
            H = 0x80000000,
            V = 0x40000000,
            D = 0x20000000,
            MASK = 0xF0000000
        };
        TileMap* map() { return m_pMap; }

    private:
        // TODO: optimized combined batch
        // In the case of a static layer, we should have a combined batch here
        // of the entire map layer

        TileMap* m_pMap; // pre-casted version of parent
        std::shared_ptr<TileLayerGroup> m_pGroup;
        std::map<std::string, std::string> m_Properties;

        /*
         * Width and height in terms of tiles
         */
        glm::uvec2 m_Size;

        bool m_Depth = false;
        int m_Level = 0;

        // Note: Tiles are stored as Node children
};

//class TileObjectLayer:
//    public ITileLayer
//{
//    public:
//        TileObjectLayer(
//            TileMap* tilemap,
//            rapidxml::xml_node<>* node,
//            std::map<std::string, std::shared_ptr<TileLayerGroup>>& groups,
//            const std::string& fn
//        ){}
//        virtual ~TileObjectLayer();
//};

/*
 * Let's use the base Zone instead
 */
//class TileMapZone:
//    public Zone
//{
//    public:
//};

/*
 * We may not need a ZoneLayer, we can just use a normal Node
 */
//class ZoneLayer:
//    public Node
//{
//    public:
//        ZoneLayer()
//        {
//        }
//        virtual ZoneLayer()

//    private:
//
//};

/*
 * Load 2D tilemap created with Tiled-qt
 *     Node-children of this are TileLayers
 */
class TileMap:
    public Node
{
    public:

        TileMap(
            const std::string& fn,
            Cache<Resource, std::string>* resources
        );
        //explicit TileMap(const boost::any& args):
        //    Grid(args) {}
        virtual ~TileMap();

        // The layers in a tilemap are nodes, so no custom rendering needed
        //void render_self(Pass* pass) const override;

        static std::map<std::string,std::string> get_xml_properties(
            const std::string& fn,
            rapidxml::xml_node<>* parent
        );
        static std::map<std::string,std::string> get_xml_attributes(
            const std::string& fn,
            rapidxml::xml_node<>* parent
        );
        
        //static size_t count_attributes(
        //    rapidxml::xml_node<>* node,
        //    std::string attr
        //){
        //    size_t count = 0;
        //    for(auto n = node->first_attribute(attr);
        //        n;
        //        attr = node->next_attribute(attr))
        //    {
        //        ++count;
        //    }
        //    return count;
        //}

        //std::shared_ptr<TileLayer> below(
        //    std::shared_ptr<TileLayer> layer
        //);
        //std::shared_ptr<TileLayer> above(
        //    std::shared_ptr<TileLayer> layer
        //);

        std::string name() const { return m_Name; }
        TileBank* bank() { return &m_Bank; }

        std::shared_ptr<IMeshGeometry> tile_geometry() {
            assert(m_pBase);
            return m_pBase->geometry();
        }
        std::shared_ptr<IMeshGeometry> tilted_tile_geometry() {
            assert(m_pTiltedBase);
            return m_pTiltedBase->geometry();
        }

        static const float GROUP_Z_OFFSET;
        static const float DECAL_Z_OFFSET;

        // TODO: need way to get base layer of a group with a specific ID
        //  This doesn't need to be a std::map yet, but might need to be later
        //  Leave room if there is a negative index (basements?)
        
        class GroupRange
        {
            public:
                int begin() { return std::get<0>(m_Range); }
                int end() { return std::get<1>(m_Range); }
            private:
                std::tuple<int, int> m_Range;
        };
        GroupRange groups() {
            return GroupRange();
        }

        
        std::vector<std::shared_ptr<TileLayer>>& layers() {
            return m_Layers;
        };
        std::vector<std::shared_ptr<TileLayer>>& object_layers() {
            return m_ObjectLayers;
        };

        
    private:

        std::string m_Name;
        glm::uvec2 m_Size;
        std::map<std::string, std::string> m_Properties;
        TileBank m_Bank;
        std::vector<std::shared_ptr<TileLayer>> m_Layers;
        std::vector<std::shared_ptr<TileLayer>> m_ObjectLayers;

        std::shared_ptr<Mesh> m_pBase;
        std::shared_ptr<Mesh> m_pTiltedBase;

        // We don't need to store these, just create them and have the
        // layers that are in the group hold the shader_ptr
        //std::vector<std::shared_ptr<TileLayerGroup>> m_Groups;

        //void read_xml_node(rapidxml::xml_node<>* node);
};

#endif

