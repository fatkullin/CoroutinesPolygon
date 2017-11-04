#pragma once

namespace AO
{
    class Task;

    class TaskExecutor
    {
    public:
        static Task* Execute(Task*& task);
    };

}
