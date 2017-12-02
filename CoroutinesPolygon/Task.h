#pragma once

namespace AO
{
    enum TaskExecutionResult
    {
        ChildTaskCreated,
        CompletedWithChildTask,
        WaitForOtherTask,
        Completed,
		CompletedCoroutine,
        Yielded,
        AsyncOperationRun
    };

    enum class TaskBlockingType
    {
        NonBlocked,
        Blocked
    };

    class Task
    {
    public:
        Task() {}
        virtual ~Task() = default;
        virtual void Execute(Task** nextTask) = 0;
        //virtual void Cancel() = 0;
        virtual TaskBlockingType GetBlockingType()
        {
            return TaskBlockingType::NonBlocked;
        }
    };

    using TaskPtr_t = std::unique_ptr<Task>;

}
