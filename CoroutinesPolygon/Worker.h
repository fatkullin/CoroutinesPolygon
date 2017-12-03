#pragma once
#include <atomic>
#include <mutex>

namespace AO
{
    enum class WorkerType;
    class TaskManager;
    struct InitialTask;

    class Worker
    {
    public:
        Worker(WorkerType type, int tag, TaskManager* taskManager, InitialTask* initialTask);
        ~Worker();

        void Run() noexcept;
        void SetType(WorkerType workerType);

    public:
        std::atomic<WorkerType> m_type;
        InitialTask* m_initialTask;

    private:
        TaskManager* m_taskManager;
        int m_tag;
        std::thread m_thread;
    };
}
