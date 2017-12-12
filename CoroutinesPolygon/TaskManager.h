#pragma once

#include "Worker.h"
#include "LockFreePtrQueue.h"
#include "ITask.h"
#include "ResultFuture.h"

#include <memory>
#include <mutex>
#include "InitialWorkerTask.h"

namespace AO
{
    class Worker;
    class TaskManager;

	enum class WorkerType : int
	{
		TaskThreadPool = 0,
		AsyncThreadPool = 1,
        SynchroThreadPool = 2
	};
	class TaskManager
        : public std::enable_shared_from_this<TaskManager>
	{
	public:
        HANDLE CompletionPort;

    public:
        static std::shared_ptr<TaskManager> Create(
            unsigned asyncOperationThreadNumber,
            unsigned blockedOperationThreadNumber,
            unsigned ioCompletionThreadNumber = 1)
        {
            return std::shared_ptr<TaskManager>(new TaskManager(asyncOperationThreadNumber,
                blockedOperationThreadNumber, ioCompletionThreadNumber));
        }

	public:
	    ~TaskManager();

        template<class T>
        auto AddNewOperation(std::unique_ptr<T> operation) -> std::unique_ptr<ResultFuture<typename T::Result_t>>
        {
            auto future = operation->PrepareToRun(CompletionPort);
            ITask* taskPtr = operation.get();
            auto result = std::make_unique<ResultFuture<typename T::Result_t>>(std::move(future), std::move(operation) /*, shared_from_this()*/);
            AddExistingTaskToQueue(taskPtr);
            return result;
        }

        ITask* GetNextTask(ITask* newTask, Worker& worker);

	private:
        explicit TaskManager(
            unsigned asyncOperationThreadNumber,
            unsigned blockedOperationThreadNumber,
            unsigned ioCompletionThreadNumber);

	    Worker* GetWorkerOrAddTask(ITask* task);

	    ITask* GetTaskOrPushWorker(Worker* worker);

	    void AddExistingTaskToQueue(ITask* task) noexcept;

    private:
        LockFreePtrQueue<ITask> m_taskQueue;
        LockFreePtrQueue<ITask> m_syncTaskQueue;

        LockFreePtrQueue<Worker> m_freeWorkers;
        LockFreePtrQueue<Worker> m_syncFreeWorkers;

        std::vector<std::unique_ptr<Worker>> m_workers;
        std::vector<std::unique_ptr<ITaskProducerInternal>> m_asyncTasks;
        std::vector<std::unique_ptr<ITaskProducerInternal>> m_sleepTasks;

        std::mutex m_mutex;

	};

}

//class Logger
//{
//public:
//    static const unsigned int Size = 1024;

//    Logger()
//    {
//        m_queue.resize(Size);
//        m_number = 0;
//    }

//    void Push(size_t thread, size_t msg)
//    {
//        auto position = m_number.fetch_add(1);

//        position = position % Size;
//        m_queue[position].first = thread;
//        m_queue[position].second = msg;
//    }

//    std::vector<std::pair<size_t, size_t>> m_queue;
//    std::atomic_uint m_number;
//};


