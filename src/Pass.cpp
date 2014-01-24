#include "Node.h"
#include "Pass.h"
#include "Pipeline.h"
#include "IPartitioner.h"

Pass :: Pass(IPartitioner* partitioner, Pipeline* pipeline, unsigned flags):
    m_pPartitioner(partitioner),
    m_pPipeline(pipeline),
    m_Flags(flags)
{}

void Pass :: matrix(const glm::mat4* m) {
    assert(m_pPipeline);
    m_pPipeline->matrix(this, m);
}

void Pass :: texture(unsigned id, unsigned slot) {
    m_pPipeline->texture(id, slot);
}

void Pass :: vertex_array(unsigned int id) {
    //if(id!=m_VertexArrayID){
        GL_TASK_START()
            glBindVertexArray(id);
        GL_TASK_END()
    //}
}

void Pass :: vertex_buffer(unsigned int id) {
    //if(id!=m_VertexBufferID) {
        GL_TASK_START()
            glBindBuffer(GL_ARRAY_BUFFER, id);
        GL_TASK_END()
    //}
}

void Pass :: element_buffer(unsigned int id) {
    //if(id!=m_VertexBufferID) {
        GL_TASK_START()
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
        GL_TASK_END()
    //}
}

//void enable_layout(Pipeline::Attribute attr) {
//    m_pPipeline->enable_layout(attr);
//}

//void disable_layout(Pipeline::Attribute attr) {
//    m_pPipeline->disable_layout(attr);
//}

void Pass :: shader(const std::shared_ptr<Program>& p) {
    m_pPipeline->shader(p);
}
void Pass :: shader(std::nullptr_t n) {
    m_pPipeline->shader(n);
}
std::shared_ptr<Program> Pass :: shader(unsigned slot) const {
    return m_pPipeline->shader(slot);
}

PassType Pass :: type() const {
    return m_pPipeline->slot();
}

unsigned Pass :: layout(unsigned attrs) {
    return m_pPipeline->layout(attrs);
}
void Pass :: texture_slots(unsigned slot_flags){
    m_pPipeline->texture_slots(slot_flags);
}
unsigned Pass :: attribute_id(unsigned id){
    return m_pPipeline->attribute_id((Pipeline::AttributeID)id);
}

