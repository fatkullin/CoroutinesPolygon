#pragma once
#include "LockFreePtrQueue.h"

#include <memory>
#include <mutex>
#include <future>

namespace AO
{
    enum class FutureState : int
    {
        Initial,
        Ready
    };

///////////////////////////////////////////////////////////////////////////////////////////////

    class Task;

    struct SharedState
    {
        std::atomic<Task*> Continuation = nullptr;
        std::atomic<FutureState> State = FutureState::Initial;

        void Set();
        void Get();

    private:
        std::mutex m_mutex;
        std::condition_variable m_cv;
    };

    using SharedStatePtr_t = std::shared_ptr<SharedState>;

///////////////////////////////////////////////////////////////////////////////////////////////

    class TaskManager;
    class Future
    {
        SharedStatePtr_t m_sharedState;
    public:
        explicit Future(SharedStatePtr_t sharedState);

        ~Future();
        
        // Return 'false' if futures already completed
        // so continuation cannot be called.
        // In this case 'task' parameter will be unchanged.
        bool SetContinuation(Task* task) const;

        void SetTask(std::unique_ptr<Task> task) noexcept;

        void Cancel()
        {
            // TODO: call Cancel for task
        }

        void SetManager(std::shared_ptr<TaskManager> value) noexcept;
    private:
        std::unique_ptr<Task> m_task;
        std::shared_ptr<TaskManager> m_manager;
    };

///////////////////////////////////////////////////////////////////////////////////////////////

    class Promise
    {
    public:
        Promise();

        // Return 'true' if continuation are extracted. 
        // (In this case 'SetContinuation' for corresponding future also returns 'true')
        bool GetContinuation(Task** task) const;

        //can be called only once
        std::unique_ptr<Future> GetFuture() const;

        void SetReady() const;
    private:
        SharedStatePtr_t m_sharedState;
    };

}
