#pragma once

namespace AO
{

    class Task;

    struct TaskExecutionResult
    {
        enum StepResult
        {
            ChildTaskCreated,
            CompletedWithChildTask,
            WaitForOtherTask,
            Completed,
            CompletedCoroutine,
            Yielded,
            AsyncOperationRun
        };

        const StepResult StepResultValue;
        Task* const Continuation;

    private:
        TaskExecutionResult(StepResult stepResult, Task* continuation)
            : StepResultValue(stepResult)
            , Continuation(continuation)
        {
            
        }
        template <class T, class V>
        friend struct OperationBase;

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
