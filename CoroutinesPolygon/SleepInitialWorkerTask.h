#pragma once
#include <mutex>
#include "InitialWorkerTask.h"

namespace AO
{
    class TaskManager;

    class SleepTask : public ITaskProducerInternal
    {
    public:
        std::condition_variable m_cv;
        std::mutex m_mutex;
        ITask* NextTask;
        bool Stop = false;
        bool TaskSet = false;
        bool Stopped = false;

        SleepTask();

        virtual void SetTask(ITask* task) override;

        virtual ITask* WaitForTask() override;

        virtual void Cancel() override;
    };
}
