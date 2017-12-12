#include "stdafx.h"
#include "TaskManager.h"
#include "InitialWorkerTask.h"
#include "AsyncWaitInitialWorkerTask.h"
#include "SleepInitialWorkerTask.h"

namespace AO
{
    static bool AppropriateTaskType(ITask* task, const Worker& worker)
    {
        WorkerType const workerType = worker.m_type;
        auto const taskType = task->GetBlockingType();

        if (workerType == WorkerType::TaskThreadPool)
            return taskType == TaskBlockingType::NonBlocked;

        return taskType == TaskBlockingType::Blocked;
    }

    TaskManager::TaskManager(
        unsigned asyncOperationThreadNumber,
        unsigned blockedOperationThreadNumber,
        unsigned ioCompletionThreadNumber)
    {
        // TODO: ErrorHandling
        CompletionPort = CreateIoCompletionPort(
            INVALID_HANDLE_VALUE, nullptr, 0, ioCompletionThreadNumber);

        int tag = 0;
        for (unsigned i = 0; i < ioCompletionThreadNumber; ++i)
        {
            auto task = std::make_unique<AsyncWaitTask>(CompletionPort);
            auto taskWorker = std::make_unique<Worker>(WorkerType::AsyncThreadPool, tag++, this, task.get());
            m_workers.emplace_back(std::move(taskWorker));
            m_asyncTasks.emplace_back(std::move(task));
        }

        for (unsigned i = 0; i < asyncOperationThreadNumber; ++i)
        {
            auto task = std::make_unique<SleepTask>();
            auto taskWorker = std::make_unique<Worker>(WorkerType::TaskThreadPool, tag++, this, task.get());
            m_freeWorkers.Push(taskWorker.get());
            m_workers.emplace_back(std::move(taskWorker));
            m_sleepTasks.emplace_back(std::move(task));
        }

        for (unsigned i = 0; i < blockedOperationThreadNumber; ++i)
        {
            auto task = std::make_unique<SleepTask>();
            auto taskWorker = std::make_unique<Worker>(WorkerType::SynchroThreadPool, tag++, this, task.get());
            m_syncFreeWorkers.Push(taskWorker.get());
            m_workers.emplace_back(std::move(taskWorker));
            m_sleepTasks.emplace_back(std::move(task));
        }
    }
    
    TaskManager::~TaskManager()
    {
        for (auto& worker : m_workers)
            worker->TaskProducer = nullptr;

        if (CompletionPort)
            auto result = CloseHandle(CompletionPort);
        // TODO: handle error

        for (auto& sleepTask : m_sleepTasks)
            sleepTask->Cancel();
    }

    // 'producedTask' is task received from worker TaskProducer 
    // or from just executed task (for ex. as continuation).
    // TaskManager should find strategy which task should be run on worker next time.
    ITask* TaskManager::GetNextTask(ITask* producedTask, Worker& worker)
    {
        // TODO: replace logic to separate class

        if (worker.m_type == WorkerType::AsyncThreadPool)
        {
            //inter worker optimization
            //auto freeworker = m_taskManager->GetWorkerOrAddTask(task);
            //if (freeworker)
            //{
            //    // swap ThreadPools of workers
            //    auto sleepTask = freeworker->m_initialTask;
            //    const WorkerType sleepType = freeworker->m_type;

            //    freeworker->m_type = WorkerType::AsyncThreadPool;
            //    freeworker->m_initialTask = worker.m_initialTask;
            //    //wake freeworker
            //    sleepTask->SetTask(nullptr);

            //    worker.m_type = sleepType;
            //    worker.m_initialTask = sleepTask;

            //    return task;
            //}

            AddExistingTaskToQueue(producedTask);
            return nullptr;
        }

        if (producedTask != nullptr)
        {
            if (AppropriateTaskType(producedTask, worker))
                return producedTask;

            AddExistingTaskToQueue(producedTask);
        }

        auto const task = GetTaskOrPushWorker(&worker);

        // if task == nullptr - worker returns to GetInitialTask
        return task;
    }

    Worker* TaskManager::GetWorkerOrAddTask(ITask* task)
    {
        auto const type = task->GetBlockingType();
        auto& workerStack = type == TaskBlockingType::Blocked ? m_syncFreeWorkers : m_freeWorkers;
        auto& taskQueue = type == TaskBlockingType::Blocked ? m_syncTaskQueue : m_taskQueue;

        auto worker = workerStack.TryPop();
        if (worker)
            return worker;

        {
            std::unique_lock<std::mutex> lock(m_mutex);

            worker = workerStack.TryPop();
            if (worker)
                return worker;

            taskQueue.Push(task);
            return nullptr;
        }
    }

    ITask* TaskManager::GetTaskOrPushWorker(Worker* worker)
    {
        WorkerType type = worker->m_type;
        
        _ASSERTE(type != WorkerType::AsyncThreadPool);

        auto& workerStack = type == WorkerType::SynchroThreadPool ? m_syncFreeWorkers : m_freeWorkers;
        auto& taskQueue = type == WorkerType::SynchroThreadPool ? m_syncTaskQueue : m_taskQueue;

        auto task = taskQueue.TryPop();
        if (task)
            return task;

        {
            std::unique_lock<std::mutex> lock(m_mutex);

            task = taskQueue.TryPop();
            if (task)
                return task;

            workerStack.Push(worker);
            return nullptr;
        }
    }

    // assume function is noexcept
    // otherwise task will never end
    // TODO: to finish task if exception
    void TaskManager::AddExistingTaskToQueue(ITask* task) noexcept
    {
        auto const worker = GetWorkerOrAddTask(task);
        if (worker)
            static_cast<ITaskProducerInternal*>(worker->TaskProducer)->SetTask(task);
    }
}
