#ifndef _MESH_H_C8QANOEW
#define _MESH_H_C8QANOEW

#include <vector>
#include <memory>
#include "kit/math/common.h"
#include "IRenderable.h"
#include "Common.h"
#include "Node.h"
#include "ITexture.h"
#include "Pipeline.h"
#include "kit/cache/cache.h"
#include "PhysicsObject.h"
#include "Graphics.h"
#include <glm/glm.hpp>

/*
 *  Future notes:
 *    For optimization purposes, a shared VBO may be necessary instead of
 *    a VBO per object.  This could also allow for automatic interleaving for
 *    geometry that is redundant and static but needs to be held separately
 *    on the CPU side.
 */

class Instance;

/*
 * A modifier is a set of mesh attributes
 *
 * Modifiers can be placed on Meshes directly or on particular Instances
 * When placed on instances, modifiers override attributes of the same type
 */
class IMeshModifier:
    public IRealtime
{
    public:

        /*
         * Applies the modifier's attributes prior to a render of a mesh
         */
        virtual void apply(Pass* pass) const = 0;

        /*
         * Ensures that the modifier's resources are allocated and available
         */
        virtual void cache(Pipeline* pipeline) const = 0;

        /*
         * Automatically called in the destructor
         */
        virtual void clear_cache() {}

        virtual unsigned layout() const {
            return 0;
        }

        virtual ~IMeshModifier() {}
    private:

};

class IMeshGeometry:
    public IMeshModifier
{
    public:
        virtual ~IMeshGeometry() {clear_cache();}
        
        virtual std::vector<glm::vec3>& verts() = 0;
        
        virtual std::vector<glm::vec3> ordered_verts() = 0;
        virtual bool empty() const { return true; }
        virtual bool indexed() const = 0;
        virtual size_t size() const = 0;
        
        //virtual std::vector<glm::vec3>& indices() {
        //    return glm::uvec3();
        //}

        //virtual void pre_apply(Pass* pass) const = 0;

        // TODO: add bounding approximations methods
};


/*
 * Vertex set
 */
class MeshGeometry:
    public IMeshGeometry
{
    public:
        explicit MeshGeometry(std::vector<glm::vec3> verts):
            m_Vertices(verts)
        {}

        virtual ~MeshGeometry() {clear_cache();}
        MeshGeometry() = default;
        MeshGeometry(const MeshGeometry& rhs):
            m_Vertices(rhs.m_Vertices)
        {}
        MeshGeometry(MeshGeometry&& rhs):
            m_Vertices(rhs.m_Vertices),
            m_VertexBuffer(rhs.m_VertexBuffer)
        {
            rhs.m_VertexBuffer = 0;
        }

        //virtual void pre_apply(Pass* pass) const override;
        virtual void apply(Pass* pass) const override;
        virtual void cache(Pipeline* pipeline) const override;
        virtual void clear_cache() override;
        //virtual std::vector<glm::vec3>& verts() {
        //    return m_Vertices;
        //}

        virtual std::vector<glm::vec3>& verts() override {
            return m_Vertices;
        }
        virtual std::vector<glm::vec3> ordered_verts() override {
            return m_Vertices;
        }

        void append(std::vector<glm::vec3> verts);
        
        virtual bool empty() const override { return m_Vertices.empty(); }
        virtual bool indexed() const override { return false;}
        virtual size_t size() const override { return m_Vertices.size(); }
        
    private:
        mutable unsigned int m_VertexBuffer = 0;
        std::vector<glm::vec3> m_Vertices;
};

/*
 * Not yet implemented
 *
 * The same thing as MeshGeometry except it includes indices
 */
class MeshIndexedGeometry:
    public IMeshGeometry
{
    public:
        MeshIndexedGeometry() = default;
        explicit MeshIndexedGeometry(
            std::vector<glm::vec3> verts,
            std::vector<glm::uvec3> indices
        ):
            m_Vertices(verts),
            m_Indices(indices)
        {}
        virtual ~MeshIndexedGeometry() {clear_cache();}
        MeshIndexedGeometry(const MeshIndexedGeometry& rhs):
            m_Vertices(rhs.m_Vertices),
            m_Indices(rhs.m_Indices)
        {}
        MeshIndexedGeometry(MeshIndexedGeometry&& rhs):
            m_Vertices(rhs.m_Vertices),
            m_Indices(rhs.m_Indices),
            m_VertexBuffer(rhs.m_VertexBuffer),
            m_IndexBuffer(rhs.m_IndexBuffer)
        {
            rhs.m_VertexBuffer = 0;
            rhs.m_IndexBuffer = 0;
        }
        
        virtual void apply(Pass* pass) const override;
        virtual void cache(Pipeline* pipeline) const override;
        virtual void clear_cache() override;
        virtual bool indexed() const override { return true; }
        
        //virtual std::vector<glm::vec3>& verts() {
        //    return m_Vertices;
        //}
        //virtual std::vector<glm::uvec3>& indices() {
        //    return m_Indices;
        //}

        virtual std::vector<glm::vec3>& verts() override {
            return m_Vertices;
        }
        virtual std::vector<glm::vec3> ordered_verts() override;
        
        virtual bool empty() const override { return m_Indices.empty(); }
        virtual size_t size() const override { return m_Indices.size(); }

    private:
        // TODO: these are just placholders, finish this
        mutable unsigned int m_VertexBuffer = 0;
        mutable unsigned int m_IndexBuffer = 0;
        std::vector<glm::vec3> m_Vertices;
        std::vector<glm::uvec3> m_Indices;
};

class MeshMaterial
{
    public:
        explicit MeshMaterial(std::shared_ptr<ITexture> tex):
            m_pTexture(tex)
        {}
        explicit MeshMaterial(
            const std::string& fn, Cache<Resource, std::string>* res
        ):
            MeshMaterial(res->cache_cast<ITexture>(fn))
        {}
        MeshMaterial(const MeshMaterial& rhs) = default;
        MeshMaterial(MeshMaterial&& rhs) = default;

        virtual ~MeshMaterial() {}

        virtual void apply(Pass* pass) const;
        virtual void cache(Pipeline* pipeline) const {}

        virtual void logic(Freq::Time t) {
            if(m_pTexture)
                m_pTexture->logic(t);
        }

        operator bool() const {
            return m_pTexture && *m_pTexture;
        }

        ITexture* texture() { return m_pTexture.get(); }

        //virtual void clear_cache() override {}
    private:
        std::shared_ptr<ITexture> m_pTexture;
};

class MeshColorKey:
    public IMeshModifier
{
        /*
         * Color keys are specified in pairs of vec4s:
         *  each pair is a high and low color for the gradient of the key
         * Each pair represents a different color range replacement
         * It is up to the render style how exactly this changes the appearance
         */
        MeshColorKey(std::vector<glm::vec4> gradient):
            m_Gradient(gradient)
        {}
        virtual ~MeshColorKey() {clear_cache();}
        MeshColorKey(const MeshColorKey& rhs):
            m_Gradient(rhs.m_Gradient)
        {}
        MeshColorKey(MeshColorKey&& rhs):
            m_Gradient(rhs.m_Gradient)
        {
            m_VertexBuffer = 0;
        }

        // TODO: Tell pass to set uniforms for the color keys if the pass
        //       (and pipeline) allow it
        virtual void apply(Pass* pass) const override {}
        virtual void cache(Pipeline* pipeline) const override {}

        //virtual void logic(Freq::Time t) override {}
        virtual void clear_cache() override {}

    private:
        mutable unsigned int m_VertexBuffer = 0;
        std::vector<glm::vec4> m_Gradient;
};

/*
 * Wrap is an attribute set containing UV coordinates for a mesh
 */
class Wrap:
    public IMeshModifier
{
    public:
        Wrap() = default;
        explicit Wrap(const std::vector<glm::vec2>& uv):
            m_UV(uv)
        {}
        Wrap(const Wrap& rhs):
            m_UV(rhs.m_UV)
            // don't copy VBO id, since content will be changing
        {}
        virtual ~Wrap() {clear_cache();}
        Wrap(Wrap&& rhs):
            m_UV(rhs.m_UV)
        {
            m_VertexBuffer = 0;
        }

        virtual void apply(Pass* pass) const override;
        virtual void cache(Pipeline* pipeline) const override;
        virtual void clear_cache() override;

        const std::vector<glm::vec2>& data() const {
            return m_UV;
        }

        virtual unsigned layout() const override;

        void append(std::vector<glm::vec2> data);
        
    private:
        mutable unsigned int m_VertexBuffer = 0;
        //mutable bool m_bNeedsCache = false;
        std::vector<glm::vec2> m_UV;
        //mutable VertexBuffer m_Buffer;
};

/*
 * MeshColors is an attribute set containing vertex colors for a mesh
 */
class MeshColors:
    public IMeshModifier
{
    public:
        explicit MeshColors(const std::vector<glm::vec4>& colors):
            m_Colors(colors)
        {}
        virtual ~MeshColors() {clear_cache();}
        MeshColors(const MeshColors& rhs):
            m_Colors(rhs.m_Colors)
        {}
        MeshColors(MeshColors&& rhs):
            m_Colors(rhs.m_Colors)
        {
            m_VertexBuffer = 0;
        }

        virtual void apply(Pass* pass) const override;
        virtual void cache(Pipeline* pipeline) const override;
        virtual void clear_cache() override;

        const std::vector<glm::vec4>& data() const {
            return m_Colors;
        }

        virtual unsigned layout() const override;
        
    private:
        mutable unsigned int m_VertexBuffer = 0;
        std::vector<glm::vec4> m_Colors;
};


class MeshTangents:
    public IMeshModifier
{
    public:
        explicit MeshTangents(const std::vector<glm::vec4>& tangents):
            m_Tangents(tangents)
        {}
        virtual ~MeshTangents() {clear_cache();}
        MeshTangents(const MeshTangents& rhs):
            m_Tangents(rhs.m_Tangents)
        {}
        MeshTangents(MeshTangents&& rhs):
            m_Tangents(rhs.m_Tangents)
        {
            m_VertexBuffer = 0;
        }

        virtual void apply(Pass* pass) const override;
        virtual void cache(Pipeline* pipeline) const override;
        virtual void clear_cache() override;

        const std::vector<glm::vec4>& data() const {
            return m_Tangents;
        }

        virtual unsigned layout() const override;
        
    private:
        mutable unsigned int m_VertexBuffer = 0;
        std::vector<glm::vec4> m_Tangents;
};

class MeshBinormals:
    public IMeshModifier
{
    public:
        explicit MeshBinormals(const std::vector<glm::vec4>& tangents):
            m_Binormals(tangents)
        {}
        virtual ~MeshBinormals() {clear_cache();}
        MeshBinormals(const MeshBinormals& rhs):
            m_Binormals(rhs.m_Binormals)
        {}
        MeshBinormals(MeshBinormals&& rhs):
            m_Binormals(rhs.m_Binormals)
        {
            m_VertexBuffer = 0;
        }

        virtual void apply(Pass* pass) const override;
        virtual void cache(Pipeline* pipeline) const override;
        virtual void clear_cache() override;

        const std::vector<glm::vec4>& data() const {
            return m_Binormals;
        }

        virtual unsigned layout() const override;
        
    private:
        mutable unsigned int m_VertexBuffer = 0;
        std::vector<glm::vec4> m_Binormals;
};



class MeshNormals:
    public IMeshModifier
{
    public:
        explicit MeshNormals(const std::vector<glm::vec3>& normals):
            m_Normals(normals)
        {}
        MeshNormals(const MeshNormals& rhs):
            m_Normals(rhs.m_Normals)
        {}
        MeshNormals(MeshNormals&& rhs):
            m_Normals(rhs.m_Normals)
        {
            m_VertexBuffer = 0;
        }
        virtual ~MeshNormals() {clear_cache();}

        virtual void apply(Pass* pass) const override;
        virtual void cache(Pipeline* pipeline) const override;
        virtual void clear_cache() override;

        const std::vector<glm::vec3>& data() const {
            return m_Normals;
        }

        virtual unsigned layout() const override;
        
    private:
        mutable unsigned int m_VertexBuffer = 0;
        std::vector<glm::vec3> m_Normals;
};


/*
 *  A mesh that can share attributes/modifiers as between other meshes
 *  It can be used as a unique mesh, an instance, or an instance with different
 *  modifications.
 *
 *  For example, calling ->instance() on the mesh gives you a unique mesh,
 *  sharing (ref-counted) internal resources.
 *  Note: When a visible Node adds a Mesh, it needs to recalcuate its
 *      bounding box immediately.
 *      If it has grown, all parents that guarentee enclosed children
 *      also need to recalculate.
 */
class Mesh:
    public Node
{
    public:
        
        struct Data:
            public Resource
        {
            Data() = default;
            Data(std::string fn, Cache<Resource, std::string>* cache);
            Data(std::tuple<std::string, ICache*> args):
                Data(
                    std::get<0>(args),
                    (Cache<Resource, std::string>*) std::get<1>(args)
                )
            {}
            virtual ~Data() {}
            
            void load_json(
                std::string fn,
                std::string this_object,
                std::string this_material
            );
            void load_obj(
                std::string fn,
                std::string this_object,
                std::string this_material
            );

            static std::vector<std::string> decompose(
                std::string fn,
                Cache<Resource, std::string>*
            );

            Box box;
            
            std::shared_ptr<IMeshGeometry> geometry;
            std::vector<std::shared_ptr<IMeshModifier>> mods;
            std::shared_ptr<MeshMaterial> material;
            //std::string filename; // stored in Resource
            Cache<Resource, std::string>* cache = nullptr;
            unsigned int vertex_array = 0;

            void calculate_tangents();
            void calculate_box();
            bool empty() const { return not geometry || geometry->empty(); }
        };

        Mesh() {
            m_pData = std::make_shared<Data>();
            m_Box = Box::Zero();
        }
        //Mesh(const std::string& fn, Cache<Resource,std::string>* resources);
        //Mesh(const std::tuple<
        //    std::string,
        //    ICache*
        //>& args):
        //    Mesh(
        //        std::get<0>(args),
        //        (Cache<Resource, std::string>*)&std::get<1>(args)
        //    )
        //{}
        Mesh(std::string fn, Cache<Resource, std::string>* cache);
        Mesh(const std::tuple<std::string, ICache*>& args):
            Mesh(std::get<0>(args), (Cache<Resource, std::string>*)std::get<1>(args))
        {}
        Mesh(std::shared_ptr<Data> internals):
            m_pData(internals)
        {
            update();
        }
        explicit Mesh(std::vector<glm::vec3> geometry)
        {
            m_pData = std::make_shared<Data>();
            m_pData->geometry = std::make_shared<MeshGeometry>(geometry);
            update();
        }
        Mesh(
            std::shared_ptr<IMeshGeometry> geometry,
            std::vector<std::shared_ptr<IMeshModifier>> mods,
            std::shared_ptr<MeshMaterial> mat = std::shared_ptr<MeshMaterial>()
        ){
            m_pData = std::make_shared<Data>();
            m_pData->geometry = geometry;
            update();
            m_pData->mods = mods;
            m_pData->material = mat;
        }
        explicit Mesh(
            std::shared_ptr<IMeshGeometry> geometry
        ){
            m_pData = std::make_shared<Data>();
            m_pData->geometry = geometry;
            update();
        }
        void update();

        virtual ~Mesh() {clear_cache();}

        void clear_cache() const;
        void cache(Pipeline* pipeline) const;
        virtual void render_self(Pass* pass) const override;

        void clear_modifiers() {
            clear_cache();
            m_pData->mods.clear();
        }
        void clear_geometry() {
            clear_cache();
            m_pData->geometry.reset();
            m_Box = Box::Zero();
            pend();
        }
        void clear() {
            clear_cache();
            m_pData->mods.clear();
            m_pData->geometry.reset();
            m_Box = Box::Zero();
            pend();
        }

        void set_geometry(std::shared_ptr<IMeshGeometry> geometry) {
            // ref-count will clean up old geometry
            m_pData->geometry = geometry;
            update();
        }

        void set_box(Box b) {
            m_Box = b;
        }

        void add_modifier(std::shared_ptr<IMeshModifier> mod) {
            m_pData->mods.push_back(mod);
        }
        
        void swap_material(std::string from, std::string to, Cache<Resource, std::string>* cache);
        void material(std::string fn, Cache<Resource, std::string>* cache);
        
        void material(std::shared_ptr<MeshMaterial> mat) {
            m_pData->material = mat;
        }

        /*
         *  Specify a new origin for the mesh
         */
        void offset(glm::vec3 o) {
            //Matrix::reset_translation(*matrix());
            Matrix::translate(*matrix(), o);
            pend();
        }

        void reset() {
            Matrix::reset_translation(*matrix());
            pend();
        }

        void swap_modifier(
            unsigned int idx,
            std::shared_ptr<IMeshModifier> mod
        );

        // by type
        template<class T>
        void swap_modifier(
            std::shared_ptr<IMeshModifier> mod
        ) {
            for(unsigned i=0;i<m_pData->mods.size();++i)
            {
                std::shared_ptr<T> typed =
                    std::dynamic_pointer_cast<T>(m_pData->mods[i]);
                if(typed) {
                    swap_modifier(i, mod);
                    return;
                }
            }
            throw std::out_of_range("mesh modifier index");
        }

        template<class T>
        std::shared_ptr<T> fork_modifier(unsigned offset = 0)
        {
            unsigned matches=0;
            for(unsigned i=0;i<m_pData->mods.size();++i)
            {
                std::shared_ptr<T> typed =
                    std::dynamic_pointer_cast<T>(m_pData->mods[i]);
                if(typed && matches++ == offset)
                {
                    auto sp = std::make_shared<T>(*typed);
                    m_pData->mods[i] = sp;
                    return sp;
                }
            }
            throw std::out_of_range("mesh modifier index");
        }

        template<class T>
        std::shared_ptr<T> get_modifier(unsigned offset = 0)
        {
            unsigned matches=0;
            for(unsigned i=0;i<m_pData->mods.size();++i)
            {
                std::shared_ptr<T> typed =
                    std::dynamic_pointer_cast<T>(m_pData->mods[i]);
                if(typed && matches++ == offset)
                    return typed;
            }
            throw std::out_of_range("mesh modifier index");
        }

        /*
         * Because world space matrices are cached inside the node itself,
         * the mesh object should not be inserted more than once into the
         * scenegraph.  There are two solutions for multiple usage:
         *
         * 1) Instance (recommended)
         * - An instance is a separate node which shares the same internals
         *   as the mesh it was instanced from.
         *
         * 2) Prototype
         * - Use this prototype() method to spawn a new mesh which shares
         *   the attributes with this mesh.  Although attributes and modifiers
         *   are shared, if you add or change an attribute or modifier 
         *   after the prototype was created, only the specific mesh that
         *   you change will be affected.  This allows for dynamic changes
         *   in instances as you need them to eliminate overhead.
         *
         * 3) Pointer (deprecated)
         * - Don't attach the Mesh node, but instead call it's render_self()
         *   from another node to fake its inclusion.  This will not allow
         *   the Mesh to use its own transforms unless you apply them manually,
         *   but it can be used in more than one place and changes will
         *   happen to all usages of the same mesh.
         */
        std::shared_ptr<Mesh> prototype() const {
            return std::make_shared<Mesh>(
                m_pData->geometry,
                m_pData->mods,
                m_pData->material
            );
        }
        std::shared_ptr<Mesh> instance() {
            return std::make_shared<Mesh>(internals());
        }

        std::shared_ptr<IMeshGeometry> geometry() { return m_pData->geometry; }

        std::shared_ptr<Data> internals() { return m_pData; }

        void compositor(Mesh* c) {
            m_pCompositor = c;
        }
        Mesh* compositor() {
            return m_pCompositor;
        }

        // Recursively bake all meshes inside of node into single set of
        //   collapsed meshes, and attach
        static void bake(
            std::shared_ptr<Node> root,
            Pipeline* pipeline = nullptr,
            std::function<bool(Node*)> predicate = std::function<bool(Node*)>()
        );

        void bakeable(bool b) {m_bBakeable=b;}
        bool bakeable() const {return m_bBakeable;}

#ifndef QOR_NO_PHYSICS

        virtual std::shared_ptr<const PhysicsObject> body() const override {
            return m_pBody;
        }
        virtual std::shared_ptr<PhysicsObject> body() override {
            return m_pBody;
        }
        virtual void reset_body() override {
            m_pBody = std::make_shared<PhysicsObject>((Node*)this);
        }
        virtual void clear_body() override {
            m_pBody = nullptr;
        }
        
        virtual Node::Physics physics() const override {
            return m_Physics;
        }
    
        virtual void set_physics_shape(Node::PhysicsShape s) {
            m_PhysicsShape = s;
        }
        virtual Node::PhysicsShape physics_shape() const override{
            return m_PhysicsShape;
        }
        
        void set_physics(Node::Physics s, bool recursive = true);
        void disable_physics() {
            set_physics(Node::Physics::NO_PHYSICS, true);
            set_physics_shape(Node::PhysicsShape::NO_SHAPE);
        }
        virtual void reset_translation() override;
        virtual void reset_orientation() override;
        virtual glm::vec3 position(Space s = Space::PARENT) const override {
            return Node::position(s);
        }
        virtual void position(const glm::vec3& v, Space s = Space::PARENT) override;
        virtual void move(const glm::vec3& v, Space s = Space::PARENT) override;
        virtual void velocity(const glm::vec3& v) override;
        virtual glm::vec3 velocity() const override;
        void update_body();

#else
        virtual void set_physics_shape(Node::PhysicsShape s) {
        }
        virtual Node::PhysicsShape physics_shape() const override{
            return Node::NO_SHAPE;
        }
        void set_physics(Node::Physics s, bool recursive = true) {}
        void disable_physics() {}

#endif
        
        virtual float mass() const override { return m_Mass; }
        virtual void mass(float f) {
            m_Mass = f;
        }
        virtual float friction() const override { return m_Friction; }
        virtual void friction(float f) {
            m_Friction = f;
        }
        void impulse(glm::vec3 imp);
        void gravity(glm::vec3 g);
        void inertia(bool inertia) {
            m_bHasInertia = inertia;
        }
        virtual bool has_inertia() const override {
            return m_bHasInertia;
        }

        bool empty() const {
            return m_pData->empty();
        }
        
        bool composite() const {
            return m_pCompositor == this;
        }

        void teleport(glm::vec3 pos);
        void teleport(glm::mat4 mat);
        
        virtual std::string type() const override { return "mesh"; }
        
    private:

        mutable std::shared_ptr<Data> m_pData;

        // if null, mesh is single
        // if m_pCompositor == this, this mesh is a composite
        // if anything else, this mesh was loaded by another
        Mesh* m_pCompositor = nullptr;
        
#ifndef QOR_NO_PHYSICS
        Node::Physics m_Physics = Node::NO_PHYSICS;
        Node::PhysicsShape m_PhysicsShape = Node::MESH;
        std::shared_ptr<PhysicsObject> m_pBody; // null with no physics
#endif

        bool m_bHasInertia = true;
        bool m_bBakeable = false;
        float m_Mass = 0.0f;
        float m_Friction = -1.0f;
};

#endif

