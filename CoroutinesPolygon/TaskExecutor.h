#pragma once

namespace AO
{
    class ITask;

    class TaskExecutor
    {
    public:
        static ITask* Execute(ITask* task);
    };

}
