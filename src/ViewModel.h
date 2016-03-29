#ifndef _VIEWMODEL_H
#define _VIEWMODEL_H

#include "Node.h"
#include "Camera.h"
#include "Tracker.h"
#include "kit/freq/animation.h"

class ViewModel:
    public Tracker
{
    public:
        
        ViewModel(
            std::shared_ptr<Camera> camera,
            std::shared_ptr<Node> node
        );
        virtual ~ViewModel();
        
        std::shared_ptr<Node> node(){
            return m_pNode;
        }
        std::shared_ptr<const Node> node() const{
            return m_pNode;
        }
        
        void logic_self(Freq::Time t) override;

        void zoom(bool b, std::function<void()> cb = std::function<void()>());
        void fast_zoom(bool b);
        void sway(bool b) {
            m_bSway = b;
        }
        bool sway() const {
            return m_bSway;
        }
        void sprint(bool b);
        bool sprint() const {
            return m_bSprint;
        }

        bool zoomed() const { return m_bZoomed; }
        void reset();
        void reset_zoom();

        void model_pos(glm::vec3 p) {
            m_ModelPos = p;
        }
        void model_move(glm::vec3 v) {
            m_ModelPos += v;
        }
        void zoomed_model_pos(glm::vec3 p) {
            m_ZoomedModelPos = p;
        }
        glm::vec3 model_pos() {
            return m_ModelPos;
        }
        void zoomed_model_move(glm::vec3 v) {
            m_ZoomedModelPos += v;
        }
        glm::vec3 zoomed_model_pos() {
            return m_ZoomedModelPos;
        }

        void recoil(Freq::Time out, Freq::Time in, float dist);
        bool recoil() const {
            return not m_RecoilAnim.elapsed();
        }
        bool idle() const;
        void equip(bool r = true, std::function<void()> cb = std::function<void()>());
        bool equipped() const { return m_bEquip; }
        bool equipping() const { return m_bEquipping; }

        void fast_equip(bool r = true);

        void equip_time(Freq::Time t) { m_EquipTime = t; }
        Freq::Time equip_time() const { return m_EquipTime; }
        Freq::Time zoom_time() const { return m_ZoomTime; }
        void zoom_time(Freq::Time t) { m_ZoomTime = t; }

        void fov(float f);
        
        virtual std::string type() const override { return "viewmodel"; }
        
    private:
        
        bool m_bZoomed = false;
        bool m_bSway = false;
        bool m_bSprint = false;
        bool m_bEquip = true;
        bool m_bEquipping = false;
        float m_DefaultFOV;
        float m_ZoomedFOV;
        float m_SwayTime = 0.0f;
        Freq::Time m_EquipTime = Freq::Time::ms(100);
        glm::vec3 m_SwayOffset;
        Freq::Time m_ZoomTime = Freq::Time::ms(100);

        glm::vec3 m_ModelPos = glm::vec3(0.05f, -0.06f, -0.06f);
        glm::vec3 m_ZoomedModelPos = glm::vec3(0.0f, -0.02f, -0.10f);
        
        std::shared_ptr<Node> m_pNode;
        Camera* m_pCamera;
        Animation<float> m_EquipAnim;
        Animation<float> m_SprintRotateAnim;
        Animation<float> m_SprintLowerAnim;
        Animation<glm::vec3> m_ZoomAnim;
        Animation<float> m_ZoomFOVAnim;
        Animation<float> m_RecoilAnim;

        std::function<void()> m_onEquip;
};

#endif

