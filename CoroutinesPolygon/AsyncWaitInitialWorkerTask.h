#pragma once
#include "InitialWorkerTask.h"

namespace AO
{
    class AsyncWaitTask
        : public ITaskProducerInternal
    {
    public:
        explicit AsyncWaitTask(HANDLE completionPort);

        virtual ITask* WaitForTask() override;

        void Cancel() override
        {
            throw std::runtime_error("Cannot cancel async task producer");
        }

        virtual void SetTask(ITask* task) override
        {
            throw std::runtime_error("Cannot set task for async task producer");
        }
    private:
        HANDLE m_completionPort;
    };

}
