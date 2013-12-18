#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <memory>
#include <functional>
#include <vector>
#include <deque>
#include <cmath>
#include "kit/freq/freq.h"

/*!
 * \brief   Animation and Frames
 * \details Does timed frame interpolation/easing and paths
 *
 * \author  Grady O'Connell <flipcoder@gmail.com>
 */

template<class T> class Animation;

namespace Interpolation {
    template<class T>
    T linear(const T& a, const T& b, float t) {
        return a + (b-a)*t;
    }
}

#define INTERPOLATE(FUNC)\
    std::bind(\
        &Interpolation::FUNC<float>,\
        placeholders::_1,\
        placeholders::_2,\
        placeholders::_3\
    )\

/*
 * Template class should be a floating point value, a (math) vector,
 * matrix, or quaternion.  But it can also be a color, or anything
 * interpolatable.
 *
 * You define the easing function in the form of:
 *     T func(const T&, const T&, float t)
 * Where t is a value that ranges from 0 to 1.
 *
 * Time is in seconds, but exists on a "timeline", so the actual real time
 * value can be whatever you want, depending on how fast you advance the
 * timeline.
 *
 * Negative time values are not yet supported.
 * I don't have derivative approximation yet, but I think it could be useful
 * for nonlinear easing functions when you need to also know the speed.
 */
template<class T>
class Frame
    //public IRealtime
{
    private:
        
        // TODO: execution callback?
        // TODO: expiry callback?
        // TODO: interpolative callback?
        T m_Value;

        Freq::Time m_Time; // length of time
        Freq::Alarm m_Alarm;

        std::function<T (const T&, const T&, float)> m_Easing;

        //Animation<T>* m_pAnimation;
        Freq::Timeline* m_pTimeline;

    public:
        
        Frame(
            T value,
            Freq::Time time = Freq::Time(0),
            std::function<T (const T&, const T&, float)> easing =
                std::function<T (const T&, const T&, float)>(),
            Freq::Timeline* timeline = nullptr
        ):
            m_Value(value),
            m_Time(time),
            m_Easing(easing),
            m_pTimeline(timeline),
            m_Alarm(timeline)
            //m_pAnimation(nav)
        {
            //assert(m_pTimeline);
        }
        //void nav(Animation<T>* nav) {
        //    assert(nav);
        //    m_pAnimation = nav;
        //}

        virtual ~Frame() {}
        void go() {
            m_Alarm.set(m_Time);
        }

        //virtual void logic(Freq::Time t) {
        //    //m_pTimeline->logic(t);
        //    // TODO: trigger callbacks here?
        //}
        
        virtual bool elapsed() {
            return m_Alarm.elapsed();
        }

        unsigned long remaining() {
            return m_Alarm.ms();
        }

        Freq::Alarm& alarm() { return m_Alarm; }

        T& value() {
            return m_Value;
        }

        void easing(std::function<T (const T&, const T&, float)> func) {
            m_Easing = func;
        }

        const std::function<T (const T&, const T&, float)>& easing() const {
            return m_Easing;
        }
        
        void timeline(Freq::Timeline* tl) {
            m_pTimeline = tl;
            m_Alarm.timer(m_pTimeline);
        }
        Freq::Timeline* timeline() { return m_pTimeline; }
};

template<class T>
class Animation
    //public IRealtime
{
    private:

        mutable std::deque<Frame<T>> m_Frames;
        mutable T m_Current = T();

        //float m_fSpeed = 1.0f;
        Freq::Timeline* m_pTimeline = nullptr;

        void reset(Freq::Time excess = Freq::Time(0)) const {
            if(m_Frames.empty())
                return;

            m_Frames.front().go();
            m_Frames.front().alarm() += excess;
        }

        void process() const {
            while(!m_Frames.empty() && m_Frames.front().elapsed())
            {
                auto& w = m_Frames.front();
                Freq::Time excess = w.alarm().excess();
                m_Current = m_Frames.front().value();
                m_Frames.pop_front();
                reset(excess);
            }
        }
        
    public:
    
        Animation() = default;
        //Animation(T&& current):
        //    m_Current(current)
        //{}
        Animation& operator=(T&& a){
            m_Current = a;
            return *this;
        }

        //Animation(
        //    Frame<T> initial,
        //    T current=T()
        //):
        //    m_Current(current)
        //    //m_fSpeed(1.0f)
        //{
        //    //initial.nav(this);
        //    initial.timeline(m_pTimeline);
        //    m_Frames.push_back(initial);
        //    reset();
        //}

        size_t size() const {
            return m_Frames.size();
        }
        
        void from(
            T current=T()
        ) {
            m_Current=current;
        }

        virtual ~Animation() {}

        operator T() const {
            return get();
        }

        virtual void logic(Freq::Time t) {
            m_pTimeline->logic(t);
        }

        void pause() {
            m_pTimeline->pause();
        }
        void resume() {
            m_pTimeline->resume();
        }

        /*
         * Append a frame to the end of the cycle
         */
        void frame(Frame<T> frame) {
            frame.timeline(m_pTimeline);
            m_Frames.push_back(frame);
            reset();
        }

        bool elapsed() const {
            process();
            return m_Frames.empty();
        }

        void abort(){
            m_Current = get();
            m_Frames.clear();
        }
        void stop(){
            process();
            if(!m_Frames.empty()) {
                m_Current = m_Frames.back().value();
                m_Frames.clear();
            }
        }
        void stop(const T& position) {
            m_Current = position;
            m_Frames.clear();
        }
        void stop(
            T position,
            Freq::Time t,
            std::function<T (const T&, const T&, float)> easing
        ){
            m_Current = t ? get() : position;
            m_Frames.clear();

            if(t)
            {
                m_Frames.push_back(Frame<T>(position, t, easing, m_pTimeline));
                reset();
            }
        }

        void ensure(
            T position,
            Freq::Time t,
            std::function<T (const T&, const T&, float)> easing,
            std::function<bool (const T&, const T&)> equality_cmp
        ){
            if(equality_cmp(m_Current, position))
            {
                // TODO: if t < current frame time left
                //       use t instead
                return;
            }

            process();
            if(m_Frames.empty()) {
                stop(position,t,easing);
                return;
            }
        }

        T get() const {
            process();
            if(m_Frames.empty())
                return m_Current;

            return ease(
                m_Frames.front().easing(),
                m_Current,
                m_Frames.front().value(),
                m_Frames.front().alarm().fraction()
            );
        }

        T ease(
            const std::function<T (const T&, const T&, float)>& easing,
            const T& a,
            const T& b,
            float t
        ) const {
            return easing(a,b,t);
        }

        bool empty() const {
            return m_Frames.empty();
        }

        //void speed(float s) {
        //    m_fSpeed = s;
        //}
        //float speed() const {
        //    return m_fSpeed;
        //}

        void timeline(Freq::Timeline* tl) {
            m_pTimeline = tl;
        }
        Freq::Timeline* timeline() { return m_pTimeline; }
};

#endif

