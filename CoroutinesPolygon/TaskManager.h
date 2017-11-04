#pragma once

#include "Worker.h"
#include "LockFreePtrQueue.h"
#include "TaskFuture.h"
#include "Task.h"

#include <memory>
#include <mutex>

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
        static const unsigned AsyncWaiterCount = 1;

	public:
        HANDLE CompletionPort;

    public:
        static std::shared_ptr<TaskManager> Create(unsigned threadNumber)
        {
            return std::shared_ptr<TaskManager>(new TaskManager(threadNumber));
        }

	public:
	    ~TaskManager();

	    std::unique_ptr<Future> AddNewTask(TaskPtr_t task);

        template<class T>
        auto AddNewOperation(std::unique_ptr<T> operation) -> ResultFuture<typename T::Result_t>
        {
            auto result = operation->GetFuture();
            return ResultFuture<typename T::Result_t>(AddNewTask(std::move(operation)), std::move(result));
        }

        Task* GetNextTask(Task* task, Task* newTask, Worker& worker);

	private:
        explicit TaskManager(unsigned threadNumber);

	    Worker* GetWorkerOrAddTask(Task* task);

	    Task* GetTaskOrPushWorker(Worker* worker);

	    void AddExistingTaskToQueue(Task* task);

    private:
        LockFreePtrQueue<Task> m_taskQueue;
        LockFreePtrQueue<Task> m_syncTaskQueue;

        LockFreePtrQueue<Worker> m_freeWorkers;
        LockFreePtrQueue<Worker> m_syncFreeWorkers;

        std::vector<std::unique_ptr<Worker>> m_workers;
        std::vector<std::unique_ptr<InitialTask>> m_asyncTasks;
        std::vector<std::unique_ptr<InitialTask>> m_sleepTasks;

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


