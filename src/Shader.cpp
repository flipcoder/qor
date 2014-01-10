#include <fstream>

#include "Shader.h"
#include "kit/kit.h"
#include "Filesystem.h"
#include "kit/log/log.h"
#include "kit/log/errors.h"

using namespace std;

Shader :: Shader(string fn, eType type, unsigned int flags)
{
    kit::scoped_dtor<Shader> dtor(this);
    if(!load(fn,type,flags))
        ERROR(READ, Filesystem::getFileName(fn));
    dtor.resolve();
}

Shader :: ~Shader()
{
    unload();
}

bool Shader :: load(string fn, eType type, unsigned int flags)
{
    int status;

    unload();

    if(!m_ID)
    {
        m_Type = type;
        GLenum gltype;
        switch(m_Type)
        {
            case FRAGMENT:
                gltype = GL_FRAGMENT_SHADER;
                break;
            case VERTEX:
                gltype = GL_VERTEX_SHADER;
                break;
            case GEOMETRY:
                gltype = GL_GEOMETRY_SHADER;
                break;
            default:
                return 0;
        }
        m_ID = glCreateShader(gltype);
        if(!m_ID)
            ERROR(READ, Filesystem::getFileName(fn));
    }

    ifstream file;
    vector<string> file_lines;
    string line;
    file.open(fn);
    while(getline(file, line))
        file_lines.push_back(line+"\n");
    file.close();

    if(file_lines.empty())
        ERROR(READ, Filesystem::getFileName(fn));

    //const char* c = &file_lines[0];
    vector<const char*> cstring_ptrs;
    cstring_ptrs.reserve(file_lines.size());
    vector<GLint> cstring_lens;
    cstring_lens.reserve(file_lines.size());

    transform(file_lines.begin(), file_lines.end(), back_inserter(cstring_ptrs), mem_fn(&string::c_str));
    transform(file_lines.begin(), file_lines.end(), back_inserter(cstring_lens), mem_fn(&string::size));
    glShaderSource(m_ID, file_lines.size(), &cstring_ptrs[0], &cstring_lens[0]);

    glCompileShader(m_ID);
    glGetShaderiv(m_ID, GL_COMPILE_STATUS, &status);

    if(!status || !m_ID)
    {
        checkError(fn); // get a full log
        ERROR(READ, Filesystem::getFileName(fn)); // or not
    }
    return m_ID!=0;
}

string Shader :: checkError(std::string fn = std::string())
{
    string log;
    int actual_len = 0;
    int len = 0;
    glGetShaderiv(m_ID, GL_INFO_LOG_LENGTH, &len);

    log.resize(len);

    glGetShaderInfoLog(m_ID, len, &actual_len, &log[0]);
    if(actual_len > 0)
        ERROR(PARSE, Filesystem::getFileName(fn)+"\n"+log);
    return string();
}

//bool Shader :: compile()
//{
//    return true;
//}

void Shader:: unload()
{
    if(m_ID)
    {
        glDeleteShader(m_ID);
        m_ID = 0;
    }
}

Program :: Program(shared_ptr<Shader> vp, shared_ptr<Shader> fp)
{
    m_ID = glCreateProgram();
    kit::scoped_dtor<Program> dtor(this);

    if(!m_ID || glGetError() != GL_NO_ERROR)
        ERROR(ACTION, "create shader program");
    if(!attach(vp))
        ERROR(ACTION, "attach vertex shader");
    if(!attach(fp))
        ERROR(ACTION, "attach fragment shader");
    if(!link())
        ERROR(ACTION, "link shader program");
    use();

    dtor.resolve();
}

Program :: ~Program()
{
    unload();
}

void Program :: unload()
{
    if(m_ID)
    {
        glDeleteProgram(m_ID);
        m_ID = 0;
    }
}

bool Program :: attach(shared_ptr<Shader>& shader)
{
    assert(shader);
    if(!shader || !shader->good())
        return false;
    glAttachShader(m_ID, shader->id());
    m_Shaders.push_back(shader);
    return true;
}

bool Program :: link()
{
    if(!m_ID)
        return false;
    int r;
    glLinkProgram(m_ID);
    glGetProgramiv(m_ID, GL_LINK_STATUS, &r);
    return r!=0;
}

bool Program :: use()
{
    if(!m_ID)
        return false;
    glUseProgram(m_ID);
    return true;
}

Program::UniformID Program :: uniform(string n) const
{
    UniformID id = glGetUniformLocation(m_ID, n.c_str());
    //if(id < 0)
    //    ERROR(READ, string("shader uniform ") + n);
    return id;
}

void Program :: uniform(UniformID uid, float v) const {
    if(!isValidUniformID(uid)) return;
    glUniform1f((GLint)uid, v);
}
void Program :: uniform(UniformID uid, float v, float v2) const {
    if(!isValidUniformID(uid)) return;
    glUniform2f((GLint)uid, v, v2);
}
void Program :: uniform(UniformID uid, float v, float v2, float v3) const {
    if(!isValidUniformID(uid)) return;
    glUniform3f((GLint)uid, v, v2, v3);
}
void Program :: uniform(UniformID uid, float v, float v2, float v3, float v4)  const {
    if(!isValidUniformID(uid)) return;
    glUniform4f((GLint)uid, v, v2, v3, v4);
}
void Program :: uniform(UniformID uid, int v) const {
    if(!isValidUniformID(uid)) return;
    glUniform1i((GLint)uid, v);
}
void Program :: uniform(UniformID uid, int v, int v2) const {
    if(!isValidUniformID(uid)) return;
    glUniform2i((GLint)uid, v, v2);
}
void Program :: uniform(UniformID uid, int v, int v2, int v3) const {
    if(!isValidUniformID(uid)) return;
    glUniform3i((GLint)uid, v, v2, v3);
}
void Program :: uniform(UniformID uid, int v, int v2, int v3, int v4) const {
    if(!isValidUniformID(uid)) return;
    glUniform4i((GLint)uid, v, v2, v3, v4);
}
void Program :: uniform(UniformID uid, const glm::mat4& matrix) const {
    if(!isValidUniformID(uid)) return;
    glUniformMatrix4fv((GLint)(uid), 1, false, glm::value_ptr(matrix));
}
void Program :: uniform(UniformID uid, const glm::vec2& vec) const {
    if(!isValidUniformID(uid)) return;
    glUniform2fv((GLint)(uid), 1, glm::value_ptr(vec));
}
void Program :: uniform(UniformID uid, const glm::vec3& vec) const {
    if(!isValidUniformID(uid)) return;
    glUniform3fv((GLint)(uid), 1, glm::value_ptr(vec));
}
void Program :: uniform(UniformID uid, const glm::vec4& vec) const {
    if(!isValidUniformID(uid)) return;
    glUniform4fv((GLint)(uid), 1, glm::value_ptr(vec));
}
//void Program :: uniform(UniformID uid, unsigned int size, unsigned int count, const int* v) {
//    if(!isValidUniformID(uid)) return;
//    // todo: add
//}
//void Program :: uniform(UniformID uid, unsigned int size, unsigned int count, const float* v){
//    if(!isValidUniformID(uid)) return;
//    // todo: add
//}

