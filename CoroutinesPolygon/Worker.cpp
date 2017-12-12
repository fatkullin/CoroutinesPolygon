#include "stdafx.h"
#include "Worker.h"

#include "TaskManager.h"
#include "TaskExecutor.h"
#include "InitialWorkerTask.h"

namespace AO
{
    Worker::Worker(WorkerType type, int tag, TaskManager* taskManager, ITaskProducer* initialTask)
        : m_type(type)
        , TaskProducer(initialTask)
        , m_taskManager(taskManager)
        , m_tag(tag)
        , m_thread([this]() { this->Run(); })
    {
    }

    Worker::~Worker()
    {
        m_thread.join();
    }

    void Worker::Run() noexcept
    {
        while (TaskProducer)
        {
            auto task = TaskProducer->WaitForTask();
            
            // TaskManager can change TaskProducer, so old producer returns null
            // then worker waits on new producer
            if (!task)
                continue;

            task = m_taskManager->GetNextTask(task, *this);

            while (task)
            {
                auto const newTask = TaskExecutor::Execute(task);
                task = m_taskManager->GetNextTask(newTask, *this);
            }
        }
    }

    void Worker::SetType(WorkerType workerType)
    {
        m_type = workerType;
    }
}
