#include "PipelineShader.h"
#include "Filesystem.h"
using namespace std;

PipelineShader :: PipelineShader(const string& fn):
    Resource(fn),
    m_pShader(make_shared<Program>(
        make_shared<Shader>(
            Filesystem::cutExtension(fn) + ".vp",
            Shader::VERTEX
        ),
        make_shared<Shader>(
            Filesystem::cutExtension(fn) + ".fp",
            Shader::FRAGMENT
        )
    ))
{
    
}
void PipelineShader :: link()
{
    if(!m_pShader->link())
        ERROR(ACTION, "link shader program");
}

bool PipelineShader :: linked() const
{
    return m_pShader->linked();
}

