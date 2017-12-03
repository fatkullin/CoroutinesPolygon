#pragma once

namespace AO
{
    class ITask;

    struct InitialTask
    {
        virtual ~InitialTask() = default;
        virtual ITask* WaitForTask() = 0;
        virtual void SetTask(ITask*) = 0;
        virtual void Cancel() = 0;
    };
}
