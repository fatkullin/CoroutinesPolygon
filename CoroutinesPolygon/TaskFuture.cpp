#include "stdafx.h"
#include "TaskFuture.h"
#include "TaskManager.h"

namespace AO
{
    void SharedState::Set()
    {
        State = FutureState::Ready;
        m_cv.notify_one();
    }

    void SharedState::Get()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (State != FutureState::Ready)
            m_cv.wait(lock);
    }

    Future::Future(SharedStatePtr_t sharedState): m_sharedState(std::move(sharedState))
    {
    }

    Future::~Future()
    {
        if (m_task)
            m_sharedState->Get();
    }

    bool Future::SetContinuation(Task* task) const
    {
        // TODO: contract requires
        // TODO: to relax memory order
        m_sharedState->Continuation.Push(task); //strong ordering
        task = nullptr;
        if (m_sharedState->State == FutureState::Ready)
        {
            task = m_sharedState->Continuation.TryPop();
            if (task != nullptr)
                return false;
        }
        return true;
    }

    void Future::SetTask(std::unique_ptr<Task> task) noexcept
    {
        m_task = std::move(task);
    }

    void Future::SetManager(std::shared_ptr<TaskManager> value) noexcept
    {
        m_manager = std::move(value);
    }

    Promise::Promise(): m_sharedState(std::make_shared<SharedState>())
    {
    }

    bool Promise::GetContinuation(Task** task) const
    {
        auto continuation = m_sharedState->Continuation.TryPop();

        if (continuation == nullptr)
            return false;

        *task = continuation;
        return true;
    }

    std::unique_ptr<Future> Promise::GetFuture() const
    {
        return std::make_unique<Future>(m_sharedState);
    }

    void Promise::SetReady() const
    {
        m_sharedState->Set();
    }
}
