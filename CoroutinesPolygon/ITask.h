#pragma once

namespace AO
{
    enum class TaskBlockingType
    {
        NonBlocked,
        Blocked
    };

    class ITask
    {
    public:
        virtual ~ITask() = default;
        virtual void Execute(ITask** nextTask) = 0;
        //virtual void Cancel() = 0;
        virtual TaskBlockingType GetBlockingType()
        {
            return TaskBlockingType::NonBlocked;
        }
    };

    using TaskPtr_t = std::unique_ptr<ITask>;
}
