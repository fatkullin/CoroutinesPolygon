#include "stdafx.h"
#include "Worker.h"

#include "TaskManager.h"
#include "TaskExecutor.h"
#include "InitialWorkerTask.h"

namespace AO
{
    Worker::Worker(WorkerType type, int tag, TaskManager* taskManager, InitialTask* initialTask)
        : m_type(type)
        , m_initialTask(initialTask)
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
        while (m_initialTask)
        {
            ITask* task = nullptr;
            ITask* newTask = nullptr;

            task = m_initialTask->WaitForTask();

            if (!task)
                continue;

            task = m_taskManager->GetNextTask(task, nullptr, *this);

            while (task)
            {
                newTask = TaskExecutor::Execute(task);
                task = m_taskManager->GetNextTask(task, newTask, *this);
            }
        }
    }

    void Worker::SetType(WorkerType workerType)
    {
        m_type = workerType;
    }
}
