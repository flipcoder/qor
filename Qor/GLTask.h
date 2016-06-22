#ifndef GLTASK_H
#define GLTASK_H

#include "TaskHandler.h"
#include <memory>

//#define GL_TASK(NAME, CODE) {\
//    if(TaskHandler::get()->is_handler())\
//    {\
//        CODE\
//    }\
//    else\
//    {\
//        auto _task = [&]{\
//            CODE\
//        };\
//        TaskHandler::get()->add_task(_task);\
//    }\
//}

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

#endif

