#ifndef _MESH_H_C8QANOEW
#define _MESH_H_C8QANOEW

#include <vector>
#include "kit/math/common.h"
#include "IRenderable.h"
#include "Common.h"
#include "Node.h"
#include "ITexture.h"
#include "IPipeline.h"
#include "kit/cache/cache.h"
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
        virtual void cache(IPipeline* pipeline) const = 0;

        /*
         * Automatically called in the destructor
         */
        virtual void clear_cache() {}

        virtual ~IMeshModifier() {clear_cache();}
    private:

};

class IMeshGeometry:
    public IMeshModifier
{
    public:
        virtual ~IMeshGeometry() {}
        //virtual std::vector<glm::vec3>& verts() {
        //    return glm::vec3();
        //}
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

        virtual ~MeshGeometry() {}

        //virtual void pre_apply(Pass* pass) const override;
        virtual void apply(Pass* pass) const override;
        virtual void cache(IPipeline* pipeline) const override;
        virtual void clear_cache() override;
        //virtual std::vector<glm::vec3>& verts() {
        //    return m_Vertices;
        //}
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
        explicit MeshIndexedGeometry(
            std::vector<glm::vec3> verts,
            std::vector<glm::uvec3> indices
        ):
            m_Vertices(verts),
            m_Indices(indices)
        {}
        virtual ~MeshIndexedGeometry() {}
        virtual void apply(Pass* pass) const override;
        virtual void cache(IPipeline* pipeline) const override;
        virtual void clear_cache() override;
        //virtual std::vector<glm::vec3>& verts() {
        //    return m_Vertices;
        //}
        //virtual std::vector<glm::uvec3>& indices() {
        //    return m_Indices;
        //}

    private:
        // TODO: these are just placholders, finish this
        mutable unsigned int m_VertexBuffer = 0;
        mutable unsigned int m_IndexBuffer = 0;
        std::vector<glm::vec3> m_Vertices;
        std::vector<glm::uvec3> m_Indices;
};

/*
 * A skin is a texture for a mesh
 */
class Skin:
    public IMeshModifier
{
    public:
        explicit Skin(std::shared_ptr<ITexture> tex):
            m_pTexture(tex)
        {}
        virtual ~Skin() {}

        virtual void apply(Pass* pass) const override;
        virtual void cache(IPipeline* pipeline) const override {}

        virtual void logic(Freq::Time t) override {
            if(m_pTexture)
                m_pTexture->logic(t);
        }

        //virtual void clear_cache() override {}
    private:
        std::shared_ptr<ITexture> m_pTexture;
};

/*
 * Vertex color set for a mesh
 */
class MeshColors:
    public IMeshModifier
{
    public:
        MeshColors(std::vector<glm::vec4> colors):
            m_Colors(colors)
        {}
        virtual ~MeshColors() {}

        virtual void apply(Pass* pass) const override {}
        virtual void cache(IPipeline* pipeline) const override {}

        //virtual void logic(Freq::Time t) override {}
        virtual void clear_cache() override {}

    private:
        mutable unsigned int m_VertexBuffer = 0;
        std::vector<glm::vec4> m_Colors;
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
        virtual ~MeshColorKey() {}

        // TODO: Tell pass to set uniforms for the color keys if the pass
        //       (and pipeline) allow it
        virtual void apply(Pass* pass) const override {}
        virtual void cache(IPipeline* pipeline) const override {}

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
        explicit Wrap(std::vector<glm::vec2> uv):
            m_UV(uv)
        {
            assert(!uv.empty());
        }
        virtual ~Wrap() {}

        virtual void apply(Pass* pass) const override;
        virtual void cache(IPipeline* pipeline) const override;
        virtual void clear_cache() override;

        const std::vector<glm::vec2>& data() const {
            return m_UV;
        }

    private:
        mutable unsigned int m_VertexBuffer = 0;
        //mutable bool m_bNeedsCache = false;
        std::vector<glm::vec2> m_UV;
        //mutable VertexBuffer m_Buffer;
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
        
        struct Internals
        {
            std::shared_ptr<IMeshGeometry> geometry;
            std::vector<std::shared_ptr<IMeshModifier>> mods;
            unsigned int vertex_array = 0;
        };

        Mesh() {
            m_pInternals = std::make_shared<Internals>();
        }
        //Mesh(const std::string& fn, Cache<IResource,std::string>* resources);
        //Mesh(const std::tuple<
        //    std::string,
        //    ICache*
        //>& args):
        //    Mesh(
        //        std::get<0>(args),
        //        (Cache<IResource, std::string>*)&std::get<1>(args)
        //    )
        //{}
        explicit Mesh(std::shared_ptr<Internals> internals):
            m_pInternals(internals)
        {}
        explicit Mesh(std::vector<glm::vec3> geometry)
        {
            m_pInternals = std::make_shared<Internals>();
            m_pInternals->geometry = std::make_shared<MeshGeometry>(geometry);
        }
        Mesh(
            std::shared_ptr<IMeshGeometry> geometry,
            std::vector<std::shared_ptr<IMeshModifier>> mods
        ){
            m_pInternals = std::make_shared<Internals>();
            m_pInternals->geometry = geometry;
            m_pInternals->mods = mods;
        }
        explicit Mesh(
            std::shared_ptr<IMeshGeometry> geometry
        ){
            m_pInternals = std::make_shared<Internals>();
            m_pInternals->geometry = geometry;
        }

        virtual ~Mesh() {}

        void clear_cache() const;
        void cache(IPipeline* pipeline) const;
        virtual void render_self(Pass* pass) const override;

        void clear_modifiers() {
            clear_cache();
            m_pInternals->mods.clear();
        }
        void clear_geometry() {
            clear_cache();
            m_pInternals->geometry.reset();
        }
        void clear() {
            clear_cache();
            m_pInternals->mods.clear();
            m_pInternals->geometry.reset();
        }

        void set_geometry(std::shared_ptr<IMeshGeometry> geometry) {
            // ref-count will clean up old geometry
            m_pInternals->geometry = geometry;
        }

        void add_modifier(std::shared_ptr<IMeshModifier> mod) {
            m_pInternals->mods.push_back(mod);
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
            for(unsigned i=0;i<m_pInternals->mods.size();++i)
            {
                std::shared_ptr<T> typed =
                    std::dynamic_pointer_cast<T>(m_pInternals->mods[i]);
                if(typed) {
                    swap_modifier(i, mod);
                    return;
                }
            }
            throw std::out_of_range("mesh modifier index");
        }

        template<class T>
        std::shared_ptr<T> get_modifier(unsigned offset = 0)
        {
            unsigned matches=0;
            for(unsigned i=0;i<m_pInternals->mods.size();++i)
            {
                std::shared_ptr<T> typed =
                    std::dynamic_pointer_cast<T>(m_pInternals->mods[i]);
                if(typed && matches++ == offset)
                    return typed;
            }
            throw std::out_of_range("mesh modifier index");
        }

        // TODO: Be able to collapse the mesh transform
        //virtual void collapse(
        //    Space s = Space::PARENT,
        //    unsigned int flags = 0
        //);

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
         *
         * TODO: Eventually it may be useful to have m_Geometry and mods
         *   stored in a separate object, but behind a shared_ptr, so we can
         *   allow modification of shared attributes.
         */
        std::shared_ptr<Mesh> prototype() const {
            return std::make_shared<Mesh>(
                m_pInternals->geometry,
                m_pInternals->mods
            );
        }
        std::shared_ptr<Mesh> instance() {
            return std::make_shared<Mesh>(internals());
        }

        std::shared_ptr<IMeshGeometry> geometry() { return m_pInternals->geometry; }

        std::shared_ptr<Internals> internals() { return m_pInternals; }

    private:

        mutable std::shared_ptr<Internals> m_pInternals;
};

#endif

