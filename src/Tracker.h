#ifndef _TRACKER_H_5G2Y7UTS
#define _TRACKER_H_5G2Y7UTS

#include "Node.h"
#include "kit/freq/animation.h"

/*
 *  A specific type of Node that changes behavior based on another node.
 *
 *  For things like turrets and cameras that need to either latch onto or face
 *  a target as it moves.
 *
 *  TODO: Should figure out a way to make this optimized, maybe a node cache
 *  callback for trackers that need to know world matrix
 */
class Tracker:
    public Node
{
    public:
        
        enum Mode {
            STICK, // stick to target, copying orientation and position
            FOLLOW, // move with target, but don't copy orientation
            ORIENT, // face the same direction as target
            WATCH // watch (orient towards) target w/o moving
        };
        
        Tracker() = default;
        Tracker(
            const std::shared_ptr<Node>& target,
            Mode mode = STICK,
            Freq::Time focus_time = Freq::Time(50)
            //std::function<float(const float&, const float&))> interp = 
            //    INTERPOLATE(linear<float>)
        ):
            m_pTarget(target),
            m_Mode(mode),
            m_FocusTime(focus_time)
            //m_Interp(interp)
        {
            update_tracking();
        }
        virtual ~Tracker() {}

        void track(
            const std::shared_ptr<Node>& target = std::shared_ptr<Node>()
        ){
            m_pTarget = target;
            update_tracking();
        }

        void finish();

        virtual void logic_self(Freq::Time t) override;

        void focal_offset(const glm::vec3& v) {
            m_FocalOffset = v;
        }
        glm::vec3 focal_offset() const {
            return m_FocalOffset;
        }
        
        void focus_time(Freq::Time t) {
            m_FocusTime = t;
        }
        Freq::Time focus_time() const {
            return m_FocusTime;
        }
        
        Mode mode() const {
            return m_Mode;
        }
        void mode(Mode m) {
            m_Mode = m;
        }
        
        std::shared_ptr<Node> target() { return m_pTarget.lock(); }
        
        virtual std::string type() const override { return "tracker"; }
        
    private:

        void sync_tracking();
        void update_tracking();

        //Freq::Time m_tDelay;
        std::weak_ptr<Node> m_pTarget;
        Animation<glm::mat4> m_Animation;
        glm::vec3 m_FocalOffset;
        Freq::Time m_FocusTime = Freq::Time(0);
        Mode m_Mode = STICK;

        /*
         * The offset to reapply after the focus has set matrix
         * Can be translation or orientation change
         */
        glm::mat4 m_Offset;

        //std::function<float(const float&, const float&)> m_Interp;
};

#endif

