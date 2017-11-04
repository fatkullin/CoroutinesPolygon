#pragma once

namespace AO
{
    class Task;

    struct InitialTask
    {
        virtual ~InitialTask() = default;
        virtual Task* WaitForTask() = 0;
        virtual void SetTask(Task*) = 0;
        virtual void Cancel() = 0;
    };
}
