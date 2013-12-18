#include "IPhysics.h"
#include "kit/log/errors.h"

IPhysics :: IPhysics()
{
    //PF->LoadPALfromDLL();
    //PF->SelectEngine(driver.c_str());
    //m_pPhysics = PF->CreatePhysics();
    //if(!m_pPhysics)
    //    throw Error(ErrorCode::LIBRARY, driver);

    //palPhysicsDesc desc;
    //desc.m_vGravity = palVector3(0.0f, -9.8f, 0.0f);
    //desc.m_nUpAxis = PAL_Y_AXIS;
    //m_pPhysics->Init(desc);
}

IPhysics :: ~IPhysics()
{
}

