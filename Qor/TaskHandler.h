#ifndef TASK_HANDLER_H
#define TASK_HANDLER_H

#include <atomic>

class TaskHandler
{
    public:
        virtual ~TaskHandler() {}
        virtual void clear_tasks() = 0;
        virtual void add_task(std::function<void()> func) = 0;
        virtual void wait_task(std::function<void()> func) = 0;
        virtual void do_tasks() = 0;
        virtual bool has_tasks() const = 0;
        virtual bool is_handler() const = 0;

        static TaskHandler* get(TaskHandler* t = nullptr) {
            static std::atomic<TaskHandler*> th = ATOMIC_VAR_INIT(nullptr);
            if(th == t)
                th = nullptr;
            else if(t)
                th = t;
            return th;
        }
};

#endif

