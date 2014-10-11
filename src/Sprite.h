#ifndef _SPRITE_H_NK3Y9RO
#define _SPRITE_H_NK3Y9RO

#include <set>
#include <vector>
#include <unordered_map>
#include "Common.h"
#include "Texture.h"
#include "kit/kit.h"
#include "Mesh.h"
#include "kit/freq/freq.h"
#include "kit/math/matrixops.h"
#include "kit/cache/cache.h"
#include <jsoncpp/json/json.h>
#include <boost/optional.hpp>

/*
 *  A sprite is a flat object that can have frame-based animation,
 *  dynamic physics, and scripted events.
 *
 *  It can be anything from a character to a particle.
 *
 *  Terminology:
 *    State - example: walking
 *    Category - example: direction
 *    Frame - A single texture image in an animation cycle
 *    Cycle - A combination of states of orthogonal states (unique category)
 *      that are part of an animation loop and associated with a set of states
 *    Tag - a user data ID that can be added to an object and used to filter
 *      objects of a specific owner or specific type
 */
class Sprite:
    public Node
{
    public:

        /*
         * Frame hints
         */
        struct Hints
        {
            bool hflip = false;
            bool vflip = false;
            float speed = 1.0f;
        };

        /*
         * A single frame (contains wrap, state, and hints)
         */
        struct Frame
        {
            Frame(
                unsigned int _state,
                Hints _hints
            ):
                state(_state),
                hints(_hints)
            {}

            unsigned int state = 0;
            Hints hints;

            std::shared_ptr<Wrap> wrap;
        };
        
        /*
         * A series of frames that can be played in a single animation cycle.
         * Single frame cycles are also poible.
         */
        struct Cycle
        {
            std::vector<Frame> frames;
        };
        
        /*
         * The current playback information (which cycle, frame, etc.)
         */
        struct Viewer
        {
            Freq::Timeline timeline;
            boost::optional<Freq::Alarm> alarm;
            Cycle* cycle = nullptr;
            unsigned int frame = 0; // frame index into cycle.frames[]
        };

        /*
         * Loads from a json file
         */
        Sprite(
            const std::string& fn,
            Cache<Resource, std::string>* resources,
            const std::string& skin = std::string(),
            glm::vec3 pos = glm::vec3(0.0f)
        );
        //Sprite(const std::string& fn, IFactory* factory, ICache* cache):
        //{}
        Sprite(const std::tuple<std::string, IFactory*, ICache*>& args):
            Sprite(
                std::get<0>(args),
                // skip std::get<1>() (don't need factory)
                (Cache<Resource, std::string>*) std::get<2>(args)
            )
        {}
        virtual ~Sprite() {}

        virtual void logic_self(Freq::Time t) override;

        unsigned int state_id(const std::string& name) {
            for(size_t i=0; i<m_Names.size(); ++i)
                if(m_Names[i] == name)
                    return i;
            throw std::out_of_range("invalid state");
        }
            
        void set_all_states(std::vector<unsigned int> state_ids) {
            sort(state_ids.begin(), state_ids.end());
            m_States = std::move(state_ids);
            ensure_cycle();
        }

        //void set_cycle(unsigned int idx) {
        //    set_all_states();
        //}

        bool set_state(unsigned int s) {
            if(!kit::has(m_States, s)) {
                remove_conflicting_states(s);
                m_States.push_back(s);
                sort(m_States.begin(), m_States.end());
                ensure_cycle();
                return true;
            }
            return false;
        }

        bool check_state(const std::string& name) const {
            assert(false); // not impl
            return false;
        }
        bool check_states(std::vector<unsigned int> state_ids) const {
            // assert unique?
            sort(state_ids.begin(), state_ids.end());
            size_t matches = 0;
            for(auto& S: m_States)
                for(auto s: state_ids) 
                    if(s == S)
                        ++matches;
            return matches == state_ids.size();
        }
        bool check_state(unsigned int state_id) const {
            return kit::has(m_States, state_id);
        }
        bool remove_conflicting_states(unsigned int state_id) {
            bool removed = false;
            for(auto itr = m_States.begin();
                itr != m_States.end();)
            {
                if(are_states_conflicting(state_id, *itr)){
                    itr = m_States.erase(itr);
                    removed = true;
                }
                else
                    ++itr;
            }
            return removed;
        }

        /*
         * Orthogonal states mean they are in different categories.
         *
         * If categories are disabled, this check is a != b
         */
        bool are_states_conflicting(unsigned int a, unsigned int b) {
            if(m_bUseCategories)
                return m_StateCategory.at(a) == m_StateCategory.at(b);
            return a==b;
        }

        const std::shared_ptr<Mesh>& mesh() { return m_pMesh; }
        const std::shared_ptr<Node>& mask() { return m_pMask; }

        void reskin(const std::string& skin);

        const std::string& skin() const { return m_sMeshMaterial; }

        void resume() {
            m_PlaySpeed = 1.0f;
        }
        void pause() {
            m_PlaySpeed = 0.0f;
        }
        void speed(float s) {
            m_PlaySpeed = s;
        }
        float speed() const {
            return m_PlaySpeed;
        }

    private:
        void load_as_json(
            const std::string& fn,
            Cache<Resource, std::string>* resources
        );
        void load_as_image(
            const std::string& fn,
            Cache<Resource, std::string>* resources
        );
        void load_mesh();

        void ensure_cycle() {
            if(m_Viewer.cycle != &m_Cycles.at(m_States))
                reset_cycle();
        }
        void reset_cycle(unsigned int frame = 0) {
            m_Viewer.cycle = &m_Cycles.at(m_States);
            m_Viewer.frame = frame; // first index
            if(!m_Viewer.alarm)
                m_Viewer.alarm = Freq::Alarm(&m_Viewer.timeline);
            m_Viewer.alarm->set(
                Freq::Time::seconds(1.0f /
                    (m_AnimationSpeed * m_Viewer.cycle->frames.at(
                        m_Viewer.frame
                    ).hints.speed)
                )
            );
            m_pMesh->swap_modifier(
                0,
                m_Viewer.cycle->frames.at(m_Viewer.frame).wrap
            );
        }

        /*
         * Load type information (inherit) from a base
         */
        void load_type(const std::string& fn, const Json::Value& type);

        /*
         * Load animation info from Json node
         */
        void load_animation(
            const std::string& fn,
            const Json::Value& animation
        );

        /*
         * Loads frames contained inside config's frames node
         * Recursive and does not include self
         */
        void load_frames(
            const std::string& fn,
            std::vector<unsigned int>& states,
            const Json::Value& frames
        );

        void load_cycles();

        void center_mesh() {
            Matrix::reset_translation(*m_pMesh->matrix());
            m_pMesh->offset(glm::vec3(
                -0.5f * m_Size.x,
                -0.5f * m_Size.y,
                0.0f
            ));
        }

        // ID => state name
        std::vector<std::string> m_Names;

        // state categories, string is optional
        std::unordered_map<unsigned int, std::string> m_CategoryNames;

        // ID combination (array of uints) => Cycle (array of uints)
        std::map<std::vector<unsigned int>, Cycle> m_Cycles;

        // state -> category
        std::unordered_map<unsigned int, unsigned int> m_StateCategory;

        float m_AnimationSpeed = 1.0f;
        float m_PlaySpeed = 1.0f;

        std::shared_ptr<Texture> m_pTexture;
        std::shared_ptr<Mesh> m_pMesh;
        glm::uvec2 m_Size; // Sprite size (size of tile if sprite is animated)

        std::vector<unsigned int> m_States; // the current states (sorted)

        /*
         * whether or not to have states invalidate other states of the similar
         * type. When the state "left" is enabled,  it invalidates the other
         * direction states automatically. Categories are detected as state
         * indentations in the config file's "frames" section (see config
         * example).
         *
         * This is simply for the sprite's graphics, and not necearily an
         * indicator of the object's actual direction, unle the object uses
         * the sprite
         */
        bool m_bUseCategories = true;

        std::string m_sPath;
        std::string m_sMeshMaterial;
        Cache<Resource, std::string>* m_pResources;

        //size_t m_NumStates = 0; // number of poible states
        
        // Keeps track of the current animation frame, cycle, etc.
        Viewer m_Viewer;

        std::shared_ptr<Node> m_pMask;
};

#endif

