#pragma once

namespace AO
{
    class ITask;

    struct ITaskProducer
    {
        virtual ~ITaskProducer() = default;
        virtual ITask* WaitForTask() = 0;
    };

    struct ITaskProducerInternal : ITaskProducer
    {
        virtual void SetTask(ITask*) = 0;
        virtual void Cancel() = 0;
    };
}
