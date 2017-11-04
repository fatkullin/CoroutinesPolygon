#pragma once
#include "InitialWorkerTask.h"

namespace AO
{
    class AsyncWaitTask
        : public InitialTask
    {
    public:
        explicit AsyncWaitTask(HANDLE completionPort);

        virtual Task* WaitForTask() override;

        void Cancel() override
        {
            // we should wait all async operation so this task
            // cannot be cancelled
        }

        virtual void SetTask(Task* task) override;
    private:
        HANDLE m_completionPort;
    };

}
