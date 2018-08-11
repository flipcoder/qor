#ifndef GLTASK_H
#define GLTASK_H

#include "TaskHandler.h"
#include <memory>

#define GL_TASK_ASYNC_START() {\
    auto task = [=]{\
        
#define GL_TASK_ASYNC_END() \
    };\
    if(TaskHandler::get()->is_handler()){\
        task();\
    }else{\
        TaskHandler::get()->add_task(task);\
    }\
}

#define GL_TASK_START() {\
    auto task = [&]{

#define GL_TASK_END() \
    };\
    if(TaskHandler::get()->is_handler()){\
        task();\
    }else{\
        TaskHandler::get()->wait_task(task);\
    }\
}

#define GL_REGION_START() \
    [=]{\
    
#define GL_REGION_END() \
    };\

#endif

