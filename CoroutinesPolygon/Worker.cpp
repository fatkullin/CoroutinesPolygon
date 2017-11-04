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
        , m_threadStarted(false)
        , m_thread([this]() { this->Run(); })
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!m_threadStarted)
            m_cv.wait(lock);
    }

    Worker::~Worker()
    {
        m_thread.join();
    }

    void Worker::Run() noexcept
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_threadStarted = true;
            m_cv.notify_one();
        }

        while (m_initialTask)
        {
            Task* task = nullptr;
            Task* newTask = nullptr;

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
