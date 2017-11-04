#pragma once
#include <mutex>
#include "InitialWorkerTask.h"

namespace AO
{
    class TaskManager;

    class SleepTask : public InitialTask
    {
    public:
        std::condition_variable m_cv;
        std::mutex m_mutex;
        Task* NextTask;
        bool Stop = false;
        bool TaskSet = false;
        bool Stopped = false;

        SleepTask();

        virtual void SetTask(Task* task) override;

        virtual Task* WaitForTask() override;

        virtual void Cancel() override;
    };
}
